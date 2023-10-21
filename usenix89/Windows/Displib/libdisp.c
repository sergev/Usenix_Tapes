
/*
   DISPLIB - a display library for CRT-based applications.

   This version is for 4.[123]bsd UNIXs using termcap.

   Donated into the public domain for any use whatever by anyone.
   Please retain this header.
  
   This display library is a seriously modified version of a
   display library posted to USENET several years ago. Many thanks.
  
   * **************************************************************** *
   * Gary K. Sloane/Computer Sciences Corporation/San Diego, CA 92110 *
   * DDD: (619) 225-8401                                              *
   * MILNET: sloane@nosc.ARPA                                         *
   * UUCP: [ihnp4,akgua,decvax,dcdwest,ucbvax]!sdcsvax!noscvax!sloane *
   * **************************************************************** *
*/  

#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <ctype.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>

#include "displib.h"

#define makelower(c) isupper(c)?tolower(c):(c)

/* pointer to environment for getenv() */
extern char **environ;

/* defined here, referenced as an extern elsewhere */
TERMDEF termctrl;

/************************************************************************/
/* routine to get terminal capabilities, validate & initialize terminal */
/************************************************************************/
/*    *** must be called before using any other displib routines ***    */
/************************************************************************/
terminit()
{
struct sgttyb ttystate;
char tbuf[1024], *ap, *pcptr, *getenv(), *term;

/* this must be run from a terminal... */
if(isatty(fileno(stdout)) == 0)
   {
   fprintf(stderr, "** This program must be run from a tty**\n");
   exit(0);
   }

/* read the current tty state from the driver */
if(ioctl(1, (int)TIOCGETP, (char *)&ttystate) != 0)
   {
   fprintf(stderr, "** Error: Cannot get tty state **\n");;
   exit(0);
   }

/* set the tty speed in the external variable ospeed for tputs()  */
/* an interesting note: UNIX only knows the speed of the device   */
/* that's actually connected to the driver. So if you have modems */
/* or some other link (like a data concentrator) in between, UNIX */
/* will pad for the speed of the device connected to the port,    */
/* NOT the speed of the terminal (which is what you want...)      */
ospeed = ttystate.sg_ospeed; 

/* clear the XTABS bit in the sg_flags     */
/* so that tab chars (^I) are NOT expanded */
/* since some terminals use them for other */
/* than tabs. (see tgoto.c in termcap)     */
ttystate.sg_flags &= ~XTABS;

/* write the sg_flags back to the tty */
if(ioctl(1, (int)TIOCSETP, (char *)&ttystate) != 0)
   {
   fprintf(stderr, "** Cannot set tty state **\n");;
   exit(0);
   }

/* delay long enough for TIOCSETP to do it's thing */
sleep(2);

/* get terminal type and termcap entry */
term = getenv("TERM");
if(term == (char *)NULL)
    {
    fprintf(stderr, "** No TERM variable in environment **\n");
    exit(0);
    }
switch(tgetent(tbuf, term))
    {
    case -1:
        {
        fprintf(stderr, "** Cannot open termcap data file **\n");
        exit(0);
        }
    case  0:
        {
        fprintf(stderr, "** %s: Unknown terminal type **\n", term);
        exit(0);
        }
    }

/* make sure it's not a hard-copy terminal */
if(tgetnum("hc") != -1)
    {
    fprintf(stderr, "** You cannot run this program from a hard copy terminal **\n");
    exit(0);
    }

/* get terminal attributes and capabilities */
ap = termctrl.tcapbuf;
termctrl.BL = tgetstr("bl", &ap);
termctrl.CD = tgetstr("cd", &ap);
termctrl.CE = tgetstr("ce", &ap);
termctrl.CL = tgetstr("cl", &ap);
termctrl.CM = tgetstr("cm", &ap);
termctrl.IS = tgetstr("is", &ap);
termctrl.SE = tgetstr("se", &ap);
termctrl.SO = tgetstr("so", &ap);
termctrl.TE = tgetstr("te", &ap);
termctrl.TI = tgetstr("ti", &ap);
termctrl.scrlen = tgetnum("li");
termctrl.scrwid = tgetnum("co");

/* make sure terminal has required capabilities */
if( (termctrl.CM == 0) || (termctrl.CL == 0) )
    {
    fprintf(stderr, "** Terminal must have clear and cursor movement capabilities **\n");
    exit(0);
    }
if( (termctrl.scrlen <= 0) || (termctrl.scrwid <= 0) )
    {
    fprintf(stderr, "** Termcap entry must show screen size **\n");
    exit(0);
    }

/* define the upline and alternate backspace string for tgoto() */
UP = tgetstr("up", &ap);
if(tgetflag("bs") == 0)
   BC = tgetstr("bc", &ap);
else
   BC = (char *)NULL;

/* get the (correct) pad character for tputs() */
/* some networks may discard NULs...           */
if((pcptr = tgetstr("pc", &ap)) != (char *)NULL)
   PC = *pcptr;   /* specified in termcap */
else
   PC = '\000';   /* use a NUL if not specified */

/* send initialization string if defined */
if(termctrl.IS != 0)
    putpad(termctrl.IS);
}

