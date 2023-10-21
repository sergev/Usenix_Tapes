      subroutine cdir(angle)
	include 'gsdata.f'
c
c     this routine is to set the labeling direction.
c
      mloccm=94
 100  if (yactan.eq.angle)  goto 700
c
c     check the dataout
c
c
 200  call datout(2)
      moutar(mcount)=119
      mcount=mcount+1
c
c     the above reset the relative rotate direction to 0
c
c     below will set the rotate in an absolute mode
c     since the rotate original will be a zero degrees.
c
      moutar(mcount)=119
      mcount=mcount+1
c
c     now convert the ang to the array
c
      call ang(angle)
c
c     store the angle away
c
c
c
c     now set up the value for logical command variable
c
 700  yactan=angle
      ylaban=angle
c
c
 800  return
      end
