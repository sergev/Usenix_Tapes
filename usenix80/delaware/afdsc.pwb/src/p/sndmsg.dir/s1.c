#
/*
Name:
	SNDMSG User Command

Function:
	A message sending program modeled after the tenex program sndmsg.

Algorithm:
	It delivers local mail by appending it to a file called mailbox in the
	user's home directory.  The delivery of network mail is not yet implemented.
	 If local mail is declared to be undeliverable, or
	some error occurs during the execution of sndmsg, a copy of the letter
	is left in a file called unsent-mail, in the sender's home directory.
	Message are delineated by a control-a at the beginning of the header
	and control-b at the beginning fo the body.  Receipts are indicated by
	combining an octal 200 with the control-a.  Mail does the rest of the
	work delivering receipts.

Parameters:
	-r	tag all messages for receipts.
	-f	forwarding a message.
	-a 	answering a message.

Returns:
	ENOENT	cannot find passwd file
	EACCES	cannot create unsent mail.

Files and Programs:
	PASSWD1

Installation Instructions:
	chmod 6655
	chown root
History:


*/
#include	"./globals.h"
#include	<error.h>
#include	<site.h>
#define	nextarg		{++argv; --argc;}		/* function to move to next argument */
int fflag, ansflag, subfd;
char *prog_id "~|^' sndmsg Release 3  Version 0\n";
char *replyhdr "/tmp/replyhdrxxxxx";
char *replysub "/tmp/replysubxxxxx";
char *savlttr;
main (argc, argv)
int	argc;							/* count of command line arguments */
char	**argv;							/* vector to pointer list of command line arguments */
{
	register int	myfd;
	register int    one;
	register char	*lttr;
	char *endprog;
	int i;
	char *uptr;

	/*	init constants		*/
	if((signal(1,1) & 01) == 0)	/* check if signals previously set */
		signal (1,0);	/*Hang-up before input terminates sndmsg*/
	if((signal(2,1) & 01) == 0)
		signal (2,0);	/*Break before input terminates sndmsg*/
	if((signal(3,0) & 01) == 1)
		signal (3,1);	/*Always ignore quit*/

	one = 1;	 /* I couldn't come up for a better use for the register */
	gtty(0,ttys);	/* get tty status so that we can flip back and forth
			 * between the mode on entry and raw mode	*/
	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	receiptall = 0;
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			case 'r':
				receiptall = 1;
				break;
			case 'f':
				fflag++;
				break;
			case 'a':	
				ansflag++;
				ansetup();
				break;
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			}
		}
		ugh_a_goto:
		nextarg;
	}
/*	flag.erase = argc-one;	/* one or more arg sets flag.erase, causing a
				fancier control-W and control-H during the
				execution of input()	*/
	fd.pw = open(PASSWD1, 0);	/* passwd file used by various routines
					 * such as ...	*/
	if(fd.pw < 0)
	{
		puts(2,PASSWD1);
		puts(2, ":  can't open\n");
		exit(ENOENT);
	}

	endprog = 0;

    do	/* MAIN LOOP *********************************************/
    {
	if (endprog != 0)
		brk(endprog);	/* remove expandabler memory */
	aflag = 0;
	qflag = 0;
	flag.unsent = 0;
	endprog = header = lttr = sbrk(BUFLEN);	/* establish the end of "expandable" memory */



	eomem = lttr+BUFLEN;	/* establish end-of-memory */

	setuphead();
	if(!ansflag) {
	puts(1,"Type letter:\n- - -\n"); lttr = input(0,0);  /* input letter */
	}
	else {
		if((subfd = open(replyhdr,0)) < 0) {
			puts(1,"Can't find original header\n");
			exit(ENOENT);
		}
		savlttr = bomem;
		lttr = input(subfd,0);
		bomem = lttr;
		puts(1,"Original message information has been included\nInput more text:\n- - -\n");
		input(0,0);
		lttr = savlttr;
	}

	normal();	/* so that the user can kill us */
	if (aflag)	continue;	/* abort, and repeat sndmsg */
	else	if(qflag)	break;
	send(to,one);	/* write to 'to' list */
	send(cc,one);	/* write to 'cc' list *//* send should return flag.unsent */

	if (flag.unsent)
	{
		puts(2,"A copy of this letter will be left in");
		puts(2,unsent_mail);
		write(1,"\n",1);
		myfd = open (unsent_mail, 2);
		if (myfd < 0)
			myfd=creat(unsent_mail,PROT);
		else
			seek (myfd, 0, 2);	/* seek end of file */
		/* the general philosophy is if it exists--append to end and
		 * don't touch the user's protection
		 */
		if (myfd < 0)
		{
			puts(2,"Can't create unsent-mail\n");
			exit(EACCES);
		}
		/* copy letter */
		seek (fd.tb,0,0);
		while ((i=read (fd.tb,bigbuf,BUFLEN)) > 0)	write (myfd,bigbuf,i);
		close(myfd);
                chown(unsent_mail,myuid);
	}

	close(fd.th);	close(fd.tb);
	unlink(htempfile);	unlink(btempfile);	/* bye-bye tempfiles */


    }while (repeat);	/* END Main Loop *************************/
	exit(0);		/* Is this trip necessary? */
}




