      subroutine symbol(x,y,size,iarry,angl,ictl)
c
c	**********  modified  **********
c
c	this version expects characters in strictly ascending
c	byte order when ictl>0.  this modification eases use
c	by pdp fortran routines.  formats for the other two
c	cases are unchanged.
c
c     symbol drawing subroutine for hp7221 graphic plotter
c
c
c     calling parameter definition:
c
c       x,y    starting location of character(s) to be drawn
c              expressed in floating point inches from origin.
c
c       size   height of character in floating point inches.
c              width will be approximately 0.5 times the height.
c
c       iarry  integer array containing ascii characters to be
c              drawn, or a single integer specifying the special
c              symbol to be drawn.
c
c       angl   angle of character baseline in floating point
c              degrees from horizontal.  positive angles rotate
c              the baseline counter-clockwise, negative angles
c              rotate clockwise.
c
c       ictl   integer specifying the number of characters to
c              to be drawn:
c
c                ictl > 0  ictl equals the number of characters
c                          in array 'iarry' to be drawn.
c
c
c                ictl = 0  draw only the single character
c                          contained in the lower byte of
c                          array 'iarry'.
c
c                ictl < 0  draw the special symbol represented
c                          by the integer in iarry(1).  if ictl
c                          equals -1, move to the starting location
c                          with the pen up.  if ictl is less than
c                          -1, move to the starting location
c                          with the pen down.
c
c
c     1.     the symbol routine is used to write characters on the
c            paper if ictl=+1.  if ictl =-1 or -2, the
c            special character represented by the decimal number
c            will be drawn.  if the pen is left down, moving the
c            pen to the place where the next character is to be
c            drawn will cause the special characters to be connected
c            like points along a graph.  if the pen is left up, the
c            points will be drawn without intervening connecting
c            lines.  when drawing graphs, it is suggested that
c            symbols that are connected be used.
c
c     2.     the coordinates specified by the user refer to the
c            lower left corner of an imaginary box enclosing the
c            character to be drawn.  if you wish to again call symbol
c            and have the set of ascii characters be continuous,
c            place the value 999.0 as the x and y coordinate.
c
c
c     3.     the x and y coordinates from one subroutine are not
c            transmitted to the other subroutines.  a transition
c            between plot and symbol or other subroutines can be made
c            quite easily by using where to supply the current
c            pen location.
c
c
c     4.     if a call has been made to lorg to invoke label
c            origin mode ( modes 1 - 9 ), characters will be
c            drawn based upon the mode specified.
c
c     integer iarry(1)
      character iarry(*),itemp
      logical ispec,icr
      dimension iwd(10)
	include 'gsdata.f'
      mnchrx=1
      do 10 i=1,10
      iwd(i)=0
 10   continue
      ifont=mcaltf
      ispec=.false.
      icr=.false.
      call cdir(angl)
      height=size
      call csizea(height,xratio,xchars)
c
c     special symbol ?
c
 20   if(ictl .lt. 0) go to 80
c
c     **  standard ascii character processing **
c
c     test for units
c
      if ((msetun.ne.1).or.(minit1.gt.1)) goto 25
c
c     continuous mode ?
c
      if((x .ge. 999.0) .and. (y .ge. 999.0)) go to 30
c
c     move to x,y with pen up
c
 25    ipen=13
      call plot(x,y,ipen)
c
c     single character ?
c
 30   if(ictl .eq. 0) go to 35
      inum=ictl
      go to 40
c
c     move character in right byte to left byte
c
 35   itemp=iarry(1)
c
c	next three lines are bypassed in this version.
c
c     iarry(1)=iarry(1)-(ifix((float(iarry(1))/(float(2**mcbits))))
c    &*(2**mcbits))
c     iarry(1)=iarry(1)*(2**(mcbits*(mnchar-1)))
      inum=1
c
c     setup for host buffer insertion
c
 40   nchars=inum
c
c     special symbol ?
c
      if(ispec) go to 130
c
c     label origin mode processing
c
      xmove=.334
      ymove=0.0
      itotal=nchars
c
c     scan for control characters
c
 42   i=1
      jchars=0
 200  idata=iarry(i)
c
c	do indexing has been reversed in this version.
c
c     do 205 l = 1,mnchar
      do 205 l = 1,mnchrx
      iwd(l)=idata-(idata/(2**mcbits))*(2**mcbits)
      idata=idata/(2**mcbits)
 205  continue
