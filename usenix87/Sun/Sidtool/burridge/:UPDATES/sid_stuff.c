
/*  sid_stuff.c
 *
 *  Various functions and procedures used by Sid Tool.
 *
 *  Written by Rich Burridge - SUN Microsystems Australia (Melbourne).
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
#include <sundev/kbd.h>
#include "bltstuff.h"
#include "sidtool.h"
#include <suntool/sunview.h>
#include <suntool/canvas.h>

extern Pixfont *pf ;
extern Pixrect *circles[4][4] ;
extern Pixwin *pw ;

extern jmp_buf exception ;
extern int val ;

extern long random() ;
extern char *getenv() ;

/* For descriptions of these external variables, see sid_main.c */

extern BOOLEAN autoplay,changed,demomode,gamestate,remove ;

extern char a_name[MAXLINE],buffer[MAXLINE],h_name[MAXLINE] ;
extern char m_name[MAXLINE], new_key_vals[9][MAXLINE] ;
extern char old_key_vals[9][MAXLINE],s_name[MAXLINE],titlestring[MAXLINE] ;

extern int blueblink,blueincblink,button,c,cirx,ciry,credits ;
extern int curbluetime[MAXNUMPLAYERS+1],curdir,dotx,doty ;
extern int fruitmaze[MAXNUMPLAYERS+1],fruiton,fruitsgotten[MAXNUMPLAYERS+1][9] ;
extern int fruitx,fruity,bugssincedot,height,highscore,inc,key_stations[9] ;
extern int lastnumplayers,numcir[MAXNUMPLAYERS+1],numplayers ;
extern int orgx,orgy,player,posx,posy,progstate,redraw,sfunc,skilllevel,speed ;
extern int started,walls[XSIZE+6][YSIZE+1],width ;

extern struct bugrec bugs[4] ;
extern struct scorerec allhighscores[11] ;
extern struct startrec startpos[4] ;


changebugs()

{
  register struct bugrec *p ;

  bugssincedot = 0 ;
  for (p = &bugs[POKEY]; p <= &bugs[SHADOW]; p++)
    if (!p->eyesonly)
      {
        drawbug(p) ;
        p->bluetime = curbluetime[player] ;
        if ((!p->boxtime) && (!p->inbox))
          p->dir = REVERSEDIR(p->dir) ;
        drawbug(p) ;                    /* Will be blue now. */
      }
}


changeplayers(startgame)
int startgame ;

{
  int cnt,i ;

  if (numplayers == 1)
    {
      if (fruiton) updatefruit() ;
      gamestate = TRUE ;
      progstate = RESETGAME ;
      longjmp(exception,val) ;
    }
  cnt = 0 ;
  do
    {
      cnt++ ;
      player = 1 + (player % numplayers) ;
      if (cnt > 5)             /* Game all over. */
        {
          progstate = DOLEAVE ;
          longjmp(exception,val) ;
        }
    }
  while (!numcir[player]) ;
  clear_screen() ;
  SPRINTF(buffer,"Player %1d",player) ;
  write_bold(348,500,buffer) ;
  for (i = 0; i < 100; i++) LONGPAUSE() ;
  drawmaze() ;
  blinkpause() ;
  if (!startgame)
    {
      gamestate = TRUE ;
      progstate = RESETGAME ;
      longjmp(exception,val) ;
    }
}


checkcollision(nx,ny,g)
register int nx,ny ;
int *g ;

{
  register struct bugrec *tg ;

  for (tg = &bugs[POKEY]; tg <= &bugs[SHADOW]; tg++)
    if (tg->mx == nx)
      if (tg->my == ny)
        if (!tg->eyesonly)
          {
            *g = GIND(tg) ;
            return(1) ;
          }
  return(0) ;
}


checkinc(dir,mx,my)
int dir,mx,my ;

{
  switch (dir)
    {
      case UP    : return(!walls[mx+2][my-1]) ;
      case RIGHT : return(!walls[mx+3][my]) ;
      case DOWN  : return(!walls[mx+2][my+1]) ;
      case LEFT  : return(!walls[mx+1][my]) ;
    }
  return(0) ;
}


destroyblue(g)
register struct bugrec *g ;
 
