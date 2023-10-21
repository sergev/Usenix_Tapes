
/*  sidtool.h
 *
 *  Definitions used by the SID tool.
 *  Written by Rich Burridge - SUN Australia, June 1986.
 *
 *  Version 1.5
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

typedef int BOOLEAN ;
char *sprintf() ;

#define  BIND           (void) bind       /* To make lint happy. */
#define  CLOSE          (void) close
#define  CONNECT        (void) connect
#define  DOINC          (void) doinc
#define  FCLOSE         (void) fclose
#define  FGETS          (void) fgets
#define  FPRINTF        (void) fprintf
#define  FSCANF         (void) fscanf
#define  GETLINE        (void) getline
#define  IOCTL          (void) ioctl
#define  KILL           (void) kill
#define  LISTEN         (void) listen
#define  READ           (void) read
#define  SELECT         (void) select
#define  SPRINTF        (void) sprintf
#define  STAT           (void) stat
#define  STRCPY         (void) strcpy
#define  UNLINK         (void) unlink
#define  WRITE          (void) write

#define  SQUARE         26                       /* Size of each square of the maze. */
#define  XBASE          45                       /* X start of maze. */
#define  YBASE          100                      /* Y start of maze. */

/* Convert from maze coordinates to screen coordinates. */
#define  TRANSPT(mx,my,scrx,scry) { scrx = (mx - 1) * SQUARE + XBASE ; \
                                    scry = (my - 1) * SQUARE + YBASE ; }

/* Convert from screen coordinates to maze coordinates. */
#define  UNTRANSPT(scrx,scry,mx,my) { mx = ((scrx - XBASE) / SQUARE) + 1 ; \
                                      my = ((scry - YBASE) / SQUARE) + 1 ; }

/* Wait for len tics. */
#define  PPAUSE(len) { int ppi ; for (ppi = 0; ppi < len; ppi++) ; }

/* Wait for a while and check keyboard for commands. */
#define  LONGPAUSE() { int loi ; for (loi = 0; loi < 5000; loi++) ; keyinput(FALSE) ; }

/* Returns the reverse direction of the parameter (left goes to right, etc.) */
#define  REVERSEDIR(dir) ((dir + 2) % 4)

/* Returns true if the screen position is in the center of a square. */
#define  GCENTERED(scrx,scry) (((scrx - XBASE) % SQUARE == 0) && \
                               ((scry - YBASE) % SQUARE == 0))

/* Set the function to be used for characters. */
#define  SCHRFUNC(f)  (sfunc = f)

/* Determine the bug index, POKEY to SHADOW. */
#define  GIND(x) ((x) - &bugs[0])

/* Machine independent rasterop calls. */

/* Manipulate a portion of the screen with itself. */
#define  BLT_SCRN(sx,sy,w,h,op) \
         (pw_writebackground(gfx->gfx_pixwin,sx,sy,w,h,op))

/* Move an offscreen raster area to the screen. */
#define  BLT_MEM_TO_SCRN(sx,sy,w,h,op,mem,mx,my) \
         (pw_write(gfx->gfx_pixwin,sx,sy,w,h,op,mem,mx,my))

/* Move an offscreen raster area to another offscreen raster area. */
#define  BLT_MEM(mem1,mx1,my1,w,h,op,mem2,mx2,my2) \
         (pr_rop(mem1,mx1,my1,w,h,op,mem2,mx2,my2))


#define  MAXLINE        80                       /* Maximum string length. */

#define  OFFCURSOR      0                        /* Mouse cursor modes. */
#define  TRACKCURSOR    1

#define  KEY_SET        0                        /* Used for function key setup. */
#define  KEY_RESET      1

#define  SOCKNAME       "/tmp/sidsocket"         /* Demonstration socket name. */
#define  SWIDTH         768                      /* Maximum screen width. */
#define  SHEIGHT        900                      /* Maximum screen height. */

#define  YSIZE          28                       /* Number of squares in y. */
#define  XSIZE          26                       /* Number of squares in x. */
#define  GOFFSET        SQUARE / 2 - 3           /* Offset of bugs and screen. */
#define  FRUITMX        13                       /* X maze position of fruit. */
#define  FRUITMY        16                       /* Y maze position of fruit. */
#define  CATCHUP        3                        /* Amount screen catches up blue bug. */
#define  ENKEY          01652                    /* Encode key used in highscore file. */
#define  DEL            127                      /* Used to start the SID tool. */
#define  BSPACE          8                       /* Backspace used by getline. */

#ifndef  CTRLQ
#define  CTRLQ          17                       /* Used to restart SID tool. */
#endif
#ifndef  CTRLS
#define  CTRLS          19                       /* Used to halt the SID tool. */
#endif

#define  CR             13
#define  MAXNUMPLAYERS  4                        /* Number of players allowed. */
#define  MINMOVE        20

#define  MASTER         0                        /* Server version of SID tool. */
#define  SLAVE          1                        /* Client version of SID tool. */ 

#define  BUTXOFF        30                       /* X offset of buttons. */
#define  BUTYOFF        10                       /* Y offset of buttons. */
#define  WIN_DAMAGED    '\01'                    /* Has window been damaged? */

#define  TRUE           1
#define  FALSE          0

#define  BUT_AUTO       0                        /* Control menu buttons. */
#define  BUT_HELP       1
#define  BUT_LEVEL      2
#define  BUT_PLAYERS    3
#define  BUT_QUIT       4
#define  BUT_SCORES     5
#define  BUT_START      6

#define  UR             0                        /* Corners. */
#define  RD             1
#define  DL             2
#define  LU             3

#define  NODOT          0                        /* Dotsize. */
#define  BIGDOT         1
#define  SMALLDOT       2

#define  RIGHT          0                        /* Direction. */
#define  UP             1
#define  LEFT           2
#define  DOWN           3

#define  POKEY          0                        /* Ghostnames. */
#define  BASHFUL        1
#define  SPEEDY         2
#define  SHADOW         3

struct scorerec
         {
           char who[MAXLINE] ;
           int score ;
         } ;

struct bugrec
         {
           int dir ;
           BOOLEAN eyesonly ;      /* Going to box. */
           BOOLEAN enteringbox ;   /* Going down into box. */
           BOOLEAN inbox ;         /* Inside or leaving. */
           BOOLEAN intunnel ;
           int delay ;
           int scrx,scry ;
           int mx,my ;
           int bluetime ;      /* If zero then not blue. */
           int boxtime ;       /* Countdown until leave. */
           int count ;         /* Incremented every tic. */
           int pic ;           /* 0 or 1. */
         } ;

struct startrec
         {
           int x,y,time ;
         } ;
