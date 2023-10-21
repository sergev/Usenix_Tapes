	subroutine bound(lx,x,lower,upper)
	real x(1),lower,upper
	integer lx
	do 100 i=1,lx
		if(lower-x(i))20,20,10
10		x(i)=lower
20		if(x(i)-upper)100,100,30
30		x(i)=upper
100	continue
	return
	stop
	end
