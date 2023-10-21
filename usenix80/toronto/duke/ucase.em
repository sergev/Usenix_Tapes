: : ( . , . ) x ucase
: : Translates all lower case characters in the given range of lines
: : to upper case.
> /dev/null
~I,~Jw ~Htmp~Ka
!tr "[a-z]" "[A-Z]" < ~Htmp~Ka > ~Htmp~Kb
~I,~Jd
~I-1r ~Htmp~Kb
>
!rm ~Htmp~K[ab]
