/*------------------------------------------------------------*/
/* to run this program: enter prolog and type ['game.pl'],go. */
/*------------------------------------------------------------*/

go :-
	noscroll,
	system('stty cbreak',_),
	start,!,
	play.

play :-
	putat(24,1,"type space to spin the chamber"),
	get0(_),
	putat(24,1,"                              "),
	random(6,N),
	move(24,60),put("you spun "),write(N),
	possibly_die(N),
	play.
	
possibly_die(6) :- die.
possibly_die(N).

putat(X,Y,Text) :- move(X,Y),put(Text).

start  :-
	cls,print_thing_at(2,5,gun),print_thing_at(1,60,head).
	
die :-
	put(7),
	print_thing_at(11,20,bang),
	un_print_thing_at(11,20,bang),
	move_thing_at(2,39,30),
	print_thing_at(2,69,bullit),
	print_thing_at(11,20,aagh),
	un_print_thing_at(11,20,aagh),            
	un_print_thing_at(1,60,head),
	cls,
	system('echo "your game got played" | mail joe@erix.UUCP',_),
	putat(24,1,"like to dice (sic) with death again (y/n):"),
	get0(X),                    
	possibly_doit_again(X).
	
possibly_doit_again(121) :- start,!,play.
possibly_doit_again(110) :- abort.

move_thing_at(X,Y,N) :- 
    (for(I,1,N),
    Y1 is Y + I -1,
    print_thing_at(X,Y1,bullit),
    un_print_thing_at(X,Y1,bullit),
    fail);true.
    

un_print_thing_at(X,Y,Z) :- un_print_thing_at_1(X,Y,Z);true.

un_print_thing_at_1(X1,Y,Z) :-
    F =.. [Z,N,L],!,
    call(F),                
    X is X1 + N - 1,
    move(X,Y),
    undraw(L),nl,fail.
    
undraw(L) :-
    (length(L,M),!,for(I,1,M),put(32),fail);true.

print_thing_at(X,Y,Z) :- print_thing_at_1(X,Y,Z);true.

print_thing_at_1(X1,Y,Z) :-
    F =.. [Z,N,L],!,
    call(F),
    X is X1 + N - 1,
    move(X,Y),
    put(L),nl,fail.

for(I,I,Upper).
for(I,Lower,Upper) :-
    Lower < Upper,
    Next is Lower + 1,
    for(I,Next,Upper).

move(X,Y) :- printf("%c%c%d%c%d%c",[27,91,X,59,Y,72]).

scroll(X,Y) :- printf("%c%c%d%c%d%c",[27,91,X,59,Y,114]),move(24,1).

noscroll :- scroll(1,24).

cls :- printf("%c%c%c%c",[27,91,50,74]).

seed(13).

random(R,N) :-
	retract(seed(S)),
	N is (S mod R) +1,
	NewSeed is (125*S+1) mod 4096,
	asserta(seed(NewSeed)),!.
	
gun(1, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").
gun(2, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").
gun(3, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx").
gun(4, "xxxxxxxxxxxxxxxxxxxxxxxxx").
gun(5, "xxxxxxxxxxxxx x    x").
gun(6, "xxxxxxxxxxxxx  x  x").
gun(7, "xxxxxxxxxxxxx  x x").
gun(8, "xxxxxxxxxxxxxxxxx").
gun(9, "xxxxxxxxxxx").
gun(10,"xxxxxxxxxxx").
gun(11,"xxxxxxx").
gun(12,"xxxxxxx").
gun(13,"xxxxxxx").
gun(14,"xxxxxxx").
gun(15,"xxxxxxx").
gun(16,"xxxxxxx").

head(1, "        xxxxxxxxx").
head(2, "      xxxxxxxxxxxx").
head(3, "     xx          xxx").
head(4, "    x         xxxxxxx").
head(5, "   x  xxx      xxxxxx").
head(6, "   x xxxxx  x    xxx").
head(7, "    x xxxx    xx  xx").
head(8, "    x  xx       x  x").
head(9, "    x  xx        x x").
head(10,"   x  x          x x").
head(11,"  x               x").
head(12," x               x").
head(13,"x               x").
head(14,"x               x").
head(15,"xxxxx           x").
head(16," xx              x").
head(17," xxx              x").
head(18,"  xx   xxx        x").
head(19,"   xxxx   x      x").
head(20,"          x      x").
head(21,"          x      x").

bullit(1,"==>").
bullit(2,"===>").
bullit(3,"==>").

bang(1, "xxxx     xx    x    x   xxx  ").
bang(2, "x   x   x  x   xx   x  x   x ").
bang(3, "x  x   x    x  x x  x  x   x ").
bang(4, "xxx    xxxxxx  x x  x  x     ").
bang(5, "x  x   x    x  x  x x  x  xxx").
bang(6, "x   x  x    x  x   xx   x  x ").
bang(7, "xxxx   x    x  x    x    xx  ").

aagh(1, "  xx     xx     xxx   x    x").
aagh(2, " x  x   x  x   x   x  x    x").
aagh(3, "x    x x    x  x   x  x    x").
aagh(4, "xxxxxx xxxxxx  x      xxxxxx").
aagh(5, "x    x x    x  x  xxx x    x").
aagh(6, "x    x x    x  x  x   x    x").
aagh(7, "x    x x    x   xx    x    x").

