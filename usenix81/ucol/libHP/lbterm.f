      subroutine lbterm(numb)
	include 'gsdata.f'
c
c     this is used to define the label terminator
c
      call datout(2)
      mloccm=85
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=92
      mcount=mcount+1
      if (numb.lt.1) numb=3
      if (numb.gt.126) numb=3
      if ((numb.eq.5).or.(numb.eq.27)) numb=3
      xtemp=float(numb)
      call agp(xtemp)
      mlterm=numb
      return
      end
