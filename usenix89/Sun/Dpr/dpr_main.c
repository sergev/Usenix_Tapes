 
/*
 *  dpr_main.c
 *
 *  General purpose screen dumper for the Sun workstations.
 *  This program will output a screen dump to the specified printer by
 *  using the printer capabilities extracted from /etc/dumpcap.
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
#include <errno.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sgtty.h>
#include <strings.h>

extern int errno ;                 /* Standard error reply. */

/* Global variables. */
int fd ;                          /* RS232 output file descriptor. */
int nbits ;                       /* Number of unique bits this byte. */
int sbit ;                        /* Start bit within graphics byte. */
int ebit ;                        /* End bit within graphics byte. */
int inc ;                         /* Increment for bit/byte direction. */
int notimes ;                     /* Number of times to output that byte. */
int created ;                     /* 1 = output 'device' was created. */
int printing ;                    /* 1 = OK to send data to the printer. */
char person[MAXSTR] ;             /* User's login id. */
 
/* Dpr switch items. */
int copies   = 1 ;                /* Number of screen dump copies to do. */
int remove   = 0 ;                /* 1 = remove screen dump after printing. */
int mail     = 0 ;                /* 1 = mail user upon completion. */
int suppress = 0 ;                /* 1 = suppress burst page. */
int filter   = 0 ;                /* 1 = generate screendump C filter code. */
char printer[MAXSTR] ;            /* Name of printer to use. */
char class[MAXSTR] ;              /* Class name for burst page. */
char job[MAXSTR] ;                /* Job name for burst page. */
char dcname[MAXSTR] ;             /* Dumpcap filename to use. */

struct sgttyb old,new ;           /* Old and new terminal characteristics. */


main(argc,argv)
int argc ;
char *argv[] ;

{
  int i ;

  what_video(&sp) ;                 /* What type of video are we running on? */
  default_params() ;                /* Set up default parameters. */
  get_options(argc,argv) ;          /* Validate command line options. */
  read_params(dcname,printer) ;     /* Get dumpcap parameters. */
  if (!filter)                      /* Does user want screendump or filter made? */
    {
      open_device(LP) ;             /* Open output device name. */
      init_handshake(HA) ;          /* Initialise Sun/printer handshake. */
      get_screen_image(FN) ;        /* From screen or saved file. */
      check_params() ;              /* Determine validity of dumpcap parameters. */
      for (i = 0; i < copies; i++)
        {
          if (!suppress) print_burst_page() ;             /* Burst page if wanted. */
          dump_screen((VS > 1) ? VERTICAL : HORIZONTAL) ;   /* Output screen dump. */
        }
    }
  else make_filter(printer) ;       /* Generate screendump C filter. */
  terminate() ;                     /* Terminate operations. */
}


default_params()                    /* Set up the default dpr switches. */

{
  char *bp,*getenv(),*getlogin(),*env ;
  struct passwd *getpwuid() ;

  if ((bp = getlogin()) == NULL &&            /* Get user's login id. */
      (bp = getpwuid(getuid())->pw_name) == NULL) bp = "Who ???" ;
  STRCPY(person,bp) ;

  STRCPY(printer,"lp") ;                     /* Default printer name. */
  if ((env = getenv("PRINTER")) != NULL) STRCPY(printer,env) ;

  STRCPY(class,"") ;                /* Null string for class name. */
  STRCPY(job,"") ;                  /* Null string for job name. */
  STRCPY(dcname,"/etc/dumpcap") ;   /* Default dumpcap file to use. */
}
 
 
get_options(argc,argv)              /* Get user options from command line. */
int argc ;
char *argv[] ;

