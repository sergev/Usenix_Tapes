
/*  sid_stuff.c
 *
 *  Various functions and procedures used by the SID tool.
 *  Written by Rich Burridge - SUN Australia - June 1986.
 *
 *  Version 1.5
 *
 *  Routines:
 *
 *  changebugs            - Change all bugs to blue.
 *  changeplayers         - Change to the next player or the next life.
 *  checkcollision        - Check for collision between screen and bug.
 *  checkinc              - Check if screen can go in desired direction.
 *  destroyblue           - Remove blue bug from the screen.
 *  destroyfruit          - Remove fruit from the screen updating score.
 *  display_settings      - Display current skill level and number of players.
 *  dohighscores          - Display high scores on the screen.
 *  dorest                - Draw credits and handle button actions.
 *  doupdate              - Update everything one clock tick.
 *  erasebugs             - Erase all the bugs from the screen.
 *  fruitscore            - Returns the score for the fruit specified.
 *  geths                 - Get one high score record in.
 *  getline               - Get characters from player until RETURN pressed.
 *  get_options           - Get user command line options.
 *  get_string            - Get next line from specified file.
 *  handlecollison        - Handle collison between bug and screen.
 *  keyinput              - Check keyboard for commands and wait if ^S.
 *  newbugs               - Generate four brand new bugs.
 *  play                  - Initialise for next player to play.
 *  puths                 - Put one high score record out.
 *  randomrange           - Return a random number between low and high.
 *  reset_function_keys   - Reset the function keys to previous values.
 *  set_function_keys     - Set the function keys to single sensible characters.
 *  set_options           - Set up initial default values.
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include "sidtool.h"
#include "exceptions.h"
#include "bltstuff.h"

exexception(delhit) ;           /* Raised when DEL hit so game can be started. */
exexception(doleave) ;          /* Raised to restart game with autoplay true. */
exexception(resetgame) ;        /* Raised to redraw maze. */

extern long random() ;

extern BOOLEAN autoplay ;            /* TRUE if the machine is playing with itself. */
extern BOOLEAN changed ;             /* Whether window has been changed. */
extern BOOLEAN demomode ;            /* TRUE if machine is in demonstration mode. */
extern BOOLEAN gamestate ;           /* State of the game, 1 = remove circle. */
extern BOOLEAN remove ;              /* Whether Sun screen should be removed. */
extern BOOLEAN started ;             /* If 1, program is running. */

extern char a_name[MAXLINE] ;        /* Animate file name. */
extern char buffer[MAXLINE] ;        /* Used to store intermediate values. */
extern char h_name[MAXLINE] ;        /* Help file name. */
extern char m_name[MAXLINE] ;        /* Maze file name. */
extern char new_key_vals[9][MAXLINE] ;     /* Function key values used by SIDtool. */
extern char old_key_vals[9][MAXLINE] ;     /* Function key string values to save. */
extern char s_name[MAXLINE] ;        /* Score file name. */
extern char sc ;                     /* Used by wgread for keyboard and mouse interaction. */
extern char titlestring[MAXLINE] ;   /* Title string for SID tool window. */

extern int blueblink ;          /* Number of "clock" ticks for blinking blue bug. */
extern int blueincblink ;       /* Increment between displays of blinking bug. */
extern int c ;                  /* Used by wgread for keyboard/ mouse interaction. */
extern int curbluetime[MAXNUMPLAYERS+1] ;
extern int fruitmaze[MAXNUMPLAYERS+1] ;
extern int fruiton ;            /* Whether fruit is being displayed or not. */
extern int fruitsgotten[MAXNUMPLAYERS+1][9] ;
extern int fruitx ;
extern int fruity ;
extern int bugssincedot ;
extern int height ;             /* Window height. */
extern int highscore ;          /* High score at current skill level. */
extern int key_stations[9] ;    /* Station values for function keys R7-R15. */
extern int lastnumplayers ;     /* Last number of players playing the game. */
extern int numcir[MAXNUMPLAYERS+1] ;  /* Number of screens for each player. */
extern int numplayers ;         /* Number of players playing SID. */
extern int orgx ;               /* Window X origin. */
extern int orgy ;               /* Window Y origin. */
extern int player ;             /* Current player. */
extern int posx ;               /* Current X position of the screen. */
extern int posy ;               /* Current Y position of the screen. */
extern int sfunc ;              /* Used by SCHRFUNC for cursor function. */
extern int skilllevel ;         /* Current game skill level. */
extern int walls[XSIZE+6][YSIZE+1] ;
extern int width ;              /* Window width. */
extern int winsetup ;           /* To prevent screen being redrawn twice at startup. */

