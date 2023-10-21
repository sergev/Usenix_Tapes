
/*  sid_sun.c
 *
 *  SUN dependent functions and procedures used by the SID tool.
 *  Written by Rich Burridge - SUN Australia, June 1986.
 *
 *  Version 1.5
 *
 *  Routines.
 *  ---------
 *
 *  lose
 *  sigwinched
 *  sigchild
 *  toolsw_selected
 *  toolsw_sighandler
 *  starttool
 *  startup
 *  iocursormode
 *  load_picture
 *  get_key
 *  set_key
 *  write_bold
 *  writeln
 *  getwindowparms
 *  wgread
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include <sundev/kbd.h>
#include "sidtool.h"
#include "exceptions.h"
#include "bltstuff.h"
#include <sys/timeb.h>
#include <suntool/tool_hs.h>
#include <suntool/emptysw.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sundev/kbio.h>
#include <signal.h>
#include <errno.h>

exexception(god) ;

extern int errno ;
struct pixwin     *pixwin ;
struct tool       *tool ;
struct toolsw     *emptysw ;
struct inputevent ie ;
struct inputmask  im ;
sigwinched(),sigchild() ;
toolsw_selected(),toolsw_sighandler() ;
int sfunc ;
int pid ;                            /* Process id of slave program. */
int finished = 0 ;                   /* 1 -> socket no longer available. */

extern int c ;           /* Used by wgread for mouse and keyboard interaction. */
extern int width,height ;            /* Dimensions of the SID tool window. */

static short sid_image[256] = {
#include "sidtool.icon"
} ;
mpr_static(sid_mpr,64,64,1,sid_image) ;

static struct icon icon =
                     {
                       64,64,(struct pixrect *)NULL,0,0,64,64,
                       &sid_mpr,0,0,0,0,NULL,(struct pixfont *)NULL,
                       ICON_BKGRDCLR
                     } ;
 
extern struct pixfont *pw_pfsysopen() ;       /* GFX stuff. */
static struct pixfont *pixfont ;
struct gfxsubwindow *gfx ;
char name[WIN_NAMESIZE] ;
 
static struct cursor nullcur,syscur ;              /* Cursor stuff. */
static short  nullicon[CUR_MAXIMAGEWORDS],sysicon[CUR_MAXIMAGEWORDS] ;
mpr_static(nullpix,8*sizeof(nullicon[0]),
           sizeof(nullicon)/sizeof(nullicon[0]),1,nullicon) ;
mpr_static(syscurpix,8*sizeof(sysicon[0]),
           sizeof(sysicon)/sizeof(sysicon[0]),1,sysicon) ;

#define  TUNIT     0                               /* Socket stuff. */
static struct timeval timeout = {TUNIT,TUNIT} ;
static struct sockaddr_un client,server ;
static int mask,sin,sout,snew,slen ;
 
struct pixrect *bigdot ;
struct pixrect *bluebug[2] ;
struct pixrect *bluepics[2] ;
struct pixrect *bugpics[4][2] ;
struct pixrect *circleexplode[9] ;
struct pixrect *circles[4][4] ;  /* Pointer to screen pictures. */
struct pixrect *corner[4] ;      /* Pointer to button corner pictures. */
struct pixrect *curcircle ;      /* Pointer to the current screen picture. */
struct pixrect *eyes[4] ;
struct pixrect *fruitpics[9] ;
struct pixrect *smalldot ;


lose(str)
char *str ;

{
  FPRINTF(stderr,str) ;
  exit(1) ;
}


static sigwinched()
{
  tool_sigwinch(tool) ;
}


static sigchild()
{
  finished = 1 ;
  tool_sigchld(tool) ;
}


/*ARGSUSED*/
static toolsw_selected(sw,ibits,obits,ebits,timer)
caddr_t sw ;
int *ibits,*obits,*ebits ;
struct timeval **timer ;

{
  char c ;
  int i ;

  if (*ibits & (1 << emptysw->ts_windowfd))
    {
      input_readevent(emptysw->ts_windowfd,&ie) ;
      switch (ie.ie_code)
        {
          case MS_LEFT   :
          case MS_MIDDLE :
          case MS_RIGHT  : if (ie.ie_locy > BUTYOFF && ie.ie_locy < BUTYOFF+2*SQUARE)
                             for (i = BUT_AUTO; i <= BUT_START; i++)
                               if (ie.ie_locx > BUTXOFF+i*100 &&
                                   ie.ie_locx < BUTXOFF+i*100+120)
                                 {
                                   c = (char) i+2 ;
                                   WRITE(snew,&c,1) ;
                                 }
        }
      if ((ie.ie_code >= ASCII_FIRST) && (ie.ie_code <= ASCII_LAST))
        {
          c = (char) ie.ie_code ;
          WRITE(snew,&c,1) ;
        }
    }
  *ibits = 0 ;
}


static toolsw_sighandler()

{
  char c ;

  if (!finished)
    {
      c = WIN_DAMAGED ;
      WRITE(snew,&c,1) ;
      pw_damaged(pixwin) ;
      pw_donedamaged(pixwin) ;
    }
}


starttool(orgx,orgy,width,height,toolname)
int orgx,orgy,width,height ;
char *toolname ;

