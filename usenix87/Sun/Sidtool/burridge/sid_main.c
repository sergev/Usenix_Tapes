 
/*  sid_main.c
 *
 *  Sid Tool - the Sun Interactive Debugger program.
 *
 *  Written by Rich Burridge - Sun Microsystems Australia (Melbourne).
 *
 *  Version 2.1.  -  April 1987.
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include <setjmp.h>
#include <sys/fcntl.h>
#include "bltstuff.h"
#include "patchlevel.h"
#include "sidtool.h"
#include <suntool/sunview.h>
#include <suntool/canvas.h>

Canvas canvas ;
Frame base_frame ;
Pixwin *pw ;

jmp_buf exception ;
int val ;

extern etext() ;
extern restore_screen() ;

Notify_value main_loop() ;
void event_proc() ;

short sid_image[] = {
#include "sidtool.icon"
} ;
DEFINE_ICON_FROM_IMAGE(sid_icon,sid_image) ;

struct scorerec allhighscores[11] ;
struct startrec startpos[4] ;
struct bugrec bugs[4] ;

char but_names[7][8] =               /*  Control panel stuff. */
     {
       " Auto  ",
       " Help  ",
       " Level ",
       "Players",
       " Quit  ",
       " Scores",
       " Start "
     } ;

char names[4][MAXLINE] =
     {
       "Time Dependencies",
       "Uninitialized Variables",
       "Fence Posts",
       "Multiple Process Interaction"
     } ;

char old_key_vals[9][MAXLINE] ;       /* Function key string values to save. */
char new_key_vals[9][MAXLINE] =       /* Function key values used by SIDtool. */
     {
       "",                   /* R7 */
       "u",                  /* R8 */
       "",                   /* R9 */
       "l",                  /* R10 */
       "",                   /* R11 */
       "r",                  /* R12 */
       "",                   /* R13 */
       "d",                  /* R14 */
       ""                    /* R15 */
     } ;

char maze[XSIZE+2][YSIZE+2] ;
char sc,buffer[MAXLINE] ;
char s_name[MAXLINE] ;          /* Score file name. */
char a_name[MAXLINE] ;          /* Animate file name. */
char h_name[MAXLINE] ;          /* Help file name. */
char m_name[MAXLINE] ;          /* Maze file name. */
char thisscore[MAXLINE] ;       /* User name for new high score. */
char titlestring[MAXLINE] ;

int blueblink,blueincblink,boxx,boxy,fruittime,fruitx,fruity,numplayers ;
int skilllevel,circatchup,pausetime,highplayer,autoscore,lastnumplayers ;
int curbluetime[MAXNUMPLAYERS+1],score[MAXNUMPLAYERS+1] ;
int numcir[MAXNUMPLAYERS+1],fruitmaze[MAXNUMPLAYERS+1] ;
int numdots[MAXNUMPLAYERS+1],fruitchances[MAXNUMPLAYERS+1] ;
int fruitsgotten[MAXNUMPLAYERS+1][9] ;
int highscore,player,cirmx,cirmy,bugssincedot ;
int walls[XSIZE+6][YSIZE+1] ;
int dots[MAXNUMPLAYERS+1][XSIZE+4][YSIZE+2] ;
int tunnel[XSIZE+4][YSIZE+2] ;

int key_stations[9] = {68, 69, 70, 91, 92, 93, 112, 113, 114} ;

int button ;            /* Indicates users selection from control panel. */
int c ;                 /* Contains latest mouse or keyboard interaction. */
int canvasfd ;          /* File descriptor for canvas subwindow. */
int curdir ;            /* Current direction of the screen. */
int oldcurdir ;         /* Old direction of the screen. */
int g,newdir,posx,posy,x,y,nx,ny,count,inc ;
int orgx,orgy,width,height ;    /* Position and dimension of window. */
int oldcx,oldcy ;       /* Old position of the screen. */
int on = 0 ;            /* Current blinking state of score. */
int canvasflags ;       /* Used to setup no delay for canvas. */
int credits ;           /* Direction of credits if on. */
int cirx ;              /* X position of screen during credits. */
int ciry ;              /* Y position of screen during credits. */
int dotx ;              /* X position of BIGDOT during credits. */
int doty ;              /* Y position of BIGDOT during credits. */
int movei,movej,movex ; /* Used to animate screen during credits. */
int progstate ;         /* State machine for main loop. */
int redraw;             /* If non-zero, then screen should be redrawn. */
int savedstate ;        /* State machine value after Ctrl S. */
int scorei ;            /* No of chars in high score user name. */
int speed ;             /* Class (speed) of this Sun machine. */
int started ;           /* Indicates if we have started a game. */

