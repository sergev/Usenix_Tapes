      subroutine sft(x,n,om,dom,nfr,st,ct)
      dimension x(n),st(nfr),ct(nfr)
c   Does fourier transform of a real sequence x, n terms long, at frequencies om
c  om+dom, . . ., om+(nfr-1)*dom.  om and dom are normalized in terms of
c  1/delta-t, so the nyquist frequency is .5.  The sine transforms are returned
c  in st and the cosine transforms in ct.  The transform used is the sum from
c  k=0 to k=n-1 of x(k)*exp(-i*k*omega), the usual definition for the dft.
c   The algorithm used is that of Goertzal, with modifications to reduce
c  roundoff error. See W. M. Gentleman, Computer Journal, 1969, pp 160-165.
c
c   Calls only system routines
c
      data tp/6.283185307/
c  loop over frequencies
      do 11 j = 1,nfr
      f = om + (j-1)*dom
      l = 6.*f
c  value of l determines way in which calculation is done
      f = tp*f
      s = sin(f)
      a = 0.
      if(l.eq.0) b = -4.*(sin(f/2.)**2)
      if(l.eq.1) b =  2.*cos(f)
      if(l.ge.2) b =  4.*(cos(f/2.)**2)
      c = 0.
      d = 0.
      e = 0.
      f = 0.
      if(l.eq.1) go to 3
      if(l.ge.2) go to 7
c  recursion for low frequencies (lt nyquist/3)
      do 1 k = 1,n
      c = a
      d = f
      a = x(n-(k-1)) + b*d + c
 1    f = a + d
      st(j) = -s*d
      ct(j) = a - (b*d)/2.
      go to 11
c  regular goertzal algorithm, for intermediate frequencies
 3    do 5 k = 1,n
      f = x(n-(k-1)) + b*d - e
      e = d
 5    d = f
      st(j) = -s*e
      ct(j) = f - (b*e)/2.
      go to 11
c  recursion for high frequencies (gt 2*nyquist/3)
 7    do 9 k = 1,n
      c = a
      d = f
      a = x(n-(k-1)) + b*d - c
 9    f = a - d
      st(j) = -s*d
      ct(j) = a - (b*d)/2.
 11   continue
      return
      end
