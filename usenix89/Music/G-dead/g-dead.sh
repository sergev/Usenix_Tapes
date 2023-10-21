
#! /bin/sh

# Produce a cassette label for a Grateful Dead concert...   [:-)]

DEDIR=/usr/users/zben/Dead

case $1 in
-v|-V) VFLAG= ; shift ;;
esac

case $# in
3) ;;
*) echo Usage: $0 [-v] month day year 1>&2 ; exit 2 ;;
esac

STUFF=`grep $1-$2-$3 $DEDIR/$3.enc`

case $? in
0) ;;
1) echo Concert not found 1>&2 ; exit 2 ;;
2) echo Year not found 1>&2 ; exit 2 ;;
*) echo Grep error 1>&2 ; exit 2 ;;
esac

DATE=`echo $STUFF | sed -n 's|^ *\([-0-9]*\) .*$|\1|p'`
# echo $DATE 1>&2

PLACE=`echo $STUFF | sed -n 's|^ *[-0-9]* *\([^~]*\)~~.*$|\1|p'`
echo $PLACE 1>&2

SETONE=`echo $STUFF | sed -n 's|^[^~]*~~\([^~]*\).*$|\1|p'`
# echo "|${SETONE}|" 1>&2
case "$SETONE" in
"") echo Set one not extracted 1>&2 ; exit 2 ;;
esac

cat <<@
%!PS-Adobe-1.0 
/in { 72 mul } def 
/date { /datevar exch def } def
/place { /placevar exch def } def
/drawcass { 2 setlinewidth 2 setlinecap .4 setgray newpath
    0    0    moveto
    0    4 in lineto  
 .625 in 4 in lineto  
 .625 in 0    lineto 
    0    0    lineto  
fill 0 setgray
 .625 in 0    moveto  .625 in 4 in lineto stroke 
    0    0    moveto     0    4 in lineto stroke 
1.125 in 0    moveto 1.125 in 4 in lineto stroke 
 3.75 in 0    moveto  3.75 in 4 in lineto stroke 
    0    0    moveto  3.75 in 0    lineto stroke 
1.125 in 2 in moveto  3.75 in 2 in lineto stroke 
    0    4 in moveto  3.75 in 4 in lineto stroke 
${VFLAG-3.725}${VFLAG+1.25 } in ${VFLAG-281}${VFLAG+7  }  translate
${VFLAG--}90 rotate } def 
/deadcass { initmatrix translate drawcass
gsave /Helvetica-Bold findfont 14 scalefont setfont  
  0      ${VFLAG--3 }${VFLAG+0.2} in translate
  0      0.2 in moveto (Grateful Dead) show
  1.6 in 0.2 in moveto datevar show
  3.2 in 0.2 in moveto show
  0      0      moveto placevar show
grestore 0 0 moveto } def
/set1 { (Set I)   4.3 in 5.1 in deadcass } def
/set2 { (Set II)  4.3 in 1.0 in deadcass } def
/set3 { (Set III) 0.5 in 5.1 in deadcass } def
/set4 { (Set IV)  0.5 in 1.0 in deadcass } def
/flip { 148 0 translate 0 0 moveto } def
/song { currentpoint 12 sub 0 exch moveto pop show } def 
/Helvetica findfont 10 scalefont setfont
@

echo \(${DATE}\)date
echo \(${PLACE}\)place
echo set1
OFFSET=`echo ${SETONE}aaa | wc -c`
OFFSET=`expr $OFFSET / 4`
# echo $OFFSET 1>&2
echo $SETONE | sed -f $DEDIR/dead1.sed | sed -f $DEDIR/dead2.sed | sed 's|^|(|' | sed 's|$|)song|' | sed $OFFSET's|$|\
flip|'

SETTWO=`echo $STUFF | sed -n 's|^[^~]*~~[^~]*~\([^~]*\)~.*$|\1|p'`
# echo "|${SETTWO}|" 1>&2
ENCORE=`echo $STUFF | sed -n 's|^[^~]*~~[^~]*~[^~]*~\(.*\)$|\1|p'`
# echo "|${ENCORE}|" 1>&2

case "${SETTWO}${ENCORE}" in
"") echo No second set 1>&2; ;;
*)
OFFSET=`echo ${SETTWO}aaa${ENCORE} | wc -c`
OFFSET=`expr $OFFSET / 4`
# echo $OFFSET 1>&2

echo set2
echo ${SETTWO}~${ENCORE} | sed -f $DEDIR/dead1.sed | sed -f $DEDIR/dead2.sed | sed 's|^|(|' | sed 's|$|)song|' | sed $OFFSET's|$|\
flip|'

esac

echo showpage