BOOLEAN autoplay ;
BOOLEAN demomode ;
BOOLEAN fruiton ;
BOOLEAN gamestate ;             /* State of the game, 1 = remove circle. */
BOOLEAN remove ;                /* Whether Sun screen should be removed. */
 
FILE *fopen() ;

extern int sfunc ;              /* Used by SCHRFUNC for cursor function. */
extern Pixfont *pf ;


drawbox(mx,my)
int mx,my ;

/*  Draws a box starting at maze position mx, my. Mx, my should be an
 *  s or S position in maze. Travels around path until reach x or s.
 */

{
  int last ;

  last = 'r' ;
  PPAUSE(pausetime*20) ;
  walls[mx+2][my] = 1 ;
  if (maze[mx][my] == 's') drawcorner(mx,my,UR) ;
  else if (maze[mx][my] == 'S') ddrawline(mx,my,'r') ;
  else if (maze[mx][my] == 'T')
    {
      ddrawline(mx,my,'l') ;
      mx -= 2 ;
      last = 'l' ;
    }
   mx++ ;
   for (;;)
     {
       PPAUSE(2*pausetime) ;
       walls[mx+2][my] = 1 ;
       switch (maze[mx][my])
         {
           case 's' :
           case 'S' :
           case 'T' : return ;
           case 'd' : if (last == 'r') drawcorner(mx,my,RD) ;
                      else if (last == 'l') drawcorner(mx,my,UR) ;
                      else ddrawline(mx,my,'d') ;
                      last = 'd' ;
                      my++ ;
                      break ;
           case 'l' : if (last == 'd') drawcorner(mx,my,DL) ;
                      else if (last == 'u') drawcorner(mx,my,RD) ;
                      else ddrawline(mx,my,'l') ;
                      last = 'l' ;
                      mx-- ;
                      break ;
           case 'r' :
           case 'R' : if (last == 'u') drawcorner(mx,my,UR) ;
                      else if (last == 'd') drawcorner(mx,my,LU) ;
                      else ddrawline(mx,my,maze[mx][my]) ;
                      last = 'r' ;
                      mx++ ;
                      break ;
           case 'u' : if (last == 'l') drawcorner(mx,my,LU) ;
                      else if (last == 'r') drawcorner(mx,my,DL) ;
                      else ddrawline(mx,my,'u') ;
                      last = 'u' ;
                      my-- ;
                      break ;
           case 'x' : ddrawline(mx,my,last) ;
                      return ;
         }
      }   
}


setdots(player)
int player ;

{
  int x,y ;

  for (y = 1; y <= YSIZE; y++)
    {
      dots[player][0][y] = NODOT ;
      dots[player][1][y] = NODOT ;
      dots[player][XSIZE+2][y] = NODOT ;
      dots[player][XSIZE+3][y] = NODOT ;
      for (x = 1; x <= XSIZE; x++)
        if (maze[x][y] == '.')
          {
            dots[player][x+1][y] = SMALLDOT ;
            numdots[player]++ ;
          }
        else if (maze[x][y] == '*')
          {
            dots[player][x+1][y] = BIGDOT ;
            numdots[player]++ ;
          }
        else dots[player][x+1][y] = NODOT ;
    }
}


readallhighscores()

/*  Reads all high scores and names into the global table allhighscores.
 *  If file not found, then sets all high scores to zero.
 */

{
  int hsfile,level ;

  if ((hsfile = open(s_name,2)) == -1)
    {
      if ((hsfile = creat(s_name,0777)) == -1)
        {
          FPRINTF(stderr,"sidtool: unable to create highscores file.\n") ;
          return ;
        }

      for (level = 0; level <= 10; level++)
        {
          allhighscores[level].score = 0 ;
          STRCPY(allhighscores[level].who," ") ;
          puths(hsfile,allhighscores[level]) ;
        }
    }
  else
    for (level = 1; level <= 10; level++) geths(hsfile,&allhighscores[level]) ;
  CLOSE(hsfile) ;
}


writehighscore()

/*  If highscore is better than old high score for this skill level then
 *  asks for player's name and enters name and score into table and writes file.
 */

