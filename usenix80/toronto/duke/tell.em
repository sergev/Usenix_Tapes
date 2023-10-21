: : syntax:  x tell [ <name> ... ]
: : Documentation is printed for each macro specified.
: : If no <name> is given, a list of available macros is printed.
: : BUG:  Each <name> is looked at by the shell, so any characters
: : in <name> having special meaning to the shell must be quoted.
!sh /usr/ebin/tell1 ~E ~A
