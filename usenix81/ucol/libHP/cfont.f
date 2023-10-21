      subroutine cfont(nstd,nalt)
	include 'gsdata.f'
c
c     this is to produce the new standard and alternate
c     character font.
c
      if ((mcstdf.eq.nstd).and.(mcaltf.eq.nalt)) goto 800
c
      if ((nstd.lt.0).or.(nstd.gt.5)) nstd=0
      if ((nalt.lt.0).or.(nalt.gt.5)) nalt=5
      mloccm=93
c
c
      call datout(2)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=80
      mcount=mcount+1
c
c     convert over the font numbers to axy pair
c
      x=float(nstd)
      y=float(nalt)
      call axy(x,y)
c
c     store the variables
c
      mcstdf=nstd
      mcaltf=nalt
c
c     finished
c
c     update the logical command variable
c
c
 800  return
      end
