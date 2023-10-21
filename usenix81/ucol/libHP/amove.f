      subroutine amove
	include 'gsdata.f'
c
c
c      this is used to determine if an absolue pen
c      up command is necessary to send to the plotter
c
c
      if (mloccm.eq.2)  goto 100
      call datout(1)
      moutar(mcount)=112
      mcount=mcount+1
      mloccm=2
 100  mpenpo=1
      return
      end
