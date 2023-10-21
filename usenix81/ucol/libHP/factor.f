      subroutine factor(fact)
	include 'gsdata.f'
c
c     this is used to change the x,y factors
c
      if (fact.eq.0.0) fact=1.0
c
c
      xfacts=fact
      yfacts=fact
      return
      end
