      subroutine penspd(nspeed)
	include 'gsdata.f'
c
c  used to set the pen velocity.
c
      mloccm=86
      call datout(2)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=86
      mcount=mcount+1
      if (nspeed.lt.1) nspeed=1
      if (nspeed.gt.36) nspeed=36
      call nmp(nspeed)
      return
      end
