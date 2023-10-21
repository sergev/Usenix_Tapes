: : syntax: ( . , . ) x pipe <shell command>
: : The shell command is executed, with the indicated
: : range of lines forming its standard input.
> /dev/null
~I,~J w ~Htmp~K
>
!~A <~Htmp~K; rm ~Htmp~K