/**************************************************************************/
/* Check an Input String to see if it is a Valid Abbreviation of a Target */
/* Ignoring Case  - returns 0 if it is an ok abbreviation, -1 if not      */
/**************************************************************************/

int abb(input, target)
char *input, *target;
{
char *malloc();
char *tarptr = target;
char *tptr, *tptr1;
char *inptr = malloc((unsigned int)strlen(target)+1);

/* reject <RET> - user *must* enter something... */
if( (strlen(input) == 0) || (strlen(input) > strlen(target)) )
   return(-1);

/* convert input to lower case for the comparison */
for(tptr=input, tptr1=inptr; *tptr!='\0'; ++tptr, ++tptr1)
   *tptr1= makelower(*tptr);
*tptr1 = '\0';

/* we allow one character abbreviations in this program */
if(*inptr == *tarptr)
   return(abbrev(inptr, tarptr));
else
   return(-1);
}

static abbrev(inptr, tarptr)
char *inptr, *tarptr;
{
for(++inptr, ++tarptr; *tarptr != '\0'; ++inptr, ++tarptr)
   {
   if(*inptr == '\0')
    return(0); 
   if(*inptr != *tarptr)
    return(-1);
   }
if(*inptr ==  '\0')
   return(0);
else
   return(-1);
}

/*************/
/* Ring Bell */
/*************/
bell()
{
/* If bell is defined, put it out */
if(termctrl.BL != 0)
   putpad(termctrl.BL);
/* If not, put out ASCII ctrl-G */
else
   putchar('\007');
}

/***************************/
/* routine to clear screen */
/***************************/ 

clear()
{
putpad(termctrl.CL);
return;
}

/**************************************/
/* routine to clear to end of display */
/**************************************/
cleod()
{
int lin, col;

/* if terminal has CD, do it */
if (termctrl.CD != 0)
    putpad(termctrl.CD);
/* if it doesn't have CD, fake it */
else
    {
    /* save cursor position */
    lin = termctrl.curline;
    col = termctrl.curcol;
    /* clear rest of current line */
    cleol();
    /* clear rest of screen */
    while (termctrl.curline < termctrl.scrlen)
        {
        gotoxy(0, ++termctrl.curline);
        cleol();
        }
    /* return to correct position */
    gotoxy(col,lin);
    }
return;
}

/***********************************/ 
/* routine to clear to end of line */
/***********************************/
cleol()
{
int i;

/* if terminal has CE, do it */
if (termctrl.CE != 0)
    putpad(termctrl.CE);
/* if terminal doesn't have CE, fake it */ 
else
    {
    for(i=termctrl.curcol; i<termctrl.scrwid; i++)
        putchar(' ');
    }
gotoxy(termctrl.curcol,termctrl.curline);
return;
}

/**************************/
/* Clear the Message Area */
/**************************/
clmsg()
{
int lin, col;

lin = termctrl.curline;
col = termctrl.curcol;
gotoxy(0,22);
cleod();
gotoxy(col, lin);
return;
}

/*******************************************/
/* clear a field and reposition the cursor */
/*******************************************/
clrfield(fld_length)
int fld_length;
{
int i;

/* write out as many blanks as required */
for (i = 0; i < fld_length; ++i)
   (void)fputc(' ', stdout);

/* move cursor back to start of field */
gotoxy(termctrl.curcol, termctrl.curline);
}

