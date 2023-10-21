      subroutine insert(number)
	include 'gsdata.f'
c
c
c     this is used to insert any number into the moutar
c     limitations should be numbers <0 and >128 however,
c
c     one should be aware no error checking is done here.
c
c
      call datout(1)
      moutar(mcount)=number
      mcount=mcount+1
      return
      end
