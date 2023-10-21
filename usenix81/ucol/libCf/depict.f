      subroutine depict(a,m,n,mdim,z,levels,iwd)
c$$$$$ calls only library routines  
c!!!!! altered for Fortran 77 by changing symbols to type
c      character and eliminating top-of-form.
c
c  produces shaded contour diagrams on the line-printer.
c                     argument list
c  a  is the matrix to be contoured
c  m,n  are the number of rows and columns used in  a.
c  mdim  is the actual dimension of the 1st index of  a  as it appears in the
c   calling program.
c  z  is an array of contour levels, increasing in value, which will be used
c  as the transition levels in the contour diagram.
c  levels  there are this number of elements in z.
c  iwd is 1 for a 132 column line printer and -1 for an 80 column terminal
c                        remarks
c  the symbol set may be altered by the user through common /symbol/, which
c  contains the array ksym each element of which is the character of a region.
c  the members of  ksym are used in order, the 1st for the smallest level etc.
c  the elements of the array  a appear in the diagram at the corners of squares
c  4 characters wide and 3 lines high - this is as nearly square as is possible
c  on a line-printer.   these dimensions may be altered by the user in
c  common /square/.    the intermediate characters are interpolated with bi-
c  linear interpolation.           If greater precision is required a
c  higher degree polynomial can be used, with a severe penalty on speed.
c  if the matrix contains too many columns to fit onto a page-width, further
c  pages are generated later to be placed at the side.  these separate units
c  are referred to as pages in the following comments. the word form is used for
c  a printer page.
c  if it is preferredto dictate the size of the diagram in inches, this is done
c by setting  xlen,ylen in /square/ to be the height and width in inches.  when
c  when xlen is zero the other mode, with so many characters per matrix element,
c  is assumed.
c  note there must be at least one more symbol set than there are contours.
c
      character*1 image(132),ksym
      dimension a(mdim,n),z(levels)
      common /symbol/ nsym,ksym(20)
      common/square/iwide,itall,xlen,ylen
c  this character set has alternate light and dark regions for best contrast.
      data ksym/1hA,1h ,1hB,1h.,1hC,1h,,1hD,1h/,1hE,1h-,1hF,1h(,1hG,1h),
     +  1hH,1h=,1hK,1h*,1hM,1h+/
      data iwide,itall,xlen,ylen/4,3,0.0,0.0/
     +,iout/6/
      if(iwd.gt.0) maxwid = 131
      if(iwd.le.0) maxwid = 79
c
c  this do-loop checks if contour levels are in increasing order.  if not, they
c  are re-ordered and a warning is printed
      iprint=0
      do 1000 l2=2,levels
      do 1000 l1=1,l2
      if (z(l1).le.z(l2)) go to 1000
      ztemp=z(l1)
      z(l1)=z(l2)
      z(l2)=ztemp
      iprint=1
 1000 continue
      if (iprint.eq.1) write(iout,101)
101   format(/10(6h *****)/
     +61h warning - contours not in order.  depict has re-ordered them )
c  kmax is the number of characters across the page, lmax the total number
c  of lines.  deltx,delty are the increments for 1 character and one line in
c  units of 1 row or column of the original matrix.
c  npage  is the number of pages generated (see remarks), and ipage is a page
c  counter.  if  xlen is nonzero it is assumed that  xlen,ylen  are the actual
c  height and width of the contour diagram in inches.   the number of lines and
c  columns is computed on the basis of 10 cols and 6 lines per inch.
      wide=iwide
      tall=itall
      if (xlen.eq.0.0) go to 1050
      wide=1.0 + (ylen*10.-1.0)/(n-1)
      tall=1.0 + (xlen*6.0-1.0)/(m-1)
 1050 kmax=(wide-1)*(n-1)+1.0
      lmax=(tall-1)*(m-1)+1.0
      deltx=1.0/(wide-1.0)
      delty=1.0/(tall-1.0)
c
      npage=1+(kmax-1)/maxwid
      last=1
      ipage=1
c
c  this do-loop steps the number of pages. kstart is the counter of the starting
c  printer-column for each page. the routine works in units of printer-cols and
c  printer-lines,  converting these to matrix indexes in x,y and interpolating
c  bilinearly.
      do 3000 kstart=1,kmax,maxwid
