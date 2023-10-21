      subroutine ang (an)
	include 'gsdata.f'
c
c
      angle=an
c
c     ang = angle of the arc you wish to convert
c     moutar= the array of the data
c     mcount = the mcount of the data
c
c
c     check the handshake
c
      call datout(3)
 30   angle=angle-(aint(angle/360.0))*360.0
      if (angle.lt.0.0) angle=360.0+angle
      moutar(mcount)=angle/22.5
      angle = angle-float(moutar(mcount))*22.5
      moutar(mcount)=moutar(mcount)+96
      mcount=mcount+1
      if (angle-.0054931640625) 100,50,50
 50   moutar(mcount)=angle/.3515625
      angle = angle-float(moutar(mcount))*.3515625
      if (moutar(mcount).lt.32) moutar(mcount)=moutar(mcount)+64
      mcount=mcount+1
      if (angle-.0054931640625) 100,60,60
 60   moutar(mcount)=angle/.0054931640625
      if (moutar(mcount).lt.32) moutar(mcount)=moutar(mcount)+64
      mcount=mcount+1
 100  mloccm=14
      return
      end
