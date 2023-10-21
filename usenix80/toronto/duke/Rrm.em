: : syntax:  ( . , . ) x Rrm <text> [i]
: : Each line is truncated, beginning at the first occurrence
: : of <text>.  Any blanks and tabs immediately preceding <text>
: : are also removed.
: : If "i" is specified, the action is done interactively.
~I,~Js~D~1~D\~D~2
~I,~Js/[ 	]*\.*//
