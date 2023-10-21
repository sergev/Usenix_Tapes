: : syntax:  x burn
: : does a "rm" on the file being edited,
: : and then leaves the editor
!sh - ~F
query "type y to remove "$1
if $R != 121 goto end
rm $1
bye
quit

: end
bye