c
c  moves to top of form and suppresses spacing at form fold.  cdc3600 only.
c     print 100,1,63600 only
c  writes top of form character (usually 1)
c     write(iout,100) iunity
c100  format(i2)
c  no top of form character - space down instead
      write(iout,100)
 100  format(/////)
c
c  kend is the number of printer-cols on this page, xstart is the origin for  x
c  equivalent of the matrix column index in floating point.
      kend=min0(maxwid,kmax-kstart+1)
      xstart=1.0+deltx*(kstart-1)
      y=1.0
c
      do 2500 line=1,lmax
      x=xstart
      do 2000 kol=1,kend
c
c  interpolate value, then check if new value is above previous contour level.
c  2-dimension bi-linear interpolation  is used.
      i=max0(1,min0(m-1,int(y)))
      j=max0(1,min0(n-1,int(x)))
      xx=y-i
      yy=x-j
      zxy=  (a(i,j)*(1.0-xx)+a(i+1,j)*xx)*(1.0-yy)
      zxy = zxy + (a(i,j+1)*(1.0-xx)+a(i+1,j+1)*xx)*yy
      if (zxy.gt.z(last)) go to 1200
c
c  new value is below previous upper-bound of region, so scan downwards to loc
c -ate lower boundary.
      level=last
      if (last.eq.1) go to 1500
      do 1100 kount=2,last
      if (z(level-1).lt.zxy) go to 1500
1100  level=level-1
      go to 1500
c
c  new value is above previous upper-boundary, so scan upwards to find new
c  upper-boundary.
1200  do 1300 level=last,levels
      if (z(level).ge.zxy) go to 1500
1300  continue
      level=levels+1
      last=levels
      go to 1600
c
c  reset last level indicator, set symbol in the line-image and advance x.
1500  last=level
1600  image(kol)=ksym(level)
2000  x=deltx+x
c
c  print line image generated in last do loop, then advance y.
      write(iout,250) (image(kol),kol=1,kend)
 250  format(132a1)
c250  format(1x,132a1)      --for use when first character does carriage control
c250  format(1h6,132a1)     -- ucsd b6700,cdc3600 only
2500  y=delty+y
c
c  if more than 1 page, print page number.
      if (kmax.gt.maxwid) write(iout,290) ipage,npage
290   format(//10x,7h...page ,i2,3h of ,i2)
c
c  if this is the 1st page print a key to shading.
      if (ipage.gt.1) go to 3000
      write(iout,300) (z(level),ksym(level),level=1,levels)
      write(iout,301) z(levels),ksym(levels+1)
300   format(/14hkey to symbols//(5hbelow,g14.5,15h shown by ...  ,a1))
301   format(5habove,g14.5,15h shown by ...  ,a1)
c  formats below for use when first character does carriage control
c300  format(15h0key to symbols//(6h below,g14.5,15h shown by ...  ,a1))
c301  format(6h above,g14.5,15h shown by ...  ,a1)
c
c  advance page counter.
3000  ipage=1+ipage
c
c  resets spacing over form fold, before returning.
c     print 100,5      cdc3600
      return
      end
      subroutine assess(a,m,n,mdim,z,nlev)
c$$$$$ calls only library routines
c  finds contour levels for arrays at nice intervals.  attempts to avoid
c  being fooled by spikes.
c  a  is an array with  m  rows,  n  columns, but is dimensioned  mdim
c  in the calling program (mdim.ge.m).  z  is an array to hold the contour
c  levels found by routine.  nlev is the approximate number desired -
c  the value is approximate because, to make the levels nice numbers, it
c  may be necessary to have fewer than nlev  in range.
      dimension a(mdim,n),z(nlev),divs(7)
      data divs /.05,.1,.2,.25,.5,1.0,2.0/, fac/2.5/
      iway=1
      b1=0.0
      b2=0.0
 1000 suma=0.0
      sumaa=0.0
      a1=a(1,1)
      a2=a1
      en=0.0
      do 1500 j=1,n
      do 1500 i=1,m
      aij=a(i,j)
      if (aij.gt.b2 .or. aij.lt.b1) go to (1300,1500),iway
 1300 suma=aij + suma
      sumaa=aij*aij + sumaa
      en= 1.0 +en
      a1=amin1(a1,aij)
      a2=amax1(a2,aij)
 1500 continue
      b1=suma/en - fac*sqrt(sumaa/en-(suma/en)**2)
      b2=2.0*suma/en - b1
      if (iway.eq.1) write(6,150) a1,a2
 150  format(/34hminimum,maximum elements in array  ,2g15.6)
c  format for use when first character in line does carriage control.
c150  format(35h0minimum,maximum elements in array  ,2g15.6)
      if (b1.le.a1 .and.b2.ge.a2) go to 2000
      iway=1 + iway
      go to (1000,1000,2000),iway
c  finds nice round numbers bracketing range of interest
 2000 amin=amax1(a1,b1)
      amax=amin1(a2,b2)
      if (amin.eq.amax) amax=2.0 + amax
      plus=1000.0 + alog10(amax-amin)
      da=10.0**amod(plus,1.0)
      do 2100 i=1,7
      ii=i
      if (nlev .ge. da/divs(ii)) go to 2110
 2100 continue
 2110 units=divs(ii)*10.0**(int(plus)-1000)
      bias=units*aint(1.0 + amax1(abs(amin),abs(amax))/units)
      amin=amin - amod(amin+bias,units)
c  generates levels, insuring the presence of a zero-level if necessary
      do 2200 i=1,nlev
      z(i)=amin + (i-1)*units
      if (abs(z(i)) .lt. 0.1*units) z(i)=0.0
 2200 continue
      return
      end
