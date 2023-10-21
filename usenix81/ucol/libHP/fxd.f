      subroutine fxd(numb)
	include 'gsdata.f'
c
c     this is used to specify the number of digit to the right
c     of the decimal is desired for label gridding and axes
c     generation.
c
c
      if (numb.gt.7) numb=7
      mfxdno=numb
      return
      end