{
  char *arg ;
  char *p ;         /* Pointer to string following argument flag. */

  while (argc > 1 && (arg = argv[1])[0] == '-')
    {
      p = arg + 2 ;
      switch (arg[1])
        {
          case 'P' : STRCPY(printer,p) ;           /* Output to a named printer. */
                     break ;
          case '#' : if ((copies = atoi(p)) < 1)   /* Produce multiple copies. */
                       {
                         FPRINTF(stderr,"dpr: copies must be > 0\n") ;
                         exit(1) ;
                       }
                     break ;
          case 'C' : STRCPY(class,p) ;             /* Class name for burst page. */
                     break ;
          case 'J' : STRCPY(job,p) ;               /* Job name for burst page. */
                     break ;
          case 'f' : filter = 1 ;                  /* Generate screendump C filter. */
                     break ;
          case 'r' : remove++ ;                    /* Remove screen dump file. */
                     break ;
          case 'm' : mail++ ;                      /* Send mail upon completion. */
                     break ;
          case 'h' : suppress++ ;             /* Suppress the burst page. */
                     break ;
          case 'D' : STRCPY(dcname,p) ;
                     break ;
          default  : FPRINTF(stderr,"dpr: USAGE %s [-Pprinter] [-#num] ",argv[0]) ;
                     FPRINTF(stderr,"[-Cclass] [-Jjob] [-r] [-m] [-h] [-Ddumpcap]\n") ;
                     exit(1) ;
        }
      argc-- ;
      argv++ ;
    }
}


check_params()

/*  Check validity of dumpcap parameters. Certain parameters will have changed
 *  if the screen image was read in from a file. The dumpcap capabilities are
 *  updated to reflect this.
 *
 *  Correct job and class names for the burst page are set up here.
 */

{
  int len = 0 ;

  if (NB < 0 || NB > 8) NB = 8 ;                    /* Check bits per byte. */
  if (SH < 0 || SH > sp.height) SH = sp.height ;    /* Check screendump height. */
  if (SW < 0 || SW > sp.width) SW = sp.width ;      /* Check screendump width. */
  if (SX < 0 || SX > sp.width) SX = 0 ;             /* Check screendump X origin. */
  if (SY < 0 || SY > sp.height) SY = 0 ;            /* Check screendump Y origin. */

  if (!strlen(job))                      /* Job name for burst page. */
    if (strlen(FN)) STRCPY(job,FN) ;
    else STRCPY(job,"Current") ;

  if (!strlen(class))                    /* Class name for burst page. */
    if (gethostname(class,len) == -1) STRCPY(class,"Hostname") ;
}


open_device(device)             /* Open output device for screen dump. */
char device[MAXSTR] ;

{
  long lseek() ;

  created = 0 ;
  if ((fd = open(device,O_RDWR)) == -1)
    {
      if ((fd = creat(device,0777)) == -1)
        {
          FPRINTF(stderr,"dpr: error %d on creating %s.\n",errno,device) ;
          exit(1) ;
        }
      else created++ ;
    }
  if (created) LSEEK(fd,0L,0) ;
}


init_speed(speed)          /* Initialise baud rate for output device. */
int speed ;

{
  int spbits ;

  switch (speed)           /* Convert to internal system code */
    {
      case 110  : spbits = B110 ;
                  break ;
      case 150  : spbits = B150 ;
                  break ;
      case 300  : spbits = B300 ;
                  break ;
      case 1200 : spbits = B1200 ;
                  break ;
      case 1800 : spbits = B1800 ;
                  break ;
      case 2400 : spbits = B2400 ;
                  break ;
      case 4800 : spbits = B4800 ;
                  break ;
      case 9600 : spbits = B9600 ;
                  break ;
      default   : FPRINTF(stderr,"dpr: Bad line speed.\n") ;
                  exit(1) ;
    }
  new.sg_ispeed = spbits ;
  new.sg_ospeed = spbits ;
}


init_handshake(type)          /* Initialise handshake for output device. */
int type ;

{

/* If the output 'device' was created then the handshake is forced to NONE. */

  if (created) HA = NONE ;
  else
    {
      GTTY(fd,&old) ;                     /* Save current mode. */
      GTTY(fd,&new) ;
      new.sg_flags |= RAW ;               /* Add raw mode 8 bit interface. */

      switch (type)
        {
          case NONE     : break ;
          case XON_XOFF : printing = 1 ;
                          break ;
          case DTR      :
          case RTS      :
          case CTS      : IOCTL(fd,(int) TIOCCDTR,(char *)0) ;   /* clear modem state. */
        }
      init_speed(BR) ;                    /* Set line speed. */
      STTY(fd,&new) ;                     /* Set new tty mode. */
    }
}


ok_to_send(type)      /* Check it's OK to output the next character. */
int type ;

