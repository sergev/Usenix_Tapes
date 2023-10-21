      subroutine labon(numb)
	include 'gsdata.f'
c
c     this routine invokes label mode
c
c     clear the host buffer
      call cdir(ylaban)
c
 50   call datout(4)
c
c     invoke label mode
      mloccm=88
c
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=39
      mcount=mcount+1
      if (numb.ge.0) goto 100
      moutar(mcount)=11
      mcount=mcount+1
      moutar(mcount)=12
      mcount=mcount+1
 100  call datout(-iabs(numb))
      mplost=1
      return
      end
