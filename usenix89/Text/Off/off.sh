:	off - run off a document.
:	the Colonel.  1986.
:	The first input line is a comment containing the invocation.
:
:	-n	substitute nroff for ditroff or troff, and neqn for eqn
:	no -n	substitute ditroff for nroff or troff
:	-p	pipe to lpr
:	-P	use pic, even in nroff
:	-xnn	run nn copies   /* only with -p or without -n */
:	-m	send mail when done
: Arguments to nroff/ditroff:
:	-om-n	select pages
:	-raN	set number register
:	-Tx	specify -Tx to ditroff or nroff -
:		also to pic and eqn, but not neqn
:
: Added -m.  GLS, 18 Dec 1986.
: Fixed bug in use of head.  GLS, 19 Dec 1986.
TMP=/usr/tmp/off$$
TMPI=/usr/tmp/offi$$
SEDSCRIPT=/usr/tmp/offss$$
POST1=/bin/cat
POST2=/bin/cat
trap 'rm -f $TMP $TMPI $SEDSCRIPT; if test "$mflag"; then \
  echo off: job $$ failed | mail $USER; fi; exit 1' 1 2
trap 'rm -f $TMP $TMPI $SEDSCRIPT; if test "$mflag"; then \
  echo off: job $$ completed | mail $USER; fi; exit 0' 0
TROFFFLAGS=
while test $# -gt 0
do
case $1 in
-m)	mflag=1;;
-n)	nflag=1;;
-o*)	TROFFFLAGS="$TROFFFLAGS $1";;
-p)	POST2=lpr;;
-P)	Pflag=1;;
-r*)	TROFFFLAGS="$TROFFFLAGS $1";;
-T*)	Tflag=$1; TROFFFLAGS="$TROFFFLAGS $1";;
-x*)	xflag=$1;;
-*)	echo $0: bad option $1 >&2; exit 1;;
*)	break;;
esac
shift
done
if test "$nflag"
then	ROFF=nroff
	EQN=neqn
	if test "$Pflag"
	then	PIC="pic"
	else	PIC="cat"
	fi
	if test "$pflag"
		then
		POST1=col
		POST2="$POST2 `echo $xflag | tr x #`"
	fi
else
	ROFF="ditroff $xflag"
	EQN="eqn $TFLAG"
	PIC="pic $TFLAG"
fi
cat $@ > $TMP
if head -1 $TMP | grep '^['"'"'\.]*\\".*roff' > $TMPI
then
cat <<ENDSED > $SEDSCRIPT
s/[a-z]*[nt]roff/$ROFF $TROFFFLAGS/
s/n*eqn/$EQN/
s/pic/$PIC/
s/^['.]*\\\\"//
ENDSED
INV=`sed -f $SEDSCRIPT $TMPI`
sh -c "$INV" < $TMP | $POST1 | $POST2
else
echo $0: no invocation on $1 >&2
exit 1
fi
exit 0
