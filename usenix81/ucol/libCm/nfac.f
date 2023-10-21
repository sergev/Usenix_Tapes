      function nfac(n0)
c
c $$$$$ calls only library routines $$$$$
c
c   Nfac returns the largest integer (less than or equal to n0) which
c   will be factorable by Singleton's subroutine "FFT".  See the
c   comments at the beginning of fft for details.  Programmed on 28
c   August 1979 by R. Buland.
c
      dimension npr(9),ipr(9)
      data npr/2,3,5,7,11,13,17,19,23/
      n=iabs(n0)
      if(n.le.1) go to 13
c   Initialize the prime factor search.
 4    k=n
      no=0
      do 5 i=1,9
 5    ipr(i)=0
c   Extract all prime factors up to 23.
      do 1 i=1,9
 3    l=k/npr(i)
      if(l*npr(i)-k.ne.0) go to 1
      k=l
      no=no+1
      ipr(i)=mod(ipr(i)+1,2)
      if(k-1)2,2,3
 1    continue
c   Fall through if the largest prime factor is larger than 23.
 6    n=n-1
      go to 4
c   Try again if more than 208 prime factors have been found.
 2    if(no.ge.209) go to 6
      k=1
c   Check the square free portion.
      do 7 i=1,9
      if(ipr(i).gt.0) k=k*npr(i)
 7    continue
c   The product of the square free factors must be less than 211.
      if(k.gt.210) go to 6
 13   nfac=n
      return
      end
