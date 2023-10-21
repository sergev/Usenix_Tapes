      subroutine iplot1(x,y,ipen)
c
c      this uses the plotter incremental moves.  used in cplot
c      and in  plot with ipen > 10 to logical send the pen
c      to the endpoint in the plotter.
c
      dimension arxy(4,2)
	include 'gsdata.f'
c
c     check to see if the routine came from the
c     cplot subroutine if so do not reconvert the angle
c
c     or update the relative location
c
      if (mloccm.eq.98) call cdir(ylaban)
      xtemp1=0.0
      ytemp1=0.0
      xtemp=x
      ytemp=y
      call change(xtemp,ytemp)
      call change(xtemp1,ytemp1)
      xtemp=xtemp-xtemp1
      ytemp=ytemp-ytemp1
      arxy(1,1)=0.0
      arxy(1,2)=0.0
      arxy(2,1)=0.0
      arxy(2,2)=0.0
      arxy(3,1)=-16384.0
      arxy(3,2)=-16384.0
      arxy(4,1)= 16383.0
      arxy(4,2)= 16383.0
      xtemp1=xtemp
      ytemp1=ytemp
c
c
 100  arxy(2,1)=xtemp1-arxy(2,1)
      arxy(2,2)=ytemp1-arxy(2,2)
      xtemp1=arxy(2,1)
      ytemp1=arxy(2,2)
      call cliper(arxy,nflag1)
      jpen=iabs(ipen)
      if (jpen.ge.10) jpen=jpen-jpen/10*10
      if ((jpen.eq.1).or.(jpen.eq.3)) goto 135
      if ((jpen.eq.2)) goto 131
      if (mpenpo.eq.1) goto 135
 131  call idraw
      goto 140
 135  call imove
 140  call ixy(arxy(2,1),arxy(2,2))
      if (nflag1.ne.0) goto 100
c
c
c     need to change the theta values
c          from degrees to  radians  so the system may
c          understand
c            1 degree = .017453292519943 radians
c
c
c
      theta=yactan*.017453292519943
c
      xlasta=xtemp*cos(theta)-ytemp*sin(theta)+xlasta
      ylasta=xtemp*sin(theta)+ytemp*cos(theta)+ylasta
      xlastl=xlasta
      ylastl=ylasta
c
c
      if (jpen) 180,800,180
 180  if ((jpen.eq.1).or.(jpen.eq.4)) call idraw
      if (jpen.eq.5) call imove
 800  return
      end
