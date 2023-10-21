: : syntax: ( . , . ) x sort [<flags>]
: : the indicated range of lines is sorted.
: : If desired, the same flags and arguments may be specified
: : as are used in sort(I).
: direct byte count messages to null file
> /dev/null
~I,~J w ~Htmp1
!sort ~A ~Htmp1 > ~Htmp2
~I,~J d
~I- r ~Htmp2
: redirect output
>
!rm ~Htmp1 ~Htmp2
