      subroutine circle(xc,yc,r,narc,iclose,iclip,thick,xbuf,ybuf,nsize)
c
c    routine circle will draw either a open or closed circle (open meaning
c    a circle with a white center, closed a circle with a black center)
c
c    inputs  - xc     = x-coordinate of the center of the circle in user units
c	       yc     = y-coordinate of the center of the circle in user units
c              r      = radius of the circle in user units
c			note:  If the user units are not isotropic with respect
c			       to the physical plot map, then you will get
c			       open and closed ellipses (whoopee!)
c              narc   = number of straight line segments to approximate the
c			circle with
c              iclose = open-closed flag
c			= 0 - circle open
c			.ne. 0 - circle closed
c              iclip  = clip flag see nplot
c              thick  = line thickness see nplot
c			note: this is passed to nplot along with iclip.
c 			      This allows big open circles with thick
c			      circumferences. Probably should not be used
c			      for closed circles.
c
      save
      dimension xbuf(nsize),ybuf(nsize)
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      data  pi  / 3.24159265 /
c
c    draw open circle
c
      if (narc .ge. nsize) narc = nsize-1
      npl = narc + 1
      domega = 2.*pi/narc
      do 10  i = 1,npl
      omega = domega*(i-1)
      xbuf(i) = xc + r*cos(omega)
   10 ybuf(i) = yc + r*sin(omega)
      call nplot(npl,xbuf,ybuf,0,iclip,thick,' ')
c
c    close circle if necessary
c
      if (iclose .eq. 0)  go to 900
      dr = r/sqrt(2.)
      xbuf(1) = xc - dr
      xbuf(2) = xc + dr
      ybuf(1) = yc
      ybuf(2) = yc
      rth = 2.*dr*yscale/1638.4
      call nplot(2,xbuf,ybuf,0,iclip,rth,' ')
      dr = r - dr
      rr = r - dr/2.
      rth = dr*xscale/1638.4
      do 20  i = 1,npl
      omega = domega*(i-1)
      xbuf(i) = xc + rr*cos(omega)
  20  ybuf(i) = yc + rr*sin(omega)
      call nplot(npl,xbuf,ybuf,0,iclip,rth,' ')
c
  900 return
      end
