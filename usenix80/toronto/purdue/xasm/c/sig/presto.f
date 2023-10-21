	subroutine presto(lx,x,preset)
	dimension x(1)
	if(lx.le.0)return
	do 10 i=1,lx
10	x(i)=preset
	return
	end
