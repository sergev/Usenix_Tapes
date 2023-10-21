      subroutine outck1(loctyp)
	include 'gsdata.f'
c
c     this is used to output the control command.
c
c     encode(3,200,linbfx) iabs(loctyp)
c200  format (2h .,1a1)
c	linbfx(1)="33
      call loadch(linbfx(1),1,io33)
      linbfx(2)='.'
      call loadch(linbfx(3),1,iabs(loctyp))
	call terstr(mdevot,3,linbfx)
      return
      end
