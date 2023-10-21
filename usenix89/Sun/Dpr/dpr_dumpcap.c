/*LINTLIBRARY*/

/*
 *  dpr_dumpcap.c
 *
 *  Routines for reading printer capabilities from specified dumpcap file.
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
#include <ctype.h>
#include <sys/file.h>
#include <strings.h>

#define  GET_FLAG    (void) get_flag        /* To make lint happy. */
#define  GET_NUMBER  (void) get_number
#define  GET_STRING  (void) get_string

extern struct dumpcap dc[] ;
static char dbuf[MAXSTR] ;     /* Current entry from dumpcap file. */
static int cp ;                /* Current dumpcap entry character pointer. */


next_field()                   /* Find start of next field in dumpcap record. */

{
  while (dbuf[cp] != '\0' && dbuf[cp] != ':') cp++ ;
  if (dbuf[cp] == '\0') return(0) ;
  if (dbuf[cp] == ':') cp++ ;
  return(1) ;
}


get_next_entry(fn)             /* Get next printer definition from dumpcap file. */
int fn ;

{
  char c ;
  int i,skip ;

  i = 0 ;
  skip = 0 ;
  for (;;)
    {
      if (read(fn,&c,1) == 0) return(0) ;
      switch (c)
        {
          case '\n' : if (!i)
                        {
                          skip = 0 ;
                          continue ;
                        }
                      if (dbuf[i-1] == '\\')
                        {
                          i-- ;
                          continue ;
                        }
                      dbuf[i] = '\0';
                      return(1) ;
          case '#'  : if (!i) skip = 1 ;
          default   : if (skip) continue ;
                      if (i >= MAXSTR)
                        {
                          FPRINTF(stderr,"dpr: dumpcap entry too long\n") ;
                          dbuf[i] = '\0' ;
                          return(1) ;
                        }
                      dbuf[i++] = c ;
          }              
    }
}


match_printer(name)      /* Compare dumpcap printer entry with current printer. */
char name[MAXSTR] ;

{
  int j ;

  cp = 0 ;
  if (dbuf[cp] == '#') return(0) ;
  for (;;)
    {
      for (j = 0; dbuf[cp] == name[j] && name[j] != '\0'; cp++, j++) continue ;
      if (name[j] == '\0' && (dbuf[cp] == '|' || dbuf[cp] == ':' || dbuf[cp] == 0))
        return (1) ;
      while (dbuf[cp] != '\0' && dbuf[cp] != ':' && dbuf[cp] != '|') cp++ ;
      if (dbuf[cp] == '\0' || dbuf[cp] == ':') return(0) ;
      cp++ ;
    }
}


get_number(n)            /* Look for numeric field in dumpcap record. */
int n ;

{
  int i,base ;

  cp = 0 ;
  for (;;)
    {
      if (!next_field()) return(NOTFOUND) ;
      if (dbuf[cp++] != dc[n].name[0] || dbuf[cp] == '\0' ||
          dbuf[cp++] != dc[n].name[1]) continue ;
      if (dbuf[cp] == '@') return(NOTFOUND) ;
      if (dbuf[cp] != '#') continue ;
      cp++ ;
      base = 10 ;
      if (dbuf[cp] == '0') base = 8 ;
      i = 0 ;
      while (isdigit(dbuf[cp]))
        {
          i *= base ;
          i += dbuf[cp++] - '0' ;
        }
      dc[n].num = i ;
      return(FOUND) ;
    }
}


get_flag(n)            /* Look for boolean field in dumpcap record. */
int n ;

{
  cp = 0 ;
  for (;;)
    {
      if (!next_field()) return(NOTFOUND) ;
      if (dbuf[cp++] == dc[n].name[0] && dbuf[cp] != '\0' &&
          dbuf[cp++] == dc[n].name[1])
        {
          if (dbuf[cp] == '\0' || dbuf[cp] == ':')
            {
              dc[n].num = 1 ;
              return(FOUND) ;
            }
          else if (dbuf[cp] == '@') return(NOTFOUND) ;
        }
    }    
}


decode(n)         /* Decode escape sequences in string field in dumpcap record. */
int n ;

{
  char dp[MAXSTR],val[MAXSTR] ;
  int c,i,j,sp ;

  sp = 0 ;
  while ((c = dbuf[cp++]) && c != ':')
    {
      switch (c)
        {
          case '^'  : c = dbuf[cp++] & 037 ;
                      break ;
          case '\\' : j = 0 ;
                      STRCPY(dp,"E\033^^\\\\::n\nr\rt\tb\bf\f") ;
                      c = dbuf[cp++] ;
nextc:
                      if (dp[j++] == c)
                        {
                          c = dp[j++] ;
                          break ;
                        }
                      j++ ;
                      if (dp[j] != '\0') goto nextc ;
                      if (isdigit(c))
                        {
                          c -= '0' ;
                          i = 2 ;
                          do
                            {
                              c <<= 3 ;
                              c |= dbuf[cp++] - '0' ;
                            }
                          while (--i && isdigit(dbuf[cp])) ;
                        }    
                      break ;
        }                
      val[sp++] = c ;
    }    
  val[sp++] = '\0' ;
  STRCPY(dc[n].str,val) ;
}
 
 
get_string(n)         /* Look for string field in dumpcap record. */
int n ;

{
  cp = 0 ;
  for (;;)
    {
      if (!next_field()) return(NOTFOUND) ;
      if (dbuf[cp++] != dc[n].name[0] || dbuf[cp] == '\0' ||
          dbuf[cp++] != dc[n].name[1]) continue ;
      if (dbuf[cp] == '@') return(NOTFOUND) ;
      if (dbuf[cp] != '=') continue ;
      cp++ ;
      decode(n) ;
      return(FOUND) ;
    }
}


read_params(dcname,printer)    /* Read printer capabilities from dumpcap file. */
char dcname[MAXSTR],printer[MAXSTR] ;

{
  int fn,n ;

  if ((fn = open(dcname,O_RDONLY)) == NULL)
    {
      FPRINTF(stderr,"dpr: can't open %s\n",dcname) ;
      exit(1) ;
    }
  while (get_next_entry(fn))
    if (match_printer(printer))
      {
        for (n = 0; n < MAXCAP; n++)
          switch(dc[n].type)
            {
              case BOOLEAN : GET_FLAG(n) ;
              case NUMBER  : GET_NUMBER(n) ;
              case STRING  : GET_STRING(n) ;
            }
        CLOSE(fn) ;
        return ;
      }
  CLOSE(fn) ;
}