/********************************************************************/
/* routine to clear display, except system title and message border */
/********************************************************************/
clscr()
{
gotoxy(0,3);
cleod();
gotoxy(0,21);
printf("--------------------------------------------------------------------------------");
gotoxy(0,3);
return;
}

/******************************************/
/* routine to move cursor to position x,y */
/******************************************/
gotoxy(col,lin)
int col, lin;
{
char *cmstr;

cmstr = tgoto(termctrl.CM, col, lin);
/* if special mode for CM, start it */
if(termctrl.TI != 0)
    putpad(termctrl.TI);
/* do the CM */
putpad(cmstr);
/* if special mode for CM, end it */
if(termctrl.TE != 0)
    putpad(termctrl.TE);
/* set current line and column */
termctrl.curline = lin;
termctrl.curcol = col;
return;
}

/*********************************/
/* routine to move cursor home   */
/* uses curxy so HO is not req'd */
/*********************************/
home()
{
gotoxy(0,0);
return;
}

/**********************************************/
/* Routine to display blank screen with title */
/**********************************************/ 
initscrn(title, sysname)
char *sysname, *title;
{
int i, j;
char *bptr;
char blank[40];

/* compute padding for centering */
j = (80-(strlen(sysname))) / 2;
for(bptr=blank, i=1; i<=j; bptr++, i++)
   *bptr = ' ';
*bptr = '\0';

/* display system name and version */
home();
clear();
inverse();
printf("%s%s%s", blank, sysname, blank);
if((strlen(sysname)) % 2)
   printf(" ");
normal();

/* print the title on the screen */
gotoxy(((80 - strlen(title))/2)-2,1);
printf("%s",title);
gotoxy(0,2);
printf("--------------------------------------------------------------------------------");

/* print the help box line */
gotoxy(0,21);
printf("--------------------------------------------------------------------------------");
return;
}

/***********************/
/* Enter Standout Mode */
/***********************/
inverse()
{
if((termctrl.SE != 0) && (termctrl.SO != 0))
   putpad(termctrl.SO);
return;
}

/**************************************************************************/
/* Display a Message in the Message area and Sleep for the Specified Time */
/* If the time is 0, leave the message, else erase it when the time is up */
/**************************************************************************/
/*VARARGS2*/
msg(msg, beep, time, a1, a2, a3, a4, a5, a6, a7, a8)
int beep;
unsigned time;
char *msg;
{
if(beep == BEEP)
   {
   bell();
   (void)fflush(stdout);
   }
tmsg(1);
printf(msg, a1, a2, a3, a4, a5, a6, a7, a8);
(void)fflush(stdout);
if(time != 0)
   {
   sleep(time);
   clmsg();
   }
tmsg(0);
return;
}

/**********************/
/* Exit Standout Mode */
/**********************/
normal()
{
if((termctrl.SE != 0) && (termctrl.SO != 0))
   putpad(termctrl.SE);
return;
}

/**************************************************************************/
/* routines to send control sequences to terminal with appropriate padding */
/**************************************************************************/
static outch(c)
register char c;
{
putchar(c);
return;
}

putpad(str)
register char *str;
{
if(str)
    tputs(str, 0, outch);
return;
}

/************************************************************************/
/* Get terminal out of CBREAK mode and restore ECHO and restore signals */
/************************************************************************/
termreset()
{
/* Unblock Signals */
(void)signal(SIGINT, SIG_DFL);
(void)signal(SIGQUIT, SIG_DFL);
(void)signal(SIGTERM, SIG_DFL);
(void)signal(SIGTSTP, SIG_DFL);
(void)signal(SIGALRM, SIG_DFL);
(void)signal(SIGTTIN, SIG_DFL);
(void)signal(SIGTTOU, SIG_DFL);

/* turn on cooked mode with echo and case sensitivity */
ttyset("cooked");
ttyset("echo");
}

