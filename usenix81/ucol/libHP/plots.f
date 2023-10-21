      subroutine plots(inttyp,login,logout)
	include 'gsdata.f'
c
c
c
c     the following are the variables used in the
c     common block.
c
c     intitalize the various varibles
      mclear=0
      mlterm=3
      minit1=1
      mclipf=0
      mdevin=login
      mdevot=logout
      mcstdf=-1
      mcaltf=-1
      mlbpos=1
      mcount=1
      mstart=1
      marsiz=200
      msetun=2
      mloccm=99
      mplost=0
      msclip=1
      mnchar=2
      mcbits=8
c
c	open /dev/hp
c
      call hpopen(mdevin,mdevot)
c
c     real array
c
      xrespu=5.0
      yrespu=5.0
c
      xfacts=1.0
      yfacts=1.0
c
      xoffpu=0.0
      yoffpu=0.0
c
c     the non array type of real data
c
      xratio=-1.0
      xchara=-1.0
      ycharh=-1.0
c
c
      xchars=-1.0
      ylaban=-2.0
c
      xrelan=-3.0
      yactan=-1.0
c
      xdllen=-1.0
c
c
      jtype=iabs(inttyp)
c
      if (inttyp.eq.19348) goto 460
c
c     turn the plotter on.  this will produce an esc . )
c     sequence to turn the plotter on if it is off.
c
c     encode(30,100,linbfx)
c100  format (30h .(;.m10;17;10;13:;.e;.j;.f;.j)
c	linbfx(1)="33
c	linbfx(4)="33
c	linbfx(19)="33
c	linbfx(22)="33
c	linbfx(25)="33
c	linbfx(28)="33
      call loadch(linbfx(1),1,io33)
      linbfx(2)='.'
      linbfx(3)='('
      call loadch(linbfx(4),1,io33)
      linbfx(5)='.'
      linbfx(6)='M'
      linbfx(7)='1'
      linbfx(8)='0'
      linbfx(9)=';'
      linbfx(10)='1'
      linbfx(11)='7'
      linbfx(12)=';'
      linbfx(13)='2'
      linbfx(14)='0'
      linbfx(15)=';'
      linbfx(16)='1'
      linbfx(17)='0'
      linbfx(18)=':'
      call loadch(linbfx(19),1,io33)
      linbfx(20)='.'
      linbfx(21)='E'
      call loadch(linbfx(22),1,io33)
      linbfx(23)='.'
      linbfx(24)='J'
      call loadch(linbfx(25),1,io33)
      linbfx(26)='.'
      linbfx(27)='F'
      call loadch(linbfx(28),1,io33)
      linbfx(29)='.'
      linbfx(30)='J'
	call terstr(mdevot,30,linbfx)
c
c    set flag on
c
      monoff=1
c
c     set up the handshake and trigger for all i/o requests
c
c     10 millisecond delay, dc1 trigger, lf echo bypass
c     character, cr terminator for output.
c
c     esc . m 10; 17;10;13:
c     set hand shake mode
c     10 millisecond delay
c     dc1 character trigger
c     lf echo by pass character
c     cr terminator for plotter output
c
c    this is used to set the handshake flag
c
c     set up the  intit cycle
c
c
c     this is used to abort all i/o requests
c
c     call outck1(74)
c
c
c
c     check to see what type of init.  if buffer should be
c     flushed before execution of this current plot
c
c     this change is used for limit call to plots
c
      if (inttyp.lt.0) call outck1(75)
c
c     wait for the buffer to empty.
c
 300  call outemp(mempty)
c
      mbufap=60
      if (mempty.gt.1200) mbufap=120
      if (mempty-mbufap.lt.0) mbufap=mempty
      mempty=mempty-mbufap
      mbufsi=mempty
c
c     jtype=iabs(inttyp)
c
c     this will decide the level of initializations
c
c
c     level abs(jtype)=1 should reinit the hardware p1,p2 points
c
      if (jtype.ne.1) goto 450
c
c     this is used to intialize the plotter and
c     default the lower left and upper right to the
c     plotter defaults.
c
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=95
      mcount=mcount+1
c     call hshake
c
c      add the mu so out current postion is no needed
c
      xminmu=520.0
      yminmu=380.0
      xmaxmu=15720.0
      ymaxmu=10380.0
      goto 460
c
c
c     level=3  should do the minimal amount of start-up.
c
 450  call outre4(71,muxmin,muymin,muxmax,muymax)
      xminmu=muxmin
      yminmu=muymin
      xmaxmu=muxmax
      ymaxmu=muymax
c
c
c     this is used to check to make sure that the delta
c     graphic limit values are not zero.
c     thus if they are the same the system  will increment
c     the value of the max by one such that the range
c     will never be exactly zero.
c
c
 460  if (xmaxmu.eq.xminmu) xmaxmu=xmaxmu+1.0
      if (ymaxmu.eq.yminmu) ymaxmu=ymaxmu+1.0
c
c
c     convert the machine units to inches using that
c     conversion factor of   .025/25.4  mm/mu/mm/in
c
c
      xmaxin=abs(xmaxmu-xminmu)*9.84252e-04
      ymaxin=abs(ymaxmu-yminmu)*9.84252e-04
c
c     calculate the max gdu space
c
      xmaxgu=(xmaxin/ymaxin)*100
      ymaxgu=100
      if (xmaxin.gt.ymaxin) goto 600
      xmaxgu=100
      ymaxgu=(ymaxin/xmaxin)*100.0
c
c     set up the uu stuff.
c
 600  xminuu=0
      yminuu=0
      xmaxuu=xmaxgu
      ymaxuu=ymaxgu
c
c     set up the grid units
c     such that they follow  the resolution of xrespu,yrespu
c
      xmaxpu=xmaxin/xrespu*1000
      ymaxpu=ymaxin/yrespu*1000
      if (xmaxpu.lt.1.0) xmaxpu=1.0
      if (ymaxpu.lt.1.0) ymaxpu=1.0
c
c
c     insert the scale command
c      the characters are a tilda, capital s followed by the
c      axy pair.
c
      moutar(mcount)=126
      moutar(mcount+1)=83
      mcount=mcount+2
c
c     followed by the axy parameter
c
      call axy(xmaxpu,ymaxpu)
c
c     level abs(jtype)=2 should preserve the hardware p1,p2
c     but move the pen to 0,0 (lower  left)
c
c
      if ((jtype.ne.2).and.(jtype.ne.1)) goto 550
c
c
c     level 2
c
c
      call newpen(1)
      call amove
      call axy(0.0,0.0)
      moutar(mcount)=125
      mcount=mcount+1
c
c     need to flush host buffer
c
c     call hshake
      xlasta=0.0
      ylasta=0.0
      npenpo=0
c
c
c     this is used to update the window,mapping points
c
 550  xwind1=0.0
      ywind1=0.0
      xwind2=xmaxpu
      ywind2=ymaxpu
c
c     the mapping points
c
      xmapl1=0
      ymapl1=0
      xmapl2=xmaxpu
      ymapl2=ymaxpu
c
c
c
      xmapt1=xmapl1
      xmapt2=xmapl2
      ymapt1=ymapl1
      ymapt2=ymapl2
c
c
      mpenpo=1
c
c     this is the part that will find which pen is currently
c     being used.  may be the same as the start of pen one if
c     the level 1 or 2 init was invoked.
c
c     bit 1-3 are the pen number currently used in the stall
c
c     call outre1(70,mstats)
c     mpenno=mstats-mstats/16*16
c     mpenno=mpenno/2
      mpenno=-1
c
c     check the dashlines.  make sure it is currently solid
c     lines.
c
c
c     this will clear the error numbers to zero
c
c     call outck1(69)
c     call outck1(74)
      merro1=0
      merro2=0
      merro3=0
c
c     convert the mu to pu
c
c     also determines the pen position
c
c
      if ((jtype.eq.1).or.(jtype.eq.2)) goto 650
      if (inttyp.eq.19348) goto 615
c
c
      call outre3(67,mxlast,mylast,npenpo)
      xlasta=float(mxlast)
      ylasta=float(mylast)
c
 615  call mutopu(xlasta,ylasta)
c
c     store the values in the logical last actual and current
c
 650  xlastl=xlasta
      ylastl=ylasta
      xrelpu=xlastl
      yrelpu=ylastl
c
c
      call cfont(0,5)
      call cdir(0.0)
      call csize(2.460,.5,00.0)
      call penspd(36)
      call lbterm(3)
      call arctol(1)
      mfxdno=2
      yrange=-1.0
      call daslna(1,0.0)
c
      call pdir(0.0)
c
c     reset the flag to indicate the intialization is done
c
      return
      end
