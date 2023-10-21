	subroutine calmean(lx,x,mean,variance)
	real mean,variance,x(1),xsqar,xsum,ai
	integer lx
	xsqar=0
	xsum=0
	do 50  i=1,lx
	xsum=xsum+x(i)
50	xsqar=xsqar+(x(i)**2)
	ai=lx
	mean=xsum/ai
	variance=sqrt((xsqar-((xsum**2)/ai))/(ai-1.0))
	return
	end
