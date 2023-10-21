      subroutine oerror(ntype,nstate,ncount)
	include 'gsdata.f'
c
c    this is used to return the error values in the plotter
c
      call outre3(-69,ntype,nstate,ncount)
c
c
c
      return
      end
