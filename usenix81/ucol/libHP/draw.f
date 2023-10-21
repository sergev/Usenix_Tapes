      subroutine draw(x,y)
	include 'gsdata.f'
c
c     this is used to draw a line (pen down) from the current
c     logical position to the passed parameter co-ordinate
c     position.
c
c
      call plot(x,y,2)
      return
      end
