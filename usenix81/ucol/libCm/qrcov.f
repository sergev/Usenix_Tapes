      subroutine qrcov(m,n,a,resq)
c  computes the covariance matrix for the solution to the least squares problem
c  ax = b, where a is m by n, m greater than n.  the program assumes that a has
c  been replaced by the upper triangular matrix r used in the qr decomposition.
c  the covariance matrix is returned in the first n rows of a.
c  resq is the sum of squares of residuals. if it is set negative, then the
c  unscaled covariance matrix is returned, otherwise the matrix is scaled by it
c  divided by m-n, which is the usual statistical estimate for the error.
c  see lawson + hanson, solving least squares problems, pp 69-70
c
c    calls no other routines
c    should be called immediately following subroutine qr
c
      dimension a(m,n)
c  return if conditions are wrong
      if (m.le.n) return
      sigsq = resq/(m-n)
      do 5 i = 1,n
 5    a(i,i) = 1./a(i,i)
      if (n.eq.1) go to 25
      n1 = n - 1
      do 20 i = 1,n1
      j1 = i + 1
      do 15 j = j1,n
      s = 0.
      l1 = j - 1
      do 10 l = i,l1
 10   s = s + a(i,l)*a(l,j)
      a(i,j) = -s*a(j,j)
 15   continue
 20   continue
 25   do 40 i = 1,n
      do 35 j = 1,n
      s = 0.
      do 30 l = j,n
 30   s = s + a(i,l)*a(j,l)
      a(i,j) = s
 35   continue
 40   continue
c  scale matrix (if requested), and fill out to make symmetric
      do 50 j = 1,n
      do 45 i = 1,j
      if(sigsq.ge.0.) a(i,j) = a(i,j)*sigsq
 45   a(j,i) = a(i,j)
 50   continue
      return
      end
