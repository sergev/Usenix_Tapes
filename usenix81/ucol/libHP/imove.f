      subroutine imove
	include 'gsdata.f'
c
c     this determines if a incremental move command is to
c     transmitted to the plotter
c
      if (mloccm.eq.32000) goto 100
      call datout(1)
      moutar(mcount)=114
      mcount=mcount+1
      mloccm=32000
 100  mpenpo=1
      return
      end
