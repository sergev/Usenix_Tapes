      subroutine cplot(x,y)
	include 'gsdata.f'
c
c     this is used for positioning the pen in character
c     cell units.
c
      call cdir(ylaban)
 50   mloccm=98
      xtemp=x*xchara
      ytemp=y*ycharh
      ntemp=minit1
      minit1=2
      call iplot1(xtemp,ytemp,3)
      minit1=ntemp
      return
      end