{
  int i,inc,x ;
 
  drawbug(g) ;                  /* Turn off. */
  g->eyesonly = 1 ;
  g->bluetime = 0 ;
  inc = 20 ;
  for (i = 1; i <= bugssincedot; i++) inc *= 2 ;
  bugssincedot++ ;
  x = g->scrx + 10 ;
  if (x > 740) x = 740 ;
  else if (x < 5) x = 5 ;
  SCHRFUNC(RXOR) ;
  SPRINTF(buffer,"%1d0",inc) ;
  WRITELN(x,g->scry+10,buffer) ;
  for (i = 0; i < 10; i++) LONGPAUSE() ;
  SPRINTF(buffer,"%1d0",inc) ;
  WRITELN(x,g->scry+10,buffer) ;
  SCHRFUNC(RRPL) ;
  drawbug(g) ;                  /* Turn on as eyesonly. */
  updatescore(inc) ;
}


destroyfruit()

{
  int i,inc ;

  fruitsgotten[player][fruitmaze[player]]++ ;
  updatefruit() ;                                       /* Turn fruit off. */
  inc = fruitscore(fruitmaze[player]) ;
  updatescore(inc) ;
  SCHRFUNC(RXOR) ;
  SPRINTF(buffer,"%1d0",inc) ;
  WRITELN(fruitx+10,fruity+10,buffer) ;
  for (i = 0; i < 10; i++) LONGPAUSE() ;
  SPRINTF(buffer,"%1d0",inc) ;
  WRITELN(fruitx+10,fruity+10,buffer) ;
  SCHRFUNC(RRPL) ;
}


display_settings()   /* Display current skill level and number of players. */

{
  int x,y ;
  char buffer[MAXLINE] ;

  x = 50 ;
  y = 70 ;
  write_bold(x,y,"Current skill level:") ;
  SPRINTF(buffer,"%1d  ",skilllevel) ;
  WRITELN(x+190,y,buffer) ;

  x = 430 ;
  write_bold(x,y,"Number of players:") ;
  SPRINTF(buffer,"%1d  ",numplayers) ;
  WRITELN(x+180,y,buffer) ;
}


dohighscores()         /* Display high scores on the screen. */

{
  char skillc ;
  int level ;

  clear_screen() ;
  write_bold(334,200,"High Scores") ;
  SCHRFUNC(ROR) ;
  WRITELN(334,201,"___________") ;
  WRITELN(200,300,"Skill level         Score      Who") ;
  WRITELN(200,301,"___________         _____      ___");
  SCHRFUNC(RRPL) ;
  for (level = 1; level <= 10; level++)
    {
      skillc = (level == skilllevel) ? '*' : ' ' ;
      if (allhighscores[level].score)
        SPRINTF(buffer,"%c   %2d        =   %5d0      %s",
                skillc,level,allhighscores[level].score,allhighscores[level].who) ;
      else
        SPRINTF(buffer,"%c   %2d        =      -      %s",
                skillc,level,allhighscores[level].who) ;
      WRITELN(200,330+level*30,buffer) ;
    }
  if (!autoplay) make_control_panel() ;
  if (!autoplay) display_settings() ;
}
 
 
make_selection()    /* Get user selection after DEL press. */

{
  switch (button)
    {
      case BUT_AUTO    : numplayers = 1 ;
                         autoplay = TRUE ;
                         started = TRUE ;
                         iocursormode(OFFCURSOR) ;
                         break ;
      case BUT_HELP    : clear_screen() ;
                         dohelp() ;
                         break ;
      case BUT_LEVEL   : skilllevel = skilllevel % 10 + 1 ;
                         display_settings() ;
                         highscore = allhighscores[skilllevel].score ;
                         break ;
      case BUT_PLAYERS : numplayers = numplayers % 4 + 1 ;
                         display_settings() ;
                         break ;
      case BUT_QUIT    : function_keys(KEY_RESET) ;
                         exit(0) ;
      case BUT_SCORES  : dohighscores() ;
                         break ;
      case BUT_START   : autoplay = FALSE ;
                         started = TRUE ;
                         iocursormode(OFFCURSOR) ;
                         lastnumplayers = numplayers ;
    }
  if (button == BUT_AUTO || button == BUT_START) progstate = INITGAME ;
  else progstate = GETBUT ;
}


restore_screen()          /* Called when window needs to be drawn. */

