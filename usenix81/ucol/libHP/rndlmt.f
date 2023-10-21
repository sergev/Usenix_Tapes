      subroutine rndlmt(bmin,bmax,offset,ticsiz,slim)
      logical slim
      logical flip
      if(bmin  .ge.  bmax)  go to 30
	go to 31
 30	continue
	bmin=0
	bmax=0
	offset=0
	ticsiz=0
      return
   31 continue
      off1=0
      offset=0
      flip=.false.
      amin=bmin
      amax=bmax
      if(amin  .lt.  0)  off1=.99
      if(amax)  5,6,4
    4 l1=alog10(amax)-2.
      p1=10.**l1
      max1=amax/p1-1
      zmax1=max1*p1
      max=amax/p1  +  .9
      smax=max*p1
      min=  100.*amin/amax  -  off1
      jmin=amin/p1  -off1
      if(iabs(min)  .ge.  10)  go to 1
      if(flip)  go to 7
      if(slim)  go to 20
      bmin=0
    2 bmax=smax
      go to 20
    1 if(min  .ge.  99)  go to 3
      if(flip)  go to 8
      if(slim)  go to 20
      bmin=float(jmin)*p1
      go to 2
    3 offset=offset+zmax1
      amin=amin-zmax1
      amax=amax-zmax1
      go to 4
    6 if(amin  .ge.  0)  go to 30
      l1=alog10(-amin)  -2.
      p1=10.**l1
      min=amin/p1  -  .99
      if(slim)  go to 20
      bmax=0
      bmin=min*p1
      go to 20
    5 flip=.true.
      if(amin  .ge.  0)  go to 30
      amin=-bmax
      amax=-bmin
      go to 4
    7 if(slim)  go to 100
      bmax=0
    9 bmin=-smax
  100 continue
      if(offset  .ne.  0)  offset=-offset
      go to 20
    8 if(slim)  go to 100
      bmax=-float(jmin)*p1
      go to 9
   20 range=bmax-bmin
      l1=alog10(range)-2.01
      p1=10.**l1
      irnge=range/p1  +.9
      do 22 ntic=10,15
      if(mod(irnge,ntic)  .eq.  0) go to 21
   22 continue
      do 23 j=2,11
      ntic=11-j
      if(mod(irnge,ntic)  .eq.  0)  go to 21
   23 continue
   21 ticsiz=range/float(ntic)
      return
      end
