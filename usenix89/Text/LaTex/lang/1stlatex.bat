;		1stquote
cp68 1stquote.c 1stquote.i
c068 1stquote.i 1stquote.1 1stquote.2 1stquote.3 -f
rm 1stquote.i
c168 1stquote.1 1stquote.2 1stquote.s
rm 1stquote.1
rm 1stquote.2
as68 -l -u 1stquote.s
rm 1stquote.s
;		1ststyle
cp68 1ststyle.c 1ststyle.i
c068 1ststyle.i 1ststyle.1 1ststyle.2 1ststyle.3 -f
rm 1ststyle.i
c168 1ststyle.1 1ststyle.2 1ststyle.s
rm 1ststyle.1
rm 1ststyle.2
as68 -l -u 1ststyle.s
rm 1ststyle.s
;		1stwart
ckwart 1stwart.w 1stwart.c
;		1stlatex (includes 1stwart)
cp68 1stlatex.c 1stlatex.i
c068 1stlatex.i 1stlatex.1 1stlatex.2 1stlatex.3 -f
rm 1stlatex.i
c168 1stlatex.1 1stlatex.2 1stlatex.s
rm 1stlatex.1
rm 1stlatex.2
as68 -l -u 1stlatex.s
rm 1stlatex.s
;		finish it off
link68 [co[1stlatex.inp]]
relmod 1stlatex
rm 1stlatex.68K
wait