/***************************************************************/
/* Put Terminal into CBREAK mode with NOECHO and block SIGNALS */
/***************************************************************/
termset(prog)
char *prog;
{
int tpgrp;

/* Don't let this program run in background */
if( (ioctl(1,(int)TIOCGPGRP,(char *)&tpgrp) != 0) || (tpgrp != getpgrp(0)) )
   {
   printf("%s cannot be run in background.\n", prog);
   exit(0);
   }

/* Block Signals */
(void)signal(SIGINT, SIG_IGN);
(void)signal(SIGQUIT, SIG_IGN);
(void)signal(SIGTERM, SIG_IGN);
(void)signal(SIGTSTP, SIG_IGN);
(void)signal(SIGALRM, SIG_IGN);
(void)signal(SIGTTIN, SIG_IGN);
(void)signal(SIGTTOU, SIG_IGN);

/* go into cbreak mode with no echo */
ttyset("cbreak");
ttyset("-echo");
return;
}

/***********************/
/* Toggle Message Mode */
/***********************/
tmsg(flag)
int flag;
{
static int col,lin;

/* branch on start or end */
switch(flag)
   {
   /* restore cursor to wher it was prior to tmsg([12]) */
   case 0:  termctrl.curline = lin;
            termctrl.curcol = col;
            gotoxy(termctrl.curcol,termctrl.curline);
            break;
   /* go to 1st line of help box */
   case 1:  lin = termctrl.curline;
            col = termctrl.curcol;
            gotoxy(0,22);
            cleod();
            break;
   /* go to 2nd line of help box (must tmsg(1) FIRST) */
   case 2:  gotoxy(0,23);
            cleod();
            break;
   /* should read: ERROR: bad programmer */
   default: printf("***ERROR: Bad argument to tmsg: %d...",flag);
            exit(1);
   }
return;
}

/**************************************/
/* Get todays date in mm/dd/yy format */
/**************************************/
today(date)
char *date;
{
struct tm *localtime(),
          *dt;
struct timeval tp;

/* get time in seconds since January 1, 1970 */
(void)gettimeofday(&tp,(struct timezone *)0);

/* get date and time and place in structure 'dt' */
dt = localtime((long *)&tp.tv_sec);

/* build the date in an ascii string */
dt->tm_mon += 1; /* months go 0-11 */
date[0] = dt->tm_mon / 10 + '0';
date[1] = dt->tm_mon % 10 + '0';
date[2] = '/';
date[3] = dt->tm_mday / 10 + '0';
date[4] = dt->tm_mday % 10 + '0';
date[5] = '/';
date[6] = dt->tm_year / 10 + '0';
date[7] = dt->tm_year % 10 + '0';
date[8] = '\0';
}

/*********************************************************/
/* set tty modes ala stty(1) - arguments are:            */
/*       raw      - turn on raw mode                     */
/*       cbreak   - turn on cbreak mode                  */
/*       cooked   - turn off raw and cbreak mode         */
/*       echo     - cause echo (full duplex)             */
/*       -echo    - turn off echo (half duplex)          */
/*       lcase    - map upper case into lower case       */
/*       -lcase   - don't map upper case to lower case   */
/*********************************************************/
ttyset(cmd)
char *cmd;
{
struct sgttyb ttystate;

/* read the current status from the driver */
if(ioctl(1, (int)TIOCGETP, (char *)&ttystate) != 0)
   {
   printf("***ERROR: ioctl TIOCGETP failure in function ttyset()...\n");
   exit(0);
   }

/* set raw mode */
if(strcmp(cmd, "raw") == 0)
   {
   ttystate.sg_flags &= ~CBREAK;
   ttystate.sg_flags |= RAW;
   }

/* set cbreak mode */
else if(strcmp(cmd, "cbreak") == 0)
   {
   ttystate.sg_flags &= ~RAW;
   ttystate.sg_flags |= CBREAK;
   }

/* set cooked mode */
else if(strcmp(cmd, "cooked") == 0)
   ttystate.sg_flags &= ~(RAW+CBREAK);

/* set echo on */
else if(strcmp(cmd, "echo") == 0)
   ttystate.sg_flags |= ECHO;

/* set echo off */
else if(strcmp(cmd, "-echo") == 0)
   ttystate.sg_flags &= ~ECHO;

/* map upper to lower case on input */
else if(strcmp(cmd, "lcase") == 0)
   ttystate.sg_flags |= LCASE;

/* differentiate between upper and lower case */
else if(strcmp(cmd, "-lcase") == 0)
   ttystate.sg_flags &= ~LCASE;

/* bad argument given */
else
   {
   printf("***ERROR: bad argument to ttyset()...\n");
   exit(0);
   }

/* write out the changes to the driver */
if(ioctl(1, (int)TIOCSETP, (char *)&ttystate) != 0)
   {
   printf("***ERROR: ioctl TIOCSETP failure in function ttyset()...\n");
   exit(0);
   }
return;
}

