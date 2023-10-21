      subroutine hann(x,n)
      dimension x(n)
c  applies a cosine window to a series in the array x, which
c  is of length n, supposed even (in the event that n is odd, the
c  window will be applied as for a record of length n-1, and the
c  n-th point will simply be zeroed). cosine approximation is from
c  abramowitz & stegun, 4.3.98.
      data pi/3.1415926536/
      sc = 2.*pi/n
      m = (n/2) + 1
      k = (n/4) + 1
c  np is n-1 for n odd, n for n even
      np = n - (n+1)/2 + n/2
      do 1 i = 2,k
      d = sc*(i-1)
      d = d*d
      d = d*(.4967-.03705*d)
      x(i) = d*x(i)
      x(np+2-i) = d*x(np+2-i)
      d = 2 - d
      j = i - 1
      x(m-j) = d*x(m-j)
 1    x(m+j) = d*x(m+j)
      x(1) = 0.
      if(np.ne.n) x(n) = 0.
      x(m) = 2*x(m)
      return
      end