{
  char reply ;
  int errind,mask,val ;

  mask = type ;
  switch (type)
    {
      default       :
      case NONE     : return(1) ;

      case XON_XOFF : IOCTL(fd,(int) FIONREAD,(char *) &val) ;
                      if (val)
                        {
                          READ(fd,&reply,1) ;
                          if (reply == 19) printing = 0 ;        /* Stop sending data. */
                          if (reply == 17) printing = 1 ;        /* Start sending data again. */
                        }
                      return(printing) ;

      case CTS      : mask = TIOCM_CTS ;
      case DTR      :
      case RTS      : errind = ioctl(fd,(int) TIOCMGET,(char *) &val) ;
                      if (errind == -1)
                        {
                          FPRINTF(stderr,"dpr: modem error %d\n",errno) ;
                          exit(1) ;
                        }
                      if ((val & mask) != 0) return(1) ;
                      else return(0) ;
    }
}
 
 
sendchar(c)         /* Send the next character to the output device / file. */
char c ;

{
  while (!ok_to_send(HA)) ;
  WRITE(fd,&c,1) ;
}


output(sequence)                    /* Output a string sequence. */
char sequence[MAXSTR] ;

{
  int i ;

  for (i = 0; i < strlen(sequence); i++) sendchar(sequence[i]) ;
}


h_BW_scan(val)          /* Output next Black and White horizontal line to the printer. */
int val[MAXWIDTH*MAXDEPTH / B_CH] ;
 
{
  int byte,i,k,l,x ;
 
  for (i = 0; i < notimes; i++)
    {
      output(SL) ;
      for (x = SX; x < SX+SW; x++)
        {
           byte = MA ;
           for (k = sbit; k != ebit; k += inc)
             for (l = 0; l < notimes; l++)
               byte = (byte << 1) + ((val[x/B_CH] >> ((B_CH-1) - x%B_CH)) & 1) ;
           sendchar(byte) ;
        } 
    }
  output(EL) ;
}
 
 
h_COL_scan(val)          /* Output next Color horizontal line to the printer. */
int val[MAXWIDTH*MAXDEPTH / B_CH] ;

{
  int byte,c,i,k,l,x ;
 
  for (i = 0; i < notimes; i++)
    {
      output(SL) ;
      for (c = 0; c < 3; c++)
        for (x = SX; x < (SX+SW) / B_CH; x++)
          {
             byte = MA ;
             for (k = sbit; k != ebit; k += inc)
               for (l = 0; l < notimes; l++)
                 byte = (byte << 1) + color_map[c][val[x*8+k]] ;
             sendchar(byte) ;
          }  
    }    
  output(EL) ;
}


v_BW_scan(val)          /* Output next Black and White vertical "line" to the printer. */
int val[MAXLINES][MAXWIDTH*MAXDEPTH / B_CH] ;

{
  int byte,i,j,k,l,x ;

  output(SL) ;
  for (x = SX; x < SX+SW; x++)
    for (i = 0; i < notimes; i++)
      for (j = 0; j < VS / NB; j++)
        {
          byte = MA ;
          for (k = sbit; k != ebit; k += inc)
            for (l = 0; l < notimes; l++)
              byte = (byte << 1) +
                     ((val[j*nbits+k][x/B_CH] >> ((B_CH-1) - x%B_CH)) & 1) ;
          sendchar(byte) ;
        } 
  output(EL) ;
}


v_COL_scan(val)        /* Output next Color vertical "line" to the printer. */
int val[MAXLINES][MAXWIDTH*MAXDEPTH / B_CH] ;

{
  int byte,i,j,k,l,x ;

  output(SL) ;
  for (x = SX; x < SX+SW; x++)
    for (i = 0; i < notimes; i++)
      for (j = 0; j < VS / NB; j++)
        {
          byte = MA ;
          for (k = sbit; k != ebit; k += inc)
            for (l = 0; l < notimes; l++)
              byte = (byte << 1) +
                     ((val[j*nbits+k][x/B_CH] >> ((B_CH-1) - x%B_CH)) & 1) ;
          sendchar(byte) ;
        }
  output(EL) ;
}


print_burst_page()      /* Print out the header page. */

