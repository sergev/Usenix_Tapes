subroutine dchols(nva,nvg,nvb,nvy,nvx,n,nono)
implicit real*8(a-h,o-z)
integer nva,nvb,nvg,nvy,nvx,put
integer*4kz,j,i,n4,kj,k
nsize(i) = ((i*(i-1))/2)
#
#	***  this routine expects the lower triangle of a ***
#
#        a= row-wise p.d. symm. system  n*(n+1)/2
#        g= cholesky storage
#        b= r.h.s. vector               n
#        y= temp. vector
#        x= answer vector
#        n= system dimension
#        nono .gt. 0 is the level at which p.d. failed
#
#        (a,g) and (b,y,x) may be equivalenced.
#
#	real*8 disvec version
#
nono = 0
n4 = n
if (get(nva,1)<=0.d0)
	nono = 1
else {
	ix = put(nvg,1,dsqrt(get(nva,1)))
	ix = put(nvy,1,get(nvb,1)/get(nvg,1))
	if (n>=2) {
		do i = 2,n {
			kz = nsize(i)
			ix = put(nvg,kz+1,get(nva,kz+1)/get(nvg,1))
			sg = get(nvg,kz+1)**2
			ix = put(nvy,i,get(nvb,i)-get(nvg,kz+1)*get(nvy,1))
			if (i!=2) {
				jmax = i-1
				do j = 2,jmax {
					gkz = get(nva,kz+j)
					kj = nsize(j)
					kmax = j-1
					do k = 1,kmax
						gkz = gkz-get(nvg,kz+k)*get(nvg,kj+k)
					temp = gkz/get(nvg,kj+j)
					ix = put(nvg,kz+j,temp)
					ix = put(nvy,i,get(nvy,i)-temp*get(nvy,j))
					sg = sg+temp**2
					}
				}
			gkz = get(nva,kz+i)-sg
			if (gkz<=0.d0)
				go to 10
			ix = put(nvg,kz+i,dsqrt(gkz))
			ix = put(nvy,i,get(nvy,i)/get(nvg,kz+i))
			}
		go to 20
		10  nono = i
		return
		}
	20  kz = nsize(n)
	ix = put(nvx,n4,get(nvy,n4)/get(nvg,kz+n4))
	if (n>1)
		do k = 2,n {
			i = n+1-k
			ix = put(nvx,i,get(nvy,i))
			jmin = i+1
			do j = jmin,n {
				kj = nsize(j)
				ix = put(nvx,i,get(nvx,i)-get(nvg,kj+i)*get(nvx,j))
				}
			kz = nsize(i+1)
			ix = put(nvx,i,get(nvx,i)/get(nvg,kz))
			}
	}
return
end



