/* This program has been modified by lln-cs!yl (Y. Ledru) to work under
   suntools and sunview */
/**********************************************************************/
/************************  Global Definitions  ************************/
/**********************************************************************/

typedef int BOOLEAN;
#define NEW(A) ((A *) malloc(sizeof(A)))
#define DEBUG   1			/* for massive printing of input */
					/* trace information; select by -d */
					/* option after filename: */
					/* dvibit filename -d */
#ifdef DEBUG
int Debug = 0;
#endif
#define ALLOW_INTERRUPT	1		/* to allow ^C for Debugging */
					/* undefine for normal catch-^C mode */

#define BINARYOPEN fopen		/* byte-oriented host version */

#define ARITHRSHIFT 1                   /* define if ">>" operator is a
                                           sign-propagating arithmetic
                                           right shift */
#define USEGLOBALMAG 1			/* when defined, the dvi global
   					   magnification is applied */

/* We leave USEGLOBALMAG undefined because there are only a very limited
   number font magnifications  provided for  this program and  if it  is
   defined, any dvi file is as likely as not to fail */
/* ---fixme--- put something into the log file if it's not mag 1000 */
#undef USEGLOBALMAG

#define  DVIFORMAT        2
#define  FALSE            0
#define  FIRSTPXLCHAR     0
#ifndef FONTAREA
#define  FONTAREA         "/usr/lib/dvifont/"
#endif
#define  LASTPXLCHAR    127
#define  MAXOPEN         30  /* new limit on number of open PXL files (yl) */
/* this limit should perhaps be lowered in order to avoid core dumps */
#define  NPXLCHARS      128
#define  PXLID         1001
#define  READ             4  /* for access() */
#define  RESOLUTION     118   /* is really 91.72192515 */
#define  hconvRESOLUTION 118
#define  vconvRESOLUTION 110
#define  STACKSIZE      100
#define  STRSIZE        257
#define  TRUE             1
#define  XSIZE         1024  
#define  YSIZE          800  

/**********************************************************************/
/***********************  external definitions  ***********************/
/**********************************************************************/

#include "commands.h"
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <pixrect/pixrect_hs.h>
/* yl begin */
#include <suntool/sunview.h>
#include <suntool/icon_load.h>
#include <suntool/canvas.h>
/* yl end */
extern struct pixrect *pr_open();
char *getenv();
char *index();
char *malloc();
char *rindex();
char *sprintf();
char *strcpy();

int dummyInt;
short dummyShort;
char dummyChar;

/*-->NoSignExtend*/
/**********************************************************************/
/***************************  NoSignExtend  ***************************/
/**********************************************************************/

int
NoSignExtend(fp, n)	/* return n byte quantity from file fd */
register FILE *fp;	/* file pointer    */
register int n;		/* number of bytes */

{
    register int x;	/* number being constructed */

    x = 0;
    while (n--)  {
	x <<= 8;
	x |= getc(fp);
    }
    return(x);
}
/*-->SignExtend*/
/**********************************************************************/
/****************************  SignExtend  ****************************/
/**********************************************************************/

int
SignExtend(fp, n)   /* return n byte quantity from file fd */
register FILE *fp;  /* file pointer    */
register int n;     /* number of bytes */

{
    int n1;         /* number of bytes	    */
    register int x; /* number being constructed */

    x = getc(fp);   /* get first (high-order) byte */
    n1 = n--;
    while (n--)  {
	x <<= 8;
	x |= getc(fp);
    }

    /* NOTE: This code assumes that the right-shift is an arithmetic, rather
    than logical, shift which will propagate the sign bit right.   According
    to Kernighan and Ritchie, this is compiler dependent! */

    x<<=32-8*n1;
    x>>=32-8*n1;  /* sign extend */

#ifdef DEBUG
    if (Debug)
    {
	fprintf(stderr,"\tSignExtend(fp,%d)=%X\n",n1,x);
    }
#endif
    return(x);
}
/**********************************************************************/
/*************************  Global Procedures  ************************/
/**********************************************************************/

/* Note: Global procedures are declared here in alphabetical order, with
   those which do not return values typed "void".  Their bodies occur in
   alphabetical order following the main() procedure.  The names are
   kept unique in the first 6 characters for portability. */

void	AbortRun();
float	ActualFactor();
void	AllDone();
FILE*	BINARYOPEN();
void	Fatal();
void	FindPostAmblePtr();
void	GetBytes();
void	GetFontDef();
void	GotInterrupt();
void	InitTerm();
int	InputWaiting();
void	LoadRast();
void	MoveDown();
void	MoveOver();
void	OpenFontFile();
int	PixRound();
void	PutOut();
int	ReadFontDef();
void	ReadPostAmble();
void	ResetTerm();
void	SetChar();
void	SetFntNum();
void	SetRule();
void	SkipFontDef();
void	Warning();

/**********************************************************************/
/*************************  Global Variables  *************************/
/**********************************************************************/

int G_errenc = FALSE;	   /* has an error been encountered?          */
char G_Logname[STRSIZE];   /* name of log file, if created            */
int G_interactive = TRUE;  /* is the program running interactively    */
                           /* (i.e., standard output not redirected)? */
int G_logging = 0;         /* Is a log file being created?            */
struct sgttyb G_intty;     /* information about stdin if interactive  */
FILE *G_logfp;             /* log file pointer (for errors)           */
char G_progname[STRSIZE];  /* program name                            */

struct char_entry {		/* character entry */
   unsigned short width, height;/* width and height in pixels */
   short xOffset, yOffset;      /* x offset and y offset in pixels */
   struct {
       int isloaded;
       union {
	   int fileOffset;
	   struct pixrect *pixrectptr;
       } address;
   } where;
   int tfmw;			/* TFM width */
};

struct font_entry {  /* font entry */
   int k, c, s, d, a, l;
   char n[STRSIZE];	/* FNT_DEF command parameters  */
   int font_space;	/* computed from FNT_DEF s parameter        */
   int font_mag;	/* computed from FNT_DEF s and d parameters */
   char name[STRSIZE];	/* full name of PXL file                    */
   FILE *font_file_id;  /* file identifier (0 if none)              */
   int magnification;	/* magnification read from PXL file         */
   int designsize;	/* design size read from PXL file           */
   struct char_entry ch[NPXLCHARS];/* character information         */
   struct font_entry *next;
};

