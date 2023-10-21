      subroutine initt(itran, name)
c
c    routine initt is an initialization routine which MUST BE CALLED prior
c    to any of the other plot calls.  It need be called only once to
c    properly initialize the plot file.
c
c    inputs  - itran  = flag which indicates which mode for plotting
c			= 0 - normal mode
c			.ne. 0 - transposed mode
c
c	NB	some changes from Harvey's original
c
      save
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,iitran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pfile /ipunit
c
      character*1 s
	character*40  name
c
      common /spc/ xll,yll,xur,yur
      integer*2 xll,yll,xur,yur
c
      data  s / 's' /
      data  xll,yll,xur,yur / 0,0,12288,12288 /
c
      iitran = itran
      tangle = 0.
      if (itran .ne. 0) tangle = 90.

c	patch on danny's code to avoid opening file

	call ropen( name, 1, ipunit )

      call rwrite(ipunit, 1, s)
      call rwrite(ipunit, 8, xll)
      call xlimit(0.,11.,0.,8.5,.5)
      call pspeed(36)
      call chrdir(0.)
      call setdim(7.,7.,.5,.5)
      call setscl(0.,7.,0.,7.)
      call chrsiz(.2,.667,0.)
c
      return
      end
c
c
