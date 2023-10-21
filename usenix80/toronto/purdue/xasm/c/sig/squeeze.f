	subroutine squeeze(lold,old,lnew,new)
	real old(1),new(1),postion,delta,scale,flold,flnew,fk
	integer lold,lnew,k
	flold=lold
	flnew=lnew
	scale=flold/flnew
	do 50 j=1,lnew
	postion=scale*j
	k=postion
	fk=k
	delta=postion-fk
50	new(j)=((old(k+1)-old(k))*delta)+old(k)
	return
	end
