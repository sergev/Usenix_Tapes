      subroutine axes(xtic,ytic,xorg,yorg,nxmaj,nymaj,ticsiz)
	include 'gsdata.f'
c
c     this is a dummy entry point to call aglax for the
c     axes to draw.
c
      call aglax(xtic,ytic,xorg,yorg,nxmaj,nymaj,ticsiz,0)
      return
      end