/**************************/
/* Enter or Edit a String */
/**************************/
/*VARARGS4*/
edits(string, len, flag, pgm, a1, a2, a3, a4, a5)
char *pgm, *string;
int flag, len;
{
register int cnt;
int initline, initcol, dline, dcol, i;
int mode = FREAD;
char *sptr, c;

/* save cursor positions and string for ctrl-u */
initline = termctrl.curline;
initcol = termctrl.curcol;

/* highlight the ':' if requested */
if(flag == COLON)
   {
   gotoxy(initcol-2, initline);
   inverse();
   printf(":");
   normal();
   }

/* highlight the entire field if requested */
if(flag == INVERSE)
   {
   inverse();
   printf("%s", string);
   for(i=strlen(string); i<len; i++)
      putchar(' ');
   }

/* set up for enter or edit */
cnt = strlen(string);
sptr = string + cnt;
gotoxy(initcol+cnt,initline);

/* flush the type ahead buffer */
(void)ioctl(0, (int)TIOCFLUSH, (char *)&mode);

/* go into raw mode */
ttyset("raw");

/* accept characters and process them */
for(;;)
   {
   switch(c = getc(stdin))
      {
      /* DEL entered - abort or continue */
      case '\177': dline = termctrl.curline;
                dcol = termctrl.curcol;
                bell();
                gotoxy(25,21);
                inverse();
                printf(" Restart(y/n)?   ");
                gotoxy(40,21);
                c = getc(stdin);
                putchar(c);
                normal();
                if(c != 'y')
                   {
                   gotoxy(25,21);
                   printf("------------------");
                   gotoxy(dcol, dline);
                   if(flag == INVERSE)
                     inverse();
                   break;
                   }
                /* go into cbreak mode */
                ttyset("cbreak");
                /* start the program over */
                if(*pgm != '\0')
                  reload(pgm, a1, a2, a3, a4, a5);
                termreset();
                exit(0);
                

      /* ctrl-u entered */
      case '\025': gotoxy(initcol,initline);
                 for(i=0; i<strlen(string); i++)
                    printf(" "); 
                 *string = '\0';
                 cnt = 0;
                 sptr = string;
                 gotoxy(termctrl.curcol,termctrl.curline);
                 break;
        
      /* carriage return entered - return the string */
      case '\015': *sptr = '\0';
                 /* un-highlight the ':' if it was requested */
                 if(flag == COLON)
                    {
                    gotoxy(termctrl.curcol-cnt-2, termctrl.curline);
                    normal();
                    printf(":");
                    }

                 /* un-highlight the field if it was requested */
                 if(flag == INVERSE)
                  {
                  gotoxy(initcol, initline);
                  normal();
                  printf("%s", string);
                  for(i=strlen(string); i<len; i++)
                     putchar(' ');
                  }

                 /* set cbreak mode & return to calling routine */
                 ttyset("cbreak");
                 return;
           
      /* backspace entered - backspace over previous character */
      /* if no previous character, ignore the backspace        */
      case '\010': if(sptr != string)
                      {
                      printf("\010 \010");
                      --termctrl.curcol;
                      --sptr;
                      --cnt;
                      }
                   else
                      bell();
                   break;
      /* check character for printability - if ok, echo it to screen   */
      /* and spool it onto string - do not allow entry past length     */
      /* and do not allow colons as they are used for field delimiters */
      default:     if((isprint(c) == 0) || (cnt >= len) || (c == ':'))
                      bell();
                   else
                      {
                      *sptr++ = c;
                      putchar(c);
                      ++termctrl.curcol;
                      ++cnt;
                      }
                   break;
      }
   }
}

