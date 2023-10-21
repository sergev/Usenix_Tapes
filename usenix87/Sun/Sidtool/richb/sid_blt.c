
/*  sid_blt.c
 *
 *  Various routines that do "rasterop" type graphics used by the SID tool.
 *  Written by Rich Burridge - SUN Australia - June 1986.
 *
 *  Version 1.5.
 *
 *  Routines:
 *
 *  clear_screen          - Clear the SID tool window.
 *  ddrawline             - Draw a maze line in the specified direction.
 *  drawbug               - Draw this bug in the SID tool window.
 *  drawcir               - Draw the specified screen on the screen.
 *  drawcorner            - Draw a corner at MAZE position mx,my.
 *  drawdot               - XORS a dot at the maze position mx,my.
 *  drawmaze              - Draw the maze,the dots,the scores, etc on the screen.
 *  docredits             - Display credits screen.
 *  dohelp                - Draw help screen with buttons if in manual mode.
 *  doplay                - Play in auto or manual mode.
 *  doupdate              - Update everything one clock tick.
 *  explodecircle         - Player has died so explode screen.
 *  fixexits              - Look for tunnels on the borders.
 *  get_button_option     - Return button value user selected.
 *  initialize            - Initialize all the variable, pictures etc.. for SIDtool.
 *  make_button           - Display a control panel button.
 *  make_control_panel    - Display the control panel at the top of the screen.
 *  removecircle          - Remove a screen life from those displayed.
 *  resetmaze             - Reset maze to initial values and draw it.
 *  restore_screen        - Called when the SID tool window needs to be redrawn.
 *  updatefruit           - Turn fruit picture on or off as appropriate.
 *  updatescore           - Update the score for the current player.
 */

#include <stdio.h>
#include <strings.h>
#include "sidtool.h"
#include "exceptions.h"
#include "bltstuff.h"
#include <sys/timeb.h>

exexception(resetgame) ;       /* Raised inside play only when need to redraw maze. */

extern BOOLEAN autoplay ;
extern BOOLEAN demomode ;
extern BOOLEAN remove ;                /* Whether Sun screen should be removed. */
extern BOOLEAN started ;               /* If 1, program is running. */

extern char a_name[MAXLINE] ;          /* Animate file name */
extern char buffer[MAXLINE] ;
extern char but_names[7][8] ;          /* Control panel stuff. */
extern char h_name[MAXLINE] ;          /* Help file name. */
extern char m_name[MAXLINE] ;          /* Maze file name. */
extern char maze[XSIZE+2][YSIZE+2] ;
extern char names[4][MAXLINE] ;
extern char sc ;
extern char titlestring[MAXLINE] ;

extern int autoscore ;
extern int blueblink ;
extern int blueincblink ;
extern int boxx ;
extern int boxy ;
extern int c ;              /* Used by wgread for keyboard and mouse interaction. */
extern int circatchup ;
extern int cirmx ;
extern int cirmy ;
extern int cirx ;           /* X position of screen during credits. */
extern int ciry ;           /* Y position of screen during credits. */
extern int count ;
extern int credits ;        /* Direction of credits if on. */
extern int curbluetime[MAXNUMPLAYERS+1] ;
extern int curdir ;         /* Current direction of the screen. */
extern int dots[MAXNUMPLAYERS+1][XSIZE+4][YSIZE+2] ;
extern int dotx ;           /* X position of BIGDOT during credits. */
extern int doty ;           /* Y position of BIGDOT during credits. */
extern int fruitchances[MAXNUMPLAYERS+1] ;
extern int fruiton ;
extern int fruittime ;
extern int fruitx ;
extern int fruity ;
extern int fruitmaze[MAXNUMPLAYERS+1] ;
extern int g ;
extern int gamestate ;
extern int height ;         /* Window height. */
extern int highplayer ;
extern int highscore ;
extern int inc ;            /* Which of the four screen animates to draw. */
extern int newdir ;
extern int numcir[MAXNUMPLAYERS+1] ;
extern int numdots[MAXNUMPLAYERS+1] ;
extern int nx ;
extern int ny ;
extern int oldcurdir ;
extern int oldcx ;          /* Old X position of the screen on the screen. */
extern int oldcy ;          /* Old Y position of the screen on the screen. */
extern int orgx ;           /* Window X origin. */
extern int orgy ;           /* Window Y origin. */
extern int pausetime ;      /* Time to pause (dependent on skill level). */
extern int player ;
extern int posx ;           /* Current X position of the screen. */
extern int posy ;           /* Current Y position of the screen. */
extern int score[MAXNUMPLAYERS+1] ;
extern int sfunc ;
extern int skilllevel ;
extern int tunnel[XSIZE+4][YSIZE+2] ;
extern int walls[XSIZE+6][YSIZE+1] ;
extern int width ;          /* Window width. */
extern int x ;
extern int y ;

