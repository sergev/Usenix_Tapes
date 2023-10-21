      subroutine clip(x1,x2,y1,y2)
	include 'gsdata.f'
c
c     this is used to set the user unit soft clipping
c     region.
c
      xtemp1=x1
      ytemp1=y1
c
c
      call change(xtemp1,ytemp1)
c
      xtemp2=x2
      ytemp2=y2
c
c
      call change(xtemp2,ytemp2)
c
c
      xwind1=amax1((amin1(xtemp1,xtemp2)),0.0)
      xwind2=amin1((amax1(xtemp1,xtemp2)),xmaxpu)
      ywind1=amax1((amin1(ytemp1,ytemp2)),0.0)
      ywind2=amin1((amax1(ytemp1,ytemp2)),ymaxpu)
c
c
      msclip=1
c
 800  return
      end
