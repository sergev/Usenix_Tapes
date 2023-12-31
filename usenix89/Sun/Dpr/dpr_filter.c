
/*
 *  dpr_filter.c
 *
 *  Routines to generate C code for screendump filter using the current parameters.
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
#include <strings.h>
#include <sys/file.h>

extern struct dumpcap dc[] ;      /* Dumpcap parameters. */

char filtername[MAXSTR] ;         /* Name of C filter file. */
FILE *fn,*fopen() ;
int nbits ;                       /* Number of unique bits in each image byte. */
int notimes ;                     /* Number of times each bit has to be output. */
int sbit ;                        /* Determine start bit in byte. */
int ebit ;                        /* Determine end bit in byte. */
int inc ;                         /* Increment for bit/byte direction. */
int complete ;                    /* Number of complete vertical passes. */
int remainder ;                   /* Number of remainder lines to output. */
int nolines ;                     /* Number of lines to do in one pass. */


make_filter(printer)              /* Generate screendump C filter code. */
char printer[MAXSTR] ;

{
  open_output(printer) ;          /* Open file to contain the C code. */
  make_headings() ;               /* Write program headings to C file. */
  make_variables() ;              /* Declare variables that are needed. */
  make_read_image() ;             /* C code for reaing rasterfile image from stdin. */
  make_output() ;                 /* Generate procedure for dumping screen image. */
  make_main() ;                   /* Produce main procedure for C filter. */
  FCLOSE(fn) ;                    /* Close C filter file. */
}


open_output(printer)              /* Open file to contain the C code. */
char printer[MAXSTR] ;

{
  SPRINTF(filtername,"%s_filter.c",printer) ;     /* Name of C filter file. */
  if ((fn = fopen(filtername,"w")) == NULL)
    {
      FPRINTF(stderr,"dpr: can't open C filter file %s\n",filtername) ;
      exit(1) ;
    } 
  SPRINTF(filtername,"%s_filter",printer) ;       /* For use in fprintf statement. */
}


make_headings()                   /* Write program headings to C file. */

{
  FPRINTF(fn,"/*\n") ;
  FPRINTF(fn," *  %s.c\n",filtername) ;
  FPRINTF(fn," *\n") ;
  FPRINTF(fn," *  C screendump filter file\n") ;
  FPRINTF(fn," *  Takes a rasterfile image on stdin and produces filtered image on stdout.\n") ;
  FPRINTF(fn," *\n") ;
  FPRINTF(fn," *  Generated by dpr, the general purpose screendump utility.\n") ;
  FPRINTF(fn," */\n\n") ;
}


make_variables()                  /* Declare variables that are needed. */

{
  FPRINTF(fn,"#include <stdio.h>\n") ;
  FPRINTF(fn,"#include <rasterfile.h>\n\n") ;
  FPRINTF(fn,"#define  MAXHEIGHT  900\n") ;
  FPRINTF(fn,"#define  MAXWIDTH   1152\n") ;
  FPRINTF(fn,"#define  MAXDEPTH   8\n") ;
  FPRINTF(fn,"#define  MAXVSCAN   32\n\n") ;
  FPRINTF(fn,"int scr[MAXHEIGHT+MAXVSCAN][MAXWIDTH*MAXDEPTH / 8] ;\n") ;
  FPRINTF(fn,"struct rasterfile header ;\n") ;
  FPRINTF(fn,"FILE *fn,*fopen() ;\n\n\n") ;
}


make_read_image()                 /* C code for reading rasterfile image from stdin. */