/*

Name:
	setuphead

Function:
	Construct header from user input.

Algorithm:
	Gather date, sender name, and system into buffer.
	From user, collect addressee names and validate
	them.  Collect subject from user.

Parameters:
	None.

Returns:
	EINVAL	bad addressee and -f flag.

Files and Programs:
	None.


*/
setuphead()
{
	register char   *lttr;
	register char   *ptr1, *ptr2;
	extern  char   *arpadate();
	extern	char	*getsys();
	int   one;
	char *ptrmem;

	one = 1;
	lttr = header;
	*lttr = '\0';	/* initially lttr points to null string--for the
			 * benefit of stcat
			 */

	/*	set up header which includes date and from lines	*/
	bomem = stcat(lttr,"Date: ");
	bomem = stcat(bomem,arpadate());
	bomem = stcat(bomem,"\nFrom: ");
	bomem = stcat(bomem,getname(from));
	bomem = stcat(bomem,getsys());
	bomem = stcat(bomem,"\nTo: ");
	bomem++;
	ptrmem = bomem;
	rawmode();	/*	required by input()	*/
	puts(1,header);	/* write out header so the user can see it */
	for(;;) {		/* while there are bad recipients */
		to = input(0,one);
		if (*to == '\0') /* we require at least one person in the to list */
		{
			ptr1 = to;
			ptr2 = &from[0];
			while(*ptr1++ = *ptr2++);
			bomem = ptr1;
		}
		flag.unsent = 0;
		send(to,0);		/* check addressees only */
		if(!flag.unsent)
			break;
		puts(2,"Please input entire 'To' recipient list again\n");
		if(fflag)
			exit(EINVAL);
		puts(1,"To: ");
		bomem = ptrmem;		/* reuse memory */
	}

	ptrmem = bomem;
	puts(1,"Cc: ");		/* input cc list */
	for(;;) {		/* while there are bad recipients */
		cc = input(0,one);
		flag.unsent = 0;
		send(cc,0);
		if(!flag.unsent)
			break;
		puts(2,"Please input entire 'Cc' recipient list again\n");
		if(fflag)
			exit(EINVAL);
		puts(1,"Cc: ");
		bomem = ptrmem;
	}

	if(!ansflag) {
		puts(1,"Subject: ");	
		for(;;) {
			subject = input(0,one);
			if(*subject != '\0')	/* require a subject */
				break;
			puts(2,"A subject is required, please.\n");
			puts(1,"Subject: ");
		}
	}
	else {
		if((subfd = open(replysub,0)) < 0) {
			puts(2,"Cannot read messageheader\\n");
			exit(EACCES);
		}
		subject = input(subfd,one);
		close(subfd);
	}
}




/*

Name:
	ask

Function:
	Collect an line from the terminal.  If the line starts with a 'y' or
	a 'Y', return true, otherwise return false;

Algorithm:
	Loop, getting characters until new line is encountered.  If a 'y' or a
	'Y' was encountered, set answer to true.

Parameters:
	pointer to string to be printed before answer is collected.

Returns:
	The variable answer, set according to algorithm above.
	1 = 'y'
	0 = not 'y'

Files and Programs:
	None.

*/
ask(str)	char	*str;
{
	register int answer;
	extern  char    getchar();

		puts(1,str);	/*	ask the question	*/
		answer = false;	/*	assumed answer is false	*/
		loop
			switch (getchar(0))
			{
			case IOERR:
			case '\n':	return answer;
			case 'Y':
			case 'y':	answer = true;
			}
}
/*

Name:
	ansetup

Function:
	Construct names of header and subject reply files.

Algorithm:
	Append process id to filenames.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
ansetup() {
	char *p1;
	char *p2;
	int ansi, anspid;
	p1 = &replysub[13];
	p2 = &replyhdr[13];
	anspid = getpid();
	for(ansi=5;ansi--;anspid =>> 3) {
		p1[ansi] = p2[ansi] = '0' + (anspid&07);
	}
}
/*

Name:
	getsys

Function:
	Construct string with system id (i.e., @WP4).

Algorithm:
	Using sysid system call information, construct string,
	starting with at-sign, followed by system id and number.

Parameters:
	None.

Returns:
	Pointer to string.

Files and Programs:
	None.


*/
char *getsys() {
	sysid(&sid);
	putptr = &sysname[0];
	*putptr++ = '@';
	*putptr++ = sid.si_type[0];
	*putptr++ = sid.si_type[1];
	putn(sid.si_num);
	*putptr++ = '\0';
	return(&sysname[0]);
}
/*

Name:
	putn

Function:
	Convert binary number to ascii string.

Algorithm:
	Convert each digit, move into place in buffer.

Parameters:
	Binary number.

Returns:
	None.

Files and Programs:
	None.


*/
putn(i)
int i;
{
	if(i > 9)
		putn(i/10);
	*putptr++ = i%10+'0';
}
