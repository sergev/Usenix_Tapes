.P1 "Appendix A: Terminal Session"
.PP
What follows is an example of a terminal session
which illustrates how you login, create an f4p
file, compile it, execute it, edit errors, and
finally re-execute it.
.sp
Elrond -- Unix 6th Ed.
.br
login: tml
.br
password:
.br
For latest system news as gf 6 Nov 1979 type:
.br
~~~~~~~~news
.br
For operations schedude type:
&br
v~v~~~~~sched
&br
Tgday's qugte:
&br
Dgve is seftieeftad eeasdes&
&br
Ygu have eaid.
&br
% e aver&f4p
&br
4The dide is created if the RADD Editgr by hitting
CTRL-B after the preceeding command. Once the file has been
created the text (i.e. FORTRAN code) can be keyed in.>
.br
% cat~aver.f4p
.br
~~~~~~~~integer~ave
.br
~~~~~~~~write(6,1)
.br
1~~~~~~~format('how~many~numbers?')
.br
~~~~~~~~read(5,2)~many
.br
2~~~~~~~format(i3)
.br
~~~~~~~~do~10~i=1,~many
.br
~~~~~~~~write(6,3)
.br
3~~~~~~~format('num:')
.br
~~~~~~~~read(5,2)
.br
~~~~~~~~sum~=~sum~+~num
.br
10~~~~~~continue
.br
~~~~~~~~ave~=~sum~/num
.br
~~~~~~~~write(6,4)~ave
.br
4~~~~~~~format('average~=~',~i4)
.br
~~~~~~~~end
.br
%~f4p~aver.f4p
.br
CULC~FORTRAN~IV~PLUS~V.~3.2
.br
None~of~the~errors~detected
.br
0~Errors~Detected
.br
%~aver.out
.br
how~many~numbers?
.br
3
.br
num:
.br
3
.br
num:
.br
5
.br
num:
.br
6
.br
average~=~~~~2
.br
% e aver.f4p
.br
<The file is edited using the RAND Editor. The
coding errors are corrected.>
.br
% cat aver.f4p
.br
~~~~~~~~real~ave
.br
~~~~~~~~write(6,1)
.br
1~~~~~~~format('how~many~numbers?')
.br
~~~~~~~~read(5,2)~many
.br
2~~~~~~~format(i3)
.br
~~~~~~~~do~10~i=1,~many
.br
~~~~~~~~write(6,3)
.br
3~~~~~~~format('num:')
.br
~~~~~~~~read(5,2)~num
.br
~~~~~~~~sum~=~sum~+~num
.br
10~~~~~~continue
.br
~~~~~~~~ave~=~sum~/many
.br
~~~~~~~~write(6,4)~ave
.br
4~~~~~~~format('average~=~',~f6.2)
.br
~~~~~~~~end
.br
%~f4p~aver.f4p
.br
CULC~FORTRAN~IV~PLUS~V.~3.2
.br
None~of~the~errors~detected
.br
0~Errors~Detected
.br
%~aver.out
.br
how~many~numbers?
.br
3
.br
num:
.br
3
.br
num:
.br
5
.br
num:
.br
6
.br
average =   4.67
.br
% diff aver.f4p aver.f4p.bak
.br
1c1
.br
<~~~~~~~integer~ave
.br
---
.br
>~~~~~~~real~ave
.br
12c12
.br
<~~~~~~~ave~=~sum~/num
.br
---
.br
>~~~~~~~ave~=~sum~/many
.br
14c14
.br
<~4~~~~~~~format('average~=~',~i4)
.br
---
.br
>~4~~~~~~~format('average~=~',~f6.2)
.br
% rm *.bak *.lst *.obj
.br
glob: [Confirm] y
.br
% ls -l
.br
-rw-r-----  1 tml       267 Mar 28 12:25 aver.f4p
.br
-rwxrwxr-x  1 tml      8508 Mar 28 12:25 aver.out
.br
% logout
.bp