struct pixel_list
{
    FILE *pixel_file_id;        /* file identifier                          */
    int use_count;              /* count of "opens"                         */
};

int hconv, vconv;		/* converts DVI units to pixels             */
int den;			/* denominator specified in preamble        */
FILE *dvifp;			/* DVI file pointer                         */
struct font_entry *fontptr;     /* font_entry pointer                       */
struct font_entry *hfontptr=NULL;/* font_entry pointer                      */
int h;				/* current horizontal position              */
int hh;				/* current horizontal position in pixels    */
int v;				/* current vertical position                */
int vv;				/* current vertical position in pixels      */
int mag;			/* magnification specified in preamble      */
int nopen;			/* number of open PXL files                 */
int num;			/* numerator specified in preamble          */
struct font_entry *pfontptr = NULL; /* previous font_entry pointer          */
struct pixel_list pixel_files[MAXOPEN+1];
                                /* list of open PXL file identifiers        */
struct pixrect *screen, *display;
/* yl begin */
    Frame frame;
    Canvas canvas;
    Pixwin *pw;
    char filename[STRSIZE]; /* file name			     */
/*yl end*/

long postambleptr;		/* Pointer to the postamble                 */
FILE *pxlfp;			/* PXL file pointer                         */
char *PXLpath;			/* PXL path name for search		    */
struct sgttyb tty;	        /* to see if program is running interactively */
#define YDEFAULT 0
int ydefault = YDEFAULT;
int xscreen = 0, yscreen = YDEFAULT;
/******************************** Unix only definitions ******************/

struct tchars tcb;		/* information about special terminal chars */

/**********************************************************/


/**********************************************************************/
/*******************************  main  *******************************/
/**********************************************************************/

main(argc, argv)
int argc;
char *argv[];

{
    struct stack_entry {  /* stack entry */
	int h, v, w, x, y, z;  /* what's on stack */
    };


    int argind;             /* argument index for flags              */
    int command;	    /* current command			     */
    long cpagep;	    /* current page pointer		     */
    char curarea[STRSIZE];  /* current file area		     */
    char curext[STRSIZE];   /* current file extension		     */
    char curname[STRSIZE];  /* current file name		     */
/*    char filename[STRSIZE]; *//* file name			     */
    register int i;	    /* command parameter; loop index	     */
    int k;		    /* temporary parameter		     */
    char n[STRSIZE];	    /* command parameter		     */
    int PagesLeft;          /* Number of pages left to skip          */
    int PreLoad = TRUE;	    /* preload the font descriptions?	     */
    long ppagep;	    /* previous page pointer		     */
    int SkipEOP = FALSE;    /* input is waiting from the terminal, skip to EOP */
    int SkipMode = FALSE;   /* in skip mode flag                     */
    int SoundBell = FALSE;   /* if false supress bell at end of page  */
    int sp;		    /* stack pointer			     */
    struct stack_entry stack[STACKSIZE];   /* stack		     */
    int t;		    /* temporary			     */
    char tc;		    /* temporary character		     */
    char *tcp, *tcp1;	    /* temporary character pointers	     */
    register struct char_entry *tcharptr;/* temporary char_entry ptr */
    int val, val2;          /* temporarys to hold command information*/
    int w;		    /* current horizontal spacing	     */
    int x;		    /* current horizontal spacing	     */
    int y;		    /* current vertical spacing		     */
    int z;		    /* current vertical spacing		     */
/*yl*/
    int mpager;             /* designed for using texview with the m key */
/*end yl*/

/*yl*/
    mpager = 0;
/*end yl*/
    nopen = 0;
    strcpy(G_progname, argv[0]);

    if (argc < 2)  {
	fprintf(stderr, "\nusage: %s -bpld +pages dvifile\n\n", G_progname);
	AbortRun(1);
    }

    if (ioctl(1,TIOCGETP,&tty) == -1)
	G_interactive = FALSE;

   /*  get enviroment variables that allows for a path name in unix */

   if ((PXLpath=getenv("BGPXLPATH")) == NULL)
	PXLpath = FONTAREA;
    argind = 1;
    while (argind < argc && (*argv[argind] == '-' || *argv[argind] == '+'))
    {   
	tcp = argv[argind];
        if (*tcp == '-')
          { tcp++;
	    switch(*tcp){
	        case 'b':       /* b supresses the bell at end of page */
		    SoundBell = TRUE;
		    break;

#ifdef DEBUG
		case 'd':	/* d selects Debug output */
		    Debug = TRUE;
		    break;
#endif

		case 'l':	/* l prohibits logging of errors */
		    G_logging = -1;
		    break;
	
		case 'p':	/* p prohibits pre-font loading */
		    PreLoad = 0;
		    break;

		default:
		    printf("%c is not a legal flag\n", *tcp);
		}
	} else
	{ sscanf(&argv[argind][1], "%d", &PagesLeft);
	  SkipMode = TRUE;
	  PagesLeft++;
        }
	argind++;
    }

    tcp = rindex(argv[argind], '/');
    if (tcp == NULL)  {
	curarea[0] = '\0';
	tcp = argv[argind];
    }
    else  {
	strncpy(curarea, argv[argind], tcp-argv[argind]+1);
	tcp += 1;
    }
    tcp1 = index(tcp, '.');
    if (tcp1 == NULL)  {
	strcpy(curname, tcp);
	tcp1 = index(tcp, '\0');
    }
    else
	strncpy(curname, tcp, tcp1-tcp);
    strcpy(curext, tcp1);

    strcpy(filename, curarea);
    strcat(filename, curname);
    if (curext[0] == '\0')
	strcat(filename, ".dvi");
    else
	strcat(filename, curext);
    if ((dvifp=BINARYOPEN(filename,"r")) == NULL)  {
	fprintf(stderr,"\n");
	fprintf(stderr,"%s: can't open \"%s\"\n\n", G_progname, filename);
	AbortRun(1);
    }

    strcpy(G_Logname, curname);
    strcat(G_Logname, ".dvi-log");

    argind++;
    while (argind < argc && (*argv[argind] == '-' || *argv[argind] == '+'))
    {   
	tcp = argv[argind];
        if (*tcp == '-')
          { tcp++;
	    switch(*tcp){
	        case 'b':       /* b supresses the bell at end of page */
		    SoundBell = TRUE;
		    break;

#ifdef DEBUG
		case 'd':	/* d selects Debug output */
		    Debug = TRUE;
		    break;
#endif

		case 'l':	/* l prohibits logging of errors */
		    G_logging = -1;
		    break;
	
		case 'p':	/* p prohibits pre-font loading */
		    PreLoad = 0;
		    break;

		default:
		    printf("%c is not a legal flag\n", *tcp);
		}
	} else
	{ sscanf(&argv[argind][1], "%d", &PagesLeft);
          argind++;
	  SkipMode = TRUE;
	  PagesLeft++;
        }
	argind++;
    }

   /* The terminal characteristics must be saved before any attempt is made */
   /* to process the DVI file, so that if the latter should fail, procedure */
   /* Fatal will be able to correctly restore the terminal state. */
    if (G_interactive)  {
	ioctl(0, TIOCGETC, &tcb);  /* get special characters */
	ioctl(0, TIOCGETP, &G_intty);  /* get modes */
	tty = G_intty;
	tty.sg_flags |= CBREAK;
	tty.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, &tty);  /* set modes */
    }

