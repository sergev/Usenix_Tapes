/* SCCS id : @(#)wp.c	1.11 (Peter J Story)                                 */

/* ------------------------------------------------------------- */
/* Copyright: P J Story, Eikervn 67, 3600 KONGSBERG, Norway 1986 */
/*   This program may be copied for any purpose, as long as the  */
/*   above credit line is retained				 */
/* ------------------------------------------------------------- */

/* trivial text formatter, operating as a filter, to enable me   */
/* to format text into roughly equal length lines.  Most often   */
/* used from vi(1).  Getting less trivial and messier, but still */
/* useful                                                        */

/* will run on System V systems, and 4.x BSD if you have the     */
/* missing System V getopt, strtok, strspn routines or           */
/* equivalents which are usually packaged with my source,        */
/* courtesy of Henry Spencer                                     */

/* modified: Sun Sep 22 14:54:10 GMT+1:00 1985, by KVVAX6!pete	 */
/*   -s option strips comment symbols and doesnt put them back   */
/*   return 0 on exit, so it works from mail                     */
/* modified: Fri Sep 6 10:00:06 GMT+1:00 1985, by KVVAX6!esa     */
/*   -f option for framing (adopted into my version as is,       */
/*   though I don't like its behaviour wrt more than one         */
/*   paragraph)                                                  */
/* modified: Fri May 24 18:16:27 GMT+1:00 1985, by kvvax6!pete   */
/*  even more corrections to comment feature - comment indents   */
/*  are handled, and non padded block comments                   */
/* modified: Mon May 20 16:43:01 GMT+1:00 1985, by kvvax6!pete   */
/*  to correctly handle leading tabs (simply replacing by        */
/*  spaces) to add the -b option for block commenting            */
/* modified: Wed Apr 24 08:57:33 MET 1985, by olhqmc!pete        */
/*  Rewritten from awk to C.  Indenting facilities added, so     */
/*  that output paragraphs retain the first and second line      */
/*  indents of the input para for the first and subsequent lines */
/*  of the output para.                                          */
/* modified: Fri Jun 22 08:04:40 CET 1984, by pete @ Olivetti HQ */
/* To give me formatting of 'commented' lines.  Normal text will */
/* be made into comments with the given comment symbol.          */
/* Existing comment text will be reformatted.                    */


#include <stdio.h>

#define LINELENGTH 67
#define WHITE " 	"
#define DASH '-'

char buf[512];
char revcomment[20];	/* comment symbol reversed */
char *progname;
char *commentstart;
char *ipline;

int oplth;		/* current length output on this line */
int iplth;
int fplth;
int initblanks;		/* actual number of WHITE chars at start of ipline */
int oplinenum;		/* in the output paragraph */
int iplinenum;		/* in the input paragraph */
int spacesbefore;	/* the next word */
int commenting = 0;	/* on if lines are to be preceded by comment symbol */
int commentlth = 0;
int blocking = 0;	/* on if lines are to be succeeded by comment symbol
			   reversed */
int framing = 0;	/* will be blocking and framing */
int padding = 1;	/* on if block commented lines are padded out */
int stripping = 0;      /* on if comment symbols are to be stripped off */
			/*  and left off */
int comindent = 0;	/* comment indent on all lines in para */
int linelength = LINELENGTH;	/* output length to aim at */


/* BEWARE, this program grew like topsy, and most of these have  */
/* side effects.  Sorry about that!                              */

int	chkblank();	/* return true if line is blank */
void	dash();		/* output this many dashes */
void	dashline();	/* block out the line with dashes */
void	emptyline();	/* output empty line */
int	initcount();	/* how many effective initial spaces are there */
void	newline();	/* clear off this op line, init for next */
int	prefix();	/* return true if pre is prefix in s */
void	space();	/* output this many spaces */
void	usage();

char	*gets();
char	*strtok();
int	strcmp();
int	strlen();
int	strspn();


