      function ai0(x)
c$$$$$ calls no other routines
c  modified bessel function of the 1st kind, 0th order, real argument
c  polynomial approximations from abramowitz+stegun, p378.
      y=abs(x)
      if (y.gt. 3.75) go to 2000
      t=(x/3.75)**2
      ai0=1.0+t*(3.5156229+t*(3.0899424+t*(1.2067492
     +  +t*(.2659732+t*(.0360768+t*.0045813)))))
      return
 2000 t=3.75/y
      a=.39894228+t*(.01328592+t*(.00225319-t*(.00157565-t*(.00916281
     +  -t*(.02057706-t*(.02635537-t*(.01647633-t*.00392377)))))))
      ai0=a*exp(y)/sqrt(y)
      return
      end
