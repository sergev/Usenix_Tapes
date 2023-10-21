
/*  sid_blt.c
 *
 *  Various routines that do "rasterop" type graphics used by sidtool.
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
#include "bltstuff.h"
#include "sidtool.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>

extern jmp_buf exception ;
extern int val ;

extern Pixfont *pf ;
extern Pixwin *pw ;

/* For descriptions of these variables, see sid_main.c */

extern BOOLEAN autoplay,demomode,remove ;

extern char a_name[MAXLINE],buffer[MAXLINE],but_names[7][8],h_name[MAXLINE] ;
extern char m_name[MAXLINE],maze[XSIZE+2][YSIZE+2],names[4][MAXLINE],sc ;
extern char titlestring[MAXLINE] ;

extern int autoscore,blueblink,blueincblink,boxx,boxy,button,c,circatchup ;
extern int cirmx,cirmy,cirx,ciry,count,credits,curbluetime[MAXNUMPLAYERS+1] ;
extern int curdir,dots[MAXNUMPLAYERS+1][XSIZE+4][YSIZE+2],dotx,doty ;
extern int fruitchances[MAXNUMPLAYERS+1],fruiton,fruittime,fruitx,fruity ;
extern int fruitmaze[MAXNUMPLAYERS+1],g,gamestate,height,highplayer ;
extern int highscore,inc,movei,movej,movex,newdir,numcir[MAXNUMPLAYERS+1] ;
extern int numdots[MAXNUMPLAYERS+1],nx,ny,oldcurdir,oldcx,oldcy,orgx,orgy ;
extern int pausetime,player,posx,posy,progstate,score[MAXNUMPLAYERS+1] ;
extern int sfunc,skilllevel,speed,tunnel[XSIZE+4][YSIZE+2],walls[XSIZE+6][YSIZE+1] ;
extern int width,x,y ;

extern Pixrect *load_picture() ;
extern long random() ;

extern Pixrect *bigdot,*bluebug[2],*bluepics[2],*bugpics[4][2],*circleexplode[9] ;
extern Pixrect *circles[4][4],*corner[4],*curcircle,*eyes[4],*fruitpics[9] ;
extern Pixrect *smalldot ;

extern struct bugrec  bugs[4] ;         /* The bad guys. */
extern struct scorerec allhighscores[11] ;
extern struct startrec startpos[4] ;


clear_screen()

{
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
Pixrect *p ;

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
        drawbox(x,y) ;
      else if (maze[x][y] == 'x') walls[x+2][y] = 1 ;   /* Borders. */
 
  PPAUSE(pausetime*30) ;
  BLT_SCRN(XBASE-(SQUARE/2)-2,YBASE-(SQUARE/2)-2,
                  SQUARE*(XSIZE+1)+6,SQUARE*(YSIZE+1)+6,RINV) ;
  fixexits() ;

  for (y = 1; y <= YSIZE; y++)
    for (x = 1; x <= XSIZE; x++)
      if (dots[player][x+1][y] == SMALLDOT)
        {
          PPAUSE(pausetime*5) ;
          drawdot(x,y,SMALLDOT) ;
        }
      else if (dots[player][x+1][y] == BIGDOT)
        {
          PPAUSE(pausetime*5) ;
          drawdot(x,y,BIGDOT) ;
        }

  for (x = 1; x <= 4; x++) showplayerscore(x) ;

  SPRINTF(buffer,"High Score (%1d)",skilllevel) ;
  WRITELN(310,20,buffer) ;
  if (!highscore) WRITELN(348,35,"0") ;
  else
    { 
      SPRINTF(buffer,"%1d0",highscore) ;
      WRITELN(348,35,buffer) ;
    }

  for (x = 1; x <= numcir[player]; x++)
    {
      PPAUSE(pausetime*30) ;
      BLT_MEM_TO_SCRN(30+(x-1)*60,20,50,50,RXOR,circles[RIGHT][0],0,0) ;
    }

  BLT_MEM_TO_SCRN(705,25,45,45,RRPL,fruitpics[fruitmaze[player]],0,0) ;
  BLT_SCRN(700,20,55,55,RINV) ;
  SPRINTF(buffer,"%1d0",fruitscore(fruitmaze[player])) ;
  WRITELN(710,15,buffer) ;

  if (autoplay && (!demomode))
    {
      SCHRFUNC(RXOR) ;
      WRITELN(339,YBASE+SQUARE*16,"GAME OVER!") ;
      SCHRFUNC(RRPL) ;
      WRITELN(300,65,"Type DEL to begin") ;
      WRITELN(480,50,"Auto Score") ;
      WRITELN(489,65,"0") ;
    }
}


docredits()
 
{
  int g,x,y,i,j ;
 
  credits = 1 ;
  clear_screen() ;
  dohelp() ;
  WRITELN(5,860,"Type DEL to begin") ;
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
  movei = 1 ;
  progstate = MOVELEFT ;
}


move_left()      /* Animate screen and bugs left. */

{
  int g ;

  if (movei % 8) cirx-- ;
  drawcir(circles[LEFT][inc],cirx,ciry) ;
  if (movei % 4 == 0) inc = (inc + 1) % 4 ;
  for (g = POKEY; g <= SHADOW; g++)
    {
      drawbug(&bugs[g]) ;               /* Erase old. */
      bugs[g].scrx-- ;
      if (movei % 13 == 0) bugs[g].pic = (bugs[g].pic + 1) % 2 ;
      if (movei % 18 == 0) bugs[g].dir = (bugs[g].dir + 1) % 4 ;
      drawbug(&bugs[g]) ;               /* Draw new. */
    }
  if (++movei > 662)
    {
      credits = 2 ;
      drawdot(dotx,doty,BIGDOT) ;
      for (g = POKEY; g <= SHADOW; g++)
        {
          drawbug(&bugs[g]) ;           /* Erase old. */
          bugs[g].bluetime = 32000 ;
          drawbug(&bugs[g]) ;           /* Draw new as blue. */
        } 
      SCHRFUNC(RXOR) ;
      movej = 200 ;
      movex = 1 ;
      movei = 1 ;
      progstate = MOVERIGHT ;
    }
}


