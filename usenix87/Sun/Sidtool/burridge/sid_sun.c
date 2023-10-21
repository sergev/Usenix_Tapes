 
/*  sid_sun.c
 *
 *  Various graphics functions used by Sid Tool.
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
#include <sundev/kbd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sundev/kbio.h>
#include "bltstuff.h"
#include "sidtool.h"
#include <suntool/sunview.h>
#include <suntool/canvas.h>

Cursor nullcur,syscur ;      /* Sid Tool cursors. */
Pixfont *pf ;
Pixrect *bigdot,*bluebug[2],*bluepics[2],*bugpics[4][2],*circleexplode[9] ;
Pixrect *circles[4][4],*corner[4],*curcircle,*eyes[4],*fruitpics[9] ;
Pixrect *smalldot ;

extern Canvas canvas ;
extern Pixwin *pw ;

short  nullcur_data[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
} ;
mpr_static(nullcur_pr,16,16,1,nullcur_data) ;

short  syscur_data[] = {
#include "main.cursor"
} ;
mpr_static(syscur_pr,16,16,1,syscur_data) ;

int sfunc ;


get_key(fd,station,value,count)
int fd,station,count ;
char value[MAXLINE] ;

{
  struct kiockey key ;

  key.kio_tablemask = 0 ;
  key.kio_entry = count ;
  key.kio_station = station ;
  IOCTL(fd,(int) KIOCGETKEY,(char *) &key) ;
  STRCPY(value,key.kio_string) ;
}


iocursormode(m)               /* Set the current cursor display mode. */
int m ;

{
  switch (m)
    {
      case OFFCURSOR   : nullcur = window_get(canvas,WIN_CURSOR) ;
                         cursor_set(nullcur,CURSOR_IMAGE,&nullcur_pr,0) ;
                         window_set(canvas,WIN_CURSOR,nullcur,0) ;
                         break ;
      case TRACKCURSOR : syscur = window_get(canvas,WIN_CURSOR) ;
                         cursor_set(syscur,CURSOR_IMAGE,&syscur_pr,0) ;
                         cursor_set(syscur,CURSOR_OP,PIX_SRC ^ PIX_DST,0) ;
                         window_set(canvas,WIN_CURSOR,syscur,0) ;
    }
}


Pixrect *load_picture(fd)
int fd ;

{
  Pixrect *name ;
  struct pr_size size ;
  int i ;
  unsigned int *ptr,temp_area[128] ;

  size.x = 64 ;
  size.y = 64 ;
  name = mem_create(size,1) ;
  READ(fd,(char *) temp_area,512) ;
  ptr = (unsigned int *) ((struct mpr_data *) name->pr_data)->md_image ;
  for (i = 0; i < 128; i++) ptr[i] = temp_area[i] ;
  return(name) ;
}


set_key(fd,station,value,count)
int fd,station,count ;
char value[MAXLINE] ;

{
  struct kiockey key ;

  key.kio_tablemask = 0 ;
  key.kio_entry = count ;
  key.kio_station = station ;
  STRCPY(key.kio_string,value) ;
  IOCTL(fd,(int) KIOCSETKEY,(char *) &key) ;
}


write_bold(x,y,text)   /* Write text in a "pseudo" bold font. */
int x,y ;
char text[MAXLINE] ;

{
  pw_text(pw,x,y,ROR,pf,text) ;
  pw_text(pw,x+1,y,ROR,pf,text) ;
}