{
  int g ;

  if (!redraw++) return ;
  clear_screen() ;
  if (!started)
    {
      iocursormode(TRACKCURSOR) ;
      make_control_panel() ;
      display_settings() ;
      dohelp() ;
    }
  else if (credits)
    {
      dohelp() ;
      BLT_SCRN(3,ciry-37,762,100,RSET) ;
      if (credits == 1)
        {
          drawdot(dotx,doty,BIGDOT) ;
          drawcir(circles[LEFT][inc],cirx,ciry) ;
        }
      else drawcir(circles[RIGHT][inc],cirx,ciry) ;
      for (g = POKEY; g <= SHADOW; g++)
        if (!bugs[g].eyesonly) drawbug(&bugs[g]) ;
    }
  else
    {
      iocursormode(OFFCURSOR) ;
      drawmaze() ;
      for (g = POKEY; g <= SHADOW; g++) drawbug(&bugs[g]) ;
      drawcir(circles[curdir][inc],posx,posy) ;
    }
}


erasebugs()         /* Erase all bugs from the screen. */
 
{
  int g ;
 
  for (g = POKEY; g <= SHADOW; g++)
    drawbug(&bugs[g]) ;              /* Erase all bugs. */
}
 
 
fruitscore(fruit)      /* Returns the score for the fruit specified. */
int fruit ;

{
  switch (fruit)
    {
      case 1  : return(10) ;
      case 2  : return(30) ;
      case 3  : return(50) ;
      case 4  : return(70) ;
      case 5  : return(100) ;
      case 6  : return(200) ;
      case 7  : return(300) ;
      case 8  : return(500) ;
    }
  return(0) ;
}   
 
 
geths(fd,record)                      /* Get one high score record in. */
struct scorerec *record ;
int fd ;

{
  char buffer[32],valuestr[7] ;
  int i ;

  i = read(fd,buffer,23) ;
  for (i = 0; i < 16; i++) record->who[i] = buffer[i] ^ ENKEY ;
  record->who[i] = '\0' ;
  for (i = 0; i < 7; i++) valuestr[i] = buffer[i+16] ^ ENKEY ;
  record->score = atoi(valuestr) ;
}


get_options(argc,argv)
int argc ;
char *argv[] ;

{
  char *arg,*env ;
  char *p ;                 /* Pointer to string following argument flag. */

  orgx = 0 ;                /* X origin of SUN SID window. */
  orgy = 0 ;                /* Y origin of SUN SID window. */
  width = SWIDTH ;          /* Width of SUN SID window. */
  height = SHEIGHT ;        /* Height of SUN SID window. */
  speed = SPEED ;           /* Default speed of Sun machine that this game is on.*/
  STRCPY(titlestring,"sidtool -  Sun Interactive Debugger V2.1.      Rich Burridge") ;
  STRCPY(m_name,M_NAME) ;   /* Default sidtool maze filename. */
  STRCPY(a_name,A_NAME) ;   /* Default sidtool animate filename. */
  STRCPY(s_name,S_NAME) ;   /* Default sidtool highscore filename. */
  STRCPY(h_name,H_NAME) ;   /* Default sidtool help filename. */
  demomode = FALSE ;

  if ((env = getenv("SID_MAZE")) != NULL) STRCPY(m_name,env) ;
  if ((env = getenv("SID_ANIMATE")) != NULL) STRCPY(a_name,env) ;
  if ((env = getenv("SID_SCORE")) != NULL) STRCPY(s_name,env) ;
  if ((env = getenv("SID_HELP")) != NULL) STRCPY(h_name,env) ;

  while (argc > 1 && (arg = argv[1])[0] == '-')
    {
      p = arg + 2 ;
      switch (arg[1])
        {
          case 'a' : STRCPY(a_name,p) ;    /* New animate filename. */
                     break ;
          case 'c' : speed = atoi(p) ;     /* New class (speed) of machine. */
                     break ;
          case 'd' : demomode = TRUE ;     /* Run in self demonstration mode. */
                     break ;
          case 'h' : STRCPY(h_name,p) ;    /* New help filename. */
                     break ;
          case 'm' : STRCPY(m_name,p) ;    /* New maze filename. */
                     break ;
          case 's' : STRCPY(s_name,p) ;    /* New high score filename. */
        }
      argc-- ;
      argv++ ;
    }
}


get_string(fd,s)            /* Get next line from specified file. */
FILE *fd ;
char s[MAXLINE] ;
 