{
  if ((highscore >= allhighscores[skilllevel].score) &&
        (highplayer != -1) && (!demomode))
    {
      clear_screen() ;
      SPRINTF(buffer,"Player %1d has beaten the high score for skill level %1d.",
                     highplayer,skilllevel) ;
      WRITELN(100,140,buffer) ;
      if (allhighscores[skilllevel].score)
        {
          SPRINTF(buffer,"The old record was %1d0 held by: %s.",
                         allhighscores[skilllevel].score,
                         allhighscores[skilllevel].who) ;
          WRITELN(100,160,buffer) ;
        }
      SPRINTF(buffer,"Type player %1d's name or initials: ",highplayer) ;
      WRITELN(100,200,buffer) ;
      scorei = 0 ;
      c = 0 ;
      thisscore[scorei] = '_' ;
      thisscore[scorei+1] = '\0' ;
      WRITELN(370,200,thisscore) ;
      progstate = NEXTLINE ;
    }
  else progstate = DOCREDIT ;
}


getnewscore(x,y)      /* Get new user name for highscore. */
int x,y ;

{
  if (c)
    {
      switch (c)
        {
          case BSPACE :
          case DEL    : if (scorei)
                          {   
                            scorei-- ;
                            thisscore[scorei] = ' ' ;
                            thisscore[scorei+1] = '\0' ;
                            WRITELN(x,y,thisscore) ;
                            thisscore[scorei] = '\0' ;
                          } 
                        break ;
          case CR     : thisscore[scorei] = '\0' ;
                        if (!scorei) WRITELN(100,220,"   ** No name given. **") ;
                        else
                          {
                            STRCPY(allhighscores[skilllevel].who,thisscore) ;
                            savescorefile() ;
                            progstate = DOCREDIT ;
                          }
                        break ;
          default     : if (c < ' ') break ;
                        thisscore[scorei++] = c ;
                        thisscore[scorei] = '\0' ;
                        WRITELN(x,y,thisscore) ;
        }
      c = 0 ;
    }
}


savescorefile()     /* Write away new highscore values. */

{
  int fd,level ;

  allhighscores[skilllevel].score = highscore ;
  if ((fd = open(s_name,1)) == -1)
    FPRINTF(stderr,"sidtool: unable to open highscores file.\n") ;
  else
    { 
      for (level = 1; level <= 10; level++) puths(fd,allhighscores[level]) ;
      CLOSE(fd) ;
    }
}


initgame()

{
  int i,j ;

  if (autoplay) autoscore = 0 ;
  pausetime = -skilllevel * 20 + (speed * 100) ;
  circatchup = -skilllevel * 4 + 46 ;
  highplayer = -1 ;
  for (j = 1; j < MAXNUMPLAYERS; j++)
    {
      numdots[j] = 0 ;
      numcir[j] = 3 ;
      fruitchances[j] = 0 ;
      setdots(j) ;
      curbluetime[j] = 1 + (-skilllevel * 60 + 900) ;
      if (!autoplay)
        {
          score[j] = 0 ;
          if (demomode) fruitmaze[j] = 8 ;
          else fruitmaze[j] = 1 ;
          for (i = 1; i < 8; i++)
            if (demomode) fruitsgotten[j][i] = 1 ;
            else fruitsgotten[j][i] = 0 ;
        }
    }
}


showplayerscore(player)
int player ;

{
  int x,y ;

  SCHRFUNC(RXOR) ;
  x = (player % 2) ? 190 : 570 ;
  y = (player < 3) ? 25  : 65 ;
  SPRINTF(buffer,"  Player %1d ",player) ;
  WRITELN(x,y,buffer) ;
  if (!score[player]) WRITELN(x+27,y+15,"0") ;
  else
    {
      SPRINTF(buffer,"%1d0",score[player]) ;
      WRITELN(x+27,y+15,buffer) ;
    }
  SCHRFUNC(RRPL) ;
}


blinkpause()

/*  Wait a while and check keyboard for commands while blinking current players score. */

{
  int i,j ;

  on = 1 ;
  if (!autoplay)
    for (i = 1; i <= 16; i++)
      {
        showplayerscore(player) ;
        on = !on ;
        for (j = 0; j < 10; j++) LONGPAUSE() ;
      }
}


