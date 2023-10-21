subroutine bsint(n,f,h,x,y,lflow,epq)

#	ode integrator
#		n	number of first-order equations
#		f	subroutine called with
#				call f(x,y,dy)
#			which computes dy from y and x
#		h	on calling: desired step size
#			on return: optimal size for next step
#		x	on calling: current x
#			on return: new x
#		y	vector of dimension n containing solution
#			usage is the same as x
#		lflow	logical; true is routine could not make
#			the entire requested step
#		epq	local truncation error allowed

#	bsint is entirely real*8

#	call bsclr to reset internal arrays if the
#	solution is altered discontinuously

define NMAX	20
implicit static (a-z)
implicit real*8 (a-h,o-z)
external f
integer r,sr
logical konv,bo,bh,fin,lflow
common/bsintx/s(NMAX)
dimension y(1),ya(NMAX),
ym(NMAX),dy(NMAX),dz(NMAX),dt(NMAX,7),d(7),yg(8,NMAX),yh(8,NMAX),
yl(NMAX)
#
#
eps = amax1(epq,1.e-13)
bh = .false.
fin = bh
call f(x,y,dz)
do i = 1,n
	ya(i) = y(i)
repeat {
	a = h+x
	fc = 1.5
	bo = .false.
	m = 1
	r = 2
	sr = 3
	jj = -1
	do j = 1,10 {
		if (bo) {
			d(2) = 16./9.
			d(4) = 4.*d(2)
			d(6) = 4.*d(4)
			}
		else {
			d(2) = 2.25
			d(4) = 9.
			d(6) = 36.
			}
		konv = j>3
		if (j<=7) {
			l = j-1
			d(j) = m*m
			}
		else {
			l = 6
			d(7) = 64.
			fc = 0.6*fc
			}
		m = 2*m
		g = h/float(m)
		b = g*2.
		if (bh&&j<9)
			do i = 1,n {
				ym(i) = yh(j,i)
				yl(i) = yg(j,i)
				}
		else {
			kk = (m-2)/2
			m = m-1
			do i = 1,n {
				yl(i) = ya(i)
				ym(i) = ya(i)+g*dz(i)
				}
			do k = 1,m {
				call f(x+k*g,ym,dy)
				do i = 1,n {
					u = yl(i)+b*dy(i)
					yl(i) = ym(i)
					ym(i) = u
					u = abs(u)
					s(i) = amax1(u,s(i))
					}
				if (k!=2&&k==kk) {
					jj = 1+jj
					do i = 1,n {
						yh(jj+1,i) = ym(i)
						yg(jj+1,i) = yl(i)
						}
					}
				}
			}
		call f(a,ym,dy)
		do i = 1,n {
			if (j==1)
				dt(i,1) = 0.
			v = dt(i,1)
			ta = (ym(i)+yl(i)+g*dy(i))*0.5
			c = ta
			dt(i,1) = c
			lp = max0(1,l)
			do k = 1,lp {
				b1 = d(k+1)*v
				b = b1-c
				u = v
				if (b!=0.) {
					b = (c-v)/b
					u = c*b
					c = b1*b
					}
				v = dt(i,k+1)
				dt(i,k+1) = u
				ta = u+ta
				}
			if (abs(y(i)-ta)>eps*s(i))
				konv = .false.
			y(i) = ta
			}
		if (konv)
			break 2
		d(3) = 4.
		d(5) = 16.
		bo = !bo
		m = r
		r = sr
		sr = 2*m
		}
	bh = !bh
	fin = .true.
	h = 0.5*h
	}
h = fc*h
hus = abs(x-a)
x = a
lflow = fin
return
end

subroutine bsclr
real*8 s
common/bsintx/s(NMAX)

do i = 1, NMAX
	s(i) = 0.d0

return
end



