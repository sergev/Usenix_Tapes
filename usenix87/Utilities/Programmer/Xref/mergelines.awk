# Input: lines of the form "name \t file \t line number"
# Output: merged lines of the form "name \t file \t n1 n2 n3 ..."
#   Starts a new line for new names or files, and when line gets long.
#   Suppresses name and previous file on continuation lines.

BEGIN { # Print header and compute usable width
	fmt = "%-14s\t%-14s\t%s\n"
	printf fmt, "NAME", "FILE", "LINE NUMBERS"
	###uwidth = '$width'-16-16-5;  # sizes of name, file, number
	if (uwidth<3) uwidth=80-16-16-5;  }

$1 != name { printf fmt, pname, pfile, line;  
	name = pname = $1;  file = pfile = $2;  line = "" }

$2 != file { printf fmt, pname, pfile, line;  
	pname = "";  file = pfile = $2;  line = "" }

length(line) > uwidth { printf fmt, pname, pfile, line;  
	pname = "";  pfile = "";  line = "" }

{ line = line " " sprintf("%4s",$3) }

END { printf fmt, pname, pfile, line }
