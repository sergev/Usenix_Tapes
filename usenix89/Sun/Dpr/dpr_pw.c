/*LINTLIBRARY*/

/*
 *  dpr_pw.c
 *
 *  Routine to dump the contents of the current window to the specified
 *  printer. The printer name is passed as the parameter to print_window
 *  and it takes the remaining details from the /etc/dumpcap file using
 *  the name specified. From these details it forks dpr as another
 *  process before terminating.
 *  Copyright (c) Rich Burridge, Sun Australia 1986.
 *
 *  Version 1.1.
 *
 *  No responsibility is taken for any errors inherent either in the comments
 *  or the code of this program, but if reported to me then an attempt will
 *  be made to fix them.
 */

#include "globals.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <strings.h>
#include <pixrect/pixrect_hs.h>
#include <sunwindow/rect.h>
#include <sunwindow/rectlist.h>
#include <sunwindow/pixwin.h>
#include <sunwindow/cms.h>
#include <sunwindow/win_struct.h>
#include <sunwindow/win_input.h>
#include <sunwindow/win_screen.h>
#include <sunwindow/win_ioctl.h>
#include <suntool/wmgr.h>

char *sprintf() ;
mpr_static(scr_pr,MAXWIDTH,MAXHEIGHT,MAXDEPTH,sp.scr) ;

static char filename[MAXSTR] ;      /* Filename of screen image in /tmp. */
static char dumpname[MAXSTR] ;      /* Customised dumpcap parameters in /tmp. */
static int  xval ;                  /* X origin of current window. */
static int  yval ;                  /* Y origin of current window. */
static int  wval ;                  /* Width of current window. */
static int  hval ;                  /* Height of current window. */
static int  dval ;                  /* Depth of current window. */
FILE *fn,*fopen() ;                 /* For screen image and dumpcap files. */

/*
 *  C and PASCAL interface.
 */

print_window(printer)
char printer[MAXSTR] ;

{
  which_window() ;             /* Determine the window, program is running in. */
  expose_window() ;            /* Pop to the front if it is not fully exposed. */
  save_window(filename) ;      /* Copy current screen image to /tmp directory. */
  read_params("/etc/dumpcap",printer) ;     /* Get dumpcap parameters. */
  create_dumpcap(dumpname,printer) ;        /* Create dumpcap file for window. */
  fork_dpr(dumpname,printer) ;              /* Fork dpr with command line. */
}


which_window()

/*  Determines the x,y origin of the window on the screen, and the width
 *  and height of the window.
 *
 *  NOTE. There really has got to be a better way of doing this. After
 *        buggering around with this for several days, here is a solution
 *        that at least works.
 */

{
  extern char *getenv() ;
  char name[WIN_NAMESIZE] ;
  int bx,by,bw,bh,bnum,link,parentfd,toolfd,wnum ;
  struct rect rect ;

  STRCPY(name,getenv("WINDOW_GFX")) ;
  SSCANF(name,"/dev/win%d",&bnum) ;
  if ((toolfd = open(name,O_RDONLY,0)) < 0)
    {
      FPRINTF(stderr,"dpr: %s would not open\n",name) ;
      exit(1) ;
    }
  win_getrect(toolfd,&rect) ;
  bx = rect.r_left ;
  by = rect.r_top ;
  bw = rect.r_width ;
  bh = rect.r_height ;
  CLOSE(toolfd) ;

  if (we_getparentwindow(name))                   /* Determine parent. */
    {
      FPRINTF(stderr,"dpr: %s not passed parent window in environment\n",name) ;
      exit(1) ;
    }

  for (;;)                                            /* Determine root. */
    {
      if ((parentfd = open(name,O_RDONLY,0)) < 0)
        {
          FPRINTF(stderr,"dpr: %s (parent) would not open\n",name) ;
          exit(1) ;
        }
      link = win_getlink(parentfd,WL_PARENT) ;
      if (link == WIN_NULLLINK) break ;
      win_numbertoname(link,name) ;
      CLOSE(parentfd) ;
    }

  for (link = win_getlink(parentfd,WL_OLDESTCHILD); link != WIN_NULLLINK;)
    {
      win_numbertoname(link,name) ;
      SSCANF(name,"/dev/win%d",&wnum) ;
      if ((toolfd = open(name,O_RDONLY,0)) < 0)
        {
          FPRINTF(stderr,"dpr: %s (tool) would not open\n", name) ;
          exit(1) ;
        } 
      if (bnum == wnum+1)
        {
          win_getrect(toolfd,&rect) ;
          if (bw+10 != rect.r_width || bh+23 != rect.r_height)
            {
              FPRINTF(stderr,"dpr: window has incorrect dimensions.\n") ;
              exit(1) ;
            }
          xval = rect.r_left + bx ;
          yval = rect.r_top + by ;
          wval = bw ;
          hval = bh ;
          dval = 1 ;               /* Only works for monochrome currently. */
        }
      link = win_getlink(toolfd,WL_YOUNGERSIB) ;
      CLOSE(toolfd) ;
    }
  CLOSE(parentfd) ;
}