{
  int c,i ;
 
  i = 0 ;
  while (i < MAXLINE-1 && (c = getc(fd)) != EOF && c != '\n')
  if (c != '\0') s[i++] = c ;
 
  if (c == EOF) return(-1) ;
  s[i] = '\0' ;
  return(i) ;
}


handlecollision(g)
register struct bugrec *g ;

{
  int i ;
  struct bugrec *tg ;

  if (g->bluetime > 0) destroyblue(g) ;
  else
    { 
      drawbug(g) ;                         /* Erase one that ate screen. */
      explodecircle(posx,posy) ;
      for (tg = &bugs[POKEY]; tg <= &bugs[SHADOW]; tg++)
        if (tg != g) drawbug(tg) ;
      if (autoplay)
        {
          progstate = DOLEAVE ;
          longjmp(exception,val) ;
        }
      else if (!numcir[player])
        {
          SCHRFUNC(RXOR) ;
          WRITELN(339,YBASE+SQUARE*16,"GAME OVER!") ;
          for (i = 0; i < 80; i++) LONGPAUSE() ;
          WRITELN(339,YBASE+SQUARE*16,"GAME OVER!") ;
          SCHRFUNC(RRPL) ;
          if (numplayers == 1)
            {
              progstate = DOLEAVE ;
              longjmp(exception,val) ;
            }
        }
      else for (i = 0; i < 50; i++) LONGPAUSE() ;
      changeplayers(0) ;
    }
}
 
 
newbugs(drawthem)
int drawthem ;

{
  register struct bugrec *p ;

  for (p = &bugs[POKEY]; p <= &bugs[SHADOW]; p++)
    {
      p->dir = UP ;
      p->mx = startpos[GIND(p)].x ;
      p->my = startpos[GIND(p)].y ;
      TRANSPT(p->mx,p->my,p->scrx,p->scry) ;
      p->bluetime = 0 ;
      p->eyesonly = 0 ;
      p->boxtime = ((-2*skilllevel+25) / 5) * startpos[GIND(p)].time ;
      if (!p->boxtime) p->inbox = 0 ;
      else p->inbox = 1 ;
      p->enteringbox = 0 ;
      p->count = 0 ;
      p->delay = 5 ;
      p->pic = GIND(p) % 2 ;
      p->intunnel = 0 ;
      if (drawthem) drawbug(p) ;
    }
}


play()                    /* Initialise for next player to play. */

{
  fruiton = 0 ;
  blueblink = 200 ;
  blueincblink = 25 ;
  if (numplayers == 1)
    {
      player = 1 ;
      drawmaze() ;
      blinkpause() ;
    }
  else
    { 
      player = numplayers ;
      changeplayers(1) ;
    }
  remove = TRUE ;
}
 
 
puths(fd,record)                       /* Put one high score record out. */
struct scorerec record ;
int fd ;

{
  char buffer[32],valuestr[7] ;
  int i,value ;

  for (i = 0; i < 16; i++) buffer[i] = record.who[i] ^ ENKEY ;
  value = record.score ;
  SPRINTF(valuestr,"%d",value) ;
  for (i = 0; i < 7; i++) buffer[i+16] = valuestr[i] ^ ENKEY ;
  WRITE(fd,buffer,23) ;
}
 
 
randomrange(low,high)      /* Return a random number between low and high. */
int low,high ;

{
  return((((int) random() & 077777) % (high-low+1)) + low) ;
}


function_keys(state)        /*  Set or reset the function keys. */
int state ;

{
  int count = STRING ;         /* 0xB0 -- the starting entry for strings. */
  int fd,i ;

  if ((fd = open("/dev/kbd",0,0)) < 0)
    {
      FPRINTF(stderr,"sidtool: can't open /dev/kbd\n") ;
      exit(1) ;
    }
  for (i = 0; i < 4; i++)       /* Set up function keys R7-R15, saving old values. */
    {
      if (state == KEY_SET)
        {
          get_key(fd,key_stations[i],old_key_vals[i],STRING + 1 + i) ;
          set_key(fd,key_stations[i],new_key_vals[i],STRING + 5 + i) ;
        }
      else set_key(fd,key_stations[i],old_key_vals[i],STRING + 1 + i) ;
    }
  CLOSE(fd) ;
}
