      subroutine change(xcurrl,ycurrl)
	include 'gsdata.f'
c
c     this is used to convert the current unit
c     system values to plotter units.
c
      if (minit1.eq.2) goto 800
      if (msetun.eq.1) call intopu(xcurrl,ycurrl)
      if (msetun.eq.2) call gutopu(xcurrl,ycurrl)
      if (msetun.eq.3) call uutopu(xcurrl,ycurrl)
 800  return
      end
