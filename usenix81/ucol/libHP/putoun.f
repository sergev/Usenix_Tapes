      subroutine putoun(x,y)
	include 'gsdata.f'
c
c
c     will convert the co-ordinate pair to the current unit system
c     from plotter units.
c
c
      if (minit1.eq.2) goto 800
      if (msetun.eq.1) call putoin(x,y)
      if (msetun.eq.2) call putogu(x,y)
      if (msetun.eq.3) call putouu(x,y)
 800  return
      end
