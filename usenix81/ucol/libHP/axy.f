      subroutine axy(x,y)
c
c     this is used to convert the passed parameter values
c     to the proper axy binary syntax.
c
c     logical ncomp
      equivalence (ncomp,nint)
	include 'gsdata.f'
      if (x.lt.0.0) x=0.0
      if (y.lt.0.0) y=0.0
      if (x.gt.16383.0) x=16383.0
      if (y.gt.16383.0) y=16383.0
c
c     check to make sure there is room
c
      call datout(6)
      numbx=ifix(x+.5)
      numby=ifix(y+.5)
c
      idelta=max0(numbx,numby)
c
c
c     set index amount
c
      index=2
c
c     check for the number of bits for significance
c
      if (idelta.gt.3) index=5
c
      if (idelta.gt.31) index=8
c
      if (idelta.gt.255) index=11
c
      if (idelta.gt.2047) index=14
c
c
c
c
c     exceeds the absolute range/check relative
c
c     proceed to calculate the number for absolute
c
 600  nbyte=mcount+(index+1)/3-1
      i=0
      nxoff=0
c
c     mask off the bytes to deposit in the moutar array
c     also use it such that it will be able to mask
c     on any integer data type size from 16 bits upwards
c
      numb=numby
c     start of the loop to calculate the moutar array
c
 625  ncomp=and(numb,
     &(2**(min0(6,15-i*6-nxoff))-1)*2**(i*6+nxoff))
c
c     right shift the data over to place it correctly
c     in the ascii character range
c
      moutar(nbyte)=nint/(2**(i*6+nxoff))
c
c     check to see if we are at last bits for this pair
c
      if (index.gt.((i+1)*6+nxoff)) goto 650
c
c     check to see if we are done with the x stuff
c     this is indicated by the nxoffset variable having
c     a value greater than zero.  if so then done
c
      if (nxoff.ne.0) goto 700
      numb=numbx
c
c     nxoff is used to indicated the x offset of bits
c     that is contained in the msb of the y data pattern
c
      nxoff=6-(index-i*6)
c
c     find the lsb of the x number and place in the same byte
c     as the msb of the y
c
      ncomp=and(numb,(2**nxoff-1))
      moutar(nbyte)=moutar(nbyte)+nint*(2**(6-nxoff))
c
c      decrement the i counter to imply we are starting
c      the x half of the pair
c
      i=-1
c
c     make sure the one of the two most significants bits are on
c
 650  if (moutar(nbyte).lt.32) moutar(nbyte)=moutar(nbyte)+64
c
c     decrement the nbyte by one this will point to the next
c     available byte in the moutar array
c
      nbyte=nbyte-1
      i=i+1
c
c     if this is the last byte exit out of the loop
c
      if (nbyte.ge.mcount) goto 625
c
c     check to make sure the leading byte has bit 7&6  on.
c
 700  ncomp=or(moutar(mcount),96)
      moutar(mcount)=nint
      mcount=mcount+(index+1)/3
c
c
c     this is used to update the mloccm
c
      if (mloccm.le.1) mloccm=0
      if (mloccm.eq.2) mloccm=-1
 800  return
      end
