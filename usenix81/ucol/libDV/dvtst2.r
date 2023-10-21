program chotst
#
#	test program for real*8 disvec of cholesky
#	decomposition subroutine.   multi-r.h.s. version dcholn.
#
integer*4i,j,k,l,m,n
real*8get,x,y,z
real*4vec(200),bec(400)
integer*4 put,set
write(6,10)
read(5,*) n1,n2,nd
call dsvprt
do n = n1,n2,nd {
	call dreset
	ia = set(2*n*n)
	ib = set(n)
	ic = set(2*n)
	id = set(n*n)
	ie = set(n)
	ib1 = set(n)
	ig = set(n*n)
	it = set(n)
#
#	fill b with noise
#
	ir = 0
	jr = 0
	do ii = 1,n {
		j = ii
		x = ran(ir,jr)
		ir = 1
		jr = 1
		bec(ii) = x
		is = put(ib,j,x)
		}
#
#	zero e and d
#
	do i = 1,n
		is = put(ie,i,0.d0)
	k = (n*(n+1))/2
	do i = 1,k
		is = put(id,i,0.d0)
#
#	compute c=a*b and d=at*a (without a)
#
	t1 = secnds(0.)
	do i = 1,2*n {
		do ii = 1,n
			vec(ii) = ran(ir,jr)
		x = 0.d0
		do ii = 1,n
			x = x+dble(vec(ii))*bec(ii)
		do j = 1,n {
			y = get(ie,j)+x*vec(j)
			is = put(ie,j,y)
			}
		m = 0
		do ii = 1,n
			do jj = 1,ii {
				m = m+1
				x = dble(vec(ii))*vec(jj)
				is = put(id,m,x+get(id,m))
				}
		}
	tset = secnds(0.)-t1
	t1 = secnds(0.)
#
#	solve system
#
	call dcholn(id,ie,n,1,nono)
	tsol = secnds(0.)-t1
	t1 = secnds(0.)
#
#	evaluate answer
#
	x = 0.d0
	y = x
	z = y
	do i = 1,n {
		x = x+get(ie,i)**2
		y = y+get(ib,i)**2
		z = z+abs(get(ib,i)-get(ie,i))
		}
	x = sqrt(x/n)
	y = sqrt(y/n)
	teval = secnds(0.)-t1
	call dsvprt
	write(6,20)n,nono,tset,tsol,teval,z
	}
stop
10  format("n1,n2,dn=?")
20  format(1x,2i4,3f10 .2,e12 .3)
end
