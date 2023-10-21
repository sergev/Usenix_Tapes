      subroutine hshake
	include 'gsdata.f'
c
c     routine will handle the handshake
c
c
      if (mcount.eq.mstart) goto 200
      nfirst=mstart
      if (mbufsi.gt.mcount-nfirst) goto 150
 10   call outre1(66,mbufsi)
      mbufsi=mbufsi-mbufap
c      if (mbufsi) 10,10,150
      if (mbufsi) 20,20,150
 20   call doze
      go to 10
 150  nlast=min0(mbufsi+nfirst-1,mcount+mstart-2)
c 100  format (200r1)
c     write (mdevot,100) (moutar(i), i=nfirst,nlast)
	length=0
	do 160 i=nfirst,nlast
	length=length+1
c	linbfx(length)=and(io377,moutar(i))
      call loadch(linbfx(length),1,and(io377,moutar(i)))
160	continue
	call terstr(mdevot,length,linbfx)
      mbufsi=mbufsi-nlast+nfirst-1
      nfirst=nlast+1
      if (nfirst.lt.mcount+mstart-1) goto 10
      mcount=1
 200  return
      end
