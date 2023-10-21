: : syntax:  ( . , . ) x ie
: : removes the indicated range of lines from the file,
: : and then executes them.
> /dev/null
~I,~J w ~Htmp~K
>
~I,~J d
: get to the correct line
~I-u
<~Htmp~K
!rm ~Htmp~K
