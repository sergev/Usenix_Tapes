      subroutine number (xp, yp, hgt, fpn, theta, nd)
c
c
c  this subroutine converts a floating point number to
c  a character string and plots the string beginning at
c  the specified coordinate point
c
c     this version of number requires the symbol version with 999.0
c     x, y feature
c     logical nlog
      equivalence (nlog,nint)
c     dimension iarry(20),jarry(20)
      integer*2 iarry(20),jarry(20)
      integer*2 nint,i2o377
	include 'gsdata.f'
      i2o377=io377
      do 5 i=1,20
      iarry(i)=0
 5    continue
      x = xp
      y = yp
      h = hgt
      fpv = fpn
      th = theta
      n = nd
      maxn=7
      minus=45
      null=48
      idecpt=46
      jptr=1
c
c     set n value to + or - maxn, if out of range
c
      if (n - maxn) 11, 11, 10
   10 n = maxn
   11 if (n + maxn) 12, 20, 20
   12 n = -maxn
c
c     insert minus sign in front of number, if negative
c
   20 if (fpv) 21, 30, 30
   21 jarry(jptr)=minus
      jptr=jptr+1
c
c     mn locates exponent value for proper rounding of number
c
   30 mn = -n
c
c     if scaling is done, mn must be adjusted
c
      if (n) 31, 32, 32
   31 mn = mn - 1
c
c     round input number and set to positive value
c
   32 fpv = abs(fpv) + (0.5 * 10. ** mn)
c
c     determine characteristic of fpv and increment it by 1
c
      i = alog (fpv) * 0.43429448 + 1.0
      ilp = i
c
c     if scaling is done, ilp must be reduced according to scali
c
      if (n + 1)  40, 41, 41
   40 ilp = ilp + n + 1
c
c     if number is less than 1 plot a zero before decimal (if an
c
   41 if (ilp) 50, 50, 51
   50 jarry(jptr)=null
       jptr=jptr+1
      go to 61
c
c     ilp is number of digits to left of decimal point
c
   51 do 60 j = 1, ilp
c
c     locate single leftmost digit of number
c
      k = fpv * 10. ** (j - i)
      k1=k+48
      jarry(jptr)=k1
      jptr=jptr+1
c
c     subtract value of previous digit from number to locate nex
c
      fpv = fpv - (float(k) * 10. ** (i - j))
 60   continue
c
c     no decimal point is plotted if n is negative, exit from ro
c
   61 if (n) 99, 70, 70
   70 jarry(jptr)=idecpt
       jptr=jptr+1
c
c     plot digits to right of decimal if n gt 0, otherwise exit
c
      if (n)  99, 99, 80
   80 do 90 j = 1, n
c
c     scale fractional remainder to give integer digit
c
      k = fpv * 10.
      k1=k+48
      jarry(jptr)=k1
       jptr=jptr+1
c
c     subtract integer value to locate next digit
c
   90 fpv = fpv * 10. - float(k)
   99 jptr=jptr-1
      i=1
      j=1
 100  do 110 l=1,mnchar
c     jarry(j)=jarry(j)-(ifix((float(jarry(j))/(float(2**mcbits))))
c    &*(2**mcbits))
c     jarry(j)=jarry(j)*(2**(mcbits*(mnchar-l)))
c     nlog=or(iarry(i),jarry(j))
c     iarry(i)=nint
	nint=and(i2o377,jarry(j))
c	nint=ishft(nint,8*(2-l))
        nint=nint*2**(8*(2-l))
	iarry(i)=or(iarry(i),nint)
      if(j .eq. jptr) go to 120
      j=j+1
 110  continue
      i=i+1
      go to 100
120	kwd=(jptr+1)/2
	call swapby(iarry,kwd)
      call symbol(x,y,h,iarry,th,jptr)
      return
      end