#ifndef ALLOW_INTERRUPT
    signal(SIGINT, SIG_IGN);  /* ignore interrupts */
#endif
    InitTerm();
#ifndef ALLOW_INTERRUPT
    signal(SIGINT, GotInterrupt);  /* catch interrupts */
#endif

    if ((i = NoSignExtend(dvifp, 1)) != PRE)  {
	fprintf(stderr,"\n");
	Fatal("%s: PRE doesn't occur first--are you sure this is a DVI file?\n\n",
	G_progname);
    }

    i = SignExtend(dvifp, 1);
    if (i != DVIFORMAT)  {
	fprintf(stderr,"\n");
	Fatal("%s: DVI format = %d, can only process DVI format %d files\n\n",
	G_progname, i, DVIFORMAT);
    }

    if (PreLoad)
    {
	ReadPostAmble();
	fseek(dvifp, (long) 14, 0);
    }
    else
    {
	num = NoSignExtend(dvifp, 4);
	den = NoSignExtend(dvifp, 4);
	mag = NoSignExtend(dvifp, 4);
	hconv = DoConv(num, den, hconvRESOLUTION);
	vconv = DoConv(num, den, vconvRESOLUTION);
    }
    k = NoSignExtend(dvifp, 1);
    GetBytes(dvifp, n, k);
    while (TRUE)

	switch (command=NoSignExtend(dvifp, 1))  {

	case SET1:case SET2:case SET3:case SET4:
	    val = NoSignExtend(dvifp, command-SET1+1);
	    if (!SkipMode) SetChar(val, command);
	    break;

	case SET_RULE:
	    val = NoSignExtend(dvifp, 4);
	    val2 = NoSignExtend(dvifp, 4);
            if (!SkipMode) SetRule(val, val2, 1);
	    break;

	case PUT1:case PUT2:case PUT3:case PUT4:
	    val = NoSignExtend(dvifp,command-PUT1+1);
	    if (!SkipMode) SetChar(val, command);
	    break;

	case PUT_RULE:
	    val = NoSignExtend(dvifp, 4);
	    val2 = NoSignExtend(dvifp, 4);
            if (!SkipMode) SetRule(val, val2, 0);
	    break;

	case NOP:
	    break;

	case BOP:
	    cpagep = ftell(dvifp) - 1;
	    for (i=0; i<=9; i++)
		NoSignExtend(dvifp, 4);
	    ppagep = NoSignExtend(dvifp, 4);
/*
	    pr_rop(display, 0, 0, display->pr_size.x, display->pr_size.y, PIX_CLR, NULL, 0, 0);
*/
	    pw_rop(pw, 0, 0, pw->pw_pixrect->pr_size.x, pw->pw_pixrect->pr_size.y, PIX_CLR, NULL, 0, 0);  /*yl*/
	    h = v = w = x = y = z = 0;
	    sp = 0;
	    fontptr = NULL;
	    if (PagesLeft)
		{ PagesLeft--;
		  if (PagesLeft <= 0) SkipMode = FALSE;
		};
	    break;

	case EOP:
	    if (SkipEOP) { SkipMode = FALSE; SkipEOP = FALSE; }
	    if (G_interactive && !SkipMode)  {
		t = FALSE;
		while (!t)  {  /* sorry about this flow of control kludge */
		    t = TRUE;
		    tc = getchar();
		    if (tc==tcb.t_eofc) AllDone();
		    switch (tc)  {
		    case ' ':		/* normal case - start next page */
		    case '\n':
		    case 'n':
			xscreen = 0; yscreen = ydefault;
			break;
		    case 'm':           /*yl using texview as a pager */
		    case 'M':
			if (mpager == 0) /* equivalent to 'k' */
			  { 
			   yscreen += (YSIZE/2); /* yl 3->2 */
			   fseek(dvifp, cpagep, 0);
			   mpager = 1;
		          }
			else
			  {
			   xscreen = 0; yscreen = ydefault;
			   mpager = 0;
			  }
			break;
		    case 'j':            /* vi keys (yl)*/
		    case 'D':		/* move down and redisplay */
			yscreen -= (YSIZE/2); /* yl 3->2 */
			fseek(dvifp, cpagep, 0);
			break;

		    case 'h':            /* vi keys (yl)*/
		    case 'L':		/* move left and redisplay */
			xscreen += (XSIZE/3);
			fseek(dvifp, cpagep, 0);
			break;

		    case 'b':
		    case 'P':
		    case 'p':		/* redisplay from previous page */
			if (ppagep != -1)  {
			    fseek(dvifp, ppagep, 0);
			}
			else  {
			    fseek(dvifp, cpagep, 0);
			    xscreen = 0; yscreen = ydefault;
			}
			break;

		    case 'q':		/* quit */
		    case 'e':		/* exit */
		    case 'Q':		/* quit */
		    case 'E':		/* exit */
	    		AllDone();
	    		break;

		    case 'l':            /* vi keys (yl)*/
		    case 'R':		/* move right and redisplay */
			xscreen -= (XSIZE/3);
			fseek(dvifp, cpagep, 0);
			break;

		    case 'k':            /* vi keys (yl)*/
		    case 'U':		/* move up and redisplay */
			yscreen += (YSIZE/2); /* yl 3->2 */
			fseek(dvifp, cpagep, 0);
			break;

		     case '-':
			/* read in val */
			val = ReadInt();
			while (val-- && ppagep != -1)
				{ fseek(dvifp, ppagep, 0);
				  NoSignExtend(dvifp, 1);
				  for(i=0; i<=9; i++)
					NoSignExtend(dvifp, 4);
				  cpagep = ppagep;
				  ppagep = NoSignExtend(dvifp, 4);
				}
			fseek(dvifp, cpagep, 0);
			xscreen = 0; yscreen = ydefault;
			break;

#ifdef DEBUG
		     case 'f':	/* fine tune y */
			ydefault = ReadInt();
			yscreen = ydefault;
			fseek(dvifp, cpagep, 0);
			break;

		     case 'H': /* fine tune hconv */
			hconv = DoConv(num, den, ReadInt());
			fseek(dvifp, cpagep, 0);
			break;

		     case 'v': /* fine tune hconv */
			vconv = DoConv(num, den, ReadInt());
			fseek(dvifp, cpagep, 0);
			break;
#endif

		     case '+':
			/* read in val */
			val = ReadInt();
			SkipMode = TRUE;
			PagesLeft = val;
			break;

		    default:
			t = FALSE;
		    }
		}
	    }
	    break;

	case PUSH:
	    if (sp >= STACKSIZE)
		Fatal("stack overflow");
	    stack[sp].h = h;
	    stack[sp].v = v;
	    stack[sp].w = w;
	    stack[sp].x = x;
	    stack[sp].y = y;
	    stack[sp].z = z;
	    sp++;
	    break;

	case POP:
	    --sp;
	    if (sp < 0)
		Fatal("stack underflow");
	    h = stack[sp].h;
	    v = stack[sp].v;
	    w = stack[sp].w;
	    x = stack[sp].x;
	    y = stack[sp].y;
	    z = stack[sp].z;
	    break;

	case RIGHT1:case RIGHT2:case RIGHT3:case RIGHT4:
            val = SignExtend(dvifp,command-RIGHT1+1);
	    if (!SkipMode) MoveOver(val);
	    break;

	case W0:
            if (!SkipMode) MoveOver(w);
	    break;

	case W1:case W2:case W3:case W4:
	    w = SignExtend(dvifp,command-W1+1);
            if (!SkipMode) MoveOver(w);
	    break;

	case X0:
            if (!SkipMode) MoveOver(x);
	    break;

	case X1:case X2:case X3:case X4:
	    x = SignExtend(dvifp,command-X1+1);
	    if (!SkipMode) MoveOver(x);
	    break;

	case DOWN1:case DOWN2:case DOWN3:case DOWN4:
            val = SignExtend(dvifp,command-DOWN1+1);
	    if (!SkipMode) MoveDown(val);
	    break;

	case Y0:
            if (!SkipMode) MoveDown(y);
	    break;

	case Y1:case Y2:case Y3:case Y4:
	    y = SignExtend(dvifp,command-Y1+1);
            if (!SkipMode) MoveDown(y);
	    break;

	case Z0:
            if (!SkipMode) MoveDown(z);
	    break;

	case Z1:case Z2:case Z3:case Z4:
	    z = SignExtend(dvifp,command-Z1+1);
	    if (!SkipMode) MoveDown(z);
	    break;

	case FNT1:case FNT2:case FNT3:case FNT4:
	    if (InputWaiting(stdin)) { SkipMode = TRUE; SkipEOP = TRUE;}
            if (!SkipMode) SetFntNum(NoSignExtend(dvifp,command-FNT1+1));
	    break;

	case XXX1:case XXX2:case XXX3:case XXX4:
	    k = NoSignExtend(dvifp,command-XXX1+1);
	    while (k--)
		NoSignExtend(dvifp, 1);
	    break;

	case FNT_DEF1:case FNT_DEF2:case FNT_DEF3:case FNT_DEF4:
	    if (PreLoad)
	    {
		SkipFontDef (NoSignExtend(dvifp, command-FNT_DEF1+1));
	    }
	    else
	    {
		ReadFontDef (NoSignExtend(dvifp, command-FNT_DEF1+1));
	    }
	    break;

	case PRE:
	    Fatal("PRE occurs within file");
	    break;

	case POST:
	    if (SkipMode) fseek(dvifp, cpagep, 0);
	           else { fseek(dvifp, (long) -2, 1); }
	    SkipMode = FALSE;
	    PagesLeft = 0;
	    break;

	case POST_POST:
 	    Fatal("POST_POST with no preceding POST");
	    break;

	default:
	    if (command >= FONT_00 && command <= FONT_63)
		{if (InputWaiting(stdin)) { SkipMode = TRUE; SkipEOP = TRUE;}
		 if (!SkipMode) SetFntNum(command - FONT_00);}
	    else if (command >= SETC_000 && command <= SETC_127)
		{if (!SkipMode) SetChar(command - SETC_000, command);}
	    else
		Fatal("%d is an undefined command", command);
	    break;

	}
}

