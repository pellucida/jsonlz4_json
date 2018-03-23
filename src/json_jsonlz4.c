/*
//	@(#) jsonl_jsonlz.c - compress firefox bookmarks
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
# define	_MOZLZ4_MAGIC	"mozLz40"
const char MOZLZ4_MAGIC[] = _MOZLZ4_MAGIC;
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
	size_t	finish	= -1;
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
// Original size is in Intel little endian byte order
// ie LSB first
static	uint32_t put_uint32_le_t (uint32_t le) {
	uint32_t	result	= 0;
	unsigned char x[sizeof(result)];
	x[0]	= le & 0xff;
	x[1]	= (le >> 8) & 0xff;
	x[2]	= (le >> 8*2) & 0xff;
	x[3]	= (le >> 8*3) & 0xff;
	memcpy (&result, x, sizeof(x));	
	return	result;
}
static 	int	header_write (FILE* output, size_t size_original) {
	size_t	result	= err;
	MOZLZ4_HDR	header	= { .magic = _MOZLZ4_MAGIC, };
	header.orig_size	= put_uint32_le_t (size_original);
	if (fwrite (&header, 1, sizeof(header), output) == sizeof(header))
		result	= ok;
	return	result;
}
//
//
//
int	main (int argc, char* argv[]) {

	FILE*	input	= stdin;
	FILE*	output	= stdout;
	char*	inputfile	= 0;
	char*	outputfile	= 0;
	
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
	if (file_load (input, &decompressed_data, &size_decompressed) != ok) {
		fatal ("Couldn't load input file into memory\n");
	}
	size_original	= size_decompressed;
	size_compressed	= LZ4_compressBound (size_original);
	compressed_data	= malloc (size_compressed);
	if (!compressed_data) {
		fatal ("Couldn't allocate memory to compress file\n");
	}
	size_compressed	= LZ4_compress_default(decompressed_data, compressed_data, size_decompressed, size_compressed);
	if (size_compressed < 0) {
		fatal ("LZ4 compression failed\n");
	}	
	if (header_write (output, size_original)!=ok) {
		fatal ("Writing header failed\n");
	}
	if (size_compressed != fwrite (compressed_data, 1, size_compressed, output)) {
		warn ("Fewer bytes were written than were requested\n");
	}
	exit (EXIT_SUCCESS);	
}
