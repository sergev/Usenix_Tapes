      subroutine nmp(numb)
	include 'gsdata.f'
c
c   used to convert the passed parameter values to the nmp
c   binary syntax.
c
      if (numb.lt.0) numb=0
      if (numb.gt.63) numb=63
 200  call datout(1)
      moutar(mcount)=numb
      if (moutar(mcount).lt.32) moutar(mcount)=moutar(mcount)+64
      mcount=mcount+1
      mloccm=15
 300  return
      end
