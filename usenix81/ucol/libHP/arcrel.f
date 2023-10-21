      subroutine arcrel(x,y,radius,angst,angen)
	include 'gsdata.f'
c
c     this is used to draw arcs and circles in ccw or cw
c      direction.
c
      call plot(x,y,10)
c
      tangle=xrelan
      call pdir(0.0)
c
      call datout(1)
      moutar(mcount)=116
      if (angen.ge.0) moutar(mcount)=117
      mcount=mcount+1
      xtemp=radius
      ytemp=0.0
      call delta(xtemp,ytemp)
      if (xtemp.lt.1.0) xtemp=1.0
      if (xtemp.gt.32767.0) xtemp=32767.0
      call agp(xtemp)
      call ang(angst)
      ytemp=angst+angen
      call ang(ytemp)
      mloccm=116
c
c     this is used to determine the logical pen position
c     at the completetion of the arc
c
c
      tang=angst*.01745329252
      xcent=xlastl-xtemp*cos(tang)
      ycent=ylastl-xtemp*sin(tang)
c
c     found the center of the arc
c
      tang=ytemp*.01745329252
      xlastl=xcent+xtemp*cos(tang)
      ylastl=ycent+xtemp*sin(tang)
c
c     found the ending co-ordinate
c
      xlasta=xlastl
      ylasta=ylastl
c
      xrelpu=xlasta
      yrelpu=ylasta
c
      call pdir(tangle)
c     finished
c
      return
      end
