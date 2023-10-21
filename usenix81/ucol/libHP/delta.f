      subroutine delta(xvalue,yvalue)
	include 'gsdata.f'
c
c    this is used to convert the current unit system
c    to the absolute plotter units from the current units
c    systems origin (zero,zero)
c
c
      if (minit1.eq.2) goto 800
      xtemp=0.0
      ytemp=0.0
c
      if (msetun.ne.1) goto 200
c
      call intopu(xvalue,yvalue)
      call intopu(xtemp,ytemp)
      goto 700
c
c
 200  if (msetun.ne.2) goto 300
c
      call gutopu(xvalue,yvalue)
      call gutopu(xtemp,ytemp)
      goto 700
c
c
 300  if (msetun.ne.3) goto 700
c
      call uutopu(xvalue,yvalue)
      call uutopu(xtemp,ytemp)
c
c
 700  xvalue=abs(xvalue-xtemp)
      yvalue=abs(yvalue-ytemp)
c
c
c
 800  return
      end
