
/*
	DISPLIB sample routine. Compile with the Makefile, or
	manually by:

		% cc -o test test.c libdisp.a -ltermlib

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
#include <sys/wait.h>

#include <ctype.h>
#include <stdio.h>
#include <strings.h>

#include "displib.h"

/* what is the system title for initscrn() */
#define SYSNAME "DISPLIB DEMONSTRATION PROGRAM"

static char Restart[128];
static char date[9];

main()
{
char *pgm = "test";
int choice;
char sel[5], filename[31];

/* set program to exec on Restart (see edits and editdate) */
(void)sprintf(Restart, "%s/%s", "", "test");

/* Set up terminal type and validate terminal */
terminit();

/* Put terminal in CBREAK mode with no ECHO */
termset(pgm);

/* get today's date */
today(date);

/* Display the menu */
menu();

/* Get the menu choice and validate it */
for(;;)
   {
   tmsg(1);
   printf("Enter a choice from the menu above, or <help> or <exit>");
   gotoxy(28,20);
   cleol();
   *sel = '\0';
   edits(sel,4, 0, Restart, 0);
   if(abb(sel,"exit") == 0)
      {
      msg("Thank you...", NOBEEP, 2, 0);
      clear();
      termreset();
      exit(0);
      }

   /* convert choice to an int */
   else 
      choice = stoi(sel);

   /* validate range of choice */
   if ( (*sel == '\0') || (choice < 1) || (choice > 4) )
      bell();
   else
      {
      switch(choice)
         {
         case 1:  load(1, "/usr/ucb/mail", 0);
						break;

         case 2:  load(2, "/usr/local/rn", 0);
						break;

         case 3:  tmsg(1);
						printf("Enter the filename you wish to edit...");
						gotoxy(15,16);
						printf("Filename:");
						gotoxy(25,16);
						*filename = '\0';
   					edits(filename, 30, 0, Restart, 0);
						load(3, "/usr/ucb/vi", filename, 0);
						break;

			/* doesn't exist yet
         case 4:  load(4, "/usr/sloane/sureBud"); 
						break;
			*/

         default: msg("Option %d is not yet implemented...",BEEP,2,choice,0);
                  clmsg();
                  break;
         }
      }
   }
}

/********************/
/* Display the Menu */
/********************/
static menu()
{
int   pix=8;

/* Display the ATRARI main menu */
initscrn("Main Menu",  SYSNAME);

gotoxy(15,pix);
inverse();
printf("THE OPTIONS");
normal();

gotoxy(18,++pix);
printf(" 1. Electronic MAIL");
gotoxy(18,++pix);
printf(" 2. Read News with RN");
gotoxy(18,++pix);
printf(" 3. Use VI");
gotoxy(18, ++pix);
printf(" 4. Expert System: Translate COBOL to Fast-C");

gotoxy(15,20);
printf("Your choice? ");
}

/******************/
/* Load a Program */
/******************/
static load(opt, prog, a1, a2, a3, a4, a5, a6)
int opt;
char *prog;
{
int s;
char errmsg[128];
union wait *status;
char *pgm = "test/load";
status = (union wait *)alloca(sizeof(status));

tmsg(1);
printf("Now loading option %d...", opt);
(void)fflush(stdout);
clear();
termreset();

/* child */
if(vfork() == 0)
   {
   for(s=3; s<=NOFILE; s++)
      (void)close(s);
   execl(prog, prog, a1, a2, a3, a4, a5, a6);
	(void)sprintf(errmsg, "\nERROR: execl failed: %s", prog);
   perror(errmsg);
   exit(-1);
   }
/* parent */
else
   {
   (void)wait(status);
   termset(pgm);
   if(status->w_retcode!= 0)
      {
      termreset();
      exit((int)status->w_retcode);
      }
   menu();
   }
}

