## jsonlz4_json
Uncompress lz4 compressed json bookmarks (Firefox)

##### USAGE
`jsonlz4_json [-i input] [-o output]`

##### BUGS
Just for Linux/Unix 

##### PREREQUISITES

rhel7/centos7
	lz4 	to run
	lz4-devel to build

##### EXAMPLES
`jsonlz4_json -i bookmarks.jsonlz4` 
`jsonlz4_json -i sample.jsonlz4 | python -m json.tool`

##### LICENSE
Creative Commons CC0
(http://creativecommons.org/publicdomain/zero/1.0/legalcode)  

##### AUTHOR
[James Sainsbury](mailto:toves@sdf.lonestar.org)

