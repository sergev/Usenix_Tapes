      subroutine rplot(x,y,ipen)
	include 'gsdata.f'
c
c     relative plotting with pen control
c
      xtemp=x
      ytemp=y
      call change(xtemp,ytemp)
      xnext=0.0
      ynext=0.0
      call change(xnext,ynext)
      xtemp=xtemp-xnext
      ytemp=ytemp-ynext
      xnext=xrelpu
      ynext=yrelpu
      ntemp=minit1
      minit1=2
c
c     check to see if soft clip should be invoked
c
      nsclip=msclip
      if ((msetun.eq.3).and.(msclip.eq.1).and.(minit1.eq.1)) msclip=4
c
      theta=xrelan*.017453292519943
      xtemp1=xtemp*cos(theta)-ytemp*sin(theta)+xrelpu
      ytemp1=xtemp*sin(theta)+ytemp*cos(theta)+yrelpu
c
c
      call plot(xtemp1,ytemp1,ipen)
      msclip=nsclip
      minit1=ntemp
      xrelpu=xnext
      yrelpu=ynext
      return
      end