/*******************************************/
/* Enter or Edit a Date in mm/dd/yy Format */
/*******************************************/
/*VARARGS7*/
editdate(edate, flag, empty, oldate, date, pgm, a1, a2, a3, a4, a5)
char *edate, *date, *pgm;
int flag, empty, oldate;
{
int cnt, cd, chcnt=0, scnt=0, erflg=0;
char *sptr, c;
int initline, initcol, dline, dcol, i, j;
int mode = FREAD;

/* save cursor positions and string for ctrl-u */
initline = termctrl.curline;
initcol = termctrl.curcol;

/* highlight the ':' if requested */
if(flag == COLON)
   {
   gotoxy(initcol-2, initline);
   inverse();
   printf(":");
   normal();
   }

/* highlight the entire field if requested */
if(flag == INVERSE)
   {
   inverse();
   printf("%s", edate);
   for(j=strlen(edate); j<8; j++)
      putchar(' ');
   }

/* set up for enter or edit */
cnt = strlen(edate);
for(sptr = edate; *sptr != '\0'; sptr++)
        if(*sptr == '/')
                scnt++;
cnt = strlen(edate);
sptr = edate + cnt;
chcnt = cntdate(edate, sptr);
gotoxy(initcol+cnt,initline);

/* flush the type ahead buffer */
if(ioctl(0, (int)TIOCFLUSH, (char *)&mode) != 0)
  {
  printf("*** ERROR: Bad call to ioctl in editdate()... ***");
  exit(0);
  }

/* go into raw mode */
ttyset("raw");

/* accept characters and process them */
for(;;)
   {
   c=getc(stdin);
   if(erflg)
      {
      erflg = 0;
      clmsg();
      }
   switch(c)
      {
      /* Delete entered - abort the program? */
      case '\177':
         dline = termctrl.curline;
         dcol = termctrl.curcol;
         bell();
         gotoxy(25,21);
         inverse();
         printf(" Restart(y/n)?   ");
         gotoxy(40,21);
         c = getc(stdin);
         putchar(c);
         normal();
         if(c != 'y')
            {
            gotoxy(25,21);
            printf("------------------");
            gotoxy(dcol, dline);
            if(flag == INVERSE)
               inverse();
            break;
            }

         /* go into cbreak mode and restart the program */
         ttyset("cbreak");
         if(*pgm != '\0')
            reload(pgm, a1, a2, a3, a4, a5);
         termreset();
         exit(0);

      /* ctrl-u entered */
      case '\025': gotoxy(initcol,initline);
         for(i=0; i<strlen(edate); i++)
            printf(" "); 
         *edate = '\0';
         cnt = 0;
         chcnt = 0;
         scnt = 0;
         sptr = edate;
         gotoxy(termctrl.curcol,termctrl.curline);
         break;

      /* carriage return entered - return the string unless last character  */
      /* is a '/' or unless an invalid date is entered. Month must be 1-12, */
      /* day must be less than or equal to max for month entered, date must */
      /* be after current date. Two slashes must have been entered          */
      case '\015':
         if((empty == 1) && (cnt == 0))
            {
            /* un-highlight the ':' if it was requested */
            if(flag == COLON)
               {
               gotoxy(termctrl.curcol-cnt-2, termctrl.curline);
               normal();
               printf(":");
               }

            /* un-highlight the field if it was requested */
            if(flag == INVERSE)
               {
               gotoxy(initcol, initline);
               normal();
               printf("%s", edate);
               for(j=strlen(edate); j<8; j++)
                  putchar(' ');
               }

            /* go into cbreak mode & return to calling routine */
            *sptr = '\0';
            ttyset("cbreak");
            return;
            }

         /* last character in date can't be a '/' */
         if( *(sptr-1) == '/')
            {
            msg("Format to enter a date is MM/DD/YY...",BEEP,0);
            erflg++;
            break;
            }
                
         /* 2 slashes must have been entered */
         if (scnt < 2)
            {
            msg("Format to enter a date is MM/DD/YY...",BEEP,0);
            erflg++;
            break;
            }

         /* put the NULL at the end of the string */
         *sptr = '\0';

         /* is date valid? */
         cd = chkdate(edate, date, oldate);

         /* date is valid */
         if (cd == 1)
             {
             /* un-highlight the ':' if it was requested */
             if(flag == COLON)
               {
               gotoxy(termctrl.curcol-cnt-2, termctrl.curline);
               normal();
               printf(":");
               }

             /* un-highlight the field if it was requested */
             if(flag == INVERSE)
               {
               gotoxy(initcol, initline);
               normal();
               printf("%s", edate);
               for(j=strlen(edate); j<8; j++)
                  putchar(' ');
               }

             /* go into cbreak mode with no echo & return to calling routine */
             ttyset("cbreak");
             return;
             }

         /* month is not 1 thru 12 */
         else if (cd == 2)
            {
            msg("Month must be 1 thru 12 - try again...",BEEP,2);
            break;
            }

         /* day not in month entered */
         else if (cd == 3)
            {
            msg("Day entered does not exist in month entered - try again...",BEEP,2);
            break;
            }

         /* date not after today's date */
         else 
            {
            msg("Date must be after today's date - try again...",BEEP,2);
            break;
            }

      /* backspace entered - backspace over previous character */
      /* if no previous character, ignore the backspace        */
      case '\010':
         /* only backspace if there is something to backspace over */
         if(sptr != edate)
            {
            /* do the backspace */
            printf("\010 \010");
            --termctrl.curcol;
            --sptr;
            --cnt;
            --chcnt;
            if(erflg)
               {
               erflg = 0;
               clmsg();
               }
            /* if a slash is being backspaced over, adjust the counts */
            if( *sptr == '/')
               {
               /* reduce field count (# of '/' in date) */
               --scnt;
               chcnt = cntdate(edate, sptr);
               }
            }
         /* No backspace is allowed, so... */
         else
            bell();
         break;

      /* make sure character is a digit - if ok, echo it to screen */
      /* and spool it onto field - do not allow entry past length  */
      /* and do not allow blanks to be entered                     */
      default:
         if( (c != '/') && (isdigit(c) == 0) )
            {
            msg("Format to enter a date is MM/DD/YY...", BEEP,0);
            erflg++;
            break;
            }
         /* is it a slash? */
         if(c == '/')
            {
            /* first char can't be a '/' */
            if(sptr == edate)
               {
               msg("Format to enter a date is MM/DD/YY...", BEEP,0);
               erflg++;
               break;
               }
            /* can't have 2 slashes next to each other */
            if( *(sptr-1) == '/')
               {
               msg("Format to enter a date is MM/DD/YY...", BEEP,0);
               erflg++;
               break;
               }
            /* can't have more than 2 slashes in date */
            if( (scnt+1) > 2)
               {
               msg("Format to enter a date is MM/DD/YY...", BEEP,0);
               erflg++;
               break;
               }
            else
               ++scnt;
               chcnt = -1;
            }
         /* If it's not a '/', is the month, day, or year too long? */
         else
            {
            if(chcnt > 1)
               {
               msg("Format to enter a date is MM/DD/YY...", BEEP,0);
               erflg++;
               break;
               }
            }
         /* everything's ok, put it on the date and bump the ctrs */
         ++chcnt;
         *sptr++ = c;
         putchar(c);
         ++termctrl.curcol;
         ++cnt;
         break;
         }
   }
}