{
  char **tool_attrs = NULL ;

  function_keys(KEY_SET) ;                    /* Set direction arrow function keys. */
  UNLINK(SOCKNAME) ;                          /* Remove socket file. */
  sout = socket(AF_UNIX,SOCK_STREAM,0) ;      /* Create server socket. */
  server.sun_family = AF_UNIX ;
  STRCPY(server.sun_path,SOCKNAME) ;
  BIND(sout,(struct sockaddr *) &server,strlen(SOCKNAME)+2) ;
  LISTEN(sout,5) ;

  tool = tool_make(WIN_LABEL,     toolname,   WIN_ICON,      &icon,
                   WIN_LEFT,      orgx,       WIN_TOP,        orgy,
                   WIN_WIDTH,     width,      WIN_HEIGHT,     height,
                   WIN_ATTR_LIST, tool_attrs, 0) ;
  if (tool == (struct tool *)NULL) lose("Couldn't create tool") ;
  tool_free_attribute_list(tool_attrs) ;

  emptysw = esw_createtoolsubwindow(tool,"",TOOL_SWEXTENDTOEDGE,TOOL_SWEXTENDTOEDGE) ;
  if (emptysw == (struct toolsw *)NULL) lose("Couldn't create empty subwindow") ;

  pixwin = pw_open(emptysw->ts_windowfd) ;
  if (pixwin == (struct pixwin *)NULL) lose("Couldn't create pixwin.") ;

  emptysw->ts_io.tio_handlesigwinch = toolsw_sighandler ;
  emptysw->ts_io.tio_selected = toolsw_selected ;

  input_imnull(&im) ;
  im.im_flags |= IM_ASCII ;
  win_setinputcodebit(&im,MS_LEFT) ;
  win_setinputcodebit(&im,MS_MIDDLE) ;
  win_setinputcodebit(&im,MS_RIGHT) ;
  win_setinputmask(emptysw->ts_windowfd,&im,(struct inputmask *)NULL,WIN_NULLLINK) ;

  (void) signal(SIGWINCH,sigwinched) ;
  tool_install(tool) ;                        /* Install the tool. */
  win_fdtoname(emptysw->ts_windowfd,name) ;   /* Make GFX window name. */
  we_setgfxwindow(name) ;

  if ((pid = fork()) == NULL) raise(god,1) ;  /* Fork graphics program. */

  slen = sizeof(server) ;
  snew = accept(sout,(struct sockaddr *) &server,&slen) ;   /* Accept client socket. */

  (void) signal(SIGCHLD,sigchild) ;
  tool_select(tool,1) ;                       /* Notification loop. */
  tool_destroy(tool) ;                        /* Die gracefully. */
  KILL(pid,SIGKILL) ;                         /* Murder the child !! */
  CLOSE(snew) ;
  CLOSE(sout) ;
  function_keys(KEY_RESET) ;                  /* Restore direction arrow function keys. */
}


startup()

{
  sfunc = PIX_SRC ;                  /* Used by writeln */

  sin = socket(AF_UNIX,SOCK_STREAM,0) ;       /* Create client socket. */
  client.sun_family = AF_UNIX ;
  STRCPY(client.sun_path,SOCKNAME) ;
  CONNECT(sin,(struct sockaddr *) &client,strlen(client.sun_path)+2) ;

  gfx = gfxsw_init(0,0) ;
  if (gfx == (struct gfxsubwindow *)NULL) lose("Couldn't create graphics window.") ;

  pixfont = pw_pfsysopen() ;
  if (pixfont == (struct pixfont *)NULL) lose("Couldn't create default font.") ;

  syscur.cur_shape = &syscurpix ;
  win_getcursor(gfx->gfx_windowfd,&syscur) ;
  syscur.cur_function = RXOR ;

  nullcur.cur_shape = &nullpix ;
  win_getcursor(gfx->gfx_windowfd,&nullcur) ;
  nullcur.cur_xhot = nullcur.cur_yhot = 0 ;
  nullcur.cur_function = RXOR ;
  nullcur.cur_shape->pr_size.x = 0 ;
  nullcur.cur_shape->pr_size.y = 0 ;

  iocursormode(OFFCURSOR) ;  
}


iocursormode(m)               /* Set the current cursor display mode. */
int m ;

{
  switch (m)
    {
      case OFFCURSOR   : win_setcursor(gfx->gfx_windowfd,&nullcur) ;
                         break ;
      case TRACKCURSOR : win_setcursor(gfx->gfx_windowfd,&syscur) ;
    }
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


struct pixrect *load_picture(fd)
int fd ;

{
  struct pixrect *name ;
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


write_bold(x,y,text)
int x,y ;
char text[MAXLINE] ;

{
  pw_text(gfx->gfx_pixwin,x,y,ROR,pixfont,text) ;
  pw_text(gfx->gfx_pixwin,x+1,y,ROR,pixfont,text) ;
}


writeln(x,y,text)                 /* Write a line of text to the screen at x,y. */
int x,y ;
char *text ;

{
  pw_text(gfx->gfx_pixwin,x,y,sfunc,pixfont,text) ;
}


getwindowparms(orgx,orgy,width,height)           /* Return window dimensions. */
int *orgx,*orgy,*width,*height ;

{
  struct rect rect ;

  win_getsize(gfx->gfx_windowfd,&rect) ;
  *orgx = rect.r_left ;
  *orgy = rect.r_top ;
  *width = rect.r_width ;
  *height = rect.r_height ;
}


wgread()        /* If key has been pressed, the value is returned. */

{
  char c ;

  c = '\0' ;
  mask = 1 << sin ;
  SELECT(32,&mask,(int *) 0,(int *) 0,&timeout) ;
  if (mask & (1 << sin)) READ(sin,&c,1) ;
  return((int) c) ;
}