{
  FPRINTF(fn,"read_image()\n\n") ;
  FPRINTF(fn,"{\n") ;
  FPRINTF(fn,"  int i,j,llength ;\n\n") ;
  FPRINTF(fn,"  fread((char *)&header,sizeof(struct rasterfile),1,stdin) ;\n") ;
  FPRINTF(fn,"  if (header.ras_magic != RAS_MAGIC)\n") ;
  FPRINTF(fn,"    {\n") ;
  FPRINTF(fn,"      fprintf(stderr,\"\%s: stdin is not rasterfile format.\\n\") ;\n",filtername) ;
  FPRINTF(fn,"      exit(1) ;\n") ;
  FPRINTF(fn,"    }\n") ;
  FPRINTF(fn,"  llength = ((header.ras_width+15) >> 4) * 16 / 8 * header.ras_depth ;\n") ;
  FPRINTF(fn,"  if (header.ras_maptype == RMT_EQUAL_RGB)\n") ;
  FPRINTF(fn,"    for (i = 0; i < header.ras_maplength; i++) getchar() ;\n") ;
  FPRINTF(fn,"  for (i = 0; i < header.ras_height; i++)\n") ; 
  FPRINTF(fn,"    for (j = 0; j < llength; j++) scr[i][j] = getchar() ;\n") ;
  FPRINTF(fn,"}\n\n\n") ; 
}


make_string(buffer,nblanks,comment)   /* Generate C code for outputing a string. */
char buffer[MAXSTR],comment[MAXSTR] ;
int nblanks ;

{
  int i,j ;

  if (strlen(buffer)) FPRINTF(fn,"/* %s */\n",comment) ;
  for (i = 0; i < strlen(buffer); i++)
    {
      for (j = 0; j < nblanks; j++) FPRINTF(fn," ") ;
      FPRINTF(fn,"putchar('\\0%o') ;\n",buffer[i]) ;
    }
}


make_output()                   /* Generate procedure for dumping screen image. */

{
  nbits   = (DD) ? NB / 2 : NB ;      /* Number of unique bits in each image byte. */
  notimes = (DD) ? 2 : 1 ;            /* Number of times each bit has to be output. */
  sbit = (RO) ? nbits-1 : 0 ;         /* Determine start bit in byte. */
  ebit = (RO) ? -1 : nbits ;          /* Determine end bit in byte. */
  inc = (RO) ? -1 : 1 ;               /* Increment for bit/byte direction. */

  FPRINTF(fn,"make_dump()\n\n") ;
  FPRINTF(fn,"{\n") ;

  FPRINTF(fn,"  int byte,i,j,k,l,x,y ;\n") ;
  FPRINTF(fn,"  int nbits = %1d ;\n",nbits) ;
  FPRINTF(fn,"  int notimes = %1d ;\n",notimes) ;
  FPRINTF(fn,"  int sbit = %1d ;\n",sbit) ;
  FPRINTF(fn,"  int ebit = %1d ;\n",ebit) ;
  FPRINTF(fn,"  int inc = %1d ;\n\n",inc) ;

  make_string(SD,2,"Start of dump character sequence.") ;
  if (VS > 1) make_ver_dump() ;
  else make_horiz_dump() ;
  make_string(ED,2,"End of dump character sequence.") ;
  FPRINTF(fn,"}\n\n\n") ;
}


make_ver_dump()

