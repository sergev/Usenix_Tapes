	subroutine  detrend(lx,x)
	real s,m,x(1),b
	integer lx
	s=0
	m=0
	do 50 i=1,lx
	s=s+x(i)
50	m=m+i*x(i)
	s=s/lx
	b=-6*(m/lx-s/2*(lx+1))/(lx-1)
	m=-2*b/(lx+1)
	do 100 i=1,lx
100	x(i)=x(i)-s-m*i-b
	return
	end
