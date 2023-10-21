      subroutine realtr (a,b,n,isn)
c
c
c title - realtr = real transform
c     fourier transform of real series from output of fft
c
c              if isn=1, this subroutine completes the fourier trans-
c              form of 2*n real data values, where the original data
c              values are stored alternately in arrays a and b, and are
c              first transformed by a complex fourier transform of
c              dimension n.
c              the cosine coefficients are in a(1),a(2),...a(n+1) and
c              the sine coefficients are in b(1),b(2),...b(n+1).
c              a typical calling sequence is
c              call fft (a,b,n,-1)
c              call realtr (a,b,n,1)
c              the results should be multiplied by 0.5/n to give the
c              usual scaling of coefficients.
c              if isn=1, the inverse transformation is done, the
c              first step in evaluating a real fourier series.
c              a typical calling sequence is
c              call realtr (a,b,n,-1)
c              call fft (a,b,n,-1)
c              the results should be multiplied by 0.5 to give the
c              usual scaling, and the time domain results alternate
c              in arrays a and b, i.e. a(1),b(1), a(2),b(2),
c              ...a(n),b(n).
c              the data may alternatively be stored in a single
c              complex array a, then the magnitude of isn changed to
c              two to give the correct indexing increment and a(2)
c              used to pass the initial address for the sequence of
c              imaginary values,e.g.
c              call fft(a,a(2),n,2)
c              call realtr(a,a(2),n,2)
c              in this case, the cosine and sine coefficients
c              alternate in a.
c
c inputs
c
c     a(i)            i=1, iabs(isn)*n, iabs(isn) contains the real
c              part of the fourier transform of a real data series
c              treated as complex.
c     b(i)            contains the imaginary part corresponding to a
c     n               the number of complex numbers represented in
c              a and b
c     isn             is negative for the inverse transform and
c              positive for the transform iabs(isn) is the advance
c              for indexing through a or b.
c
c
c outputs
c
c     a(1)     the real part of the fourier transform of a real data
c              series
c     b(1)     the imaginary part of the fourier transform of a
c              real data series
c     f        = 0 if no errors
c              = 1 if n .le. 0
c
c
c examples
c
c  suppose  x  is a real array with  n  elements (n=2**l), and
c  we wish the digital fourier cosine and sine transforms of it;
c  then these calls are required (using pdp-11 routines)
c      call ffttwo(x,n/2)
c      call realtr(x,x(2),n/2,2)
c  now  x  contains the cos and sin transforms (unnormalized) ,
c  starting at zero frequency and alternating cos,sin,cos,sin,....
c  note that  n  has been halved in the calls to ffttwo  and  realtr
c
c ***** warning - the transforms of the nyquist frequency
c      appear in the positions  x(n+1),x(n+2)  so that
c      the array  x  must be dimensioned at least  x(n+2) in
c      the calling  program   ***************
c
c
      dimension a(1),b(1)
      real im
      if(n .le. 1) return
      inc = iabs(isn)
      nk = n * inc + 2
      nh = nk / 2
      sd = 2. * atan(1.) / float(n)
      cd = 2. * sin(sd)**2
      sd = sin(sd+sd)
      sn = 0.
      if(isn .gt. 0) go to 10
      cn = -1.
      sd = -sd
      go to 20
 10   cn = 1.
      a(nk-1) = a(1)
      b(nk-1) = b(1)
 20   do 30 j=1,nh,inc
      k = nk - j
      aa = a(j) + a(k)
      ab = a(j) - a(k)
      ba = b(j) + b(k)
      bb = b(j) - b(k)
      re = cn * ba + sn * ab
      im = sn * ba - cn * ab
      b(k) = im - bb
      b(j) = im + bb
      a(k) = aa - re
      a(j) = aa + re
      aa = cn - (cd * cn + sd * sn)
      sn = (sd * cn - cd * sn) + sn
 30   cn = aa
c
      return
      end
