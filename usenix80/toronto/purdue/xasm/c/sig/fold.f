	subroutine fold(la,a,lb,b,lc,c,sample)
	dimension a(1),b(1),c(1)
	real sample
	lc=la+lb-1
	call presto(lc,c,0.)
	do 10 i=1,la
	do 10 j=1,lb
	k=i+j-1
10	c(k)=c(k)+(a(i)*b(j))/sample
	return
	end
