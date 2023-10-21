      subroutine move(x,y)
	include 'gsdata.f'
c
c      absolute pen up move from current location.
c
      call plot(x,y,13)
      return
      end