extern struct pixrect *load_picture() ;
extern long random() ;

extern struct pixrect *bigdot ;
extern struct pixrect *bluebug[2] ;
extern struct pixrect *bluepics[2] ;
extern struct pixrect *bugpics[4][2] ;
extern struct pixrect *circleexplode[9] ;
extern struct pixrect *circles[4][4] ;  /* Pointer to screen pictures. */
extern struct pixrect *corner[4] ;      /* Pointer to button corner pictures. */
extern struct pixrect *curcircle ;      /* Pointer to the current screen picture. */
extern struct pixrect *eyes[4] ;
extern struct pixrect *fruitpics[9] ;
extern struct pixrect *smalldot ;

extern struct bugrec  bugs[4] ;         /* The bad guys. */
extern struct scorerec allhighscores[11] ;
extern struct startrec startpos[4] ;


clear_screen()

{
  getwindowparms(&orgx,&orgy,&width,&height) ;
  BLT_SCRN(orgx,orgy,width,height,RCLR) ;

/*  To get over displaying the last cursor incorrectly after a cleared screen,
 *  the old cursor position is forced off the screen.
 */

  oldcx = 1000 ;
  oldcy = 1000 ;
}


ddrawline(mx,my,dir)
int mx,my,dir ;

/*  Draw a maze line from mx,my in the direction dir.
 *  Parameters: dir should be d,u,l,r, (or R for a thin line).
 */

{
  int x,y ;

  TRANSPT(mx,my,x,y) ;
  switch (dir)
    {
      case 'd' :
      case 'u' : BLT_SCRN(x+SQUARE/2,y,2,SQUARE,RSET) ;
                 break ;

      case 'l' :
      case 'r' : BLT_SCRN(x,y+SQUARE/2,SQUARE,2,RSET) ;
                 break ;

      case 'R' : BLT_SCRN(x,y+SQUARE/2,SQUARE,1,RSET) ;
    }
}


drawbug(g)
register struct bugrec *g ;
 
{
  int inc,winc ;
 
  inc = g->scrx - GOFFSET ;
  winc = 0 ;
 
  if (inc < 0)
    if (inc <= 45) return ;
    else  winc = inc ;
  else if (inc > SWIDTH-45)
    if (inc > SWIDTH - 1) return ;
    else
      {
        winc = SWIDTH-45-inc ;
        inc = 0 ;
      }
  else inc = 0 ;

  if (g->eyesonly)
    {
      BLT_MEM_TO_SCRN(g->scrx-GOFFSET-inc,g->scry-GOFFSET,
                      45+winc,21,RXOR,eyes[g->dir],-inc,0) ;
/* Fake BLT to get speed same. */
      BLT_MEM(bugpics[GIND(g)][g->pic],0,0,45+winc,21,RRPL,
              bugpics[GIND(g)][g->pic],0,0) ;
    }
  else if (g->bluetime > 0)
    {
      if ((g->bluetime < blueblink) &&
          (g->bluetime % blueincblink > blueincblink / 2))
        BLT_MEM_TO_SCRN(g->scrx-GOFFSET-inc,g->scry-GOFFSET,
                        45+winc,45,RXOR,bluepics[g->pic],-inc,0) ;
      else
        BLT_MEM_TO_SCRN(g->scrx-GOFFSET-inc,g->scry-GOFFSET,
                        45+winc,45,RXOR,bluebug[g->pic],-inc,0) ;
/* Fake BLT to get speed same. */
      BLT_MEM(eyes[g->dir],0,0,45+winc,21,RRPL,eyes[g->dir],0,0) ;
    }
  else
    { 
      BLT_MEM_TO_SCRN(g->scrx-GOFFSET-inc,g->scry-GOFFSET,
                      45+winc,45,RXOR,bugpics[GIND(g)][g->pic],-inc,0) ;
      BLT_MEM_TO_SCRN(g->scrx-GOFFSET-inc,g->scry-GOFFSET,
                      45+winc,21,RXOR,eyes[g->dir],-inc,0) ;
    }
}


