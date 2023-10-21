integer*4 function argstr( key, svalue, sdef, sundef)
character*1 key
character*(*) svalue, sdef, sundef
include arglst.h
integer*4 iargc, ln
logical isxarg
character*ARGL x
automatic x

if( !isxarg(key) ) {
	svalue = sundef
	return UNDEFINED
}
ln = kurrnt
call getarg( ln, x )
usedup(kurrnt) = .true.
if( x(3:3) != ' ' ) {
	svalue = x(3:len(x))
	return VALUED
}
if( iargc() <= kurrnt ) {
	svalue = sdef
	return DEFINED
}
call getarg( ln+1, x)
if( x(1:1) == '-' ) {
	svalue = sdef
	return DEFINED
}
else {
	usedup( ln+1 ) = .true.
	svalue = x
	return VALUED
}
return
end

real*8 function r8xval( x )
character*(*) x
read( x, 01 ) r8xval
return
01	format( d22.0 )
end

logical function isxflt( x )
character*(*) x
real*8 y
read( x, 01, err=100 ) y
return .true.
100	continue
return .false.
01	format( d22.12 )
end

integer*4 function argr4( key, svalue, sdef, sundef)
character*1 key
real*4 svalue, sdef, sundef
include arglst.h
integer*4 iargc, ln
logical isxarg, isxflt
real*8 r8xval
character*ARGL x
automatic x

if( !isxarg(key) ) {
	svalue = sundef
	return UNDEFINED
}
ln = kurrnt
call getarg( ln, x )
usedup(kurrnt) = .true.
if( x(3:3) != ' ' ) {
	svalue = r8xval( x(3:len(x)) )
	return VALUED
}
if( iargc() <= kurrnt ) {
	svalue = sdef
	return DEFINED
}
call getarg( ln+1, x)
if( !isxflt(x) ) {
	svalue = sdef
	return DEFINED
}
else {
	usedup( ln+1 ) = .true.
	svalue = r8xval( x )
	return VALUED
}
return
end

integer*4 function argr8( key, svalue, sdef, sundef)
character*1 key
real*8 svalue, sdef, sundef
include arglst.h
integer*4 iargc, ln
logical isxarg, isxflt
real*8 r8xval
character*ARGL x
automatic x

if( !isxarg(key) ) {
	svalue = sundef
	return UNDEFINED
}
ln = kurrnt
call getarg( ln, x )
usedup(kurrnt) = .true.
if( x(3:3) != ' ' ) {
	svalue = r8xval( x(3:len(x)) )
	return VALUED
}
if( iargc() <= kurrnt ) {
	svalue = sdef
	return DEFINED
}
call getarg( ln+1, x)
if( !isxflt(x) ) {
	svalue = sdef
	return DEFINED
}
else {
	usedup( ln+1 ) = .true.
	svalue = r8xval( x )
	return VALUED
}
return
end


integer*4 function argi2( key, svalue, sdef, sundef)
character*1 key
integer*2 svalue, sdef, sundef
include arglst.h
integer*4 iargc, ln
logical isxarg, isxflt
real*8 r8xval
character*ARGL x
automatic x

if( !isxarg(key) ) {
	svalue = sundef
	return UNDEFINED
}
ln = kurrnt
call getarg( ln, x )
usedup(kurrnt) = .true.
if( x(3:3) != ' ' ) {
	svalue = r8xval( x(3:len(x)) )
	return VALUED
}
if( iargc() <= kurrnt ) {
	svalue = sdef
	return DEFINED
}
call getarg( ln+1, x)
if( !isxflt(x) ) {
	svalue = sdef
	return DEFINED
}
else {
	usedup( ln+1 ) = .true.
	svalue = r8xval( x )
	return VALUED
}
return
end

integer*4 function argi4( key, svalue, sdef, sundef)
character*1 key
integer*4 svalue, sdef, sundef
include arglst.h
integer*4 iargc, ln
logical isxarg, isxflt
real*8 r8xval
character*ARGL x
automatic x

if( !isxarg(key) ) {
	svalue = sundef
	return UNDEFINED
}
ln = kurrnt
call getarg( ln, x )
usedup(kurrnt) = .true.
if( x(3:3) != ' ' ) {
	svalue = r8xval( x(3:len(x)) )
	return VALUED
}
if( iargc() <= kurrnt ) {
	svalue = sdef
	return DEFINED
}
call getarg( ln+1, x)
if( !isxflt(x) ) {
	svalue = sdef
	return DEFINED
}
else {
	usedup( ln+1 ) = .true.
	svalue = r8xval( x )
	return VALUED
}
return
end

#	see if key is a defined argument

logical function isxarg( key )
character*1 key
include arglst.h
integer*4 iargc, l
integer*2 i, n
character*ARGL x
automatic x
data usedup /MAXARGS * .false. /

n = iargc()
for( i=1; i<= n; i = i + 1 )
	if( !usedup(i) ) {
		l = i
		call getarg( l, x)
		if( x(1:1) == '-' )
			if( x(2:2) == key ) {
				kurrnt = i
				return .true.
			}
	}
return .false.
end

#	consume a solo flag

integer*4 function argdef( key )
character*1 key
include arglst.h
logical isxarg
if( isxarg( key ) ) {
	usedup( kurrnt ) = .true.
	return DEFINED
}
else return UNDEFINED
end

#	pick up any unused arguments

logical function morarg( it )
character*(*) it
include arglst.h

integer*4 iargc
integer*4 i, n
n = iargc()
for( i=1; i<=n; i = i + 1 )
	if( !usedup(i) ) {
		call getarg( i, it )
		usedup(i) = .true.
		return .true.
	}
return .false.
end
