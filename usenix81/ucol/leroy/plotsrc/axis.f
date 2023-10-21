      subroutine axis(xdim,ydim,xmarg,ymarg,xlow,ylow,xmax,xmin,
     1  ymax,ymin,dxtics,dytics,fmtx,fmty,labelx,labely,title,
     2  iclear)
c
c    routine axis will draw a box with tic marks, label the x and y axes
c    and label the plot with a title.
c
c    inputs  - xdim   = x dimension of the box in inches
c              ydim   = y dimension of the box in inches
c              xmarg  = margin below the x-axis in inches - determines
c			the vertical spacing of the x-axis label
c              ymarg  = margin to the left of the y-axis in inches - determines
c			the horizontal spacing of the y-axis label
c              xlow   = x-location of the lower left hand corner of the box
c			in inches from the lower left hand corner of the
c			plot
c              ylow   = y-location etc.
c              xmax   = x value of the right edge of the box in user units
c              xmin   = x value of the left edge of the box in user units
c              ymax   = y value of the top of the box in user units
c              ymin   = y value of the bottom of the box in user units
c              dxtics = increment between tic marks along the
c			x-axis
c	       nytics = etc. for the y-axis
c              fmtx   = character string format specification including 
c			paranthesis which determines the x-axis numerical
c			labeling format
c              fmty   = etc. for the y-axis
c              labelx = character string label for the x-axis
c              labely = etc. for y-axis
c              title  = character string title which is placed on top
c			of the plot
c              iclear = clear flag for tektronix (if .ne. 0 then clear)
c    note: the UNIX plot file descriptor must be in common block /pfile/
c
      character*(*) labelx,labely,title,fmtx,fmty
c
c    clear if appropriate
c
      if (iclear .ne. 0) call clear
c
c    box first
c
      x = xdim + .03
      y = ydim + .03
      xl = xlow - .015
      yl = ylow - .015
      call setdim(x,y,xl,yl)
      call setscl(0.,1.,0.,1.)
      call box(0.,1.,0.,1.,.03,1)
c
c    title next
c
      call cfont(15)
      call setscl(0.,1.,0.,y)
      call chrsiz(.2,1.,0.)
      y2 = y + .2
      call text(.5,y2,0.,3,title,1)
      call chrsiz(.14,1.,0.)
c
c    x-axis
c
      call setdim(xdim,ydim,xlow,ylow)
      call setscl(xmin,xmax,0.,ydim)
      if (dxtics .le. 0.)  go to 100
      xmx = xmax
      xmn = xmin
      if (xmax .lt. xmin) xmx = xmin
      if (xmax .lt. xmin) xmn = xmax
      xx = (xmn/dxtics) + 1.0001
      nmin = xx
      xx = (xmx/dxtics) - .0001
      nmax = xx
      ntics = nmax - nmin + 1
      if (ntics .le. 0)  go to 90
      stic = nmin*dxtics - xmn
      iref = 1
      if (xmax .lt. xmin) iref = -1
      call tics(xmin,0.,xmax,0.,stic,ntics,dxtics,.15,.022,iref)
      iref = -iref
      call tics(xmin,ydim,xmax,ydim,stic,ntics,dxtics,
     1  .15,.022,iref)
      if (abs(stic-dxtics)/dxtics .lt. .00001) nmin = nmin - 1
      if (abs(xmx-nmax*dxtics-dxtics)/dxtics .lt. .00001)
     1  nmax = nmax + 1
      do 50  i = nmin,nmax
      x = dxtics*i
      call number(x,-.1,0.,5,x,fmtx)
   50 continue
      go to 100
   90 call number(xmin,-.1,0.,5,xmin,fmtx)
      call number(xmax,-.1,0.,5,xmax,fmtx)
      ix = xmax
  100 xx = -xmarg
      yy = (xmin+xmax)/2.
      call text(yy,xx,0.,3,labelx,1)
c
c    y-axis
c
      call setscl(0.,xdim,ymin,ymax)
      if (dytics .le. 0.)  go to 200
      xmx = ymax
      xmn = ymin
      if (ymax .lt. ymin) xmx = ymin
      if (ymax .lt. ymin) xmn = ymax
      xx = (xmn/dytics) + 1.0001
      nmin = xx
      xx = (xmx/dytics) - .0001
      nmax = xx
      ntics = nmax - nmin + 1
      if (ntics .le. 0)  go to 290
      stic = nmin*dytics - xmn
      iref = -1
      if (ymax .lt. ymin) iref = 1
      call tics(0.,ymin,0.,ymax,stic,ntics,dytics,.15,.022,iref)
      iref = -iref
      call tics(xdim,ymin,xdim,ymax,stic,ntics,dytics,
     1  .15,.022,iref)
      if (abs(stic-dytics)/dytics .lt. .00001) nmin = nmin - 1
      if (abs(xmx-nmax*dytics-dytics)/dytics .lt. .00001)
     1  nmax = nmax + 1
      do 250  i = nmin,nmax
      x = dytics*i
      call number(-.1,x,0.,7,x,fmty)
  250 continue
      go to 200
  290 call number(ymin,-.1,0.,5,ymin,fmty)
      call number(ymax,-.1,0.,5,ymax,fmty)
  200 xx = -ymarg
      yy = (ymin+ymax)/2.
      call text(xx,yy,90.,5,labely,1)
c
      return
      end
      subroutine number(x,y,ang,iref,xnum,fmt)
c
c    routine number causes a number to be printed on a plot
c
      character*(*) fmt
      character*20 numb(2)
c
      ix = xnum
      if (fmt(2:2) .eq. 'i') write (numb(1),fmt) ix
      if (fmt(2:2) .ne. 'i') write (numb(1),fmt) xnum
      call text(x,y,ang,iref,numb(1),1)
c
      return
      end
