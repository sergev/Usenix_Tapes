
/*
 *  dpr.h
 *
 *  Definitions and structure declarations used by dpr and associated software.
 *  Copyright (c) Rich Burridge, Sun Australia 1986.
 *
 *  Version 1.1.
 *
 *  No responsibility is taken for any errors inherent either in the comments
 *  or the code of this program, but if reported to me then an attempt will
 *  be made to fix them.
 */

#define  CLOSE         (void) close            /* To make lint happy. */
#define  FCLOSE        (void) fclose
#define  FPRINTF       (void) fprintf
#define  GETTIMEOFDAY  (void) gettimeofday
#define  GTTY          (void) gtty
#define  IOCTL         (void) ioctl
#define  LSEEK         (void) lseek
#define  PR_CLOSE      (void) pr_close
#define  PRINTF        (void) printf
#define  READ          (void) read
#define  SSCANF        (void) sscanf
#define  SPRINTF       (void) sprintf
#define  STRCAT        (void) strcat
#define  STRCPY        (void) strcpy
#define  STTY          (void) stty
#define  UNLINK        (void) unlink
#define  WRITE         (void) write

#define  HORIZONTAL  1             /* Direction of printer scan. */
#define  VERTICAL    2

#define  BOOLEAN     1             /* Printer capability types. */
#define  NUMBER      2
#define  STRING      3

/* Variable storage lengths. */
#define  B_CH        8             /* Number of bits per character. */
#define  B_SHORT     16            /* Number of bits per short. */
#define  B_INT       32            /* Number of bits per integer. */

/* Sun/printer handshake types. */
#define  NONE        0             /* No handshake. */
#define  XON_XOFF    1             /* Send/receive XONs and XOFFs. */
#define  DTR         2             /* Monitor data terminal ready. */
#define  CTS         3             /* Monitor clear to send. */
#define  RTS         4             /* Monitor request to send. */

/* Screen sizing parameters. */
#define  MAXHEIGHT   900           /* Maximum screen height in pixels. */
#define  MAXWIDTH    1152          /* Maximum screen width in pixels. */
#define  MAXDEPTH    1             /* Maximum screen depth in pixels. */
#define  MAXCMAP     256           /* Maximum number of screen colors. */

#define  MAXLINES    32            /* Maximum possible printer lines per pass. */
#define  MAXSTR      512           /* Maximum length of string sequences. */
#define  MAXCAP      19            /* Maximum number of printer capabilities. */

#define  FOUND       1             /* Used for matching printer capabilities. */
#define  NOTFOUND    -1

struct sparams                                     /* Video parameters. */
         {
           int width ;                             /* Screen width in pixels. */
           int height ;                            /* Screen height in pixels. */
           int depth ;                             /* Screen depth in pixels. */
           int csize ;                             /* Screen colormap size. */
           int fbsize ;                            /* Frame buffer size. */
           int scr[MAXHEIGHT][MAXWIDTH*MAXDEPTH / B_CH] ;
         } ;

struct dumpcap                     /* Dumpcap internal record structure. */
         {
           char name[3] ;
           int type ;
           int num ;
           char str[MAXSTR] ;
         } ;

/* Variable printer aliases. */
#define  BR  dc[0].num            /* Printer baud rate. */
#define  CO  dc[1].num            /* Number of colors in output. */
#define  DD  dc[2].num            /* Double size screen image. */
#define  ED  dc[3].str            /* End of dump output sequence. */
#define  EL  dc[4].str            /* End of 'line' output sequence. */
#define  FN  dc[5].str            /* Screen image file to use. */
#define  HA  dc[6].num            /* Sun/printer handshake type. */
#define  LP  dc[7].str            /* Output device name. */
#define  MA  dc[8].num            /* Image data bit mask. */
#define  NB  dc[9].num            /* Bits/byte of image output. */
#define  RO  dc[10].num           /* Reverse order of bits in graphics byte. */
#define  SB  dc[11].str           /* Start of burst page sequence. */
#define  SD  dc[12].str           /* Start of dump output sequence */
#define  SH  dc[13].num           /* Height of screen dump. */
#define  SL  dc[14].str           /* Start of 'line' output sequence. */
#define  SW  dc[15].num           /* Width of screen dump. */
#define  SX  dc[16].num           /* X origin of screen dump. */
#define  SY  dc[17].num           /* Y origin of screen dump. */
#define  VS  dc[18].num           /* Vertical printer lines per pass. */