{
  struct timeval tp ;
  struct timezone tzp ;
  char *ctime(),*sprintf(),dprtime[MAXSTR],text[MAXSTR],xstr[MAXSTR] ;
  int x ;

  output(SB) ;
  SPRINTF(xstr,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n") ;
  for (x = 0; x < 4; x++) output(xstr) ;     /* Four lines of X's. */

  GETTIMEOFDAY(&tp,&tzp) ;
  STRCPY(dprtime,ctime((time_t *)&tp.tv_sec)) ;
  SPRINTF(text,"\n\nUser: %s\n\nJob: %s\n\nClass: %s\n\nPrinter: %s\n\n%s\n\n",
                          person,    job,         class,         printer, dprtime) ;
  output(text) ;

  for (x = 0; x < 4; x++) output(xstr) ;     /* Four lines of X's. */
  sendchar('\015') ;                         /* Carriage return. */
  sendchar('\014') ;                         /* Form feed. */
}


dump_screen(direction)   /* Dump the required screen portion to the printer. */
int direction ;

{
  nbits   = (DD) ? NB / 2 : NB ;      /* Number of unique bits in each image byte. */
  notimes = (DD) ? 2 : 1 ;            /* Number of times each bit has to be output. */
  sbit = (RO) ? nbits-1 : 0 ;         /* Determine start bit in byte. */
  ebit = (RO) ? -1 : nbits ;          /* Determine end bit in byte. */
  inc = (RO) ? -1 : 1 ;               /* Increment for bit/byte direction. */
  output(SD) ;
  switch (direction)
    {
      case HORIZONTAL : do_horiz_dump() ;    /* Do horizontal screen dump to printer. */
                        break ;
      case VERTICAL   : do_ver_dump() ;      /* Do vertical screen dump to printer. */
    }
  output(ED) ;
}    
 
 
do_horiz_dump()       /* Do horizontal screen dump to printer. */

{
  int complete,nolines,y ;

  complete  = SH / VS ;           /* Number of complete horizontal passes. */
  nolines   = VS ;                /* Number of lines to be done in one pass. */
  switch (CO)
    {
      case 1  : for (y = 0; y < complete; y++)                  /* Black and White. */
                  h_BW_scan(&sp.scr[SY+y*nolines][0]) ;
                break ;
      default : for (y = 0; y < complete; y++)                  /* Color. */
                  h_COL_scan(&sp.scr[SY+y*nolines][0]) ;
    }
}


do_ver_dump()         /* Do vertical screen dump to printer. */

{
  int complete,nolines,remainder,x,y ;
  int lines[MAXLINES][MAXWIDTH*MAXDEPTH / B_INT] ;

  complete  = (DD) ? SH / (VS / 2) : SH / VS ;   /* Number of complete vertical passes. */
  remainder = (DD) ? SH % (VS / 2) : SH % VS ;   /* Number of remainder lines to output. */
  nolines   = (DD) ? VS / 2 : VS ;               /* Number of lines to do in one pass. */
  switch (CO)
    {
      case 1  : for (y = 0; y < complete; y++)
                  v_BW_scan(&sp.scr[SY+y*nolines][0]) ;
                break ;
      default : for (y = 0; y < complete; y++)
                  v_COL_scan(&sp.scr[SY+y*nolines][0]) ;
    }

  for (y = 0; y < remainder; y++)
    for (x = 0; x < MAXWIDTH*MAXDEPTH / B_INT; x++)
      lines[y][x] = sp.scr[SY+(complete*nolines)+y][x] ;
  for (y = remainder; y < nolines; y++)
    for (x = 0; x < MAXWIDTH*MAXDEPTH / B_INT; x++) lines[y][x] = 0 ;

  if (CO == 1) v_BW_scan(&lines[0][0]) ;
  else v_COL_scan(&lines[0][0]) ;
}


terminate()     /* Terminate operations tidely. */

{
  FILE *pn,*popen() ;
  char tt[20] ;

  if (!filter)
    {
      if (remove && strlen(FN)) UNLINK(FN) ;         /* Remove screen file? */
      if (!created) STTY(fd,&old) ;                  /* Reset old characteristics. */
      else CLOSE(fd) ;                               /* Close output file. */
    }
  if (mail)
    {
      STRCPY(tt,"mail ") ;
      STRCAT(tt,person) ;
      if ((pn = popen(tt,"w")) != NULL)
        {
          FPRINTF(pn, "From dpr: Screen dump complete to printer %s\n",printer) ;
          FCLOSE(pn) ;
        }
    }
}