/*-->AbortRun*/
/*---------end of first part (remove this line from the shar file)---------*/
/**********************************************************************/
/***************************  AbortRun  *******************************/
/**********************************************************************/

void
AbortRun(code)
int code;
{
    exit(code);
}


/*-->ActualFactor*/
/**********************************************************************/
/**************************  ActualFactor  ****************************/
/**********************************************************************/

float		/* compute the actual size factor given the approximation */
ActualFactor(unmodsize)
int unmodsize;  /* actually factor * 1000 */
{
    float realsize;	/* the actual magnification factor */

    realsize = (float)unmodsize / 1000.0;
    /* a real hack to correct for rounding in some cases--rkf */
    if(unmodsize==1095) realsize = 1.095445;	/*stephalf*/
    else if(unmodsize==1315) realsize=1.314534;	/*stepihalf*/
    else if(unmodsize==2074) realsize=2.0736;	/*stepiv*/
    else if(unmodsize==2488) realsize=2.48832;  /*stepv*/
    else if(unmodsize==2986) realsize=2.985984;	/*stepiv*/
    /* the remaining magnification steps are represented with sufficient
	   accuracy already */
    return(realsize);
}


/*-->AllDone*/
/**********************************************************************/
/****************************** AllDone  ******************************/
/**********************************************************************/

void
AllDone()
{
    char t;

    ResetTerm();
    if (G_errenc && G_logging == 1)  {
	fseek(G_logfp, 0, 0);
	while ((t=getc(G_logfp)) != EOF)
	    putchar(t);
    }
    if (G_logging == 1) printf("Log file created\n");
    AbortRun(G_errenc);
}

