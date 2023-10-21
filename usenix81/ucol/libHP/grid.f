      subroutine grid(xtic,ytic,xorg,yorg,nxmay,nymaj)
	include 'gsdata.f'
c
c     this is used to transfer control to aglax routine.
c
      call aglax(xtic,ytic,xorg,yorg,nxmay,nymaj,0.1,2)
      return
      end
