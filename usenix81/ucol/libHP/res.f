      subroutine res(x,y)
	include 'gsdata.f'
c
c      this routine will take the addresablity and
c      providde the clostest number to the upper
c      right so the gird spacing approximates the
c      the desired resolution.  however, may not be
c      exact.
c
      if (x.lt.1.0) x=1.0
      if (x.gt.5000.0) x=5000.0
      if (y.lt.1.0) y=1.0
      if (y.gt.5000.0) y=5000.0
c
      ntemp=minit1
      minit1=2
c
c
      xfact=xrespu/x
      yfact=yrespu/y
c
      xlastl=xlastl*xfact
      ylastl=ylastl*yfact
c
      xlasta=xlasta*xfact
      ylasta=ylasta*yfact
c
      xoffpu=xoffpu*xfact
      yoffpu=yoffpu*yfact
c
      xmapl1=xmapl1*xfact
      ymapl1=ymapl1*yfact
      xmapl2=xmapl2*xfact
      ymapl2=ymapl2*yfact
c
      xmapt1=xmapt1*xfact
      ymapt1=ymapt1*yfact
      xmapt2=xmapt2*xfact
      ymapt2=ymapt2*yfact
c
c
      xwind1=xwind1*xfact
      ywind1=ywind1*yfact
      xwind2=xwind2*xfact
      ywind2=ywind2*yfact
c
      xrelpu=xrelpu*xfact
      yrelpu=yrelpu*yfact
c
      xmaxpu=xmaxin/x*1000.0
      ymaxpu=ymaxin/y*1000.0
      if (xmaxpu.lt.1.0) xmaxpu=1.0
      if (ymaxpu.lt.1.0) ymaxpu=1.0
c
c
      tratio=xratio
      tcharh=ycharh*yfact
      tchara=xchara*xfact
      call csize(tcharh,tchara/tcharh,xchars)
      xratio=tratio
c
c
      tratio=xdllen*xfact
      call daslna(mlntyp,tratio)
c
c
      call datout(2)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=83
      mcount=mcount+1
      mloccm=96
c
      call axy(xmaxpu,ymaxpu)
c
c
c
c     reset the values
c
      xrespu=x
      yrespu=y
c
c
      minit1=ntemp
c
c
      return
      end