drawcir(p,x,y)             /* Draw the specified screen on the screen. */
int x,y ;
struct pixrect *p ;

{
  if (x < 0) return ;                   /* Fully off left side. */
  else if (x > SWIDTH+1) return ;       /* Fully off right side. */

  if (!(oldcx == x && oldcy == y))
    {
      BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      curcircle = p ;
      BLT_MEM_TO_SCRN(x-GOFFSET+5,y-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      oldcx = x ;
      oldcy = y ;
    }
}


drawcorner(mx,my,dir)
int dir,mx,my ;

/*  Draw a corner at MAZE position mx,my turning the direction dir. */

{
  int x,y ;

  TRANSPT(mx,my,x,y) ;
  BLT_MEM_TO_SCRN(x,y,SQUARE,SQUARE,RRPL,corner[dir],0,0) ;
}
 
 
drawdot(mx,my,size)        /* XORS a dot at the maze position mx,my. */
int mx,my,size ;

{
   TRANSPT(mx,my,mx,my) ;
   if (size == BIGDOT) BLT_MEM_TO_SCRN(mx,my,24,24,RXOR,bigdot,0,0) ;
   else if (size == SMALLDOT) BLT_MEM_TO_SCRN(mx+9,my+9,7,7,RXOR,smalldot,9,9) ;
}


drawmaze()     /* Draw the maze,the dots,the scores, etc on the screen. */
 
{
  int x,y ;
 
  clear_screen() ;
  for (y = 0; y <= YSIZE+1; y++)
    {
      walls[1][y] = 1 ;
      walls[0][y] = 1 ;
      walls[XSIZE+4][y] = 1 ;
      walls[XSIZE+5][y] = 1 ;
      for (x = 0; x <= XSIZE+1; x++) walls[x+2][y] = 0 ;
    }
 
  for (y = 0; y <= YSIZE+1; y++)
    for (x = 0; x <= XSIZE+1; x++)
      if ((maze[x][y] == 's') || (maze[x][y] == 'S') || (maze[x][y] == 'T'))
        {
          drawbox(x,y) ;
          keyinput(FALSE) ;                             /* Check for abort. */
        }
      else if (maze[x][y] == 'x') walls[x+2][y] = 1 ;   /* Borders. */
 
  PPAUSE(pausetime*10) ;
  BLT_SCRN(XBASE-(SQUARE/2)-2,YBASE-(SQUARE/2)-2,
                  SQUARE*(XSIZE+1)+6,SQUARE*(YSIZE+1)+6,RINV) ;
  fixexits() ;

  for (y = 1; y <= YSIZE; y++)
    for (x = 1; x <= XSIZE; x++)
      if (dots[player][x+1][y] == SMALLDOT)
        {
          PPAUSE(pausetime) ;
          drawdot(x,y,SMALLDOT) ;
        }
      else if (dots[player][x+1][y] == BIGDOT)
        {
          PPAUSE(pausetime) ;
          drawdot(x,y,BIGDOT) ;
        }
      else keyinput(FALSE) ;                           /* Check for abort. */

  for (x = 1; x <= 4; x++) showplayerscore(x) ;

  SPRINTF(buffer,"High Score (%1d)",skilllevel) ;
  writeln(310,20,buffer) ;
  if (!highscore) writeln(348,35,"0") ;
  else
    { 
      SPRINTF(buffer,"%1d0",highscore) ;
      writeln(348,35,buffer) ;
    }

  for (x = 1; x <= numcir[player]; x++)
    {
      PPAUSE(pausetime*10) ;
      BLT_MEM_TO_SCRN(30+(x-1)*60,20,50,50,RXOR,circles[RIGHT][0],0,0) ;
    }

  BLT_MEM_TO_SCRN(705,25,45,45,RRPL,fruitpics[fruitmaze[player]],0,0) ;
  BLT_SCRN(700,20,55,55,RINV) ;
  SPRINTF(buffer,"%1d0",fruitscore(fruitmaze[player])) ;
  writeln(710,15,buffer) ;

  if (autoplay && (!demomode))
    {
      SCHRFUNC(RXOR) ;
      writeln(339,YBASE+SQUARE*16,"GAME OVER!") ;
      SCHRFUNC(RRPL) ;
      writeln(300,65,"Type DEL to begin") ;
      writeln(480,50,"Auto Score") ;
      writeln(489,65,"0") ;
    }
}


docredits()
 
{
  register int g ;
  int x,y,i,j ;
 
  credits = 1 ;
  clear_screen() ;
  dohelp() ;
  writeln(5,860,"Type DEL to begin") ;
  for (i = 0; i < 100; i++) LONGPAUSE() ;

  newbugs(0) ;
  UNTRANSPT(130,350,dotx,doty) ;
  UNTRANSPT(860,350,x,y) ;
  TRANSPT(x,y,i,ciry) ;
  BLT_SCRN(3,ciry-37,762,100,RINV) ;

  for (g = POKEY; g <= SHADOW; g++)
    {
      bugs[g].mx = x + g * 2 ;
      bugs[g].my = doty ;
      bugs[g].dir = g ;
      TRANSPT(bugs[g].mx,bugs[g].my,bugs[g].scrx,bugs[g].scry) ;
      drawbug(&bugs[g]) ;                        /* Should be invisible. */
    }
  drawdot(dotx,doty,BIGDOT) ;
  cirx = 720 ;
  inc = 0 ;
  for (i = 1; i <= 662; i++)
    {
      keyinput(TRUE) ;
      if (i % 8) cirx-- ;
      drawcir(circles[LEFT][inc],cirx,ciry) ;
      if (i % 4 == 0) inc = (inc + 1) % 4 ;
      for (g = POKEY; g <= SHADOW; g++)
        {
          drawbug(&bugs[g]) ;               /* Erase old. */
          bugs[g].scrx-- ;
          if (i % 13 == 0) bugs[g].pic = (bugs[g].pic + 1) % 2 ;
          if (i % 18 == 0) bugs[g].dir = (bugs[g].dir + 1) % 4 ;
          drawbug(&bugs[g]) ;               /* Draw new. */
        }
    }
  credits = 2 ;
  drawdot(dotx,doty,BIGDOT) ;
  for (g = POKEY; g <= SHADOW; g++)
    {
      drawbug(&bugs[g]) ;                   /* Erase old. */
      bugs[g].bluetime = 32000 ;
      drawbug(&bugs[g]) ;                   /* Draw new as blue. */
    }
  j = 200 ;
  x = 1 ;
  SCHRFUNC(RXOR) ;
  for (i = 1; i <= 665; i++)
    {
       keyinput(TRUE) ;
       PPAUSE(10*x) ;
       if (i % 26) cirx++ ;
       drawcir(circles[RIGHT][inc],cirx,ciry) ;
       if (i % 4 == 0) inc = (inc + 1) % 4 ;
       for (g = POKEY; g <= SHADOW; g++)
         if (!bugs[g].eyesonly)
           {
             drawbug(&bugs[g]) ;            /* Erase old. */
             if (i % 2) bugs[g].scrx++ ;
             if (i % 13 == 0) bugs[g].pic = (bugs[g].pic + 1) % 2 ;
             if (cirx >= bugs[g].scrx-20)
               {
                 bugs[g].eyesonly = 1 ;
                 SPRINTF(buffer,"%1d",j) ;
                 writeln(bugs[g].scrx-20,440,buffer) ;
                 for (i = 0; i < 60; i++) LONGPAUSE() ;
                 SPRINTF(buffer,"%1d",j) ;
                 writeln(bugs[g].scrx-20,440,buffer) ;
                 j *= 2 ;
                 x++ ;
               }
             else drawbug(&bugs[g]) ;       /* Draw new. */
           }
    }
  SCHRFUNC(RRPL) ;
  for (i = 0; i < 100; i++) LONGPAUSE() ;
  credits = 0 ;
}


dohelp()

{
  int g,i,x,y ;
  char line[MAXLINE] ;
  FILE *fn ;

  write_bold(55,100,titlestring) ;
  writeln(90,120,"Original version by Brad A. Myers with pictures of fruit by Terry Vavra.") ;

  for (g = POKEY; g <= SHADOW; g++)
    {
      if (g > 1) y = YBASE + 130 ;
      else y = YBASE + 50 ;
      x = (g % 2) ? 384 : 100 ;

      BLT_MEM_TO_SCRN(x,y,45,45,RRPL,bugpics[g][0],0,0) ;
      BLT_MEM_TO_SCRN(x,y,45,21,RXOR,eyes[g],0,0) ;
      for (i = 0; i < 40; i++) LONGPAUSE() ;
      SPRINTF(buffer,"- %s",names[g]) ;
      writeln(x+60,y+25,buffer) ;
      for (i = 0; i < 40; i++) LONGPAUSE() ;
    }

  if ((fn = fopen(h_name,"r")) == NULL)
    {
      FPRINTF(stderr,"\nsidtool: can't open %s\n",h_name) ;
      exit(1) ;
    }
  x = 105 ;
  y = 465 ;
  i = 0 ;
  while (get_string(fn,line))
    {
      writeln(x,i*15+y,line) ;
      i++ ;
    }
  if (!autoplay) make_control_panel() ;
  if (!autoplay) display_settings() ;
}


doplay()

{

  when(resetgame)
    etype(1)
      remove = gamestate ;
      forget(resetgame) ;
      return ;
  endwhen

  if (remove)                /* Jump here if have been eaten or starting new game. */
    {
      removecircle() ;
      numcir[player]-- ;
    }
  curdir = LEFT ;             /* Jump here if got all dots. */
  fruiton = 0 ;
  sc = ' ' ;
  inc = 0 ;
  count = 1 ;
  posx = (SWIDTH / 2) - 11 ;
  posy = YBASE + SQUARE * 21 ;
  fruittime = randomrange(1000,2500) ;
  UNTRANSPT(posx,posy,cirmx,cirmy) ;
  drawcir(circles[curdir][inc],posx,posy) ;
  newbugs(1) ;
  if (demomode || !autoplay)
    {
      SCHRFUNC(RXOR) ;
      writeln(357,YBASE+SQUARE*16,"READY!") ;
      blinkpause() ;
      SCHRFUNC(RXOR) ;
      writeln(357,YBASE+SQUARE*16,"READY!") ;
      SCHRFUNC(RRPL) ;
    }
  for (;;)
    {
      updatebugs() ;
      if (checkcollision(cirmx,cirmy,&g)) handlecollision(&bugs[g]) ;
      if (fruittime != -1)
        {
          fruittime-- ;
          if (!fruittime) updatefruit() ;
        }
      newdir = curdir ;
      if (GCENTERED(posx,posy))
        {
          keyinput(TRUE) ;
          if (autoplay)
            newdir = dorandomdir(curdir,posx,posy,cirmx,cirmy,&x,&y,&nx,&ny,1) ;
          else
            { 
              switch (curdir)
                {
                  case UP    :      if (sc == 'r' && !walls[cirmx+3][cirmy]) newdir = RIGHT ;
                               else if (sc == 'l' && !walls[cirmx+1][cirmy]) newdir = LEFT ;
                               else if (sc == 'd' && !walls[cirmx+2][cirmy+1]) newdir = DOWN ;
                               break ;

                  case DOWN  :      if (sc == 'r' && !walls[cirmx+3][cirmy]) newdir = RIGHT ;
                               else if (sc == 'l' && !walls[cirmx+1][cirmy]) newdir = LEFT ;
                               else if (sc == 'u' && !walls[cirmx+2][cirmy-1]) newdir = UP ;
                               break ;

                  case RIGHT :      if (sc == 'l' && !walls[cirmx+1][cirmy]) newdir = LEFT ;
                               else if (sc == 'u' && !walls[cirmx+2][cirmy-1]) newdir = UP ;
                               else if (sc == 'd' && !walls[cirmx+2][cirmy+1]) newdir = DOWN ;
                               break ;

                  case LEFT  :      if (sc == 'r' && !walls[cirmx+3][cirmy]) newdir = RIGHT ;
                               else if (sc == 'u' && !walls[cirmx+2][cirmy-1]) newdir = UP ;
                               else if (sc == 'd' && !walls[cirmx+2][cirmy+1]) newdir = DOWN ;
                               break ;
                }
              if (newdir != curdir) sc = ' ' ;
            }
        }    
      if (doinc(newdir,posx,posy,cirmx,cirmy,&x,&y,&nx,&ny)) doupdate() ;
      else
        { 
          if (!GCENTERED(posx,posy)) doupdate() ;          /* Until centered. */
          else
            { 
              if (oldcurdir != curdir)
                {
                  BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,
                                  45,45,RXOR,curcircle,0,0) ;
                  curcircle = circles[curdir][0] ;
                  BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,
                                  45,45,RXOR,curcircle,0,0) ;
                  oldcurdir = curdir ;
                }
            }    
        }    
      if (checkcollision(cirmx,cirmy,&g)) handlecollision(&bugs[g]) ;
    }
}


