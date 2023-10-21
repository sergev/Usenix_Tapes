	subroutine swapby(list,n)
c
c	this subroutine added to ease pdp conversion.
c	the list of n i*2 words is byte-swapped.
c
	integer*2 list(n),k
	if(n.le.0) return
	do 100 i=1,n
c	k=ishft(list(i),8)
        k=list(i)*2**8
c	list(i)=or(k,ishft(list(i),-8))
        list(i)=or(k,list(i)/(2**8))
100	continue
	return
	end
