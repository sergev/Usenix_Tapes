      subroutine datout(numb)
	include 'gsdata.f'
c
c
c    this will check to see if the buffer should be emptied
c    this is done by checking the ncount and compare it
c    with the marsiz amount to make sure that the largest
c    next command or parameter will not overflow the
c    host array called moutar.  this amount is set by the
c     passed parameter numb.   numb is a integer
c     number determined by the calling subroutine.
c
c
c
c    this new section is used to take care of the plotter
c    buffer.  as in the label on mode.
c    when the numb parameter is pased on to this
c     this routine the routine will call hshake which clears
c     the host buffer area of all data.
c     then request output buffer size until the available
c     plotter buffer is greater than the amount requested.
c
c     if the numb=0 then the system will treat it like a
c     negative number and thus it will clear the host buffer
c     and do a output buffer request and put the latest
c     mbufsi value in mbufsi.
c
c
c
c     test for the negatives
c
      if (numb.gt.0) goto 300
      call hshake
      if ((mempty+numb).lt.0) numb=mempty*(-1)
 100  call outre1(66,mbufsi)
      mbufsi=mbufsi-mbufap
c      if (mbufsi+numb)  100,400,400
      if (mbufsi+numb)  200,400,400
 200  call doze
      go to 100
 300  if ((mcount+numb).ge.marsiz) call hshake
 400  return
      end
