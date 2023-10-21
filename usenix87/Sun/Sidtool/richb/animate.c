
/*  animate.c
 *
 *  Make the Sid Tool animate file from the individual icons.
 *  Written by Rich Burridge - SUN Australia - June 1986.
 *
 *  Version 1.5
 *
 *  No responsibility is taken for any errors or inaccuracies inherent
 *  either to the comments or the code of this program, but if reported
 *  to me, then an attempt will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include <errno.h>

#define  CLOSE    (void) close      /* To make lint happy. */
#define  FCLOSE   (void) fclose
#define  FPRINTF  (void) fprintf
#define  SSCANF   (void) sscanf
#define  STRCPY   (void) strcpy
#define  STRNCPY  (void) strncpy
#define  WRITE    (void) write

#define  BUFSIZE  512               /* Output buffer size in characters. */
#define  MAXLINE  256               /* Maximum length of character string. */

extern int errno ;
int fd ;                            /* File descriptor for animate data. */


getline(fd,s)      /* Checks each character, and ignore's it if its NULL. */
FILE *fd ;
char s[MAXLINE] ;

{
  int c,i ;

  i = 0 ;
  while (i < MAXLINE-1 && (c = getc(fd)) != EOF && c != '\n')
  if (c != '\0') s[i++] = c ;

  if (c == EOF) return ;
  if (c == '\n') s[i++] = c ;
  s[i] = '\0' ;
}


load_cursor(name)
char name[MAXLINE] ;

{
  unsigned short buf[BUFSIZE/2] ;
  int i,j,temp ;
  char arg[MAXLINE],line[MAXLINE] ;
  FILE *fin,*fopen() ;

  if ((fin = fopen(name,"r")) == NULL)
    {
      FPRINTF(stderr,"animate: can't open %s.\n",name) ;
      exit(1) ;
    }
  getline(fin,line) ;
  getline(fin,line) ;
  for (i = 0; i < 32; i++)
    {
      getline(fin,line) ;
      for (j = 0; j < 8; j++)
        {
          STRNCPY(&arg[0],&line[j*7+3],4) ;
          arg[4] = '\0' ;
          SSCANF(arg,"%X",&temp) ;
          buf[i*8+j] = (short) temp ;
        }
    }
  WRITE(fd,(char *) buf,BUFSIZE) ;
  FCLOSE(fin) ;
}


main()

{
  char filename[MAXLINE] ;

  STRCPY(filename,"sidtool.animate") ;
  if ((fd = open(filename,2)) == -1)
    if ((fd = creat(filename,0777)) == -1)
      {
        FPRINTF(stderr,"\nanimate: unable to open %s.  Error %d\n",
                filename,errno) ;
        exit(1) ;
      }

  load_cursor("cornerUR.pic") ;
  load_cursor("cornerRD.pic") ;
  load_cursor("cornerDL.pic") ;
  load_cursor("cornerLU.pic") ;

  load_cursor("picbigdot.pic") ;
  load_cursor("picsmalldot.pic") ;

  load_cursor("cirRIGHT0.pic") ;
  load_cursor("cirRIGHT1.pic") ;
  load_cursor("cirRIGHT2.pic") ;
  load_cursor("cirRIGHT3.pic") ;

  load_cursor("cirUP0.pic") ;
  load_cursor("cirUP1.pic") ;
  load_cursor("cirUP2.pic") ;
  load_cursor("cirUP3.pic") ;

  load_cursor("cirLEFT0.pic") ;
  load_cursor("cirLEFT1.pic") ;
  load_cursor("cirLEFT2.pic") ;
  load_cursor("cirLEFT3.pic") ;

  load_cursor("cirDOWN0.pic") ;
  load_cursor("cirDOWN1.pic") ;
  load_cursor("cirDOWN2.pic") ;
  load_cursor("cirDOWN3.pic") ;

  load_cursor("POKEY0.pic") ;
  load_cursor("POKEY1.pic") ;

  load_cursor("BASHFUL0.pic") ;
  load_cursor("BASHFUL1.pic") ;

  load_cursor("SPEEDY0.pic") ;
  load_cursor("SPEEDY1.pic") ;

  load_cursor("SHADOW0.pic") ;
  load_cursor("SHADOW1.pic") ;

  load_cursor("blueghost0.pic") ;
  load_cursor("blueghost1.pic") ;

  load_cursor("bluepics0.pic") ;
  load_cursor("bluepics1.pic") ;

  load_cursor("eyesRIGHT.pic") ;
  load_cursor("eyesUP.pic") ;
  load_cursor("eyesLEFT.pic") ;
  load_cursor("eyesDOWN.pic") ;

  load_cursor("circleexplode0.pic") ;
  load_cursor("circleexplode1.pic") ;
  load_cursor("circleexplode2.pic") ;
  load_cursor("circleexplode3.pic") ;
  load_cursor("circleexplode4.pic") ;
  load_cursor("circleexplode5.pic") ;
  load_cursor("circleexplode6.pic") ;
  load_cursor("circleexplode7.pic") ;
  load_cursor("circleexplode8.pic") ;

  load_cursor("fruit1.pic") ;
  load_cursor("fruit2.pic") ;
  load_cursor("fruit3.pic") ;
  load_cursor("fruit4.pic") ;
  load_cursor("fruit5.pic") ;
  load_cursor("fruit6.pic") ;
  load_cursor("fruit7.pic") ;
  load_cursor("fruit8.pic") ;
  CLOSE(fd) ;
}