extern struct bugrec bugs[4] ;           /* The bad guys. */
extern struct scorerec allhighscores[11] ;   /* High scores at all skill levels. */
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
      raise(resetgame,1) ;
    }
  cnt = 0 ;
  do
    {
      cnt++ ;
      player = 1 + (player % numplayers) ;
      if (cnt > 5) raise(doleave,1) ;          /* Game all over. */
    }
  while (!numcir[player]) ;
  clear_screen() ;
  SPRINTF(buffer,"Player %1d",player) ;
  write_bold(348,500,buffer) ;
  for (i = 0; i < 100; i++) LONGPAUSE() ;
  keyinput(TRUE) ;
  drawmaze() ;
  blinkpause() ;
  if (!startgame)
    {
      gamestate = TRUE ;
      raise(resetgame,1) ;
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
  writeln(x,g->scry+10,buffer) ;
  for (i = 0; i < 10; i++) LONGPAUSE() ;
  SPRINTF(buffer,"%1d0",inc) ;
  writeln(x,g->scry+10,buffer) ;
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
  writeln(fruitx+10,fruity+10,buffer) ;
  for (i = 0; i < 10; i++) LONGPAUSE() ;
  SPRINTF(buffer,"%1d0",inc) ;
  writeln(fruitx+10,fruity+10,buffer) ;
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
  writeln(x+170,y,buffer) ;

  write_bold(430,70,"Number of players:") ;

  x = 430 ;
  write_bold(x,y,"Number of players:") ;
  SPRINTF(buffer,"%1d  ",numplayers) ;
  writeln(x+160,y,buffer) ;
}


dohighscores()         /* Display high scores on the screen. */

{
  char skillc ;
  int level ;

  clear_screen() ;
  write_bold(334,200,"High Scores") ;
  SCHRFUNC(ROR) ;
  writeln(334,201,"___________") ;
  writeln(200,300,"Skill level         Score      Who") ;
  writeln(200,301,"___________         _____      ___");
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
      writeln(200,330+level*30,buffer) ;
    }
  if (!autoplay) make_control_panel() ;
  if (!autoplay) display_settings() ;
}
 
 
dorest()                /* Draw credits and handle button actions. */

