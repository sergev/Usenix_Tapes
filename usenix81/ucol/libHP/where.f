      subroutine where(x,y,fact)
	include 'gsdata.f'
c
c     where is used to tell the user where the current pen
c     location is
c     and what the current factor is.
c
      x=xlastl
      y=ylastl
      call putoun(x,y)
      fact=xfacts
      return
      end
