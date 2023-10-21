      subroutine ixy(x,y)
c
c      this is used to determine the incremental numbers
c
c
c     logical ncomp
      equivalence (ncomp,nint)
	include 'gsdata.f'
c
c     itype = 0    y number
c           = 1    x number
c
      xa=.5
      if(x.lt.0.0) xa=-.5
      itype=1
      nvalue=ifix(x+xa)
      call datout(8)
 50   nopts=1
      if((nvalue.lt.-16).or.(nvalue.gt.15))nopts=2
      if((nvalue.lt.-512).or.(nvalue.gt.511))nopts=3
c
      if (nvalue.lt.-16384) nvalue=-16384
      if (nvalue.gt.16383)  nvalue=16383
c
c     error !  the number is out of range
c
 200  i=0
      l=mcount+nopts-1
 210  ncomp=and(nvalue,31*32**i)
      moutar(l)=nint/(32**i)+(32+32*itype)
      if (l.eq.mcount) goto 220
      i=i+1
      l=l-1
      goto 210
 220  mcount=mcount+nopts
      if (itype.eq.0) goto 300
      xa=.5
      if (y.lt.0.0) xa=-.5
      nvalue=ifix(y+xa)
      itype=0
      goto 50
 300  if (mloccm.ge.32001) mloccm=32002
      if (mloccm.eq.32000) mloccm=32003
      return
      end
