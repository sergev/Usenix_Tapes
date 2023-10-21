      subroutine idraw
	include 'gsdata.f'
c
c      this determines if a incrmental draw command is necessary
c      to send to the plotter.
c
      if (mloccm.ge.32001) goto 100
      call datout(1)
      moutar(mcount)=115
      mcount=mcount+1
      mloccm=32001
 100  mpenpo=0
      return
      end
