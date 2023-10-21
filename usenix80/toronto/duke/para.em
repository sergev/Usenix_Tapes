: : syntax: ( . , . ) x para
: : The indicated range of lines is run thru nroff.
: : Right justification is not done.
:
: Suppose right justification WERE done.  Then after running
: a piece of text thru para 5 or 6 times, it would be mostly
: blanks!
:
: direct byte count messages to null file
> /dev/null
~I,~J w ~Htmp~Ka
!nroff /usr/ebin/para1 ~Htmp~Ka /usr/ebin/para2 > ~Htmp~Kb
~I,~J d
~I- r ~Htmp~Kb
: redirect output
>
!rm ~Htmp~Ka ~Htmp~Kb
