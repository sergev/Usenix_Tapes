

b=7; f=23
case $1 in
-d)	b=7; f=55; shift;;
-g)	b=10; f=52; shift;;
-h)	b=14; f=112; shift;;
-?)	shift;;
esac
case $# in
1)	m=$1; e=0;;
2)	m=$1; e=$2;;
*)	echo Usage: fh [-d|g|h] number \[exp]; exit;;
esac
bc << END
scale = $f*0.3+2
obase = 16
m=$m
e=$e
if(m < 0) {
	m = -m
	s = 1
}
while(m >= 2) {
	m /= 2
	b += 1
}
while(m < 1) {
	m *= 2
	b -= 1
}
while(e > 0) {
	m *= 10
	e -= 1
	while(m >= 2) {
		m /= 2
		b += 1
	}
}
while(e < 0) {
	m /= 10
	e += 1
	while(m < 1) {
		m *= 2
		b -= 1
	}
}
define h(e,f) {
	scale = 0
	return ( ((b+2^e+1 + m-1 + s*2^(e+1)) * 2^f +0.5)/1 )
}
h($b,$f)
quit
END

