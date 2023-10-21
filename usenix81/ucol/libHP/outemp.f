      subroutine outemp(int1)
	include 'gsdata.f'
c
c    used to return the plotter buffer size when all plotting
c    in the plotter buffer has been completed.
c
      call hshake
c     encode(3,100,linbfx)
c100  format (3h .l)
c	linbfx(1)="33
      call loadch(linbfx(1),1,io33)
      linbfx(2)='.'
      linbfx(3)='L'
	call terstr(mdevot,3,linbfx)
	call teread(int1,int2,int3,int4,1)
c	call terstr(mdevot,1,io12)
      return
      end
