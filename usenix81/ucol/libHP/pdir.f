      subroutine pdir(angle)
	include 'gsdata.f'
c
c    used to set the relative plotting direction for
c     incremental and relative plotting.
c
c
c     this routine is to set the relative plotting direction
c
c
c
      mloccm=97
c
c
 100  if (yactan.eq.angle) goto 700
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
 700  xrelan=angle
c
c
c     now set up the value for logical command variable
c
      yactan=angle
c
c
 800  return
      end
