      subroutine line (x,y,n,k,j,l)
c
c  this subroutine  is used to plot scaled data
c  values in an array. the array may be plotted as
c  solid lines or with symbols at every point,every
c  n points,or with symbols only
c
      dimension x(1),y(1),label(1)
	include 'gsdata.f'
      lmin = n*k+1
      ldx  = lmin+k
      nl   = lmin-k
      xmin = x(lmin)
      dx   = x(ldx)
      ymin = y(lmin)
      dy   = y(ldx)
      label(1)=l
c
c     save units
c
      iunit=msetun
c
c     set units to inches
c
      call setin
c
c     find end of line closest to current pen position
c
      call where (xn,yn,xfact)
      df = abs ((x(1)-xmin)/dx-xn)
      df2 = abs ((y(1)-ymin)/dy-yn)
      dl = abs ((x(nl)-xmin)/dx-xn)
      dl2 = abs ((y(nl)-ymin)/dy-yn)
      if ( df - df2 ) 100,101,101
c
  100 df = df2
  101 if (dl - dl2) 102,103,103
  102 dl = dl2
  103 ic = 3
      is = -1
      nt =iabs(j)
      if (j) 2,1,2
    1 nt = 1
    2 if (df-dl) 4,4,3
    3 nf = nl
      na = ((n-1)/nt)*nt+nt-(n-1)
      kk = -k
      go to 5
    4 nf = 1
      na = nt
      kk = k
    5 if (j) 6,7,8
    6 ica = 3
      isa = -1
      lsw = 1
      go to 10
    7 na = ldx
    8 ica = 2
      isa = -2
      lsw = 0
   10 do 30 i = 1,n
      xn = (x(nf)-xmin)/dx
      yn = (y(nf)-ymin)/dy
c
c     test for -0 values of x and y
c     if (xn) 14,12,14
c  12 xn = 0.0
c  14 if (yn) 18,16,18
c  16 yn = 0.0
c
   18 if (na-nt) 20,21,22
   20 if (lsw) 23,22,23
21	continue
      call symbol(xn,yn,0.07,label,0.0,is)
      xn=xn+1
      na = 1
      go to 25
  22  call plot(xn,yn,ic)
   23 na = na + 1
   25 nf = nf+kk
      is = isa
   30 ic = ica
c
c      this will re-establish the lower left hand corner as zero
c
      call plot(0.0,0.0,-3)
      msetun=iunit
      return
      end
