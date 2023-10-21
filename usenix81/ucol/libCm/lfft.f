      function lfft(n,a,ndir)
c
c $$$$$ calls fft and realtr $$$$$
c
c   If |ndir| = 1, lfft Fourier transforms the n point real time series
c   in array a.  The result overwrites a stored as (n+2)/2 complex
c   numbers (non-negative frequencies only).  If |ndir| = 2, lfft
c   Fourier transforms the (n+2)/2 complex Fourier coefficients (non-
c   negative frequencies only) in array a (assuming that the series is 
c   Hermitian).  The resulting n point real time series overwrites a.
c   If ndir > 0 the forward transform uses the sign convention 
c   exp(i*w*t).  If ndir < 0 the forward transform uses the sign
c   convention exp(-i*w*t).  the forward transform is normalized such
c   that a sine wave of unit amplitude is transformed into a delta
c   functions of amplitude 1/2.  the backwards transform is normalized
c   such that transforming forward and then back recovers the original
c   series.  Lfft returns the series length actually transformed (lfft
c   will be the largest integer factorable by fft which is not larger
c   than n) if sucessful and -1 otherwise.  If n is odd the last point
c   will be ignored.  Revised on 27 August 1979 by R. Buland.
c
      dimension a(1)
      idir=iabs(ndir)
      if(n.le.0.or.idir.le.0.or.idir.gt.2) go to 13
      n2=nfac(n/2)
      nn=2*n2
      go to (1,2),idir
c   Do forward transform (ie. time to frequency).
 1    call fft(a,a(2),n2,n2,n2,2,ierr)
      if(ierr.ne.0) go to 13
      call realtr(a,a(2),n2,2)
      n1=2*n2+2
      scale=.5/nn
      if(ndir.gt.0) go to 3
      do 5 i=4,nn,2
 5    a(i)=-a(i)
      go to 3
c   Do backward transform (ie. frequency to time).
 2    if(ndir.gt.0) go to 6
      do 7 i=4,nn,2
 7    a(i)=-a(i)
 6    a(2)=0.
      a(2*n2+2)=0.
      call realtr(a,a(2),n2,-2)
      call fft(a,a(2),n2,n2,n2,-2,ierr)
      if(ierr)13,8,13
c   Normalize the transform.
 3    do 4 i=1,n1
 4    a(i)=scale*a(i)
 8    lfft=nn
      return
c   Error return.
 13   lfft=-1
      return
      end