/*-->DoConv*/

int DoConv(num, den, convResolution)
{
    register float conv;
    conv = ((float)num/(float)den) * 
#ifdef USEGLOBALMAG
/*	ActualFactor(mag) * why was this in as Actual Factor?  jls */
	((float) mag/1000.0) *
#endif
	((float)convResolution/254000.0);
    return((int) (1.0 / conv + 0.5));
}

/*-->Fatal*/
/**********************************************************************/
/******************************  Fatal  *******************************/
/**********************************************************************/

void
Fatal(fmt, args)/* issue a fatal error message */
char *fmt;	/* format */
char *args;	/* arguments */

{
    if (G_logging == 1)
    {
	fprintf(G_logfp, "%s: FATAL--", G_progname);
	_doprnt(fmt, &args, G_logfp);
	fprintf(G_logfp, "\n");
    }

    ResetTerm();
    fprintf(stderr,"\n");
    fprintf(stderr, "%s: FATAL--", G_progname);
    _doprnt(fmt, &args, stderr);
    fprintf(stderr, "\n\n");
    if (G_logging == 1) printf("Log file created\n");
    AbortRun(1);
}


/*-->FindPostAmblePtr*/
/**********************************************************************/
/************************  FindPostAmblePtr  **************************/
/**********************************************************************/

void
FindPostAmblePtr(postambleptr)
long	*postambleptr;

/* this routine will move to the end of the file and find the start
    of the postamble */

{
    int     i;

    fseek (dvifp, (long) 0, 2);   /* goto end of file */
    *postambleptr = ftell (dvifp) - 4;
    fseek (dvifp, *postambleptr, 0);

    while (TRUE) {
	fseek (dvifp, --(*postambleptr), 0);
	if (((i = NoSignExtend(dvifp, 1)) != 223) &&
	    (i != DVIFORMAT))
	    Fatal ("Bad end of DVI file");
	if (i == DVIFORMAT)
	    break;
    }
    fseek (dvifp, (*postambleptr) - 4, 0);
    (*postambleptr) = NoSignExtend(dvifp, 4);
    fseek (dvifp, *postambleptr, 0);
}


/*-->GetBytes*/
/**********************************************************************/
/*****************************  GetBytes  *****************************/
/**********************************************************************/

void
GetBytes(fp, cp, n)	/* get n bytes from file fp */
register FILE *fp;	/* file pointer	 */
register char *cp;	/* character pointer */
register int n;		/* number of bytes  */

{
    while (n--)
	*cp++ = getc(fp);
}


/*-->GetFontDef*/
/**********************************************************************/
/**************************** GetFontDef  *****************************/
/**********************************************************************/

void
GetFontDef()

/***********************************************************************
   Read the font  definitions as they  are in the  postamble of the  DVI
   file.  Note that the font directory  is not yet loaded.  In order  to
   adapt ourselves to the existing "verser" the following font paramters
   are  copied	 onto  output	fontno  (4   bytes),  chksum,	fontmag,
   fontnamelength (1 byte), fontname.  At the end, a -1 is put onto  the
   file.
***********************************************************************/

{
    char    str[50], *calloc ();
    unsigned char   byte;
    int     i, fnamelen;

    while (((byte = NoSignExtend(dvifp, 1)) >= FNT_DEF1) &&
	(byte <= FNT_DEF4)) {
	switch (byte) {
	case FNT_DEF1:
	    ReadFontDef (NoSignExtend(dvifp, 1));
	    break;
	case FNT_DEF2:
	    ReadFontDef (NoSignExtend(dvifp, 2));
	    break;
	case FNT_DEF3:
	    ReadFontDef (NoSignExtend(dvifp, 3));
	    break;
	case FNT_DEF4:
	    ReadFontDef (NoSignExtend(dvifp, 4));
	    break;
	default:
	    Fatal ("Bad byte value in font defs");
	    break;
	}
    }
    if (byte != POST_POST)
	Fatal ("POST_POST missing after fontdefs");
}


/*-->GotInterrupt*/
/**********************************************************************/
/***************************  GotInterrupt  ***************************/
/**********************************************************************/

void
GotInterrupt(sig)   /* don't leave terminal in a weird state */
int sig;

{
    ResetTerm();
    if (G_logging == 1) printf("Log file created\n");
    AbortRun(G_errenc);
}


/*-->InitTerm*/
/**********************************************************************/
/*****************************  InitTerm  *****************************/
/**********************************************************************/

void
InitTerm()	/* initialize terminal */

{
/* yl begin */
Icon icon;
struct pixrect *pr1;
 char error[256];
/* yl end */
/*    screen = pr_open("/dev/fb");
    if (screen == NULL) {
	perror("pixel: ");
	exit(1);
    }
    display = screen; */ /* old version (yl) */
 /* yl begin */
  frame = window_create(NULL,FRAME,0);
  canvas = window_create(frame,CANVAS,
			 CANVAS_AUTO_SHRINK,FALSE,
/*			 CANVAS_WIDTH,1024,
			 CANVAS_HEIGHT,800,
			 WIN_VERTICAL_SCROLLBAR, scrollbar_create(0),
			 WIN_HORIZONTAL_SCROLLBAR, scrollbar_create(0),
			 WIN_ASCII_EVENTS, my_event_proc,*/
			 0);
  pw = canvas_pixwin(canvas);
  window_set(frame,WIN_WIDTH,900,0);
  window_set(frame,WIN_HEIGHT,900,0);
  window_set(frame,WIN_X,100,0);
  window_set(frame,WIN_Y,0,0);
  window_set(frame,FRAME_LABEL,"TeXview",0);
  window_set(frame,WIN_SHOW,TRUE,0);
pr1 = icon_load_mpr("/usr/include/images/texview.icon",error); 
icon = icon_create(ICON_LABEL,filename,
		   ICON_IMAGE,pr1,0);
window_set(frame,FRAME_ICON,icon,0);

  notify_do_dispatch();

 /* yl end */
}

/*-->InputWaiting*/
/**********************************************************************/
/************************* InputWaiting *******************************/
/**********************************************************************/

int
InputWaiting(fp)
FILE *fp;

/*   this routine returns TRUE if there is input waiting to be read
	and FALSE otherwise.  note that it may take time to realize
	that there is input waiting so the routine may occasionaly
	be wrong.   */
{
    long retval;

    if (fp->_cnt != 0) return(TRUE);
    ioctl(fileno(fp), FIONREAD, &retval);
    return((int)retval);
}


