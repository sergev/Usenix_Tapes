      subroutine prol(p,n)
c  Applies a 4pi prolate spheroidal taper to a series p, of length
c  n, using the formula in D. J. Thomson, Bell Sys Tech Jour, 1977,
c  p. 1003.
      dimension p(n)
      m = (n+1)/2
      do 1 i = 1,m
      x = (2.*i-1)/float(n) - 1.
      x = 1. - x*x
      x = 1.980369766*(((((((((((((((((((((
     $ 2.6197747177e-11*x +2.9812025862e-10)*x +3.0793023553e-9)*x
     $+2.8727486380e-8)*x +2.4073904863e-7)*x +1.8011359410e-6)*x
     $+1.1948784162e-5)*x +6.9746276641e-5)*x +3.5507361197e-4)*x
     $+1.5607376779e-3)*x +5.8542015072e-3)*x +1.8482388296e-2)*x
     $+4.8315671140e-2)*x +1.0252816895e-1)*x +1.7233583271e-1)*x
     $+2.2425255852e-1)*x +2.1163435698e-1)*x +1.4041394473e-1)*x
     $+5.9923940533e-2)*x +1.4476509898e-2)*x +1.5672417352e-3)*x
     $+4.290463314e-5)
      p(i) = x*p(i)
      j = n+1 - i
 1    p(j) = x*p(j)
c  correction for n odd, in which case center term is done twice
      if(n+1-m.eq.m) p(m) = p(m)/x
      return
      end
