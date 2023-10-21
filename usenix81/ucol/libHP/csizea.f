      subroutine csizea(height,ratio,slant)
	include 'gsdata.f'
c
c    this is to convert the actual character size to the
c    character size spacing.
c
c      height,  ratio,   slant
c
      tratio=ratio
c
c
c     first the height and width of the character cell
c
c     convert the height to the new height
c
      ycha=2.0*height
c
c     convert the ratio to the proper width
c
      xcha=height*ratio*1.5
c
c     now convert it to the ratio for character spacing.
c
      ratio1=xcha/ycha
c
c     send it to the csize routine to be converted and stored.
c
      call csize(ycha,ratio1,slant)
c
c
      xratio=tratio
      return
      end