/*-->LoadRast*/
/**********************************************************************/
/*****************************  LoadRast  *****************************/
/**********************************************************************/

void
LoadRast(pxlfp, w, h)	/* load raster pattern into BitGraph */
FILE *pxlfp;		/* PXL file pointer	   */
int w;			/* width of raster in pixels */
int h;			/* height of raster in pixels */

{
    register int ew;  /* extra word at end of row? */
    register int t;   /* temporary		  */
    register int ww;  /* width of raster in words */

    ww = (w+15) / 16;
    ew = ww & 1;
    while (h--)  {
	t = ww;
	while (t--)
	    PutOut(NoSignExtend(pxlfp, 2));
	if (ew)
	    NoSignExtend(pxlfp, 2);
    }
}


/*-->MoveDown*/
/**********************************************************************/
/****************************  MoveDown  ******************************/
/**********************************************************************/

void
MoveDown(a)
int a;
{
    v += a;
}


/*-->MoveOver*/
/**********************************************************************/
/****************************  MoveOver  ******************************/
/**********************************************************************/

void
MoveOver(b)
int b;
{
    h += b;
}


/*-->OpenFontFile*/
/**********************************************************************/
/************************** OpenFontFile  *****************************/
/**********************************************************************/

void
OpenFontFile()
/***********************************************************************
    The original version of this dvi driver reopened the font file  each
    time the font changed, resulting in an enormous number of relatively
    expensive file  openings.   This version  keeps  a cache  of  up  to
    MAXOPEN open files,  so that when  a font change  is made, the  file
    pointer, pxlfp, can  usually be  updated from the  cache.  When  the
    file is not found in  the cache, it must  be opened.  In this  case,
    the next empty slot  in the cache  is assigned, or  if the cache  is
    full, the least used font file is closed and its slot reassigned for
    the new file.  Identification of the least used file is based on the
    counts of the number  of times each file  has been "opened" by  this
    routine.  On return, the file pointer is always repositioned to  the
    beginning of the file.

***********************************************************************/
{
    register int i,least_used,current;

#ifdef DEBUG
    if (Debug) printf("Open Font file\n");
#endif
    if (pfontptr == fontptr)
        return;                 /* we need not have been called */

    for (current = 1;
	(current <= nopen) &&
	    (pixel_files[current].pixel_file_id != fontptr->font_file_id);
	++current)
	;                       /* try to find file in open list */

    if (current <= nopen)       /* file already open */
    {
	pxlfp = pixel_files[current].pixel_file_id;
	fseek(pxlfp,0,0);	/* reposition to start of file */
    }
    else                        /* file not in open list */
    {
        if (nopen < MAXOPEN)    /* just add it to list */
            current = ++nopen;
	else                    /* list full -- find least used file, */
	{                       /* close it, and reuse slot for new file */
	    least_used = 1;
            for (i = 2; i <= MAXOPEN; ++i)
	        if (pixel_files[least_used].use_count >
                    pixel_files[i].use_count)
		    least_used = i;
	    fclose(pixel_files[least_used].pixel_file_id);
	    current = least_used;
        }
        if ((pxlfp=BINARYOPEN(fontptr->name,"r")) == NULL)
	    Fatal("PXL file \"%s\" could not be opened; %d PXL files are open",
		fontptr->name,nopen);
	pixel_files[current].pixel_file_id = pxlfp;
	pixel_files[current].use_count = 0;
    }
    pfontptr = fontptr;			/* make previous = current font */
    fontptr->font_file_id = pxlfp;	/* set file identifier */
    pixel_files[current].use_count++;	/* update reference count */
}


/*-->PixRound*/
/**********************************************************************/
/*****************************  PixRound  *****************************/
/**********************************************************************/

int
PixRound(x, conv)	/* return rounded number of pixels */
register int x;		/* in DVI units     */
int conv;		/* conversion factor */
{
    return((int)((x + (conv >> 1)) / conv));
}


/*-->PutOut*/
/**********************************************************************/
/******************************  PutOut  ******************************/
/**********************************************************************/

void
PutOut(x)	/* put a 16-bit raster pattern to the BitGraph */
register int x; /* number to put out */

{
    int negative;		       /* was x negative? */
    register int part1, part2, part3;  /* parts of number */

    negative = FALSE;
    if (x < 0)  {
	negative = TRUE;
	x = -x;
    }
    part1 = (x & 0xfc00) >> 10;
    part2 = (x & 0x03f0) >>  4;
    part3 = (x & 0x000f);
    if (part1)  {
	putchar(0100+part1);
	putchar(0100+part2);
    }
    else
	if (part2)
	    putchar(0100+part2);
    if (negative)
	putchar(040+part3);
    else
	putchar(060 + part3);
}


/*-->ReadFontDef*/
/**********************************************************************/
/****************************  ReadFontDef  ***************************/
/**********************************************************************/

