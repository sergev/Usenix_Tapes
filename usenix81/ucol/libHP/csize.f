      subroutine csize(height,ratio,slant)
	include 'gsdata.f'
c
c    this is to set up the character size in
c      height,  ratio,   slant
c
c
c     first the height and width of the character cell
c
      ycha=abs(height)*yfacts
      xcha=0.0
      call delta(xcha,ycha)
      xcha=ycha*abs(ratio)
      xcha=amin1(xcha,16383.0)
      ycha=amin1(ycha,16383.0)
      xcha=amax1(xcha,1.0)
      ycha=amax1(ycha,1.0)
c
      if ((xcha.eq.xchara).and.(ycha.eq.ycharh)) goto 100
c
c
      mloccm=93
c
      call datout(10)
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=37
      mcount=mcount+1
c
c
      call axy(xcha,ycha)
c
c
 100  if (slant.eq.xchars) goto 700
c
      mloccm=93
c
c     this is used to determine the slant
c
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=47
      mcount=mcount+1
c
c     now the degrees
c
      call ang(90.0-slant)
c
c      save the variables
c
 700  xchars=slant
      xratio=ratio
      xchara=xcha
      ycharh=ycha
c
c
      return
      end
