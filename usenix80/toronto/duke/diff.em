: : syntax:  x diff [<name>]
: : does a "diff" to compare the file <name>
: : with the in-core copy of the file being edited.
: : <name> defaults to the name of the file being edited;
: : thus, "x diff" will tell what changes have been made during
: : the current edit session.
:
: direct byte count message to null file
> /dev/null
: use of addresses guarantees no reset of changes flag
1,$w ~Htmp~K
: reset editor output
>
!sh /usr/ebin/cmpdiff diff "~1" ~F ~Htmp~K