move_right()    /* Animate eating screen and bugs right. */

{
  int g,i ;

   PPAUSE(8*movex) ;
   if (movei % 26) cirx++ ;
   drawcir(circles[RIGHT][inc],cirx,ciry) ;
   if (movei % 4 == 0) inc = (inc + 1) % 4 ;
   for (g = POKEY; g <= SHADOW; g++)
     if (!bugs[g].eyesonly)
       {
         drawbug(&bugs[g]) ;            /* Erase old. */
         if (movei % 2) bugs[g].scrx++ ;
         if (movei % 13 == 0) bugs[g].pic = (bugs[g].pic + 1) % 2 ;
         if (cirx >= bugs[g].scrx-20)
           {
             bugs[g].eyesonly = 1 ;
             SPRINTF(buffer,"%1d",movej) ;
             WRITELN(bugs[g].scrx-20,440,buffer) ;
             for (i = 0; i < 60; i++) LONGPAUSE() ;
             SPRINTF(buffer,"%1d",movej) ;
             WRITELN(bugs[g].scrx-20,440,buffer) ;
             movej *= 2 ;
             movex++ ;
           }
         else drawbug(&bugs[g]) ;       /* Draw new. */
       }
  if (++movei > 665)
    {
      SCHRFUNC(RRPL) ;
      for (i = 0; i < 100; i++) LONGPAUSE() ;
      credits = 0 ;
      progstate = INITGAME ;
    }
}


dohelp()

{
  int g,i,x,y ;
  char line[MAXLINE] ;
  FILE *fn ;

  write_bold(105,100,titlestring) ;
  WRITELN(105,120,"Original version by Brad A. Myers with pictures of fruit by Terry Vavra.") ;

  for (g = POKEY; g <= SHADOW; g++)
    {
      if (g > 1) y = YBASE + 130 ;
      else y = YBASE + 50 ;
      x = (g % 2) ? 384 : 100 ;

      BLT_MEM_TO_SCRN(x,y,45,45,RRPL,bugpics[g][0],0,0) ;
      BLT_MEM_TO_SCRN(x,y,45,21,RXOR,eyes[g],0,0) ;
      for (i = 0; i < 40; i++) LONGPAUSE() ;
      SPRINTF(buffer,"- %s",names[g]) ;
      WRITELN(x+60,y+25,buffer) ;
      for (i = 0; i < 40; i++) LONGPAUSE() ;
    }

  if ((fn = fopen(h_name,"r")) == NULL)
    {
      FPRINTF(stderr,"\nsidtool: can't open %s\n",h_name) ;
      exit(-1) ;
    }
  x = 105 ;
  y = 465 ;
  i = 0 ;
  while (get_string(fn,line) != -1)
    {
      WRITELN(x,i*15+y,line) ;
      i++ ;
    }
  if (!autoplay) make_control_panel() ;
  if (!autoplay) display_settings() ;
}


doplay()

{
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
      WRITELN(357,YBASE+SQUARE*16,"READY!") ;
      blinkpause() ;
      SCHRFUNC(RXOR) ;
      WRITELN(357,YBASE+SQUARE*16,"READY!") ;
      SCHRFUNC(RRPL) ;
    }
}


make_play()         /* Perform next movement of each sid tool object. */

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
      if (autoplay)
        newdir = dorandomdir(curdir,posx,posy,cirmx,cirmy,&x,&y,&nx,&ny,1) ;
      else
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
          progstate = RESETGAME ;
          longjmp(exception,val) ;
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
      PPAUSE(pausetime*70) ;
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

  PPAUSE(pausetime*30) ;
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

  PPAUSE(pausetime*30) ;
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
  if (!c) return ;
  else if (c >= BUT_AUTO+2 && c <= BUT_START+2)
    {
      BLT_SCRN(SQUARE/2+100*(c-2)+BUTXOFF,SQUARE/2+BUTYOFF,70,SQUARE,RINV) ;
      PPAUSE(pausetime*100) ;
      BLT_SCRN(SQUARE/2+100*(c-2)+BUTXOFF,SQUARE/2+BUTYOFF,70,SQUARE,RINV) ;
      progstate = MAKESEL ;
      button = c - 2 ;
      c = 0 ;
    }
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
      exit(-1) ;
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
      exit(-1) ;
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
  int i,j ;

  erasebugs() ;
  LONGPAUSE() ;
  for (i = 1; i <= 20; i++)
    {
      BLT_MEM_TO_SCRN(oldcx-GOFFSET+5,oldcy-GOFFSET+5,45,45,RXOR,curcircle,0,0) ;
      for (j = 0; j < 10; j++) LONGPAUSE() ;
    }
  LONGPAUSE() ;
  if (fruitmaze[player] < 8) fruitmaze[player]++ ;
  fruitchances[player] = 0 ;
  setdots(player) ;
  drawmaze() ;
  if (curbluetime[player] > 1) curbluetime[player] -= 60 ;
}


updatefruit()

{
  Pixrect *p ;

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
        WRITELN(489,65,buffer) ;
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
  WRITELN(x,y,buffer) ;
  if (score[player] > highscore)
    {
      highplayer = player ;
      highscore = score[player] ;
      SPRINTF(buffer,"%1d0",highscore) ;
      WRITELN(348,35,buffer) ;
    }
}
