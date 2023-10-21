      subroutine outre4(loctyp,int1,int2,int3,int4)
	include 'gsdata.f'
c
c    used to return requests with four return parameters from
c     the plotter.
c
      call outck1(loctyp)
	call teread(int1,int2,int3,int4,4)
c	call terstr(mdevot,1,io12)
      return
      end