doupdate()

{
  count++ ;
  if (count % circatchup == 0) return ;       /* Go slower than bugs. */
  drawcir(circles[newdir][inc],x,y) ;
  if (count % 4 == 0) inc = (inc + 1) % 4 ;
  if (fruiton)
    if ((nx == FRUITMX) && (ny == FRUITMY)) destroyfruit() ;
  if (dots[player][nx+1][ny] != NODOT)
    {
      if (dots[player][nx+1][ny] == SMALLDOT) updatescore(1) ;
      else
        { 
          changebugs() ;
          updatescore(5) ;
        }
      numdots[player]-- ;
      drawdot(nx,ny,dots[player][nx+1][ny]) ;
      dots[player][nx+1][ny] = NODOT ;
      if (!numdots[player])
        {
          resetmaze() ;
          gamestate = FALSE ;
          raise(resetgame,1) ;
        }
    }    
  curdir = newdir ;
  posx = x ;
  posy = y ;
  cirmx = nx ;
  cirmy = ny ;
}


explodecircle(posx,posy)
int posx,posy ;
 
{
  int i ;
 
  for (i = 0; i <= 8; i++)
    {
      BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      curcircle = circleexplode[i] ;
      BLT_MEM_TO_SCRN(posx-GOFFSET+5,posy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      oldcx = posx ;
      oldcy = posy ;
      PPAUSE(7000) ;
    } 
  for (i = 0; i < 80; i++) LONGPAUSE() ;
}


fixexits()

/*  Look for tunnels on the borders.  For each, show the area as black
 *  on the screen and set the walls and tunnel global variables to
 *  reflect the presence of the tunnel.
 */

{
  int x,y,t ;

  PPAUSE(pausetime*10) ;
  for (y = 1; y <= YSIZE; y++)
    if (maze[0][y] == ' ')
      {
        walls[1][y] = 0 ;
        walls[0][y] = 0 ;
        x = -1 ;
        do
          {
            x++ ;
            tunnel[x][y] = 1 ;
          }
        while (maze[x][y] == ' ') ;
        TRANSPT(0,y,x,t) ;
        BLT_SCRN(3,t-2-SQUARE/2,XBASE-(SQUARE/2)-5,SQUARE*2+6,RINV) ;
      }

  PPAUSE(pausetime*10) ;
  for (y = 1; y <= YSIZE; y++)
    if (maze[XSIZE+1][y] == ' ')
      {
        walls[XSIZE+4][y] = 0 ;
        walls[XSIZE+5][y] = 0 ;
        x = XSIZE+1 ;
        do
          {
            x-- ;
            tunnel[x][y] = 1 ;
          }
        while (maze[x][y] == ' ') ;
        TRANSPT(0,y,x,t) ;
        BLT_SCRN(XBASE-(SQUARE/2)+SQUARE*(XSIZE+1)+4,t-2-SQUARE/2,
                           XBASE-(SQUARE/2)-5,SQUARE*2+6,RINV) ;
      }
}


get_button_option()

{
  int pressed ;

  pressed = 0 ;
  do
    {
      do
        keyinput(TRUE) ;
      while (!c) ;
      if (c >= BUT_AUTO+2 && c <= BUT_START+2)
        {
          pressed = 1 ;
          BLT_SCRN(SQUARE/2+100*(c-2)+BUTXOFF,SQUARE/2+BUTYOFF,70,SQUARE,RINV) ;
          PPAUSE(pausetime*30) ;
          BLT_SCRN(SQUARE/2+100*(c-2)+BUTXOFF,SQUARE/2+BUTYOFF,70,SQUARE,RINV) ;
        }
    }    
  while (!pressed) ;
  return(c-2) ;
}


initialize()

{
  int fd,i,j,g,x,y ;
  struct timeb tp ;
  FILE *fn ;

  ftime(&tp) ;
  for (x = 1; x < tp.millitm % 10; x++) y = (int) random() ;    /* Randomize start. */

  if (!demomode) FPRINTF(stdout," Random") ;
  if (!demomode) FPRINTF(stdout," Memory") ;

  if ((fd = open(a_name,0)) == -1)
    {
      FPRINTF(stderr,"sidtool: unable to open %s.\n",a_name) ;
      exit(1) ;
    }

  for (i = UR; i <= LU; i++) corner[i] = load_picture(fd) ;

  bigdot = load_picture(fd) ;
  smalldot = load_picture(fd) ;

  for (i = RIGHT; i <= DOWN; i++)
    for (j = 0; j <= 3; j++) circles[i][j] = load_picture(fd) ;
  for (i = POKEY; i <= SHADOW; i++)
    for (j = 0; j <= 1; j++) bugpics[i][j] = load_picture(fd) ;

  for (i = 0; i <= 1; i++) bluebug[i] = load_picture(fd) ;
  for (i = 0; i <= 1; i++) bluepics[i] = load_picture(fd) ;
  for (i = 0; i <= 3; i++) eyes[i] = load_picture(fd) ;
  for (i = 0; i <= 8; i++) circleexplode[i] = load_picture(fd) ;
  for (i = 1; i <= 8; i++) fruitpics[i] = load_picture(fd) ;

  CLOSE(fd) ;

  if (!demomode) FPRINTF(stdout," File") ;

  if ((fn = fopen(m_name,"r")) == NULL)
    {
      FPRINTF(stderr,"\nsidtool: can't open %s\n",m_name) ;
      exit(1) ;
    }

  for (y = 0; y <= YSIZE+1; y++)
    {
      FGETS(buffer,MAXLINE,fn) ;
      for (x = 0; x <= XSIZE+1; x++) maze[x][y] = buffer[x] ;
    }
  FCLOSE(fn) ;

 if (!demomode) FPRINTF(stdout," Maze") ;

  TRANSPT(FRUITMX,FRUITMY,fruitx,fruity) ;
  readallhighscores() ;
  highscore = allhighscores[skilllevel].score ;
  if (!demomode) FPRINTF(stdout," HighScore") ;

  g = POKEY ;
  for (y = 1; y <= YSIZE; y++)
    for (x = 1; x <= XSIZE; x++)
      if ((maze[x][y] >= '0') && (maze[x][y] <= '9'))
        {
          startpos[g].x = x ;
          startpos[g].y = y ;
          startpos[g].time = maze[x][y] - '0' ;
          if (g < SHADOW) g++ ;
          if (maze[x][y] == '0')
            {
              boxx = x ;
              boxy = y ;
            }
        }    

  if (!demomode) FPRINTF(stdout," Starting\n") ;
  startup() ;
  getwindowparms(&orgx,&orgy,&width,&height) ;

  for (x = 0; x < XSIZE+3; x++)
    for (y = 0; y < YSIZE; y++) tunnel[x][y] = 0 ;
}


make_button(x,y,but_name)
int x,y ;
char but_name[MAXLINE] ;
 
{
  int len ;

  len = strlen(but_name) * 10 ;
  BLT_MEM_TO_SCRN(x,y,SQUARE,SQUARE,RRPL,corner[UR],0,0) ;
  BLT_MEM_TO_SCRN(x,y+SQUARE,SQUARE,SQUARE,RRPL,corner[LU],0,0) ;

  BLT_SCRN(x+SQUARE,y+SQUARE/2,len,2,RSET) ;
  BLT_SCRN(x+SQUARE,y+SQUARE*3/2,len,2,RSET) ;

  BLT_MEM_TO_SCRN(x+len,y,SQUARE,SQUARE,RRPL,corner[RD],0,0) ;
  BLT_MEM_TO_SCRN(x+len,y+SQUARE,SQUARE,SQUARE,RRPL,corner[DL],0,0) ;
  write_bold(x+SQUARE-5,y+SQUARE*3/2-10,but_name) ;
}


make_control_panel()

{
  int i ;

  BLT_SCRN(0,0,width,YBASE-20,RCLR) ;                 /* Clear panel area. */
  for (i = BUT_AUTO; i <= BUT_START; i++)
    make_button(100*i+BUTXOFF,BUTYOFF,but_names[i]) ; /* Make option buttons. */
}


removecircle()

{
  BLT_MEM_TO_SCRN(30+(numcir[player]-1)*60,20,50,50,RXOR,circles[RIGHT][0],0,0) ;
}


resetmaze()

{
  int i ;

  erasebugs() ;
  LONGPAUSE() ;
  for (i = 1; i <= 20; i++)
    {
      BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      for (i = 0; i < 10; i++) LONGPAUSE() ;
    }
  keyinput(FALSE) ;
  LONGPAUSE() ;
  if (fruitmaze[player] < 8) fruitmaze[player]++ ;
  fruitchances[player] = 0 ;
  setdots(player) ;
  drawmaze() ;
  if (curbluetime[player] > 1) curbluetime[player] -= 60 ;
}


restore_screen()       /* Called when the window needs to be redrawn. */

{
  int g ;

  getwindowparms(&orgx,&orgy,&width,&height) ;
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


updatefruit()

{
  struct pixrect *p ;

  p = fruitpics[fruitmaze[player]] ;
  BLT_MEM_TO_SCRN(fruitx-GOFFSET,fruity-GOFFSET,45,45,RXOR,p,0,0) ;
  if (fruiton)                        /* Turning fruit off. */
    {
      fruitchances[player]++ ;
      if (fruitchances[player] > 2) fruittime = -1 ;    /* Already had 2 chances. */
      else fruittime = randomrange(1000,2500) ;
    }
  else fruittime = randomrange(500,1000) ;              /* Turning fruit on. */
  fruiton = !fruiton ;
}


updatescore(amt)
int amt ;

{
  int i,temp,x,y ;

  if (autoplay)
    if (!demomode)
      {
        autoscore += amt ;
        SPRINTF(buffer,"%1d0",autoscore) ;
        writeln(489,65,buffer) ;
        return ;
      }
  temp = score[player] + amt ;
  if (temp >= 1000)
    if (score[player] < 1000)
      {
        for (i = 1; i < 7; i++)
          {
            BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
            for (i = 0; i < 10; i++) LONGPAUSE() ;
          }
        numcir[player]++ ;
        BLT_MEM_TO_SCRN(30+(numcir[player]-1)*60,20,50,50,RXOR,circles[RIGHT][0],0,0) ;
      }
  score[player] = temp ;
  x = (player % 2) ? 217 : 597 ;
  y = (player < 3) ? 40  : 80 ;
  SPRINTF(buffer,"%1d0",score[player]) ;
  writeln(x,y,buffer) ;
  if (score[player] > highscore)
    {
      highplayer = player ;
      highscore = score[player] ;
      SPRINTF(buffer,"%1d0",highscore) ;
      writeln(348,35,buffer) ;
    }
}
