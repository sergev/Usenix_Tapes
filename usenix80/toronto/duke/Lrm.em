: : syntax: ( . , . ) x Lrm <text> [i]
: : The string of characters from the beginning of the line
: : up to the first occurrence of <text> is removed.
: : Any blanks and tabs following <text> are also removed.
: : If "i" is specified, the action is done interactively.
~I,~Js~D~1~D\~D~2
~I,~Js/.*[ 	]*//
