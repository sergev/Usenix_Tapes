#ifndef lint
static char rcsid[] = "$Header: deadletter.c,v 4.0 86/11/17 16:02:11 sob Exp $";
#endif
/**************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of resolve, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************/
/* attempt to return dead letter */
/* $Log:	deadletter.c,v $
 * Revision 4.0  86/11/17  16:02:11  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 1.2  86/10/21  15:32:50  sob
 * Added RETURNMAIL
 * 
 * Revision 1.1  86/09/04  17:53:44  sob
 * Initial revision
 * 
 *
 */
#include "uuconf.h"
EXTERN char progname[];
char letter[] = "/usr/tmp/.rlXXXXXX";

deadletter(retlet, here,reason,host)
FILE *retlet;
char * host;
int here,reason;
{

	int i;
	long iop;
	struct tm *bp, *localtime();
	FILE * letf;
	char * date, *asctime();

	if(getlogin() != NULL) syserr("Letter failed....returned to sender\n");
#ifdef RETURNMAIL
    /*
     * make a place to create the return letter
     */
	mktemp(letter);
	unlink(letter);
	if((letf = fopen(letter, "w")) == NULL){
		fprintf(stderr, "%s : can't open %s for writing\n", progname,letter);
		exit(EX_CANTCREAT);
	}
	/*
	 * Format time
	 */
	time(&iop);
	bp = localtime(&iop);
	date = asctime(bp);
	/* build the return header */
	fprintf(letf,"From %s!%s %.16s %.4s remote from %s\n",
		Myname,MAILERDAEMON, date, date+20, Myname);
	fprintf(letf,"From: %s@%s (UUMAIL ROUTER)\nTo: %s\n",MAILERDAEMON,Myname,from);
	fprintf(letf,"Subject: Failed Mail\nMessage-Id: <%d.%d.%s@%s>\n\n",
		getpid(),time(0),MAILERDAEMON,Myname);
#else
	letf = stderr;
#endif

 	fprintf(letf,"Your mail failed to reach its destination because:\n");
	switch(reason){
		case EX_NOHOST:
			fprintf(letf,"The path (%s) cannot be resolved.\n",host);
			break;
		case EX_OSERR:
			fprintf(letf,"An Operating System error occurred while processing your\nmail. Please resend.\n");
			break;
		default:
			fprintf(letf,"An unknown error (code = %d) occured.\n",reason);
			break;
		}
#ifdef RETURNMAIL
		fprintf(letf,"Your returned mail follows:\n");
		fprintf(letf,"-------------------------------------------\n");
		while (fgets(lbuf, sizeof lbuf, retlet))
			{
				fprintf(letf,"\t");
				fputs(lbuf, letf);
			}


			     /* return the mail */

		fclose(letf);
		sprintf(lbuf,"%s %s < %s",MAILER,from,letter);
#ifdef DEBUG
		if (Debug)
			fprintf(stderr,"Command is %s\n",lbuf);
		else
		{
#endif
			system(lbuf);
			unlink(letter);
#ifdef DEBUG
		}
#endif
#endif
}

