
/*  sid_main.c
 *
 *  SIDtool - the SUN Interactive Debugger program.
 *  Written by Rich Burridge - SUN Australia - June 1986.
 *
 *  Version 1.5
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include "sidtool.h"
#include "bltstuff.h"
#include "exceptions.h"

extern etext() ;

/*  Exceptions.
 *  -----------
 *
 *  delhit    - Raised when DEL key hit so game can be restarted.
 *
 *  doleave   - Raised when game should be restarted with autoplay true.
 *
 *  god       - Raised to create the slave SIDtool process.
 *
 *  resetgame - Raised inside play only when need to redraw maze.
 *              Parameters: If died then remove one of the screens.
 */

exception(delhit) ;
exception(doleave) ;
exception(god) ;
exception(resetgame) ;

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
char titlestring[MAXLINE] ;

int blueblink,blueincblink,boxx,boxy,c,fruittime,fruitx,fruity,numplayers ;
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

int curdir ;                            /* Current direction of the screen. */
int oldcurdir ;                         /* Old direction of the screen. */
int g,newdir,posx,posy,x,y,nx,ny,count,inc ;
int orgx,orgy,width,height ;            /* Used by getwindowparms. */
int oldcx,oldcy ;                       /* Old position of the screen. */
int on = 0 ;                            /* Current blinking state of score. */
int prog_type ;                         /* Whether program is server or client. */
int credits ;                           /* Direction of credits if on. */
int cirx ;                              /* X position of screen during credits. */
int ciry ;                              /* Y position of screen during credits. */
int dotx ;                              /* X position of BIGDOT during credits. */
int doty ;                              /* Y position of BIGDOT during credits. */
int winsetup = -1 ;                     /* To prevent screen being redrawn twice. */

BOOLEAN autoplay ;
BOOLEAN changed = FALSE ;               /* Whether window has been changed. */
BOOLEAN demomode ;
BOOLEAN fruiton ;
BOOLEAN gamestate ;                     /* State of the game, 1 = remove circle. */
BOOLEAN remove ;                        /* Whether Sun screen should be removed. */
BOOLEAN started = TRUE ;                /* If 1, program is running. */
 
FILE *fopen() ;

extern int sfunc ;                      /* Used by SCHRFUNC for cursor function. */


drawbox(mx,my)
int mx,my ;

/*  Draws a box starting at maze position mx, my. Mx, my should be an
 *  s or S position in maze. Travels around path until reach x or s.
 */

{
  int last ;

  last = 'r' ;
  PPAUSE(pausetime*10) ;
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
       PPAUSE(pausetime) ;
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
  int fd,level,len ;

  if ((highscore >= allhighscores[skilllevel].score) &&
        (highplayer != -1) && (!demomode))
    {
      clear_screen() ;
      make_control_panel() ;
      display_settings() ;
      SPRINTF(buffer,"Player %1d has beaten the high score for skill level %1d.",
                     highplayer,skilllevel) ;
      writeln(100,140,buffer) ;
      if (allhighscores[skilllevel].score)
        {
          SPRINTF(buffer,"The old record was %1d0 held by: %s.",
                         allhighscores[skilllevel].score,
                         allhighscores[skilllevel].who) ;
          writeln(100,160,buffer) ;
        }
      do
        {
          len = 1 ;
          SPRINTF(buffer,"Type player %1d's name or initials: ",highplayer) ;
          writeln(100,200,buffer) ;
          if (!(len = getline(allhighscores[skilllevel].who,365,200)))
            writeln(100,220,"   ** No name given. **") ;
        }
      while (!len) ;
      allhighscores[skilllevel].score = highscore ;
      if ((fd = open(s_name,1)) == -1)
        FPRINTF(stderr,"sidtool: unable to open highscores file.\n") ;
      else
        { 
          for (level = 1; level <= 10; level++) puths(fd,allhighscores[level]) ;
          CLOSE(fd) ;
        }
    }
}


initgame()

{
  int i,j ;

  if (autoplay) autoscore = 0 ;
  pausetime = -skilllevel * 20 + 200 ;
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
  writeln(x,y,buffer) ;
  if (!score[player]) writeln(x+27,y+15,"0") ;
  else
    {
      SPRINTF(buffer,"%1d0",score[player]) ;
      writeln(x+27,y+15,buffer) ;
    }
  SCHRFUNC(RRPL) ;
}


blinkpause()

/*  Wait a while and check keyboard for commands while blinking current players score. */

{
  int i,j ;

  when(delhit)
    etype(1)
      if (!on) showplayerscore(player) ;
      forget(delhit) ;
      raise(delhit,1) ;
      break ;
  endwhen

  on = 1 ;
  if (!autoplay)
    for (i = 1; i <= 16; i++)
      {
        showplayerscore(player) ;
        on = !on ;
        for (j = 0; j < 10; j++) LONGPAUSE() ;
      }
  forget(delhit) ;
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
          PPAUSE(2*pausetime) ;               /* This should probabily be improved. */
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
  prog_type = MASTER ;

  when (god)
    etype(1)
      prog_type = SLAVE ;
      break ;
  endwhen ;

  get_options(argc,argv) ;          /* Is this the server or the client? */
  set_options() ;                   /* Initialise tool variables. */
  if (prog_type == MASTER)
    {
      starttool(orgx,orgy,width,height,titlestring) ;
      exit(0) ;                     /* Slave is dead, master commits suicide. */
    }

  initrandom() ;

  numplayers = 1 ;
  skilllevel = 5 ;
  lastnumplayers = 1 ;
  autoplay = FALSE ;
  initgame() ;
  initialize() ;
  autoplay = 1 ;

  for (;;)
    {
      initgame() ;
      play() ;
      dorest() ;
    }
}
