: : ( . , . ) x ucase
: : Translates all upper case characters in the given range of lines
: : to lower case.
> /dev/null
~I,~Jw ~Htmp~Ka
!tr "[A-Z]" "[a-z]" < ~Htmp~Ka > ~Htmp~Kb
~I,~Jd
~I-1r ~Htmp~Kb
>
!rm ~Htmp~K[ab]
