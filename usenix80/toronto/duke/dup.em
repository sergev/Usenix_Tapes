: : syntax:  ( . , . ) x dup <number>
: : creates <number> new copies of the indicated range of lines
> /dev/null
!sh - ~1 ~H~K ~I ~J
set a = 1
if $1x = x goto skip
set a = $1
: skip
/usr/ebin/dup $a $3,$4"t." > $2
bye
< ~H~K
!rm ~H~K
>