doinc(dir,posx,posy,mx,my,x,y,nx,ny)
int dir,posx,posy,mx,my,*x,*y,*nx,*ny ;

{
  register int status,tx,ty ;

  *x = posx ;
  *y = posy ;
  tx = mx ;
  ty = my ;

  switch (dir)
    {
      case UP    : *y = posy - 2 ;
                   ty = my - 1 ;
                   break ;
      case DOWN  : *y = posy + 2 ;
                   ty = my + 1 ;
                   break ;
      case LEFT  : *x = posx - 2 ;
                   tx = mx - 1 ;
                   break ;
      case RIGHT : *x = posx + 2 ;
                   tx = mx + 1 ;
                   break ;
    }
  UNTRANSPT(*x,*y,*nx,*ny) ;
  if (tx == -2) tx = XSIZE + 2 ;
  else if (tx == XSIZE + 3) tx = -1 ;
  status = 1 ;

  if ((*nx == -2) && (dir == LEFT))
    {
      *nx = XSIZE + 2 ;
      TRANSPT(*nx,*ny,*x,*y) ;
    }
  else if ((*nx == XSIZE + 3) && (dir == RIGHT))
    {
      *nx = -1 ;
      TRANSPT(*nx,*ny,*x,*y) ;
    }
  else if (!(walls[*nx+2][*ny] ||
            (GCENTERED(posx,posy) && walls[tx+2][ty]))) /* do nothing. */ ;
  else status = 0 ;
  return(status) ;
}


headto(destx,desty,scrx,scry,mx,my,dir,x,y,nx,ny)   /* Only called when GCENTERED. */
int *dir,destx,desty,scrx,scry,mx,my,*x,*y,*nx,*ny ;

{
  int dirar[5],rev,i,s,xinc,yinc ;

  rev = REVERSEDIR(*dir) ;
  xinc = mx - destx ;
  yinc = my - desty ;
  if (abs(xinc) > abs(yinc)) s = 2 ;
  else s = 1 ;
  if (xinc < 0)
    {
      dirar[3-s] = RIGHT ;
      dirar[s+2] = LEFT ;
    }
  else
    {
      dirar[3-s] = LEFT ;
      dirar[s+2] = RIGHT ;
    }
  if (yinc < 0)
    {
      dirar[s] = DOWN ;
      dirar[5-s] = UP ;
    }
  else
    {
      dirar[s] = UP ;
      dirar[5-s] = DOWN ;
    }

  for (i = 1; i <= 4; i++)     /* Adjust so reverse is last choice. */
    if (dirar[i] == rev)
      {
        for (s = i; s <= 3; s++) dirar[s] = dirar[s+1] ;
        dirar[4] = rev ;
        break ;
      }

  for (s = 1; s <= 4; s++)
    {
      if (checkinc(dirar[s],mx,my))
        {
          *dir = dirar[s] ;
          DOINC(*dir,scrx,scry,mx,my,x,y,nx,ny) ;
          return ;
        }
    }
}


dorandomdir(dir,scrx,scry,mx,my,x,y,nx,ny,ranrange)
int dir,scrx,scry,mx,my,*x,*y,*nx,*ny,ranrange ;

{
  int i,test,newdir,rev,status ;

  test = randomrange(1,ranrange) ;
  rev = REVERSEDIR(dir) ;
  if ((test == 1) || (!checkinc(dir,mx,my)))
    {
      newdir = randomrange(0,3) ;
      for (i = 0; i <= 3; i++)
        {
          if (newdir != rev)
            if (checkinc(newdir,mx,my))
              {
                status = newdir ;
                DOINC(newdir,scrx,scry,mx,my,x,y,nx,ny) ;
                return(status) ;
              }
          newdir = (newdir + 1) % 4 ;
        }
    }
  else
    {
      DOINC(dir,scrx,scry,mx,my,x,y,nx,ny) ;
      status = dir ;
    }
  return(status) ;
}


updatebugs()

/* Move each bug one bit in appropriate direction; change direction if appropriate. */