int
ReadFontDef(k)
int k;
{
    int t, i;
    register struct font_entry *tfontptr;    /* temporary font_entry pointer   */
    register struct char_entry *tcharptr;/* temporary char_entry pointer  */
    char *direct, *tcp, *tcp1;
    int found;
    char curarea[STRSIZE];

    if ((tfontptr = NEW(struct font_entry)) == NULL)
	Fatal("can't malloc space for font_entry");
    tfontptr->next = hfontptr;
    fontptr = hfontptr = tfontptr;

    tfontptr->k = k;
    tfontptr->c = NoSignExtend(dvifp, 4); /* checksum */
    tfontptr->s = NoSignExtend(dvifp, 4); /* space size */
    tfontptr->d = NoSignExtend(dvifp, 4); /* design size */
    tfontptr->a = NoSignExtend(dvifp, 1); /* area length for font name */
    tfontptr->l = NoSignExtend(dvifp, 1); /* device length */
    GetBytes(dvifp, tfontptr->n, tfontptr->a+tfontptr->l);
    tfontptr->n[tfontptr->a+tfontptr->l] = '\0';
    tfontptr->font_space = tfontptr->s/6; /* never used */
    tfontptr->font_mag = (int)((ActualFactor((int)(((float)tfontptr->s/
    			(float)tfontptr->d)*1000.0 + 0.5)) * 
#ifdef USEGLOBALMAG
			ActualFactor(mag) *
#endif
			(float)RESOLUTION * 5.0) + 0.5);
    if (tfontptr->a != 0) {
	sprintf(tfontptr->name, "%s.%dpxl", tfontptr->n, tfontptr->font_mag);
    } else {
	direct = PXLpath;
	found = FALSE;
        do { 
	    tcp = index(direct, ':');
	    if (tcp == NULL) tcp = strlen(direct) + direct;
	    strncpy(curarea, direct, tcp-direct);
	    tcp1 = curarea + (tcp - direct);
	    *tcp1++ = '/';
	    *tcp1++ = '\0';

	    sprintf(tfontptr->name, "%s%s.%dpxl", curarea, tfontptr->n, tfontptr->font_mag);
	    found = (access(tfontptr->name, READ) == 0);
	    if (*tcp) direct = tcp + 1; else direct = tcp;
	} while ( !found && *direct != '\0');
    }
    if (tfontptr != pfontptr)
	OpenFontFile();
    if ((t = NoSignExtend(pxlfp, 4)) != PXLID)
	Fatal("PXL ID = %d, can only process PXL ID = %d files",
	      t, PXLID);
    fseek(pxlfp, -20, 2);
    t = NoSignExtend(pxlfp, 4);
    if ((tfontptr->c != 0) && (t != 0) && (tfontptr->c != t))
	Warning("font = \"%s\",\n-->font checksum = %d,\n-->dvi checksum = %d",
	        tfontptr->name, tfontptr->c, t);
    tfontptr->magnification = NoSignExtend(pxlfp, 4);
    tfontptr->designsize = NoSignExtend(pxlfp, 4);

    fseek(pxlfp, NoSignExtend(pxlfp, 4) * 4, 0);

    for (i = FIRSTPXLCHAR; i <= LASTPXLCHAR; i++) {
	tcharptr = &(tfontptr->ch[i]);
	tcharptr->width = NoSignExtend(pxlfp, 2);
	tcharptr->height = NoSignExtend(pxlfp, 2);
	tcharptr->xOffset= SignExtend(pxlfp, 2);
	tcharptr->yOffset = SignExtend(pxlfp, 2);
	tcharptr->where.isloaded = FALSE;
	tcharptr->where.address.fileOffset = NoSignExtend(pxlfp, 4) * 4;
	tcharptr->tfmw = ((float)NoSignExtend(pxlfp, 4)*(float)tfontptr->s) /
	    (float)(1<<20);
    }
}


/**********************************************************************/
/****************************  ReadInt  *******************************/
/**********************************************************************/

/*  this routine is used to read in an integer from the stdin stream.
	This routine is necessary since the terminal is running in
	CBREAK mode and therefore will not do editing of the input
	stream for one. */

ReadInt()
{
   int value = 0;
   int byte;

   while ((byte = getchar()) != /* tcb.t_brkc */ 10)
	{ if (byte >= 48 /* "0" */ && byte <= 57 /* "9" */)
	      value = value * 10 + byte - 48;
	  if (byte == tty.sg_erase) value = value / 10;
        }
   return(value);
}
		  

/*-->ReadPostAmble*/
/**********************************************************************/
/**************************  ReadPostAmble  ***************************/
/**********************************************************************/

void
ReadPostAmble()
/***********************************************************************
    This  routine  is  used  to  read  in  the  postamble  values.    It
    initializes the magnification and checks  the stack height prior  to
    starting printing the document.
***********************************************************************/
{
    FindPostAmblePtr (&postambleptr);
    if (NoSignExtend(dvifp, 1) != POST)
	Fatal ("POST missing at head of postamble");
#ifdef DEBUG
    if (Debug) fprintf (stderr, "got POST command\n");
#endif
    /*	  lastpageptr = */ NoSignExtend(dvifp, 4);
    num = NoSignExtend(dvifp, 4);
    den = NoSignExtend(dvifp, 4);
    mag = NoSignExtend(dvifp, 4);
    hconv = DoConv(num, den, hconvRESOLUTION);
    vconv = DoConv(num, den, vconvRESOLUTION);

    NoSignExtend(dvifp, 4);	/* height-plus-depth of tallest page */
    NoSignExtend(dvifp, 4);	/* width of widest page */
    if (NoSignExtend(dvifp, 2) >= STACKSIZE)
	Fatal ("Stack size is too small");
    NoSignExtend(dvifp, 2);	/* this reads the number of pages in */
    /* the DVI file */
#ifdef DEBUG
    if (Debug) fprintf (stderr, "now reading font defs");
#endif
    GetFontDef ();
}


/*-->ResetTerm*/
/**********************************************************************/
/****************************  ResetTerm  *****************************/
/**********************************************************************/

void
ResetTerm()   /* Reset Terminal */
{
    if (G_interactive)
	ioctl(0, TIOCSETP, &G_intty);  /* restore modes */
}

/*-->SetChar*/
/**********************************************************************/
/*****************************  SetChar  ******************************/
/**********************************************************************/

int buffer[8];

LoadAChar(ptr)
register struct char_entry *ptr;
{
    register struct pixrect *pr;
    register int nshorts, i, col, nints;
    register short *dp, *sp;
    if (ptr->where.address.fileOffset == 0) {
	ptr->where.address.pixrectptr = NULL;
	return;
    }
    OpenFontFile();
    fseek(pxlfp, ptr->where.address.fileOffset, 0);
    nshorts = (ptr->width + 15) >> 4;
    pr = mem_create(ptr->width, ptr->height, 1);
    nints = (nshorts + 1) >> 1;
    dp = ((struct mpr_data *)pr->pr_data)->md_image;
    for (col = 0; col < ptr->height; col++) {
	fread(buffer, 4, nints, pxlfp);
	sp = (short *) &buffer[0];
	for (i = nshorts; i > 0; i--) *dp++ = *sp++;
    }
    ptr->where.address.pixrectptr = pr;
    ptr->where.isloaded = TRUE;
}

void
SetChar(c, command)
int c, command;
{
    register struct char_entry *ptr;  /* temporary char_entry pointer */
    int k;

    ptr = &(fontptr->ch[c]);
    hh = PixRound(h, hconv);
    vv = PixRound(v, vconv);
    if (!ptr->where.isloaded) LoadAChar(ptr);
    pw_rop(pw, hh-ptr->xOffset-xscreen, vv-ptr->yOffset-yscreen, ptr->width, ptr->height, PIX_SRC | PIX_DST, ptr->where.address.pixrectptr, 0, 0); /*yl*/
    if (command <= SET4) h += ptr->tfmw;
}


