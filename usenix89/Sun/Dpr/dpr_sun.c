/*LINTLIBRARY*/
 
/*
 *  dpr_sun.c
 *
 *  SUN dependent routines for the dpr program, and associated software.
 *  Copyright (c) Rich Burridge, Sun Australia 1986.
 *
 *  Version 1.1.
 *
 *  No responsibility is taken for any errors inherent either in the comments
 *  or the code of this program, but if reported to me then an attempt will
 *  be made to fix them.
 */

#include "dpr.h"
#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <pixrect/pixrect_hs.h>

extern int errno ;                             /* Standard error reply. */
extern struct sparams sp ;                     /* Screen image parameters. */
extern unsigned char color_map[3][MAXCMAP] ;   /* Colormap values. */
extern struct dumpcap dc[] ;                   /* Dumpcap parameters. */
mpr_static(scr_pr,MAXWIDTH,MAXHEIGHT,MAXDEPTH,sp.scr) ;


what_video(sp)                     /* Extract screen size parameters. */
struct sparams *sp ;

/*  Opens /dev/fb. If successful, tries an IO control call to stuff
 *  frame buffer attributes into the fbinfo struct.
 */

{
  int fd ;
  struct fbtype fbinfo ;           /* fbtype defined in fbio(4s). */

  if ((fd = open("/dev/fb",O_RDWR)) == -1)
    {
      FPRINTF(stderr,"dpr: error %d opening /dev/fb.\n",errno) ;
      exit(1) ;
    }
  if ((ioctl(fd,(int)FBIOGTYPE,(char *)&fbinfo)) == -1)
    {
      FPRINTF(stderr,"dpr: ioctl error %d on /dev/fb.\n",errno) ;
      exit(1) ;
    }
  CLOSE(fd) ;
  sp->width = fbinfo.fb_width ;     /* Screen width in pixels. */
  sp->height = fbinfo.fb_height ;   /* Screen height in pixels. */
  sp->depth = fbinfo.fb_depth ;     /* Screen depth in pixels. */
  sp->csize = fbinfo.fb_cmsize ;    /* Colormap size. */
  sp->fbsize = fbinfo.fb_size ;     /* Total frame buffer size. */
}


get_screen_image(filename)         /* Get screen image from screen or file. */
char filename[MAXSTR] ;

{
  int cval,i,j,len_cmap,llength ;
  struct pixrect *pr, *pr_open() ;
  struct rasterfile header ;
  FILE *fn,*fopen() ;

  if (!strlen(filename))
    {
      if ((pr = pr_open("/dev/fb")) == NULL)
        {
          FPRINTF(stderr,"dpr: can't open /dev/fb.\n") ;
          exit(1) ;
        }
      pr_rop(&scr_pr,0,0,pr->pr_width,pr->pr_height,PIX_SRC,pr,0,0) ;
      PR_CLOSE(pr) ;
    }
  else
    {
      if ((fn = fopen(filename,"r")) == NULL)
        {
          FPRINTF(stderr,"dpr: can't open %s\n",filename) ;
          exit(1) ;
        }
      if (fread((char *)&header,sizeof(struct rasterfile),1,fn) != 1)
        {
          FPRINTF(stderr,"dpr: can't read %s\n",filename) ;
          exit(1) ;
        }
      if (header.ras_magic != RAS_MAGIC)
        {
          FPRINTF(stderr,"dpr: %s is not a raster file.\n",filename) ;
          exit(1) ;
        }
      llength = ((header.ras_width+15) >> 4) * B_SHORT / B_CH * header.ras_depth ;
      if (header.ras_maptype == RMT_EQUAL_RGB)         /* Color image. */
        {
          cval = 8 - (CO >> 3) ;                       /* Color reduction factor. */
          len_cmap = header.ras_maplength / 3 ;        /* Calculate color map length. */
          for (i = 0 ; i < 3; i++)                     /* Load color map. */
            for (j = 0; j < len_cmap; j++) color_map[i][j] = getc(fn) >> cval ;
        }
      for (i = 0; i < header.ras_height; i++)
        for (j = 0; j < llength; j++) sp.scr[i][j] = getc(fn) ;
      FCLOSE(fn) ;
    }
}
