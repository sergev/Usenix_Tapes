      subroutine lgrid(xtic,ytic,xorg,yorg,nxmaj,nymaj)
	include 'gsdata.f'
c
c     this is a dummy entry point to pass parameters
c     to the aglax routine.
c
      call aglax(xtic,ytic,xorg,yorg,nxmaj,nymaj,00.1,3)
      return
      end
