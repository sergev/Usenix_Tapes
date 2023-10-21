      subroutine aglax(xtic,ytic,xorg,yorg,nxmaj,nymaj,ticsiz,ntype)
c
c     ntype 0-axis; 1-laxis; 2-grid; 3-lgrid
c
c
c
c     dimension inaray(10)
      integer*2 inaray(10)
	include 'gsdata.f'
c
c
c
c     this is to intial the character height
c
      tcharh=ycharh/2.0
c
c
      nlntyp=mlntyp
      tdllen=xdllen
      call daslna(1,0.0)
      nsclip=msclip
      msclip=2
      tlaban=ylaban
c
c
c
      ytemp=0.0
c
      xticpu=abs(xtic)
      yticpu=abs(ytic)
      call delta(xticpu,yticpu)
      xorgpu=xorg
      yorgpu=yorg
      call change(xorgpu,yorgpu)
      majtcx=iabs(nxmaj)
      majtcy=iabs(nymaj)
      ticpu=abs(ticsiz)
      ntemp=msetun
      if (msetun.eq.3) msetun=2
      call delta (ticpu,ytemp)
      msetun=ntemp
c
      ntemp=minit1
      minit1=2
      if (ntype.gt.1) goto 500
      call plot(xwind2,yorgpu,3)
      call plot(xwind1,yorgpu,2)
      if (xtic.eq.0.0) goto 300
      ticnum=aint(((xwind1-xorgpu)/xticpu)-1.0)
 200  xstart=ticnum*xticpu+xorgpu
      if (xstart.lt.(xwind1-xwind1/32768.0)) goto 250
      if ((xstart).gt.(xwind2+xwind2/32768.0)) goto 300
      ticpua=ticpu/2.0
      if (majtcx.eq.0) goto 245
      if (aint(ticnum/float(majtcx)).eq.(ticnum/(float(majtcx))))
     &ticpua=ticpu
 245  call plot(xstart,yorgpu-ticpua,3)
      call plot(xstart,yorgpu+ticpua,2)
 250  ticnum=ticnum+1.0
      goto 200
 300  call plot(xorgpu,ywind2,3)
      call plot(xorgpu,ywind1,2)
      if (ytic.eq.0.0) goto 500
      ticnum=aint(((ywind1-yorgpu)/yticpu)-1.0)
 400  ystart=ticnum*yticpu+yorgpu
      if (ystart.lt.(ywind1-ywind1/32768.0)) goto 450
      if ((ystart).gt.(ywind2+ywind2/32768.0)) goto 500
      ticpua=ticpu/2.0
      if (majtcy.eq.0) goto 425
      if (aint(ticnum/float(majtcy)).eq.(ticnum/(float(majtcy))))
     &ticpua=ticpu
 425  call plot(xorgpu+ticpua,ystart,3)
      call plot(xorgpu-ticpua,ystart,2)
 450  ticnum=ticnum+1.0
      goto 400
 500  minit1=ntemp
 501  if (ntype.lt.2) goto 570
      ntemp=minit1
      minit1=2
      nyflag=0
      ticnox=aint(((xwind1-xorgpu)/xticpu)-1.0)
      ticnum=ticnox
      ticnoy=aint((ywind1-yorgpu)/yticpu-1.0)
 510  ystart=ticnoy*yticpu+yorgpu
      if (ystart.lt.(ywind1-ywind1/32768.0)) goto 560
      if ((ystart).gt.(ywind2+ywind2/32768.0)) goto 565
      if (aint(ticnoy/float(majtcy)).ne.(ticnoy/(float(majtcy))))
     &goto 520
c      draw start line
      call plot(xwind1,ystart,3)
      call plot(xwind2,ystart,2)
 520  xstart=ticnox*xticpu+xorgpu
      if (xstart.lt.(xwind1-xwind1/32768.0 )) goto 550
      if ((xstart).gt.(xwind2+xwind2/32768.0)) goto 555
 530  if (aint(ticnox/float(majtcx)).ne.(ticnox/(float(majtcx))))
     &goto 540
      if (nyflag.eq.1) goto 540
      call plot(xstart,ywind1,3)
      call plot(xstart,ywind2,2)
 540  inaray(1)=3
      call symbol(xstart,ystart,tcharh,inaray,0.0,-1)
 550  ticnox=ticnox+1.0
      goto 520
 555  nyflag=1
 560  ticnoy=ticnoy+1.0
      ticnox=ticnum
      goto 510
 565  minit1=ntemp
      call frame
 570  if ((ntype.ne.1).and.(ntype.ne.3)) goto 1300
      msclip=0
      nlbpos=mlbpos
      if ((xtic.eq.0.0).or.(majtcx.eq.0)) goto 900
      nfxdno=mfxdno
      xuu=xwind2
      yuu=0.0
      call putouu(xuu,yuu)
      tsign=xwind1
      yuu=0.0
      call putouu(tsign,yuu)
      xuu=amax1(abs(xuu),abs(tsign))
      ntics=ifix(alog10(xuu))
      nsign=0
      if (5-ntics-nfxdno.gt.0) goto 590
      nfxdno=5-ntics
      if (nfxdno.ge.-1) goto 590
