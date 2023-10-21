      subroutine laxes(xtic,ytic,xorg,yorg,nxmaj,nymaj,ticsiz)
	include 'gsdata.f'
c
c     this is a dummy entry point to pass parameters to aglax.
c
      call aglax(xtic,ytic,xorg,yorg,nxmaj,nymaj,ticsiz,1)
      return
      end
