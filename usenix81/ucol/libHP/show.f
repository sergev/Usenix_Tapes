      subroutine show(x1,x2,y1,y2)
	include 'gsdata.f'
c
c
c     define a "region of interest" in the user's
c     co-ordinate system.  (makes square user units)
c
      xtemp1=x1
      xtemp2=x2
      ytemp1=y1
      ytemp2=y2
c
      if (xtemp1.eq.xtemp2) xtemp2=xtemp2+1.0
      if (ytemp1.eq.ytemp2) ytemp2=ytemp2+1.0
c
      xran=xtemp2-xtemp1
      yran=ytemp2-ytemp1
      xranl=xmapt2-xmapt1
      yranl=ymapt2-ymapt1
      temp1=yranl/xranl-yran/xran
      if (temp1.lt.0.0) goto 200
      ytemp1=ytemp1-xran*temp1/2.0
      ytemp2=ytemp2+xran*temp1/2.0
      goto 300
 200  temp1=xranl/yranl-xran/yran
      xtemp1=xtemp1-yran*temp1/2.0
      xtemp2=xtemp2+yran*temp1/2.0
 300  call mapuu(xtemp1,xtemp2,ytemp1,ytemp2)
      return
      end
