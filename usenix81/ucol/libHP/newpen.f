      subroutine newpen(numb)
	include 'gsdata.f'
c
c    this is used to select a new pen
c
      mloccm=90
      call datout(1)
      moutar(mcount)=118
      mcount=mcount+1
      numb=mod(iabs(numb),5)
      mpenno=numb
      call nmp(numb)
      return
      end
