      subroutine margin(xleft,xright,ylower,ytop)
	include 'gsdata.f'
c
c     sets the locate values in character cell units.
c
      if (xleft.lt.0.) xtemp1=(xleft*ycharh)*(-1.)+.333333*ycharh
      if (xleft.ge.0.0) xtemp1=(xleft*xchara)+.333333*xchara
c
      if (xright.lt.0.0) xtemp2=xmaxpu-(xright*ycharh)*(-1.0)-.3333333
     &*ycharh
      if (xright.ge.0.0) xtemp2=xmaxpu-(xright*xchara)-.33333*xchara
c
      if (ylower.lt.0.0) ytemp1=(ylower*ycharh)*(-1.0)
      if (ylower.ge.0.0) ytemp1=(ylower*xchara)+.333333*xchara
c
      if (ytop.lt.0.0) ytemp2=ymaxpu-(ytop*ycharh)*(-1.0)
      if (ytop.ge.0.0) ytemp2=ymaxpu-(ytop*xchara)-.33333*xchara
c
      ntemp=minit1
      minit1=2
      call locate(xtemp1,xtemp2,ytemp1,ytemp2)
      minit1=ntemp
      return
      end