c
c     need exponent
c
      nsign=ntics-2
      nfxdno=2
      inaray(1)=69+10*2**mcbits
      inaray(2)=32*2**mcbits
      if (ntics.le.0) inaray(2)=45*2**mcbits
      inaray(2)=inaray(2)+mod(iabs(nsign/100),10)+48
      inaray(3)=(mod(iabs(nsign/10),10)+48)*2**mcbits
      inaray(3)=inaray(3)+mod(iabs(nsign),10)+48
 590  ntics=8
      dir=90.0
      if (nxmaj.ge.0.0) goto 600
      ntics=6
      dir=0.0
 600  call lorg(ntics)
      ytemp=-1.0
      ticnum=aint((xwind1-xorgpu)/(majtcx*xticpu)-1.0)
      ntemp=minit1
      minit1=2
 700  xstart=ticnum*float(majtcx)*xticpu+xorgpu
      if (xstart.lt.(xwind1-xwind1/32768.0)) goto 800
      if ((xstart).gt.(xwind2+xwind2/32768.0)) goto 850
      xuu=xstart
      ytemp=0.0
      call putouu(xuu,ytemp)
      xuu=xuu/10.0**float(nsign)
      call number(xstart,ywind1,tcharh,xuu,dir,nfxdno)
      ytemp=xstart
 800  ticnum=ticnum+1.0
      goto 700
 850  if ((nsign.eq.0).or.(ytemp.eq.-1.0)) goto 875
      if (ytemp.lt.xwind1-xwind1/32768.0) goto 875
	call swapby(inaray,3)
      call symbol(ytemp,ywind1,tcharh,inaray,dir,6)
	call swapby(inaray,3)
 875  minit1=ntemp
 900  if ((ytic.eq.0.0).or.(majtcy.eq.0))  goto 1250
      nfxdno=mfxdno
      xuu=0.0
      yuu=ywind2
      call putouu(xuu,yuu)
      tsign=ywind1
      xuu=0.0
      call putouu(xuu,tsign)
      yuu=amax1(abs(yuu),abs(tsign))
      ntics=ifix(alog10(yuu))
      nsign=0
      if (5-ntics-nfxdno.gt.0.0) goto 990
      nfxdno=5-ntics
      if (nfxdno.ge.-1) goto 990
c
c     need exponent
c
      nsign=ntics-2
      nfxdno=2
      inaray(1)=69+10*2**mcbits
      inaray(2)=32*2**mcbits
      if (ntics.le.0.0) inaray(2)=45*2**mcbits
      inaray(2)=inaray(2)+mod(iabs(nsign/100),10)+48
      inaray(3)=(mod(iabs(nsign/10),10)+48)*2**mcbits
      inaray(3)=inaray(3)+mod(iabs(nsign),10)+48
 990  ntics=7
      xtemp1=0.0
      ytemp1=ycharh*.250
      dir=0.0
      if (nymaj.ge.0.0) goto 1000
      ntics=1
      dir=90.0
      xtemp1=ycharh*.333333
      ytemp1=0.0
 1000 call lorg(ntics)
      ticnum=aint((ywind1-yorgpu)/(float(majtcy)*yticpu)-1.0)
      ntemp=minit1
      minit1=2
 1100 ystart=ticnum*float(majtcy)*yticpu+yorgpu
      if (ystart.lt.(ywind1-ywind1/32768.0)) goto 1200
      if (ystart.gt.(ywind2+ywind2/32768.0)) goto 1210
      yuu=ystart
      ytemp=0.0
      call putouu(ytemp,yuu)
      yuu=yuu/10.0**float(nsign)
      if ((((ticnum+1.)*float(majtcy)*yticpu+yorgpu).gt.(ywind2+ywind2/
     &32768.)).and.(dir.eq.90.0).and.(nsign.ne.0)) xtemp1=ycharh*1.3333
      call number(xwind1-xtemp1,ystart-ytemp1,tcharh,yuu,dir,nfxdno)
      ytemp=ystart
 1200 ticnum=ticnum+1
      goto 1100
 1210 if ((nsign.eq.0).or.(ytemp.eq.-1.0)) goto 1220
      if (ytemp.lt.ywind1-ywind1/32768.0) goto 1220
	call swapby(inaray,3)
      call symbol(xwind1-xtemp1,ytemp-ytemp1,tcharh,inaray,dir,6)
	call swapby(inaray,3)
 1220 minit1=ntemp
 1250 mlbpos=nlbpos
 1300 call daslna(nlntyp,tdllen)
      call cdir(tlaban)
      msclip=nsclip
      call penup
      return
      end