static cntdate(edate,sptr)
char *edate, *sptr;
{
char *tptr;
int chcnt;

/* find out how many characters in previous field */
tptr = sptr;
chcnt = 0;
while (--tptr != (edate-1))
   {
   if(*tptr == '/')
      break;
   else
      chcnt++;
   }
return(chcnt);
}

static chkdate(edate, date, oldate)
char *edate, *date;
int oldate;
{
char *malloc();
char *testdate = malloc(10);
char *emonth = malloc(3);
char *eday = malloc(3);
char *eyear = malloc(3);
int i, eimonth, eiday;

/* make a copy of edate to play around with */
(void)strcpy(testdate,edate);

/* replace each '/' with ':' in testdate */
for(i = 0; i < strlen(testdate); i++)
   if (*(edate + i) == '/')
      *(testdate + i) = ':';

/* add a ":" on testdate for extract */
(void)strcat(testdate,":");

/* get month, day, and year into fields */
extract(emonth,testdate,1,':');
extract(eday,testdate,2,':');
extract(eyear,testdate,3,':');

/* make month, day, and year integers */
eimonth = stoi(emonth);
eiday = stoi(eday);

/* check for valid month entered */
if ( (eimonth > 12) || eimonth == 0)
   return(2);

/* check for valid day entered */
if (eiday == 0)
   return(3);

/* is the day valid for the entered month? */
switch(eimonth)
   {
   case 1:
   case 3:
   case 5:
   case 7:
   case 8:
   case 10:
   case 12:
      if (eiday > 31) 
         return(3);
      else 
         break;
   case 4:
   case 6:
   case 9:
   case 11:
      if (eiday > 30)
         return(3);
      else
         break;
   case 2:
      if (eiday > 29)
         return(3);
      else
         break;
   }

/* make sure that the entered date is after today's date */
if((oldate!= 0) && (strcmp(date2ymd(edate),date2ymd(date)) <= 0))
   return(4);

/* it's a good date */
return(1);
}