c210  do 220 j=1,mnchar
 210  do 220 j=1,mnchrx
      jchars=jchars+1
      if(jchars .gt. inum) go to 240
      if(iwd(j) .lt. 32) itotal=itotal-1
c
c     check for cr character ( lorg 4,5,6 usage )
c
      if(iwd(j) .eq. 13) icr=.true.
 220  continue
      i=i+1
      go to 200
c
c     lorg mode adjustments for x and y
c
 240  if(mlbpos.eq.1.or.mlbpos.eq.4.or.mlbpos.eq.7)ymove=.250
      if(mlbpos.eq.2.or.mlbpos.eq.5.or.mlbpos.eq.8)ymove=-.25
      if(mlbpos.eq.3.or.mlbpos.eq.6.or.mlbpos.eq.9)ymove=-.750
      if(mlbpos .lt. 4) go to 260
      if(mlbpos .gt. 6) go to 245
      xmove=xmove-float(itotal)/2.0
      xmove=xmove+(-.167)
      go to 260
 245  xmove=xmove-itotal
      xmove=xmove+(-.334)
 260  call cplot(xmove,ymove)
c
c     invoke label mode
c
 130  call datout(nchars+2)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=39
      mcount=mcount+1
c
c     special symbol ?
c
      if(.not. ispec) go to 45
c
c     insert shift out sequence in host buffer
c
      moutar(mcount)=14
      mcount=mcount+1
 45   i=1
      ichars=0
c
c     special symbol ?
c
      if(ispec) go to 55
c
c     extract characters
c
 50   idata=iarry(i)
c
c	the order of the following do loop has been reversed
c	for compatibility with the pdp-11 packing order.
c
c     do 52 k = mnchar,1,-1
c     iwd(k)=idata-idata/(2**mcbits)*(2**mcbits)
c     idata=idata/(2**mcbits)
c52   continue
c
c	do 52 k=1,mnchar
        do 52 k=1,mnchrx
	iwd(k)=idata-(idata/(2**mcbits))*(2**mcbits)
	idata=idata/(2**mcbits)
52	continue
c55   do 60 j = 1,mnchar
 55   do 60 j = 1,mnchrx
      ichars=ichars+1
      if(ichars .gt. inum) go to 72
      moutar(mcount)=iwd(j)
      mcount=mcount+1
 60   continue
      i=i+1
      go to 50
 72   if(.not. ispec) go to 73
c
c     insert shift in sequence in host buffer
c
      moutar(mcount)=15
      mcount=mcount+1
c
c     insert cr character if lorg 4,5 or 6
c
 73   if(ispec) go to 75
      if(mlbpos.lt.4.or.mlbpos.gt.6) go to 75
      if(icr) go to 75
      icr=.true.
      moutar(mcount)=13
      mcount=mcount+1
c
c     turn off label mode
c
 75   call laboff
      call cfont(mcstdf,ifont)
c
c     undo the lorg mode adjustments for x and y
c
      if(ispec) go to 78
      if(icr) go to 78
      xmove1=0.0
      if(mlbpos .lt. 4) xmove1=-.334
      ymove1=-ymove
      call cplot(xmove1,ymove1)
c
c     move character back to right byte
c
 78   if(ictl .eq. 0) iarry(1)=itemp
c
c     this flag is used to signify a lost mode
c
c     if (iarry(1) .lt. 15) go to 79
      iiarry=iarry(1)
      if (iiarry.lt.15) goto 79
      if (mlbpos .lt. 4)  mplost=1
 79   return
c
c     **  special symbol processing **
c
c     check for special font designation
c
 80   call cfont(mcstdf,5)
c
c     continuous mode ?
c
c
c     test for units
c
      if ((msetun.ne.1).or.(minit1.gt.1)) goto 95
      if((x .ge. 999.0) .and. (y .ge. 999.0)) go to 100
 95   ipen=13
      if(ictl .lt. -1) ipen=2
c
c     move to x,y with pen up or down
c
      call plot(x,y,ipen)
c
c     setup to utilize standard character logic
c
 100  inum=2
      ispec=.true.
c
c     uncentered symbol ?
c
c     if(iarry(1) .gt. 14) iwd(1)=iwd(1)+17
c     iwd(1)=iwd(1)+iarry(1)+65
      iiarry=iarry(3)
      if(iiarry.gt.14) iwd(1)=iwd(1)+17
      iwd(1)=iwd(1)+iiarry+65
      go to 40
      end
