	subroutine scale(ldata,data,low,high)
	real data(1),low,high,min,max
	integer ldata
	call minmax(ldata,data,min,max)
	scal=(high-low)/(max-min)
	do 50 i=1,ldata
50	data(i)=((data(i)-min)*scal)+low
	return
	end
