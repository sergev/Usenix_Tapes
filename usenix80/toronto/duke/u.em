: : syntax: ( . , . ) x u <text>
: : performs the equivalent of
: :	( . , . ) u.<text>
: : except that tabs and blanks are first removed
: : at the point of the unite.
:
: "goto" rather than exit so that dot will be set.
!if ~I = ~J goto end
~I+,~Js/^[ 	]*//
: end
~I,~Ju.~A
