      subroutine arctol(numbpu)
	include 'gsdata.f'
c
c
c     this is used to specify the arc tolerance
c     for the arcs draw in the arcrel command.
c
      mloccm=83
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=84
      mcount=mcount+1
      ntemp=numbpu
      if (ntemp.lt.1) ntemp=1
      if (ntemp.gt.63) ntemp=63
      call nmp(ntemp)
      return
      end