{
  complete  = (DD) ? SH / (VS / 2) : SH / VS ;   /* Number of complete vertical passes. */
  remainder = (DD) ? SH % (VS / 2) : SH % VS ;   /* Number of remainder lines to output. */
  nolines   = (DD) ? VS / 2 : VS ;               /* Number of lines to do in one pass. */

  FPRINTF(fn,"  for (y = 0; y < %1d; y++)\n",complete) ;
  FPRINTF(fn,"    {\n") ;
  make_string(SL,6,"Start of line character sequence.") ;
  FPRINTF(fn,"      for (x = 0; x < header.ras_width; x++)\n") ;
  FPRINTF(fn,"        for (i = 0; i < notimes; i++)\n") ;
  FPRINTF(fn,"          for (j = 0; j < %1d; j++)\n",VS / NB) ;
  FPRINTF(fn,"            {\n") ;
  FPRINTF(fn,"              byte = %1d ;\n",MA) ;
  FPRINTF(fn,"              for (k = sbit; k != ebit; k += inc)\n") ;
  FPRINTF(fn,"                for (l = 0; l < notimes; l++)\n") ;
  FPRINTF(fn,"                  byte = (byte << 1) +\n") ;
  FPRINTF(fn,"                         ((scr[(y*nolines)+j*nbits+k][x/8] >> (7 - x%%8)) & 1) ;\n") ;
  FPRINTF(fn,"              putchar(byte) ;\n") ;
  FPRINTF(fn,"            }\n") ;
  make_string(EL,6,"End of line character sequence.") ;
  FPRINTF(fn,"    }\n\n") ;

  FPRINTF(fn,"/* Do remainder potion.\n\n") ;
  FPRINTF(fn,"  for (y = %1d; y < %1d; y++)\n",remainder,nolines) ;
  FPRINTF(fn,"    for (x = 0; x < MAXWIDTH*MAXDEPTH / 32; x++)\n") ;
  FPRINTF(fn,"      scr[%1d+y][x] = 0 ;\n\n",complete*nolines) ;
  make_string(SL,2,"Start of line character sequence.") ;
  FPRINTF(fn,"  for (x = 0; x < header.ras_width; x++)\n") ;
  FPRINTF(fn,"    for (i = 0; i < %1d; i++)\n",notimes) ;
  FPRINTF(fn,"      for (j = 0; j < %1d; j++)\n",VS / NB) ;
  FPRINTF(fn,"        {\n") ;
  FPRINTF(fn,"          byte = %1d ;\n",MA) ;
  FPRINTF(fn,"          for (k = %1d; k != %1d; k += %1d)\n",sbit,ebit,inc) ;
  FPRINTF(fn,"            for (l = 0; l < %1d; l++)\n",notimes) ;
  FPRINTF(fn,"              byte = (byte << 1) +\n") ;
  FPRINTF(fn,"                     ((scr[(y*%1d)+j*%1d+k][x/8] >> (7 - x%%8)) & 1) ;\n",nolines,nbits) ;
  FPRINTF(fn,"          putchar(byte) ;\n") ;
  FPRINTF(fn,"        }\n") ;
  make_string(EL,2,"End of line character sequence.") ;
  FPRINTF(fn,"\n") ;
}


make_horiz_dump()

{
  FPRINTF(fn,"  for (y = 0; y < header.ras_height; y++)\n") ;
  FPRINTF(fn,"    {\n") ;
  FPRINTF(fn,"      for (i = 0; i < notimes; i++)\n") ;
  FPRINTF(fn,"        {\n\n") ;
  make_string(SL,10,"Start of line character sequence.") ;
  FPRINTF(fn,"          for (x = 0; x < header.ras_width)\n") ;
  FPRINTF(fn,"            {\n") ;
  FPRINTF(fn,"              byte = %1d\n",MA) ;
  FPRINTF(fn,"              for (k = sbit; k != ebit; k += inc)\n") ;
  FPRINTF(fn,"                for (l = 0; l < notimes; l++)\n") ;
  FPRINTF(fn,"                  byte = (byte << 1) + ((scr[y][x/8] >> (7 - x%%8)) & 1) ;\n") ;
  FPRINTF(fn,"                putchar(byte) ;\n") ;
  FPRINTF(fn,"            }\n") ;
  FPRINTF(fn,"        }\n") ;
  make_string(EL,6,"End of line character sequence.") ;
  FPRINTF(fn,"    }\n") ;
}


make_main()                     /* Produce main procedure for C filter. */

{
  FPRINTF(fn,"main()\n\n") ;
  FPRINTF(fn,"{\n") ;
  FPRINTF(fn,"  read_image() ;\n") ;
  FPRINTF(fn,"  make_dump() ;\n") ;
  FPRINTF(fn,"}\n") ;
}
