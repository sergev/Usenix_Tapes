
/*  sidtool.h
 *
 *  Definitions used by the SID tool.
 *  Written by Rich Burridge - SUN Microsystems Australia (Melbourne).
 *
 *  Version 2.1.  -  April 1987.
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

/*  These are the variables you might wish to change at compile time, or they
 *  can be overridden at run time by command line options or environment
 *  variables.
 */

#define  A_NAME         "sidtool.animate"        /* Default animate filename. */
#define  H_NAME         "sidtool.help"           /* Defaults help filename. */
#define  M_NAME         "sidtool.maze"           /* Default maze filename. */
#define  SPEED          3                        /* Speed of this Sun. */
#define  S_NAME         "sidtool.hs"             /* Default highscore filename. */

typedef int BOOLEAN ;
char *sprintf() ;

#define  BIND           (void) bind              /* To make lint happy. */
#define  CLOSE          (void) close
#define  CONNECT        (void) connect
#define  DOINC          (void) doinc
#define  FCLOSE         (void) fclose
#define  FCNTL          (void) fcntl
#define  FGETS          (void) fgets
#define  FPRINTF        (void) fprintf
#define  FSCANF         (void) fscanf
#define  IOCTL          (void) ioctl
#define  KILL           (void) kill
#define  LISTEN         (void) listen
#define  READ           (void) read
#define  SSCANF         (void) sscanf
#define  SELECT         (void) select
#define  SETJMP         (void) setjmp
#define  SPRINTF        (void) sprintf
#define  STAT           (void) stat
#define  STRCPY         (void) strcpy
#define  UNLINK         (void) unlink
#define  WRITE          (void) write


#define  SQUARE         26        /* Size of each square of the maze. */
#define  XBASE          45        /* X start of maze. */
#define  YBASE          100       /* Y start of maze. */

/* Convert from maze coordinates to screen coordinates. */
#define  TRANSPT(mx,my,scrx,scry) { scrx = (mx - 1) * SQUARE + XBASE ; \
                                    scry = (my - 1) * SQUARE + YBASE ; }

/* Convert from screen coordinates to maze coordinates. */
#define  UNTRANSPT(scrx,scry,mx,my) { mx = ((scrx - XBASE) / SQUARE) + 1 ; \
                                      my = ((scry - YBASE) / SQUARE) + 1 ; }

/* Wait for len tics. */
#define  PPAUSE(len) { int ppi ; for (ppi = 0; ppi < len; ppi++) ; }

/* Wait for a while and check keyboard for commands. */
#define  LONGPAUSE() { int loi ; for (loi = 0; loi < (1000*speed); loi++) ; }

/* Returns the reverse direction of the parameter (left goes to right, etc.) */
#define  REVERSEDIR(dir) ((dir + 2) % 4)

/* Returns true if the screen position is in the center of a square. */
#define  GCENTERED(scrx,scry) (((scrx - XBASE) % SQUARE == 0) && \
                               ((scry - YBASE) % SQUARE == 0))

/* Set the function to be used for characters. */
#define  SCHRFUNC(f)  (sfunc = f)

/* Determine the bug index, POKEY to SHADOW. */
#define  GIND(x) ((x) - &bugs[0])

#define  OFFCURSOR      0         /* Mouse cursor modes. */
#define  TRACKCURSOR    1

#define  KEY_SET        0         /* Used for function key setup. */
#define  KEY_RESET      1

#define  BSPACE         8         /* Backspace used by getline. */
#define  BUTXOFF        30        /* X offset of buttons. */
#define  BUTYOFF        10        /* Y offset of buttons. */
#define  CATCHUP        3         /* Amount screen catches up blue bug. */
#define  CR             13
#define  DEL            127       /* Used to start the sid tool game. */
#define  ENKEY          01652     /* Encode key used in highscore file. */
#define  FRUITMX        13        /* X maze position of fruit. */
#define  FRUITMY        16        /* Y maze position of fruit. */
#define  GOFFSET        SQUARE / 2 - 3           /* Offset of bugs and screen. */
#define  MAXNUMPLAYERS  4         /* Number of players allowed. */
#define  MAXLINE        80        /* Maximum string length. */
#define  MINMOVE        20
#define  SWIDTH         768       /* Maximum screen width. */
#define  SHEIGHT        900       /* Maximum screen height. */
#define  XSIZE          26        /* Number of squares in x. */
#define  YSIZE          28        /* Number of squares in y. */

/* States for the Sid Tool automation. */
#define  STARTUP        0         /* Define setjmp variable. */
#define  INITGAME       1         /* Initialise start of game variables. */
#define  PLAY           2         /* Start play mode. */
#define  DOPLAY         3         /* Jump here if been eaten, or starting new game. */
#define  MAKEPLAY       4         /* Perform next movement of each Sid Tool object. */
#define  DOREST         5         /* Initial routine for credits and button actions.*/
#define  HIGHSCORE      6         /* Initial routine for getting the highscores. */
#define  NEXTLINE       7         /* Get a user name for the new high score. */
#define  DOCREDIT       8         /* Initial routine for display the credits. */
#define  MOVELEFT       9         /* Animate screen left during credits. */
#define  MOVERIGHT      10        /* Animate screen right during credits. */
#define  DELHIT         11        /* Del key has been pressed. */
#define  GETBUT         12        /* Get a pseudo-button press from the user. */
#define  MAKESEL        13
#define  DOLEAVE        14
#define  RESETGAME      15
#define  CTRLSHIT       16        /* ^S has been hit, do nothing until ^Q. */

#ifndef  CTRLQ
#define  CTRLQ          17        /* Used to restart SID tool. */
#endif
#ifndef  CTRLS
#define  CTRLS          19        /* Used to halt the SID tool. */
#endif

#define  TRUE           1
#define  FALSE          0

#define  BUT_AUTO       0         /* Control menu buttons. */
#define  BUT_HELP       1
#define  BUT_LEVEL      2
#define  BUT_PLAYERS    3
#define  BUT_QUIT       4
#define  BUT_SCORES     5
#define  BUT_START      6

#define  UR             0         /* Corners. */
#define  RD             1
#define  DL             2
#define  LU             3

#define  NODOT          0         /* Dotsize. */
#define  BIGDOT         1
#define  SMALLDOT       2

#define  RIGHT          0         /* Direction. */
#define  UP             1
#define  LEFT           2
#define  DOWN           3

#define  POKEY          0         /* Ghostnames. */
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
           int bluetime ;          /* If zero then not blue. */
           int boxtime ;           /* Countdown until leave. */
           int count ;             /* Incremented every tic. */
           int pic ;               /* 0 or 1. */
         } ;

struct startrec
         {
           int x,y,time ;
         } ;