expose_window()     /*  NOTE.  This is intentionally left empty in V1.0. */

{
}


save_window(filename)    /* Copy current window image to /tmp directory. */
char filename[MAXSTR] ;

{
  int i,j,llength ;
  struct rasterfile header ;
  struct pixrect *pr,*pr_open() ;

  SPRINTF(filename,"/tmp/dpr_screen%1d",(int) getpid()) ;
  if ((pr = pr_open("/dev/fb")) == NULL)
    {
      FPRINTF(stderr,"dpr: can't open /dev/fb.\n") ;
      exit(1) ;
    } 
  if ((fn = fopen(filename,"w")) == NULL)
    {
      FPRINTF(stderr,"dpr: can't open %s\n",filename) ;
      exit(1) ;
    }
  pr_rop(&scr_pr,0,0,wval,hval,PIX_SRC,pr,xval,yval) ;
  PR_CLOSE(pr) ;

  header.ras_magic = RAS_MAGIC ;
  header.ras_width = wval ;
  header.ras_height = hval ;
  header.ras_depth = dval ;
  header.ras_length = (wval+15) / B_CH * dval * hval ;
  header.ras_type = RT_STANDARD ;
  header.ras_maptype = RMT_NONE ;
  header.ras_maplength = 0 ;
  if (fwrite((char *)&header,sizeof(struct rasterfile),1,fn) != 1)
    {
      FPRINTF(stderr,"dpr: can't write %s\n",filename) ;
      exit(1) ;
    }

  llength = (wval+15) / B_CH * dval ;
  for (i = 0; i < hval; i++)
    for (j = 0; j < llength; j++) FPRINTF(fn,"%c",sp.scr[i][j]) ;
  FCLOSE(fn) ;
}


dcwrite(entry,term)
char entry[MAXSTR],term[MAXSTR] ;

{
  int i ;

  FPRINTF(fn,"\t:%s=",term) ;
  for (i = 0; i < strlen(entry); i++)
    if (entry[i] < 64) FPRINTF(fn,"\\0%2o",entry[i]) ;
    else FPRINTF(fn,"%c",entry[i]) ;
  FPRINTF(fn,":\\\n") ;
}


create_dumpcap(dumpname,printer)
char dumpname[MAXSTR],printer[MAXSTR] ;

{
  SPRINTF(dumpname,"/tmp/dpr_dumpcap%1d",(int) getpid()) ;
  if ((fn = fopen(dumpname,"w")) == NULL)
    {
      FPRINTF(stderr,"dpr: can't open %s\n",dumpname) ;
      exit(1) ;
    }
  FPRINTF(fn,"%s:\\\n",printer) ;

  if (BR != 9600) FPRINTF(fn,"\t:br#%1d:\\\n",BR) ;
  if (DD == TRUE) FPRINTF(fn,"\t:dd:\\\n") ;
  if (HA)         FPRINTF(fn,"\t:ha#%1d:\\\n",HA) ;
  if (strlen(ED)) dcwrite(ED,"ed") ;
  if (strlen(EL)) dcwrite(EL,"el") ;
  if (NB != 8)    FPRINTF(fn,"\t:nb#%1d:\\\n",NB) ;
  if (strlen(SB)) dcwrite(SB,"sb") ;
  if (strlen(SD)) dcwrite(SD,"sd") ;
  if (strlen(SL)) dcwrite(SL,"sl") ;
  if (VS != 1)    FPRINTF(fn,"\t:vs#%1d:\\\n",VS) ;

  FPRINTF(fn,"\t:sh#%1d:fn=%s:lp=%s:\n",hval,filename,LP) ;
  FCLOSE(fn) ;
}


fork_dpr(dumpname,printer)
char dumpname[MAXSTR],printer[MAXSTR] ;

{
  char dcparam[MAXSTR],prparam[MAXSTR] ;

  SPRINTF(dcparam,"-D%s",dumpname) ;             /* Assemble dpr parameters. */
  SPRINTF(prparam,"-P%s",printer) ;
  if (fork() == NULL)                            /* Fork dpr program. */
    execl("dpr","dpr",dcparam,prparam,"-r",0) ;
}

/*
 *  FORTRAN interface.
 *
 *  F77 appends an underscore to the name of a procedure to distinguish
 *  it from C procedures or external variables with the same user-assigned name.
 */

priwin_(printer)
char printer[MAXSTR] ;

{
  print_window(printer) ;
}
