	subroutine minmax(ldata,data,min,max)
	real data(1),min,max
	integer ldata
	min=10000.
	max=-10000.
	do 50 i=1,ldata
	if(max-data(i))20,30,30
20	max=data(i)
30	if(data(i)-min)40,50,50
40	min=data(i)
50	continue
	return
	end
