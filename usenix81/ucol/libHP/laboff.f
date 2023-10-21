      subroutine laboff
	include 'gsdata.f'
c
c     this routine turns label mode off
c
c     turn label mode off
c
      mloccm=87
      moutar(mcount)=mlterm
      mcount=mcount+1
      call penup
      call hshake
      return
      end
