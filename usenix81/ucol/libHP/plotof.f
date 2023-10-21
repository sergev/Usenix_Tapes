      subroutine plotof
	include 'gsdata.f'
c
c      to turn the plotter off
c
      call datout(1)
      moutar(mcount)=125
      mcount=mcount+1
      mloccm=82
      call hshake
      call outck1(41)
      monoff=0
      return
      end
