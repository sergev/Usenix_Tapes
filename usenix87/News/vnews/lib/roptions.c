#include <stdio.h>
#include "config.h"
#include "defs.h"
#include "roptions.h"


char *savestr(), *getenv(), *index(), *hfgets();


struct optable options[] = { /*
optlet	filchar	flag	newstate oldmode	newmode	buf	*/
'p',	'\0',	FALSE,	OPTION,	UNKNOWN,	UNKNOWN,NULL,	
't',	'\1',	FALSE,	STRING,	ANY,		UNKNOWN,NULL,
'a',	' ',	FALSE,	STRING,	ANY,		UNKNOWN,NULL,
'n',   NGDELIM,	FALSE,	STRING,	ANY,		UNKNOWN,NULL,
'c',	' ',	FALSE,	STRING,	UNKNOWN,	UNKNOWN,NULL,
'l',	' ',	FALSE,	OPTION,	UNKNOWN,	UNKNOWN,NULL,
'r',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
/* the s option used to read a list of newsgroups.  Not any more */
's',   NGDELIM,	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'x',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'h',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'M',	'\0',	FALSE,	OPTION,	UNKNOWN,	MAIL,	NULL,
'f',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'u',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'e',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'1',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'2',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'A',	'\0',	FALSE,	OPTION,	ANY,		UNKNOWN,NULL,
'\0',	'\0',	0,	0,	0,		0,	NULL
};

char *progname ;
char *titles[10] ;		/* list of titles */
extern char bfr[] ;


roptions(argv, rcfp)
      char **argv ;
      FILE *rcfp ;
      {
      char *p ;
      char **pp ;

      progname = *argv++ ;
      if ((p = getenv("NEWSOPTS")) != NULL) {
            scopyn(p, bfr, LBUFLEN - 2) ;
            procostr(bfr) ;
      }
      if (rcfp != NULL && hfgets(bfr, LBUFLEN - 2, rcfp) != NULL && prefix(bfr, "options ")) {
            procostr(bfr + 8) ;
      }
      if (sublist == NULL)
            sublist = savestr(DFLTSUB);
#ifdef ADMSUB
      sprintf(bfr, "%s,%s", sublist, ADMSUB);
      free(sublist);
      sublist = savestr(bfr);
#endif

      procopt(argv) ;

      if (cflag) {
            oneflag = FALSE;
            twoflag = TRUE;
      }
      if (pflag) {
            oneflag = FALSE;
            twoflag = FALSE;
      }
      if (!oneflag && !twoflag && !pflag) {
            oneflag = TRUE;
            twoflag = TRUE;
      }
      if (oneflag && !twoflag)
            cflag = 1;
      strcpy(bfr, sublist);
      makehimask(bfr, "junk");
      makehimask(bfr, "control");
      /* makehimask(bfr, "test"); */
      free(sublist);
      sublist = savestr(bfr);
      if (toptbuf) {
            pp = titles ;
            p = toptbuf ;
            for (;;) {
                  if (pp >= titles + 9)
                        xerror("Too many titles") ;
                  *pp++ = p ;
                  if ((p = index(p, '\1')) == NULL)
                        break ;
                  *p++ = '\0' ;
            }
      }
}
