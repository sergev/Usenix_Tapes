/*  epx - take the output of 'nroff -Tepson | col -xf'	*/
/*	  and generate output for an epson printer	*/
/*							*/
/*	Author: Thomas E. Tkacik  uvaee!tet		*/
/*							*/
/*	To be modified or given away freely		*/
/*							*/
#include <stdio.h>
#define	NORMAL	0
#define	BOLD	1
#define	UNDER	2	/* underscore - italic */
#define	ITALIC	4
#define	SPEC	8	/* special characters - greek */
#define	NUMSPEC	60	/* number of defined special characters */
#define	FIRST	'@'	/* first special character is defined as FIRST */
#define	NO	0
#define	YES	1

int	cur;	/* current character */
int	ctype;
int	italics = NO;	/* use italics instead of underline */
int	nlqmode = NO;	/* print in Near Letter Quality mode */
int	copy=0;	/* no special characters yet */
int	cmark[NUMSPEC];
extern	char	special[NUMSPEC][12];	/* character definitions	*/
extern	char	nlq[NUMSPEC][48];	/* nlq character definitions	*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int c;
	int i,x;
	void putch(),putstr(),inspec(),outspec(),donormal();
	void tobold(),frbold(),tounder(),frunder(),toitalic(),fritalic();
	void tonlq(),frnlq(),halfline(),backhalf(),backfull();

	/* cannot use both NLQ and italics at the same time */
	nlqmode = NO;
	italics = NO;
    
	for(x=0;x<argc;x++)
		if(argv[x][0] = '-')
			switch(argv[x][1]) {
			case 'i': italics = YES;
			  	break;
			case 'n': nlqmode = YES;
			  	tonlq();	/* put printer in NLQ mode */
			  	break;
			}

	/* remove this statement if your printer can print italics in NLQ */
	if(nlqmode==YES) italics = NO;
    
    for(i=0;i<NUMSPEC;i++) cmark[i]=0; /* initialize special characters */
    cur =  0; /* no current character */
    ctype = NORMAL;  /* normal character */
    while((c=getchar()) != EOF) {
      switch (c) {
      default:          /* normal or special characters being printed */
          donormal();
          cur = c;
          break;
      case '\016':	/* special character set */
          inspec();
          break;
      case '\017':	/* normal character set */
          outspec();
          break;
      case '\033':	/* escape character */
	  if(cur)
	    putch(cur);
	  cur = 0;
	  if((c=getchar())==EOF) break; /* reached the end of file */
	  switch(c) {
	    case '7':
		backfull();		/* reverse full line feed */
		break;
	    case '8':
		backhalf();		/* reverse half line feed */
		break;
	    case '9':
		halfline();		/* forward half line feed */
	        break;
	    default:
	        putch('\033');	/* output escape character */
	        putch(c);
	        break;
	  }
	  break;
      case '\b':   /* a backspace - see if we must change print mode */
          while(((c=getchar())=='\016') || (c=='\017'))
            if(c=='\016')
              inspec();
            else
              outspec();
          if(c==EOF) break;  /* reached end of the file */
          if((cur==0) && (c!=0)) {     /* just do the overstrike */
            (void)putchar('\b');
	    (void)ungetc(c,stdin);  /* check the character next time around */
          }
          else if((cur=='_') && (c!='_')) { /**** ITALIC MODE ****/
            switch(ctype) {
              case BOLD:
              case (BOLD | SPEC):
		frbold();
                ctype &= ~BOLD;
		toitalic();
                ctype |= ITALIC;
                break; 
              case UNDER:
              case (UNDER | SPEC):
		frunder();
                ctype &= ~UNDER;
		toitalic();
                ctype |= ITALIC;
                break;
              case NORMAL:
              case SPEC:
		toitalic();
                ctype |= ITALIC;
                break;
              default:  /* ITALIC or (ITALIC | SPEC) */
                break;
            }
            putch(c);
            cur = 0; /* have output the current character */
          }
          else if((cur!='_') && (c=='_')) { /**** UNDERSCORE MODE ****/
	    /* do not print in underscore mode */
	    /* because '_' and underscore are not the same thing */
            putch(cur);
	    putch('\b');
            putch('_');
            cur = 0; /* have output the current character */
          }
          else if(cur==c) {             /**** BOLD MODE ****/
            switch(ctype) {
              case UNDER:
              case (UNDER | SPEC):
		frunder();
                ctype &= ~UNDER;
		tobold();
                ctype |= BOLD;
                break;
              case ITALIC:
              case (ITALIC | SPEC):
		fritalic();
                ctype &= ~ITALIC;
		tobold();
                ctype |= BOLD;
                break;
              case NORMAL:
              case SPEC:
		tobold();
                ctype |= BOLD;
                break;
              default:
                break;
            }
            putch(cur);
            /* find the beginning of the next character */
            while((c=getchar())=='\b') {
              c=getchar();
              if(cur != c) {
                putch('\b');  /* a different character - print it */
                (void)ungetc(c,stdin); /* check  character next time around */
                cur = 0;
		break;
              }
            }
	    if(cur) {
	      (void)ungetc(c,stdin); /* check character next time around */
	      cur = 0;
	    }
          }
          else { /* simple overstrike - do not change mode */
            if(cur) putch(cur);
	    cur = 0;
            (void)putchar('\b');
	    (void)ungetc(c,stdin); /* check character next time around */
          }
          break;
      } 
    }
    if(cur!=0) putch(cur);    /* print the last character */
    if(nlqmode==YES) frnlq();
}

