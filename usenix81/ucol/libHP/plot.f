      subroutine plot (x,y,ipen)
c
      dimension arxy(4,2)
	include 'gsdata.f'
c
c     hp7221a plotter control subroutine for hp3000 computer systems.
c     main control subroutine for hp7221a graphic plotter
c     library, part no. 07221-18001  rev. a  (aug 1, 1977)
c
c     each call to the subroutine will cause the plotter to move
c     the pen to the point specified as 'x,y' with the pen position
c     controlled by the parameter 'ipen' as defined below.
c     all calling parameters are left unchanged after return.  plotter
c     control characters are buffered into blocks.  successive calls
c     to the subroutine with identical parameters are ignored.
c     the x,y data points are mapped to the user unit(x,y min and
c     x,y max) mapping points to the locate reference points.
c     if one is using the scale, axis, or line subroutines, the user
c     unit is in inches from the origin.
c
c     calling parameter definition:
c
c       x,y    coordinates of the vector endpoint in inches from origin
c       ipen   pen control parameter:
c          ipen>0  move made relative to current origin
c          ipen<0  move made relative to current origin, then origin
c                  redefined to the vector end-point.
c                  (xmax-xmin),(ymax-ymin).
c     iabs(ipen)>9   then the routine will send the logical endpoint
c                    to the plotter.
c     iabs(ipen)<9  then the routine will send the clipped endpoint
c                     to the plotter.
c          ipen=0  pen left unchanged.
c          ipen=1  point plot -- pen is raised, then lowered at
c                  each endpoint.
c          ipen=2  pen lowered before move.
c          ipen=3  pen raised before move.
c          ipen=4  pen moves in the current status(pen up or down)
c                  then at the endpoint does a pen down.
c          ipen=5  pen moves in the current status(pen up or down)
c                  then at the endpoint does a pen up.
c          ipen=999  pen raised and moved to upper-right corner
c                    of plot area, buffer transmitted and plotter
c                    turned off.
c     1.     the plot subroutine is one of the central subroutine in the
c            hp-plot/21 graphic package.
c
c
c     2.     the origin (0,0) is the original reference of the
c            lower left-hand corner of the paper or is the last-
c            known position from the previous plot with ipen<0
c
c     3.     offscale points will be clipped.  dashlines are
c            used for texture only.  dashlines with different scaling
c            may not look identical.
c
c     4.     user units are created by mapping the user unit
c            points to the locate reference points.
c
c     5.     xfacts and yfacts effect both the user unit and
c            the inch scaling.
c
c     subroutine initialization
c
      if (minit1.ge.1) goto 100
c
c     should intialize the various values for scaling function
c
      call plots(3,5,6)
c
c     scale data values and determine move to be made
c
 100  jpen=iabs(ipen)
c
c
      if (jpen.lt.999) goto 105
c
c     pen raise and redefine the x,y pair to the upper-right
c     hand corner.
c
      arxy(2,1)=xmaxpu
      arxy(2,2)=ymaxpu
      xcurrl=xmaxpu
      ycurrl=ymaxpu
c
      goto 135
c
c     this is the scaling function for user units to plotter
c     units with the factor being in here also.
c
 105  xcurrl=x
      ycurrl=y
      if ((yrange.eq.-1.0).or.(minit1.gt.1).or.(msetun.ne.3)) goto 110
      if (yrange.gt.0.0) goto 108
      xminrg=x
      xmaxrg=x
      yminrg=y
      ymaxrg=y
      goto 109
 108  xminrg=amin1(xminrg,x)
      xmaxrg=amax1(xmaxrg,x)
      yminrg=amin1(yminrg,y)
      ymaxrg=amax1(ymaxrg,y)
 109  yrange=yrange+1.0
      goto 800
 110  call change(xcurrl,ycurrl)
      if (ipen.ge.0) goto 112
      xoffpu=xcurrl-xoffpu
      yoffpu=ycurrl-yoffpu
c
c
c
c
c
c     change the arxy(1,1) to arxy(2,2) for the clip routine
c
 112  arxy(1,1)=xlastl
      arxy(1,2)=ylastl
      arxy(2,1)=xcurrl
      arxy(2,2)=ycurrl
