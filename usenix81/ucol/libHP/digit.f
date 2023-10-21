      subroutine digit(x,y,npenup,npenno)
	include 'gsdata.f'
c
c      read the pen position in digitizing
c
c
c     read the pen position in digitizing
c
      call outemp(mbufsi)
      mbufsi=mbufsi-mbufap
      call outre4(68,nx,ny,npenup,npenno)
c
      x=float(nx)
      y=float(ny)
c
c      convert the plotter units to the current unit system
c
      call putoun(x,y)
c
      mloccm=81
c
      return
      end
