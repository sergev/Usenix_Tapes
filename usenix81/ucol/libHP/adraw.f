      subroutine adraw
	include 'gsdata.f'
c
c     this is used to determine if an absolute pen down
c     command is needed.
c
      if ((mloccm.eq.1).or.(mloccm.eq.0))  goto 100
      call datout(1)
      moutar(mcount)=113
      mcount=mcount+1
      mloccm=1
 100  mpenpo=0
      return
      end
