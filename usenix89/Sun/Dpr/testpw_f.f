
C
C  testpw_f.f
C
C  Fortran test program for print_window, the routine which will dump
C  the current window in the correct format for use by dpr.
C  Copyright (c) Rich Burridge, Sun Australia 1986.
C
C  Version 1.1.
C
C  No responsibility is taken for any errors inherent either in the comments
C  or the code of this program, but if reported to me then an attempt will
C  be made to fix them.
C

      PROGRAM TEST
      CHARACTER*256 PRINTER
      DATA PRINTER /'lp\0'/

      CALL PRIWIN(PRINTER)
      STOP
      END
