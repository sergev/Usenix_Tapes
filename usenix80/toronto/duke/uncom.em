: : syntax: ( . , . ) x uncom
: : this command undoes the action of com:
: : it changes lines of the form
: :	/* <text> */
: : to the form
: :	<text>
~I,~Js,/\* \(.*\) \*/,\1,