/* put printer into alternate character set - user defined characters */
void tospec()
{
	void putstr();
	putstr("\033%");
	(void)putchar(1);
	(void)putchar(0);
}

/* the next characters are normal characters */
void frspec()
{
	void putstr();
	putstr("\033%");
	(void)putchar(0);
	(void)putchar(0);
}

/* print next characters in bold */
void tobold()
{
	void putstr();
	putstr("\033E");
}

/* do not print next characters in bold */
void frbold()
{
	void putstr();
	putstr("\033F");
}

/* print the next characters in italic */
void toitalic()
{
	void putstr(),tounder();
	if(italics == YES)
		putstr("\0334"); /* italicize next chatacters */
	else
		tounder();	 /* underline next characters */
}

/* do not print the next characters in italic */
void fritalic()
{
	void putstr(),frunder();
	if(italics == YES)
		putstr("\0335");
	else
		frunder();
}

/* underline the next characters */
void tounder()
{
	void putstr();
	putstr("\033-1");
}

/* do not underline the next characters */
void frunder()
{
	void putstr();
	putstr("\033-0");
}

/* put the printer in NLQ mode - this command may vary depending on printer */
void tonlq()
{
	void putstr();
	putstr("\033(");
}

/* restore printer mode */
void frnlq()
{
	void putstr();
	putstr("\033@");
}

/* cause the printer to generate a half linefeed */
void halfline()
{
	void putstr();
	putstr("\033J\022");
}

/* cause the printer to back up a half line spacing */
void backhalf()
{
	void putstr();
	putstr("\033j\022");
}

/* cause the printer to back up a full line spacing */
void backfull()
{
	void putstr();
	putstr("\033j\044");
}

/* download a special character before printing it */
void download(c)
char c;
{
	int i,j;
	void putstr(),frspec(),tospec();
	j = c - FIRST;
	putstr("\033&");
	(void)putchar(0);
	(void)putchar(c);
	(void)putchar(c);
	if(nlqmode==YES) {
		for(i=0;i<48;i++)
			(void)putchar(nlq[j][i]);
	} else {
		for(i=0;i<12;i++)
			(void)putchar(special[j][i]);
	}
}

void putstr(s)
char *s;
{
    while(*s!=0)
      (void)putchar(*s++);
}

/* check if the character to be printed is a special character */
/*     if it is and it has not been loaded into the printer then do it first */
void putch(c)
char c;
{
	int j;
	void download();
	j = c-FIRST;		/* index of special character */
	/* check if we are in SPEC mode  and the special character is defined */
	if(((ctype & SPEC) == SPEC) && (j>=0) && (j<NUMSPEC) && (cmark[j]==0)) {
		if((copy==0)&&(nlqmode==NO)) { /* copy the PGC ROM to RAM */
			(void)putchar('\033');
			(void)putchar(':');
			(void)putchar(0);
			(void)putchar(0);
			(void)putchar(0);
			copy = 1;
		} 

		download(c);
		cmark[j] = 1;
	}
	if(c!=0) {
		if((ctype & BOLD)&&(nlqmode==YES)) {
			(void)putchar(c);
			(void)putchar('\b');
			(void)putchar(c);
		} else
			(void)putchar(c);
	}
}

/* output the current character as a normal character */
/* 	then change to the special characters */
void inspec()
{
	void tospec();
	if(cur!=0) putch(cur); /* output the current character first */
	tospec();
	ctype |= SPEC;
	cur = 0;
}

/* output the current character as a special character */
/*	then change to normal character output */
void outspec()
{
	void frspec();
	if(cur!=0) putch(cur); /* output the current character first */
	frspec();
	ctype &= ~SPEC;
	cur = 0;
}

/* force the printer to print normal characters */
/*	then output the current character	*/
void donormal()
{
    if(cur!=0) {
      switch(ctype) { /* if not in normal mode - then enter normal mode */
        case BOLD:
        case (BOLD | SPEC):
	  frbold();
          ctype &= ~BOLD;
          break;
        case UNDER:
        case (UNDER | SPEC):
	  frunder();
          ctype &= ~UNDER;
          break;
        case ITALIC:
        case (ITALIC | SPEC):
	  fritalic();
          ctype &= ~ITALIC;
          break;
        case SPEC:  /* if special or normal mode, then do not change */
        case NORMAL:
        default:
          break;
      }
      putch(cur); /* output the current character */
    }
}
