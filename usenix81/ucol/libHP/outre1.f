      subroutine outre1(loctyp,int1)
	include 'gsdata.f'
c
c   used to return requests with one return parameter from
c   the plotter
c
      call outck1(loctyp)
	call teread(int1,int2,int3,int4,1)
c	call terstr(mdevot,1,io12)
      return
      end
