      subroutine iplot(x,y,ipen)
	include 'gsdata.f'
c
c    this is used to for the agl incremental plotting
c    however, this translates the co-ordinates to absolute
c    co-ordinates.
c
      xtemp=x
      ytemp=y
      call change(xtemp,ytemp)
      xtemp1=0.0
      ytemp1=0.0
      call change(xtemp1,ytemp1)
      xtemp=xtemp-xtemp1
      ytemp=ytemp-ytemp1
      ntemp=minit1
      nsclip=msclip
      if ((msclip.eq.1).and.(msetun.eq.3).and.(minit1.eq.1)) msclip=4
      minit1=2
c
      theta=xrelan*.017453292519943
      xtemp1=xtemp*cos(theta)-ytemp*sin(theta)+xlastl
      ytemp1=xtemp*sin(theta)+ytemp*cos(theta)+ylastl
c
c
c
      call plot(xtemp1,ytemp1,ipen)
c
      minit1=ntemp
      msclip=nsclip
c
c
      return
      end
