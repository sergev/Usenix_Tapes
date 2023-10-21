      subroutine frame
	include 'gsdata.f'
c
c      this is used to draw a border around the current
c      soft clip region.  regardless of the current
c      soft clip flag setting.
c
c
      noline=mlntyp
      tpatln=xdllen
      ntemp=minit1
      minit1=2
      call daslna(1,0.0)
      call plot(xwind1,ywind1,3)
      call plot(xwind2,ywind1,2)
      call plot(xwind2,ywind2,2)
      call plot(xwind1,ywind2,2)
      call plot(xwind1,ywind1,5)
      call daslna(noline,tpatln)
      minit1=ntemp
      return
      end
