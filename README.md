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

`jsonlz4_json -i bookmarks.jsonlz4 -o bookmarks.json`

Pretty up the output<br/> 
`jsonlz4_json -i sample.jsonlz4 | python -m json.tool`

##### LICENSE
Creative Commons
[CC0](http://creativecommons.org/publicdomain/zero/1.0/legalcode)  

##### AUTHOR
The code already exists in various forms on the net.

This version is reduced to running on Linux or Unix systems
with lz4 installed and lz4 development headers available.

[James Sainsbury](mailto:toves@sdf.lonestar.org)

