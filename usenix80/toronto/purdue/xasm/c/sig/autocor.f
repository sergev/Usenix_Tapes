	subroutine autocor(lx,x,lout,out)
	real x(1),out(1),sum,foo
	do 20 j=1,lout
	nj=lx-j+1
	sum=0.
	do 10 i=1,nj
	ij=i+j-1
10	sum=sum+x(i)*x(ij)
	foo=float(nj)
20	out(j)=sum/foo
	return
	end
