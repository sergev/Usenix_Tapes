: : syntax:  ( . , . ) x sep <text> [i|g]
: : starting at the first occurrence of <text>, the
: : indicated range of lines is separated into two lines.
: : any tabs or blanks preceding <text> are removed.
: : "i" or "g" indicates "interactive" or "global".
~I,~J s~D[ 	]*\(~1\)~D\n\1~D~2
