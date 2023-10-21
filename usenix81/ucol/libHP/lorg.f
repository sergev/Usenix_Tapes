      subroutine lorg(lmode)
	include 'gsdata.f'
c
c     this is used to set the position labeling global
c     variable to be used by symbol and number.
c
      lmode=mod(iabs(lmode),10)
      if (lmode.eq.0) lmode=1
      mlbpos=lmode
      return
      end
