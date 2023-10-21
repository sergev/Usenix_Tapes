.CH "Deviations from the standard"
There is one serious defect in the standard proposal (and in the language
as described by Jensen and Wirth).
This defect causes all implementors of Pascal on machines with a
time-sharing environment to deviate from the standard.
The defect is that interactive input is impossible.
More precisely: it is not possible to prompt a user that the program is
running and requires input, because the file window input^ must be
initialized with the first character of input before any other statement
can be executed.
This is caused by the requirement of the standard that the file input,
if mentioned in the program heading, must be initialized by the statement
reset(input).
.sp
The solution we adopted is to replace this requirement by:
.IS
The file input, if mentioned in the program heading, is opened
such that:
.IS
.LB -
the statement reset(input) is not required to start reading,
.LB -
the file window input^ is initialized with a blank,
.LB -
the function eoln(input) yields true.
.IE
Moreover, the statement reset(input) is equivalent to get(input)
as stated before.
.IE
This solution offers several advantages:
.IS
.LB -
Interactive input is possible with a minimal change to the language.
.LB -
Most standard programs will work without any modification.
Only if programs skip leading spaces, problems may occur.
.LB -
If programs conforming to the standard must be modified, then this modification
is trivial: add the statement get(input).
.LB -
Programs created on the Pascal-VU system can be ported easily to
other installations.
They will work without any modification, if an empty line
is prepended to the input.
Modification is probably trivial: removal of the statement get(input).
.LB -
The first character of the input file can be obtained in three different ways:
     get(input),
     readln(input) and
     reset(input).
.IE
We present two examples demonstrating why the alternatives for initializing
the window input^ are useful.
The first of these alternatives, get(input), is probably best for portability reasons.
However, a call of readln(input) makes reading of all lines
similar, as is demonstrated by the following program:
.EQ
     program add(input,output);
     {This program adds integers}
     var stop:boolean;
         i,total:integer;
     begin total:=0; stop:=false;
         repeat
             write('next number: ');
             readln;
             if eof then
                 stop:=true
             else
                 begin
                     read(i);
                     total:=total+i
                 end
         until stop;
         writeln('these numbers add to ',total:1)
     end.
.EN
Transporting this program to a fully standard Pascal system is more difficult.
Initializing the file window by reset(input) may be used in programs like:
.EQ
     program append(input,secondin,output);
     {This program concatenates two files}
     var secondin:text;
     
     procedure copy(var fi:text);
     var c:char;
     begin
         reset(fi);
         while not eof(fi) do
             begin
                 while not eoln(fi) do
                     begin
                         read(fi,c);
                         write(c)
                     end;
                 readln(fi);
                 writeln
             end
     end;
     
     begin {main}
         copy(input);
         copy(secondin)
     end.
.EN