/*-->SetFntNum*/
/**********************************************************************/
/****************************  SetFntNum  *****************************/
/**********************************************************************/

void
SetFntNum(k)
int k;

/*  this routine is used to specify the font to be used in printing future
    characters */

{
    fontptr = hfontptr;
    while ((fontptr!=NULL) && (fontptr->k!=k))
	fontptr = fontptr->next;
    if (fontptr == NULL)
	Fatal("font %d undefined", k);
}


/*-->SetRule*/
/**********************************************************************/
/*****************************  SetRule  ******************************/
/**********************************************************************/

void
SetRule(a, b, Set)
int a, b;
BOOLEAN Set;

{	    /*	 this routine will draw a rule on the screen */
    int ehh, evv;
    hh = PixRound(h, hconv);
    vv = PixRound(v-a, vconv);
    ehh = PixRound(h + b, hconv);
    evv = PixRound(v, vconv);
    if (hh == ehh) ehh++;
    if (vv == evv) vv--;
    if ((a > 0) && (b > 0))
    	pw_rop(pw, hh-xscreen, vv-yscreen, ehh-hh, evv-vv, PIX_SET, NULL, 0, 0); /*yl*/
    if (Set) {
	h += b;
/*        v += a; */
    }
}

/*-->SkipFontDef*/
/**********************************************************************/
/****************************  SkipFontDef  ***************************/
/**********************************************************************/

void
SkipFontDef(k)
int k;
{
    int a, l;
    char n[STRSIZE];

    NoSignExtend(dvifp, 4);
    NoSignExtend(dvifp, 4);
    NoSignExtend(dvifp, 4);
    a = NoSignExtend(dvifp, 1);
    l = NoSignExtend(dvifp, 1);
    GetBytes(dvifp, n, a+l);
}


/*-->Warning*/
/**********************************************************************/
/*****************************  Warning  ******************************/
/**********************************************************************/

void
Warning(fmt, args)  /* issue a warning */
char *fmt;	/* format   */
char *args;	/* arguments */
{
    if (G_logging == 0)
    {
	G_logfp=fopen(G_Logname,"w+");
	G_logging = 1;
	if (G_logfp == NULL) G_logging = -1;
    }

    G_errenc = TRUE;
    if (G_logging == 1)
    {
	_doprnt(fmt, &args, G_logfp);
	fprintf(G_logfp,"\n");
    }
}
@//E*O*F texview.c//
chmod u=r,g=r,o=r texview.c
 
echo x - texview.n
sed 's/^@//' > "texview.n" <<'@//E*O*F texview.n//'
@.TH TEXVIEW 1
@.SH NAME
texview \- TeX previewer for suntools
@.SH SYNOPSIS
@.B texview
[
@.B \-p
] [
@.B \-l
] [ \+n
] dvifile[.dvi]
@.SH DESCRIPTION
The
@.I texview
program is an enhanced version of the "dvisun" command designed to run under
suntools on SUN3s.
The
@.I texview
program displays a
@.I .dvi
file on a SUN3.
After each page of the
output is displayed the program will wait for a response from
the user.  Type
@.B U
, 
@.B D
, 
@.B L
, 
@.B R
or
@.B k
, 
@.B j
, 
@.B h
, 
@.B l
in the
@.B PARENT
window
to move the displayed
page of the document up, down, left or right.  This provides a facility to 
examine the entire page if the page is larger than the screen.
A 
@.B p
will display
the previous page.  <RETURN> goes to the next page.  When a new page is 
displayed the top left corner of the page will be displayed in the top
left corner of the screen.
Multiple pages may be skipped forwards or backwards by entering 
@.B \+n <RETURN>
or
@.B \-n <RETURN>
to skip n pages.  The program will not skip past the begining or the end
of the document.  Prior to display of the first page in the
document several pages may be skipped by including a
@.B \+n
option on the command line.  This will cause n pages to be skipped
prior to displaying the first page on the screen.
Typing 
@.B q
will cause
the program to exit.
@.PP
The 'm' and 'M' keys provide the user with a pager (as 'more').
Pressing these keys is equivalent to a sequence of ' ' and 'k'.
@.PP
After the document is processed a list of errors is printed
on standard error.  This error listing is also saved in
the file dvifile.loh.  The creation of the log file may be prevented
by using the
@.B \-l
option when invoking the program.
@.PP
This is a `bare bones' DVI-to-Sun program.  Minimal error checking
is done.  For more complete error checking
@.IR dvitype(1)
should be run.
@.PP
Ocasionally there may be insufficent memory to hold the information about
all of the fonts in the system.  Portions of the document may be previewed
by including the
@.B \-p
option on the command line.  This will prevent the preloading of all fonts
into the system and instead use demand loading of the font tables.
@.PP
Some new keys have been added to the original dvisun program
to make texview perform like any other Berkeley
program. The lowercase keys
l, d, u, r
of the original dvisun have been replaced by
h, j, k, l
but it is still possible to use the old keys as uppercases (L, D, U, R).
@.PP
The
@.B b
key is equivalent to
@.B p
and
@.B n
is equivalent to <RETURN>.
@.SH FILES
@.TP 2.5i
*.dvi
TeX DeVice Independent output file
@.TP
/usr/lib/dvifont/*.*pxl
TeX default font rasters
@.br
@.SH "SEE ALSO"
dvitype(1),
dvisun(1),
tex(1)
@.SH BUGS
When using fonts with decent resolution only part of a page appears
on the screen.
@.PP
For unexplained reasons, texview logs you out when it encounters a request
for a non-existant PXL file.
@.PP
The number of fonts files is limited (see in the sources for current limitation).
@.PP
A limitation of the maximum number of opened files for a process makes it
sometimes impossible to close or move the texview window. It may also lead to a
core dump. So be careful when TeXviewing a text with too many fonts.
@.SH AUTHOR
Mark Senn wrote the early versions of this program for the BBN BitGraph.
Stephan Bechtolsheim, Bob Brown, Richard Furuta, James Schaad
and Robert Wells improved it.  Norm Hutchinson ported the program to the Sun.
Yves Ledru ported it under suntools.

@//E*O*F texview.n//
chmod u=r,g=r,o= texview.n
 
exit 0
/*---------end of second part (remove this line from the shar file)---------*/