{
  int button,i ;

  when(delhit)
    etype(1)
      SCHRFUNC(RRPL) ;
      autoplay = FALSE ;
      dorest() ;
      forget(delhit) ;
      return ;
  endwhen

  if (autoplay)
    {
      numplayers = 1 ;
      writehighscore() ;
      if (!demomode)
        {
          dohighscores() ;
          writeln(5,860,"Type DEL to begin") ;
          for (i = 1; i <= 200; i++) LONGPAUSE() ;
        }
      docredits() ;
    }
  else
    { 
      started = FALSE ;
      iocursormode(TRACKCURSOR) ;
      make_control_panel() ;
      display_settings() ;
      do
        {
          button = get_button_option() ;
          switch (button)
            {
              case BUT_AUTO    : numplayers = 1 ;
                                 autoplay = TRUE ;
                                 started = TRUE ;
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
              case BUT_QUIT    : exit(0) ;
              case BUT_SCORES  : dohighscores() ;
                                 break ;
              case BUT_START   : started = TRUE ;
                                 autoplay = FALSE ;
            }
        }    
      while (!started) ;
      iocursormode(OFFCURSOR) ;
    }
  if (!autoplay) lastnumplayers = numplayers ;
  forget(delhit) ;
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


getline(s,x,y)                 /* Get characters from user until a RETURN. */
char s[MAXLINE] ;
int x,y ;

{
  int c,i ;

  i = 0 ;
  for (;;)
    {
      s[i] = '_' ;
      s[i+1] = '\0' ;
      writeln(x,y,s) ;
      do
        c = wgread() ;
      while (!c) ;
      switch (c)
        {
          case BSPACE :
          case DEL    : if (i)
                          {   
                            s[i] = ' ' ;
                            s[i+1] = '\0' ;
                            i-- ;
                            writeln(x,y,s) ;
                          }
                        break ;
          case CR     : s[i] = '\0' ;
                        return(i) ;
          default     : s[i++] = c ;
        }
    }    
}


get_options(argc,argv)
int argc ;
char *argv[] ;

{
  char *arg ;

  demomode = FALSE ;
  while (argc > 1 && (arg = argv[1])[0] == '-')
    {
      switch (arg[1])
        {
          case 'd' : demomode = TRUE ;     /* Run in self demonstration mode. */
                     break ;
          default  : FPRINTF(stderr,"sidtool: USAGE %s [-d]\n",argv[0]) ;
                     exit(1) ;
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
 
  if (c == EOF) return(0) ;
  if (c == '\n') s[i++] = c ;
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
      if (autoplay) raise(doleave,1) ;
      else if (!numcir[player])
        {
          SCHRFUNC(RXOR) ;
          writeln(339,YBASE+SQUARE*16,"GAME OVER!") ;
          for (i = 0; i < 80; i++) LONGPAUSE() ;
          writeln(339,YBASE+SQUARE*16,"GAME OVER!") ;
          SCHRFUNC(RRPL) ;
          if (numplayers == 1) raise(doleave,1) ;
        }
      else for (i = 0; i < 50; i++) LONGPAUSE() ;
      changeplayers(0) ;
    }
}
 
 
keyinput(redraw)           /* Check keyboard for commands and wait if ^S. */
BOOLEAN redraw ;

{
  c = wgread() ;
  if (c)
    {   
      if (c == WIN_DAMAGED) changed = TRUE ;
      else if ((c == DEL) && !demomode) raise(delhit,1) ;
      else if (c == CTRLS)
        {
          do
            {
              c = wgread() ;
              if (c == WIN_DAMAGED) changed = TRUE ;
              else if ((c == DEL) && !demomode) raise(delhit,1) ;
            }
          while (c != CTRLQ) ;
        } 
      else if (!autoplay) sc = c ;
    } 
  if (changed)
    {
      if (winsetup <= 0) changed = FALSE ;
      else if (redraw)
        {
          getwindowparms(&orgx,&orgy,&width,&height) ;
          changed = FALSE ;
          restore_screen() ;
       }
     winsetup++ ;
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
  when(delhit)
    etype(1)
      SCHRFUNC(RRPL) ;
      autoplay = FALSE ;
      forget(delhit) ;
      forget(doleave) ;
      return ;
  endwhen

  when(doleave)
    etype(1)
      SCHRFUNC(RRPL) ;
      autoplay = TRUE ;
      forget(delhit) ;
      forget(doleave) ;
      return ;
  endwhen

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

/* Loop terminated by raising delhit or doleave. */

  for (;;) doplay() ;
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
  int count = 176 ;         /* 0xB0 -- the starting entry for strings. */
  int fd,i ;

  if ((fd = open("/dev/kbd",0,0)) < 0)
    {
      FPRINTF(stderr,"sidtool: can't open /dev/kbd\n") ;
      exit(1) ;
    }
  for (i = 0; i < 9; i++)       /* Set up function keys R7-R15, saving old values. */
    {
      if (state == KEY_SET) get_key(fd,key_stations[i],old_key_vals[i],count) ;
      set_key(fd,key_stations[i],new_key_vals[i],count++) ;
    }
  CLOSE(fd) ;
}
 
 
set_options()

{
  orgx = 0 ;             /* X origin of SUN SID window. */
  orgy = 0 ;             /* Y origin of SUN SID window. */
  width = SWIDTH ;       /* Width of SUN SID window. */
  height = SHEIGHT ;     /* Height of SUN SID window. */
  STRCPY(titlestring,"      SUN Interactive Debugger V1.5.") ;
  STRCPY(s_name,"sidtool.hs") ;
  STRCPY(a_name,"sidtool.animate") ;
  STRCPY(h_name,"sidtool.help") ;
  STRCPY(m_name,"sidtool.maze") ;
}