main(argc,argv)
   char **argv;
{
   extern char *optarg;
   extern int optind;

   int c;
   int errflg = 0;
   int indent;		/* text indent on the current line */
   int indent1;		/* text indent on 1st o/p line in para */
   int indent2;		/* text indent on subs o/p line in para */

   progname=argv[0];

   while ((c=getopt(argc, argv, "Bbc:fs")) != EOF) {
      switch (c) {
	 case 'B':	/* block commenting, do not pad to full lth */
	    padding = 0;
	    blocking++;
	    break;
	 case 'f':	/* framing, which implies a block comment */
	    framing++;
	    blocking++;
	    break;
	 case 'b':	/* block commenting */
	    blocking++;
	    break;
	 case 's':      /* stripping comments */
	    stripping++;
	    break;
	 case 'c':
	    commenting++;
	    commentstart = optarg;
	    commentlth = strlen(commentstart);
	    break;
	 case '?':
	    errflg++;
	    break;
      }
   }
   argc -= optind;
   argv = &argv[optind];

   if ((blocking || stripping) && ! commenting) errflg++;

   if (errflg) usage();

   if (argc > 0) {
      if ((linelength = atoi(*argv)) <= 0 ) {
	 usage();
      }
   }

   if (blocking) {
      /* this section attempts a reasonable inverse of the comment string */
      char *s = commentstart + commentlth - 1;
      char *d = &revcomment[0];
      while (s >= commentstart) {
	 char c = *s--;
	 switch (c) {
	 case '(':
	    c = ')'; break;
	 case '{':
	    c = '}'; break;
	 case '[':
	    c = ']'; break;
	 case '<':
	    c = '>'; break;
	 }
	 *d++ = c;
      }
      *d = '\0';
      linelength -= commentlth;
   }

   iplinenum = 0;
   oplinenum = 0;

   while ((ipline = gets(buf)) != NULL) {
      int initspaces;	/* number of spaces equivalent */

      iplth = strlen(ipline);
      initblanks = strspn(ipline,WHITE);

      if (iplth == initblanks) { /* current line is really empty */
	 if (oplth) newline();
	 emptyline();
	 comindent = 0;
      } else if ((*ipline == '.') && ! commenting) {
	 /* current line is an nroff command */
	 if (oplth) newline();
	 /* Yugh, this really uses a side effect of emptyline */
	 printf("%s",ipline);
	 oplth += iplth;
	 emptyline();
	 comindent = 0;
      } else {			/* current line is non empty */
	 char *word;

	 iplinenum++;

	 if (blocking) {
	    /* strip trailing comment marker */
	    char *p = ipline + strlen(ipline) - commentlth;
	    if (strcmp(p,revcomment) == 0) {
	       *p = '\0';
	    }
	    if (chkblank()) continue; /* resets iplth and initblanks */
	 }

	 initspaces = initcount(ipline,initblanks,0);

	 if (commenting) {
	    if (prefix(commentstart,ipline + initblanks)) {
	       int offset;
	       if (iplinenum == 1) {
		  comindent = initspaces;	/* comment indentation */
	       }

	       /* strip leading comment marker */
	       offset = initblanks + commentlth;
	       ipline += offset;
	       if (chkblank()) continue; /* sets iplth and initblanks */

	       /* now reset for text indents inside the comments */
	       initspaces = initcount(ipline,initblanks,offset);
	    } else {
	       if (iplinenum == 1) {
		  comindent = 0;	/* comment indentation */
	       }
	    }
         }

	 if (iplinenum <= 2) {
	    if (iplinenum == 1) {
	       if (framing) {
		  space(comindent);
		  dashline();
	       }
	       indent2 = indent1 = initspaces;	/* 1st line indent */
	    } else {
	       indent2 = initspaces;		/* subs line indent */
	    }
	 }

         while ((word = strtok(ipline,WHITE)) != NULL) {
	    int wordlth = strlen(word);
	    char lastchar = word[wordlth - 1];

	    if ((oplth + spacesbefore + wordlth) > linelength) {
	       newline();
	    }

            if (oplth == 0) {	/* start of new line */
	       if (oplinenum < 2) {
		  indent = (oplinenum == 1) ? indent2 : indent1;
	       }
	       if (commenting) {
		  space(comindent);
		  if (!stripping) {
		     printf("%s",commentstart);
		     oplth += commentlth;
		  }
	       }
	       space(indent);
            }

	    space(spacesbefore);
	    printf("%s",word);
	    oplth += wordlth;

	    spacesbefore =
	       ((lastchar=='.') || (lastchar=='!') || (lastchar=='?')) ? 2 : 1;

	    ipline = (char *) NULL;
	 }
      }
   }
   if (oplth) newline();
   if (framing && !stripping) {
      space(comindent);
      dashline();
   }
   return(0);
}