{
  register struct bugrec *g ;
  int bemean ;

  for (g = &bugs[POKEY]; g <= &bugs[SHADOW]; g++)
    {
      g->count++ ;
      if (g->inbox || g->intunnel)
        if (g->count % 2 == 0) return ;               /* Slow in box. */

      if (g->bluetime > 0)
        {
          if (g->count % CATCHUP == 0) return ;       /* Go slower if blue. */
          drawbug(g) ;                        /* Erase old before change blueTime. */
          g->bluetime-- ;
        }
      else drawbug(g) ;                    /* Erase old. */

      if (g->count % 7 == 0) g->pic = (g->pic + 1) % 2 ;

      if (GCENTERED(g->scrx,g->scry))
        {
          g->intunnel = tunnel[g->mx+1][g->my] ;

          if (!g->bluetime)
            if (skilllevel < 5)
              bemean = randomrange(0,10-skilllevel-GIND(g)) == 0 ;
            else bemean = randomrange(0,skilllevel-5+GIND(g)) != 0 ;
          else bemean = FALSE ;

          if (g->inbox)
            if ((g->mx == boxx) && (g->my == boxy)) g->inbox = FALSE ;

          if (g->eyesonly)
            {
              if ((!g->enteringbox) && (g->mx == boxx) && (g->my == boxy))
                {
                  g->dir = DOWN ;
                  g->enteringbox = TRUE ;
                  DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ; 
                }
              else if (g->enteringbox)
                if ((g->my > boxy + 2) &&
                    (!doinc(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny)))
                  {
                    g->dir = UP ;
                    g->enteringbox = FALSE ;
                    g->inbox = TRUE ;
                    g->eyesonly = FALSE ;
                    DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ;
                  }
                else DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ;
              else headto(boxx,boxy,g->scrx,g->scry,g->mx,g->my,&g->dir,&x,&y,&nx,&ny) ;
            }
          else if (g->boxtime)          /* Inbox should be true also. */
            {
              g->boxtime-- ;
              if (g->boxtime < 0)       /* Heading to exit. */
                {
                  if (g->mx == boxx)    /* Found exit. */
                    {
                      g->boxtime = 0 ;
                      g->dir = UP ;
                      DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ;
                    }
                  else headto(boxx,boxy,g->scrx,g->scry,g->mx,g->my,&g->dir,&x,&y,&nx,&ny) ;
                }
              else if (!g->boxtime)      /* Start heading to exit. */
                {
                  g->boxtime = -1 ;
                  headto(boxx,boxy,g->scrx,g->scry,g->mx,g->my,&g->dir,&x,&y,&nx,&ny) ;
                }
              else if (!doinc(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny))
                {
                  g->dir = REVERSEDIR(g->dir) ; /* Bounce up a down a while. */
                  DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ;
                }
            }
           else if (g->inbox)     /* Must be leaving the box; just keep going. */
                  DOINC(g->dir,g->scrx,g->scry,
                        g->mx,g->my,&x,&y,&nx,&ny) ;
           else if (bemean)              /* Chase the circle. */
             headto(cirmx,cirmy,g->scrx,g->scry,g->mx,g->my,&g->dir,&x,&y,&nx,&ny) ;
           else g->dir = dorandomdir(g->dir,g->scrx,g->scry,
                                            g->mx,g->my,&x,&y,&nx,&ny,3) ;
         }
       else DOINC(g->dir,g->scrx,g->scry,g->mx,g->my,&x,&y,&nx,&ny) ;

       g->scrx = x ;
       g->scry = y ;
       g->mx = nx ;
       g->my = ny ;
       drawbug(g) ;                /* Draw new. */
    }
}


main(argc,argv)
int argc ;
char *argv[] ;

