/*
//	@(#) jsonlz4_json.c - decompress firefox bookmarks
*/
# include	<stdio.h>
# include	<stdlib.h>
# include	<stdint.h>
# include	<stdarg.h>

# include	<string.h>
# include	<errno.h>
# include	<unistd.h>

# include	<lz4.h>

//
// MOZLZ4 header
//
const char MOZLZ4_MAGIC[] = "mozLz40";
enum	{
	MOZLZ4_MAGIC_SIZE	= sizeof(MOZLZ4_MAGIC),
};
typedef	struct	{
	char		magic [MOZLZ4_MAGIC_SIZE];
	uint32_t	orig_size; // Little endian
	char		lz4data[0];
}	MOZLZ4_HDR;

//
// Utils
//
enum	{
	ok	= 0,
	err	= -1,
};
enum	{
	false	= 0,
	true	= !false,
};
static	char*	progname_set (char* name) {
	static	char*	progname	= 0;
	if (name) {
		char*	t	= strrchr (name, '/');
		if (t) {
			progname	= strdup (t+1);
		}
		else	{
			progname	= strdup (name);
		}
	}
	return	progname;
}
char*	progname() {
	return	progname_set (0);
}

void	fatal (char*  fmt,...) {
	va_list	ap;
	va_start (ap, fmt);
	fprintf (stderr, "ERROR: %s: ", progname());
	vfprintf (stderr, fmt, ap);
	va_end (ap);
	exit (EXIT_FAILURE);
}
void	warn (char*  fmt,...) {
	va_list	ap;
	va_start (ap, fmt);
	fprintf (stderr, "WARNING: %s: ", progname());
	vfprintf (stderr, fmt, ap);
	va_end (ap);
}
//
//
void	Usage () {
	fprintf (stderr, "Usage: %s [-i input] [-o output]\n", progname());
	exit (EXIT_SUCCESS);
}
//
//
enum	{
	MALLOC_START	= 64 * 1024,
};

//
// Read a file into malloc()ed storage
//
static	int	file_load (FILE* input, char** memp, size_t* sizep) {
	int	result	= -ENOMEM;
	int	ch	= 0;
	size_t	i	= 0;
	size_t	finish	= err;
	char*	buffer	= 0;
	size_t	buffer_size	= 0;
	
	while (i!=finish) {
		ch = fgetc (input);
		if (ch == EOF) {
			result	= ok;
			*memp	= buffer;
			*sizep	= i;
			finish	= i;
		}
		else if (i < buffer_size) {
				buffer [i++]	= ch;
		}
		else	{
			size_t	newsize	= buffer_size ? 2 * buffer_size : MALLOC_START;
			char*	newbuf	= realloc (buffer, newsize);
			if (newbuf) {
				buffer_size	= newsize;
				buffer	= newbuf;
				buffer [i++]	= ch;
			}
			else {
				i	= finish;
			}
		}
	}
	return	result;
}

//
// MOZLZ4 header load/verify magic/get original size
//
static	int	header_load (FILE* input, MOZLZ4_HDR* hdr, size_t hdrsize) {
	int	result	= err;
	if (fread (hdr, 1, hdrsize, input) == hdrsize)
		result	= ok;
	return	result;
}
static	inline int header_verify (MOZLZ4_HDR data) {
	int	result	= false;
	if (memcmp (MOZLZ4_MAGIC, data.magic, MOZLZ4_MAGIC_SIZE)==0) {
		result	= true;
	}
	return	result;
}
//
// Original size is in Intel little endian byte order
// ie LSB first
static	uint32_t get_uint32_le_t (uint32_t le) {
	unsigned char x[sizeof(le)];
	memcpy (x, &le, sizeof(x));	
	// 8 is bits_per_byte
	return (((x[3]<<8 | x[2])<<8) | x[1])<<8 | x[0];
}
static	inline size_t header_orig_size (MOZLZ4_HDR data) {
	return	get_uint32_le_t (data.orig_size);
}

//
//
//
int	main (int argc, char* argv[]) {

	FILE*	input	= stdin;
	FILE*	output	= stdout;
	char*	inputfile	= 0;
	char*	outputfile	= 0;
	
	MOZLZ4_HDR	header;	
	char*	compressed_data	= 0;
	size_t	size_compressed	= 0;
	char*	decompressed_data	= 0;
	size_t	size_original		= 0;
	size_t	size_decompressed	= 0;

	int	opt	= EOF;
	progname_set (argv [0]);

	while ((opt = getopt (argc, argv, "hi:o:")) != EOF) {
		switch (opt) {
		case	'i':
			if (inputfile) 
				Usage();
			inputfile	= optarg;
		break;
		case	'o':
			if (outputfile) 
				Usage();
			outputfile	= optarg;
		break;
		case	'h':
		case	'?':
		default:
			Usage();
		break;
		}
	}
	if (inputfile) {
		input	= fopen (inputfile, "r");
		if (!input) {
			fatal ("Couldn't open input file '%s'\n", inputfile);
		}
	}
	if (outputfile) {
		output	= fopen (outputfile, "w");
		if (!output) {
			fatal ("Couldn't open output file '%s'\n", outputfile);
		}
	}
	if (header_load (input, &header, sizeof(header)) != ok) {
		fatal ("Couldn't load file header into memory\n");
	}
	if (file_load (input, &compressed_data, &size_compressed) != ok) {
		fatal ("Couldn't load input file into memory\n");
	}
	if (!header_verify (header)) {
		fatal ("Header verify - input file doesn't seem to be MOZLZ4 compressed\n");
	}

	size_original	= header_orig_size (header);

	decompressed_data	= malloc (size_original);
	if (!decompressed_data) {
		fatal ("Couldn't allocate memory to decompress file\n");
	}
	size_decompressed	= LZ4_decompress_safe(compressed_data, decompressed_data, size_compressed, size_original);
	if (size_decompressed < 0) {
		fatal ("LZ4 decompression failed\n");
	}	

	if (size_decompressed != size_original)
		warn ("Actual decompressed size differs from header value\n");
	if (size_decompressed != fwrite (decompressed_data, 1, size_decompressed, output)) {
		warn ("Fewer bytes were written than were requested\n");
	}
	exit (EXIT_SUCCESS);	
}
