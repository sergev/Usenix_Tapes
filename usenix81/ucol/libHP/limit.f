      subroutine limit(x1,x2,y1,y2)
	include 'gsdata.f'
c
c     this is used to set the graphic limits from software
c
      call datout(3)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=87
      mcount=mcount+1
c
c     set graph limits
c
      mloccm=95
c
c
      xtemp1=x1*1016.000121593483
      ytemp1=y1*1016.000121593483
c
c    check the boundary conditions
c
      if (xtemp1.lt.0.0) xtemp1=0.0
      if (ytemp1.lt.0.0) ytemp1=0.0
      if (xtemp1.gt.16000.0) xtemp1=16000.0
      if (ytemp1.gt.11400.0) ytemp1=11400.0
c
c
      call axy(xtemp1,ytemp1)
c
c
      xtemp2=x2*1016.000121593483
      ytemp2=y2*1016.000121593483
c
      if (xtemp2.lt.0.0) xtemp2=0.0
      if (ytemp2.lt.0.0) ytemp2=0.0
      if (xtemp2.gt.16000.0) xtemp2=16000.0
      if (ytemp2.gt.11400.0) ytemp2=11400.0
c
      if (xtemp1.eq.xtemp2) xtemp2=xtemp2+1.0
      if (ytemp1.eq.ytemp2) ytemp2=ytemp2+1.0
c
c      check for over flow
c
      if (xtemp2.gt.16000.0) xtemp2=15999.0
      if (ytemp2.gt.11400.0) ytemp2=11399.0
c
c
c
      call axy(xtemp2,ytemp2)
c
c
c     this is needed to perform this command
c
c
      call datout(1)
      moutar(mcount)=125
      mcount=mcount+1
c
      call hshake
c
      call putomu(xlasta,ylasta)
c
      xminmu=aint(xtemp1+.5)
      yminmu=aint(ytemp1+.5)
      xmaxmu=aint(xtemp2+.5)
      ymaxmu=aint(ytemp2+.5)
c
      call plots(19348,mdevin,mdevot)
      return
      end