{
  get_options(argc,argv) ;          /* Get command line options. */
  function_keys(KEY_SET) ;          /* Set direction arrow function keys. */

  base_frame = window_create(NULL, FRAME,
                             FRAME_LABEL, titlestring,
                             FRAME_ICON, &sid_icon,
                             WIN_X, orgx,
                             WIN_Y, orgy,
                             WIN_WIDTH, width,
                             WIN_HEIGHT, height,
                             FRAME_ARGS, argc, argv,
                             0) ;
  canvas = window_create(base_frame, CANVAS,
                         CANVAS_RETAINED, FALSE,
                         CANVAS_FAST_MONO, TRUE,
                         CANVAS_REPAINT_PROC, restore_screen,
                         WIN_EVENT_PROC, event_proc,
                         0) ;

  window_set(canvas, WIN_CONSUME_KBD_EVENTS, WIN_ASCII_EVENTS, 0) ;
  window_set(canvas, WIN_CONSUME_KBD_EVENTS, WIN_LEFT_KEYS, 0) ;
  window_set(canvas,WIN_IGNORE_PICK_EVENT,LOC_MOVE,0) ;

  pf = pf_default() ;
  pw = canvas_pixwin(canvas) ;

/* Set up no delay for events within the canvas. */
  canvasfd = (int) window_get(canvas,WIN_FD) ;
  canvasflags = fcntl(canvasfd,F_GETFL,0) ;
  canvasflags |= FNDELAY ;
  FCNTL(canvasfd,F_SETFL,canvasflags) ;

  sfunc = PIX_SRC ;                  /* Used by WRITELN. */
  iocursormode(OFFCURSOR) ;

  initrandom() ;
  numplayers = 1 ;
  skilllevel = 5 ;
  lastnumplayers = 1 ;
  autoplay = FALSE ;
  initgame() ;
  initialize() ;
  redraw = 0 ;                       /* Don't redraw the screen, the first time. */
  started = 1 ;
  autoplay = 1 ;
  progstate = STARTUP ;

  (void) notify_set_itimer_func(base_frame, main_loop, ITIMER_REAL,
                        &NOTIFY_POLLING_ITIMER, ((struct itimerval *) 0)) ;
  window_main_loop(base_frame) ;
  function_keys(KEY_RESET) ;         /* Restore direction arrow function keys. */
  exit(0) ;
}


/*ARGSUSED*/
void
event_proc(window,event,arg)
Window *window ;
Event *event ;
caddr_t arg ;

{
  int x,y ;     /* Position of mouse when button pressed. */
  int i ;

  if (event_is_ascii(event))
    {
      c = event_id(event) ;
      if (progstate == NEXTLINE) return ;
      if ((c == DEL) && !demomode)
        {
          c = 0 ;
          started = 0 ;
          progstate = DELHIT ;
          return ;
        }
      else if (c == CTRLS)
        {
          savedstate = progstate ;
          progstate = CTRLSHIT ;
          return ;
        }
      else if (c == CTRLQ)
        {
          progstate = savedstate ;
          return ;
        }
      else if (!autoplay) sc = c ;
    }
  else if (event_is_down(event) && event_is_button(event))
    {
      x = event_x(event) ;
      y = event_y(event) ;
      if (y > BUTYOFF && y < BUTYOFF+2*SQUARE)
        for (i = BUT_AUTO; i <= BUT_START; i++)
          if (x > BUTXOFF+i*100 && x < BUTXOFF+i*100+120) c = i+2 ;
    }
}


/*ARGSUSED*/
static Notify_value
main_loop(client, itimer_type)
Notify_client client ;
int itimer_type ;

{
  int i ;

  switch (progstate)
    {
      case STARTUP   : SETJMP(exception) ;
                       if (progstate) break ;
                       else progstate = INITGAME ;
                       break ;
      case INITGAME  : initgame() ;
                       progstate = PLAY ;
                       break ;
      case PLAY      : play() ;
                       progstate = DOPLAY ;
                       break ;
      case DOPLAY    : doplay() ;
                       progstate = MAKEPLAY ;
                       break ;
      case MAKEPLAY  : make_play() ;
                       break ;
      case DOREST    : numplayers = 1 ;
                       progstate = HIGHSCORE ;
                       break ;
      case HIGHSCORE : writehighscore() ;
                       break ;
      case NEXTLINE  : getnewscore(365,200) ;
                       break ;
      case DOCREDIT  : if (!demomode)
                         {
                           dohighscores() ;
                           WRITELN(5,860,"Type DEL to begin") ;
                           for (i = 1; i <= 200; i++) LONGPAUSE() ;
                         }
                       docredits() ;
                       break ;
      case MOVELEFT  : move_left() ;
                       break ;
      case MOVERIGHT : move_right() ;
                       break ;
      case DELHIT    : autoplay = FALSE ;
                       iocursormode(TRACKCURSOR) ;
                       make_control_panel() ;
                       display_settings() ;
                       progstate = GETBUT ;
                       break ;
      case GETBUT    : get_button_option() ;
                       break ;
      case MAKESEL   : make_selection() ;
                       break ;
      case DOLEAVE   : SCHRFUNC(RRPL) ;
                       autoplay = TRUE ;
                       progstate = DOREST ;
                       break ;
      case RESETGAME : remove = gamestate ;
                       progstate = DOPLAY ;
                       break ;
      case CTRLSHIT  : break ;
    } 
}
