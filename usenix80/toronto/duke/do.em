: : syntax:  ( . , . ) x do <shell command>
: : For each line, the shell command is executed.  In the
: : shell command, all occurrences of $1 are replaced by
: : the first word in the line, all occurrences of $2 are
: : replaced by the second word, etc.
> ~Htmp~Ka
ms.~A
: direct byte count message to null file
> /dev/null
~I,~J w ~Htmp~Kb
: redirect output
>
!/usr/bin/gres . "^" "sh ~Htmp~Ka " < ~Htmp~Kb | sh -; rm ~Htmp~K[ab]
