      subroutine cfont(ifont)
c
c    routine cfont changes the plotting font for labeling
c
c    input   - ifont  = font number
c			= 0 - default characters (not hershy fonts)
c			= 1 - KRoman
c			= 2 - KGreek
c			= 3 - SRoman
c			= 4 - SGreek
c			= 5 - SScript
c			= 6 - PIRoman
c			= 7 - PIGreek
c			= 8 - PIItalics
c			= 9 - PNRoman
c			= 10 - PNGreek
c			= 11 - PNItalics
c			= 12 - DRoman
c			= 13 - CScript
c			= 14 - CCyrilic (not working yet)
c			= 15 - TRoman
c			= 16 - TItalics
c			= 17 - Gothic German
c			= 18 - Gothic English
c			= 19 - Githic Italian
c
c
      save
      common /pfile/ ipunit
      character*1 i
      integer*2 font
c
      data  i  / 'i' /
c
      font = ifont
      call rwrite(ipunit,1,i)
      call rwrite(ipunit, 2, font)
      return
      end
c
c