/* ------------------------------------------------------------- */
/* output routines of various kinds 				 */
/* ------------------------------------------------------------- */

void
space(howmany)
   register int howmany;
{
   register int t;
   t = howmany;
   while (t-- > 0) putchar(' ');
   oplth += howmany;
}

void
dash(howmany)
   register int howmany;
{
   register int t;
   t = howmany;
   while (t-- > 0) putchar(DASH);
   oplth += howmany;
}

void
newline()
{
   if (blocking) {
      if (padding) {
	 space(linelength - oplth);
      }
      if (!stripping) {
	 printf("%s",revcomment);
	 oplth += commentlth;
      }
   }
   putchar('\n');
   oplth = 0;
   spacesbefore = 0;
   oplinenum++;
}

void
emptyline()
{
   if (blocking && padding) {
      /* need comment start and end as well */
      space(comindent);
      if (!stripping) {
	 printf("%s",commentstart);
	 oplth += commentlth;
      }
      newline();
   } else {
      /* ordinary text or start only commenting just get blank lines */
      putchar('\n');
      oplth = 0;
      spacesbefore = 0;
   }
   iplinenum = 0;
   oplinenum = 0;
}

void
dashline(len)  		/* block out the line with a dash comment */
   int len;
{
   if (!stripping) {
      printf("%s",commentstart);
      oplth += commentlth;
   }
   dash(linelength - oplth);
   newline();
   oplinenum--; /* messy - dashlines shouldn't count */
}

/* ------------------------------------------------------------- */
/* input routines of various kinds 				 */
/* ------------------------------------------------------------- */

int
prefix(pre, s)			/* return true if pre is prefix in s */
   register char *pre, *s;
{
   register char c;
   while ((c = *pre++) == *s++)
   if (c == '\0')
       return(1);
   return((c == '\0')?1:0);
}

int
chkblank()	/* return true if line is blank, with side effects */
		/* like the line is output as well!		   */
{
   /* any comment symbols should already have been stripped */
   iplth = strlen(ipline);
   initblanks = strspn(ipline,WHITE);
   if (iplth == initblanks) { /* current text is empty */
      if (oplth) newline();
      emptyline();
      return(1);	/* was blank, indicate to caller */
   }
   if (framing) { /* then a line of dashes should be lost */
      char *s = ipline + initblanks;
      char *e = ipline + iplth;
      while (s < e && *s++ == DASH) ;
      if (s == e) {
	 /* effectively an empty line, but don't output it */
	 iplinenum = 0;
	 oplinenum = 0;
	 return(1);
      }
   }
   return(0);
}

int
initcount(line,blanks,offset)	 /* count initial tabs into spaces */
   char* line;			 /* return effective blank length */
   int blanks;
   int offset;
{  
   char *s,*e;
   int spaces;
   s = line;
   e = line + blanks;
   spaces = 0;
   while (s < e) {
      if (*s++ == '\t') {
	 spaces += (8 - ((offset + spaces + 8) % 8));
      } else {
	 spaces++;
      }
   }
   return(spaces);
}

/* ------------------------------------------------------------- */
/* miscellaneous routines 					*/
/* ------------------------------------------------------------- */

void
usage()
{
   fprintf(stderr, "usage: %s [-c commentchar [-bBfs] ] linelength\n",progname);
   exit(1);
}