c
c
c     set up the boundaries conditions
c
      arxy(3,1)=-0.000001
      arxy(3,2)=-0.000001
      arxy(4,1)=xmaxpu+xmaxpu/32768.0
      arxy(4,2)=ymaxpu+ymaxpu/32768.0
      if (msclip.eq.4) goto 113
      if (((msetun.ne.3).or.(msclip.eq.0).or.(jpen.eq.13))
     &.and.(msclip.ne.2)) goto 114
 113  arxy(3,1)=xwind1-xwind1/32768.0
      arxy(3,2)=ywind1-ywind1/32768.0
      arxy(4,1)=xwind2+xwind2/32768.0
      arxy(4,2)=ywind2+ywind2/32768.0
c
c     everything is in plotter unit according to g1,g2
c
c     offscale will be clipped and set the physical start point
c     to the clipping boundaries
c
 114  call cliper(arxy,nflag1)
c
c     check for an error
c
c     c
c     check for both being out of bounds
c
      if (nflag1.eq.-1) goto 300
c
c     check to make sure no erros
c
      if (nflag1.ge.0) goto 118
c
c      do something for the error
c
 115  goto 800
c
c
c     this is used to check if the current physical position
c     is different than the physical start clipped point
c
 118  if ((arxy(1,1).eq.(xlasta)).and.(arxy(1,2).eq.(ylasta)))
     & goto 120
c
c     yes they are different so move the pen to the new place
c     with the pen up
c
c     store the mpenpo for use in jpen determination
c
      ntemp=mpenpo
c
      call amove
      call axy(arxy(1,1),arxy(1,2))
c
c     restore the mpenpo for use in jpen determination
c
      mpenpo=ntemp
c
c     should determine if the pen should be raise or lowered before
c     the move.
c
c
 120  if (jpen.ge.10) jpen=jpen-jpen/10*10
      if (mplost.eq.1) goto 135
c
c     the above will cause a pen up move directly following
c     a label mode
c
      if((jpen.eq.1).or.(jpen.eq.3)) goto 135
      if (jpen.eq.2) goto 131
 130  if (mpenpo.eq.1) goto 135
 131  call adraw
      goto 140
 135  call amove
c
c     check to see if we move to the same place
c
 140  call axy(arxy(2,1),arxy(2,2))
c
c     record the xstartp, ystartp position so that
c     the next interation will be correct actual physical
c     point
c
      xlasta=arxy(2,1)
      ylasta=arxy(2,2)
c
c
 200  if (jpen.lt.999) goto 300
      call plotof
      goto 775
 300  if ((jpen.lt.10).or.((nflag1.lt.10).and.
     &(nflag1.ge.0))) goto 400
      npenpo=mpenpo
      ntemp=minit1
      minit1=2
      tangle=xrelan
      call pdir(0.0)
c
cc     this is used to check out the outof bounds to an outof
c     bounds with the ipen >9.
c
      if (nflag1.ge.0) goto 375
      if (mplost.eq.0) goto 375
c
c     this above is for any  ipen>9 move with the pen location
c     known
c
c     going to clip the xlastl to the center of the paper
c     limits  this will make an absoute move there
c     then iplot can be used to move it the the actual start
c     location.
c
c     hopefull this will not cause to many extranous moves
c
      arxy(1,1)=xlastl
      arxy(1,2)=ylastl
      arxy(2,1)=xmaxpu/2.0
      arxy(2,2)=ymaxpu/2.0
c
c
      call cliper(arxy,nflag1)
c
c
      call amove
      call axy(arxy(1,1),arxy(1,2))
c
c
c     reset the actual position point
c
      xlasta=arxy(1,1)
      ylasta=arxy(1,2)
c
c
 375  call iplot1(xcurrl-xlasta,ycurrl-ylasta,3)
      minit1=ntemp
      call pdir(tangle)
      mpenpo=npenpo
      goto 500
 400  if ((nflag1.lt.10).and.(nflag1.ne.-1)) goto 600
 500  if ((jpen.eq.1).or.(jpen.eq.2).or.(jpen.eq.4)) mpenpo=0
      if ((jpen.eq.3).or.(jpen.eq.5)) mpenpo=1
      goto 775
 600  if (jpen) 700,775,700
 700  if ((jpen.eq.1).or.(jpen.eq.4)) call adraw
      if (jpen.eq.5) call amove
 775  xlastl=xcurrl
      ylastl=ycurrl
      xrelpu=xlastl
      yrelpu=ylastl
      mplost=0
c
 800  return
      end
