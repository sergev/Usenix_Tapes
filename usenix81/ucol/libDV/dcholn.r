subroutine dcholn(nva,nvb,n,m,nono)
implicit real*8(a-h,o-z)
integer*4nva,nvb,put
integer*4kz,j,i,n4,kj,k,l,il,jl
nsize(i) = ((i*(i-1))/2)
#
#	***  this routine expects the lower triangle of a ***
#
#        a= row-wise p.d. symm. system  n*(n+1)/2
#	 b= rhs  m*n stored row-wise
#        n= system dimension
#	 m=number of right-hand sides
#        nono .gt. 0 is the level at which p.d. failed
#
#	real*8 disvec version
#
nono = 0
n4 = n
if (get(nva,1)<=0.d0)
	nono = 1
else {
	ix = put(nva,1,dsqrt(get(nva,1)))
	xx = get(nva,1)
	do i = 1,m
		ix = put(nvb,i,get(nvb,i)/xx)
	if (n>=2)
		do i = 2,n {
			kz = nsize(i)
			ix = put(nva,kz+1,get(nva,kz+1)/get(nva,1))
			sg = get(nva,kz+1)**2
			xx = get(nva,kz+1)
			do j = 1,m {
				l = m*(i-1)+j
				ix = put(nvb,l,get(nvb,l)-xx*get(nvb,j))
				}
			if (i!=2) {
				jmax = i-1
				do j = 2,jmax {
					gkz = get(nva,kz+j)
					kj = nsize(j)
					kmax = j-1
					do k = 1,kmax
						gkz = gkz-get(nva,kz+k)*get(nva,kj+k)
					temp = gkz/get(nva,kj+j)
					ix = put(nva,kz+j,temp)
					do l = 1,m {
						il = m*(i-1)+l
						jl = m*(j-1)+l
						ix = put(nvb,il,get(nvb,il)-temp*get(nvb,jl))
						}
					sg = sg+temp**2
					}
				}
			gkz = get(nva,kz+i)-sg
			if (gkz<=0.d0) {
				write(6,10)i,gkz
				nono = i
				return
				}
			ix = put(nva,kz+i,dsqrt(gkz))
			do l = 1,m {
				il = m*(i-1)+l
				ix = put(nvb,il,get(nvb,il)/get(nva,kz+i))
				}
			}
	kz = nsize(n)
	xx = get(nva,kz+n4)
	do l = 1,m {
		il = m*(n4-1)+l
		ix = put(nvb,il,get(nvb,il)/xx)
		}
	if (n>1)
		do k = 2,n {
			i = n+1-k
			jmin = i+1
			do j = jmin,n {
				kj = nsize(j)
				xx = get(nva,kj+i)
				do l = 1,m {
					il = m*(i-1)+l
					jl = m*(j-1)+l
					ix = put(nvb,il,get(nvb,il)-xx*get(nvb,jl))
					}
				}
			kz = nsize(i+1)
			xx = get(nva,kz)
			do l = 1,m {
				il = m*(i-1)+l
				ix = put(nvb,il,get(nvb,il)/xx)
				}
			}
	}
return
10  format(1x,"pd failure at ",i4,2x,d12 .5)
end