/********************************************/
/* convert a MM/DD/YY date to a YYMMDD date */
/********************************************/
char *date2ymd(date)
char *date;
{
char tdate[10], mo[3], dy[3], yr[3];
static char ymddate[7];

/* extract the yr, mo, and dy from the date */
sprintf(tdate, "%s/", date);
extract(yr, tdate, 3, '/');
extract(mo, tdate, 1, '/');
extract(dy, tdate, 2, '/');

/* build the YYMMDD date */
if(strlen(yr) == 2)
   (void)strcpy(ymddate, yr);
else
   sprintf(ymddate, "0%s", yr);

if(strlen(mo) == 1)
   (void)strcat(ymddate, "0");
(void)strcat(ymddate, mo);

if(strlen(dy) == 1)
   (void)strcat(ymddate, "0");
(void)strcat(ymddate, dy);

/* return the YYMMDD date */
return(ymddate);
}

/***********************************************************************/
/* Given a String With delimiter Separated Fields, and Given the       */
/* Position of the Field You Want, Extract that Field                  */
/***********************************************************************/
extract(fieldnam,bufr,fieldnum, delim)
char delim, *bufr, *fieldnam; 
int fieldnum;
{
int fieldcnt = 1;
char *tptr, *fptr;

tptr = bufr;

/* move pointer to first address of desired field */
while (fieldcnt < fieldnum)
   {
   if( (*tptr == '\0') || (*tptr == delim) )
      ++fieldcnt;
   ++tptr;
   }

/* put a null at end of desired field and return field */
fptr = fieldnam;
for(;;)
   {
   if( (*tptr == '\0') || (*tptr == delim) )
      break;
   *fptr = *tptr;
   ++fptr;
   ++tptr;
   }
*fptr = '\0';
}

/*************************************************/
/* reload the program on an abort (DEL entered)  */
/* called by edits and editdate                  */
/*************************************************/
/*VARARGS1*/
reload(prog, a1, a2, a3, a4, a5)
char *prog;
{
int s;

/* close all descriptors except stdin, stdout, stderr */
for(s = 3; s < NOFILE; s++)
   (void) close(s);

/* overlay the caller with a 'new' program */
execl(prog, prog, a1, a2, a3, a4, a5);

/* if the execl fails... */
perror("execl failed:");
exit(-1);
}

/*****************************************************************/
/* This function returns a positive integer from a string       */
/* If the string contains invalid character(s) it returns a -1.  */
/* If the string converts to a negative number it returns a -2.  */
/*****************************************************************/
int stoi(string)
char *string;
{
register int index;
int exp, numchars, newint, placeint[64];     /* declare the integers */
int neg = 0;                                /* 1 if number is negative */

/*count number of characters and convert each to integer value*/
for(index=0, numchars=0; *(string+index)!='\0'; ++index,++numchars)
   {
   /* it must be a numeric */
   if (*(string+index) >= '0' && *(string+index) <= '9')
      placeint[numchars] = *(string+index) - '0';
   else if ( (index == 0) && (*(string+index) == '-') )
      neg = 1;
   else
      return(-1);
   }

/* return -2 if number is negative */
if (neg == 1)
   return(-2);

/* make sure there's some characters to convert */
if(numchars==0)return(-1);

/* convert integers in placeint to an integer by multiplying them */
/* by their corresponding place values                            */
newint = 0;
for(exp = 0,index=numchars-1; index > -1;exp++,index--)
   newint += power(10, exp) * placeint[index];
return(newint);
}

/***********************/
/* raise x to the n-th */
/***********************/
int power(x,n)
int x, n;
{
int p;

for(p=1; n>0; --n)
   p *= x;
return(p);
}
