#
#include	"./extern.h"
#include <error.h>
/*
Name:
	getchar

Function:
	To read a character from a specified file.

Algorithm:
	Do a read, and if we got an end of file, return a -1.
	CAVEAT	Input is unbuffered.

Parameters:
	file descriptor of file to be read from.

Returns:
	character
	-1 = end of file

Files and Programs:
	None.

*/
char	getchar(cfd)	int	cfd;
{
	char c;

	return read(cfd, &c, 1) <= 0 ? IOERR : c&0177;
}

/*

Name:
	getpwchar

Function:
	Read a character from the passwrod file.
	Input is buffered.

Algorithm:
	Read chunk of password file.  Until end of buffer, return
	next character from buffer.

Parameters:
	None.

Returns:
	-1 = read error or end of file
	character

Files and Programs:
	None.


*/
char getpwchar()
{
	loop
		if (pwcount > 0)
		{
			--pwcount;
			return *pwptr++;
		}
		else
		{
			pwcount = read(fd.pw, pwbuf, BUFLEN);
			if (pwcount <= 0) return -1;
			pwptr = pwbuf;
		}
}

/*

Name:
	namestr

Function:
	Determine whether we have a name delimiting character.

Algorithm:
	Check character against all eegal delimiters.

Parameters:
	Character to be checked.

Returns:
	0 = character is delimiter
	1 = not delimiter

Files and Programs:
	None.


*/
namestr(ch)	int ch;
{
	register int	c;

	c = ch;

	return c!=' ' && c != ',' && c!='\t' && c!='\n';
}
/*

Name:
	valid

Function:
	Determine whether we have a valid name character.

Algorithm:
	Check character against all legal terminators and delimiters.

Parameters:
	Character to be checks.

Returns:
	0 = not a name character
	1 = chara is valid name character.

Files and Programs:
	None.


*/
valid(ch)
char ch;
{
	register char c;

	c = ch;
	return !terminator(c) && namestr(c);

	/* c!=IOERR&c!='\0'&c!='@'&c!=':'&c!=' '&c!=','&c!='\t'&c!='\n' */
}
/*

Name:
	terminator

Function:
	Determine whether we have a name terminating character.

Algorithm:
	Check character against all legal terminators.

Parameters:
	Character to be checked.

Returns:
	1 = cjaracter is terminator
	0 = not terminator

Files and Programs:
	None.


*/
terminator(ch)
{
	register int	c;

	c = ch;
	return c==IOERR || (c =& 0177)=='\0' || c=='@' || c==':';
}


/*

Name:
	stcat

Function:
	Concatenate strings.

Algorithm:
	Scan for end of 1st string, copy 2nd string onto end of 1st
	(including null).

Parameters:
	two string pointers.

Returns:
	Pointer to terminating null of concatenated pair.

Files and Programs:
	None.


*/
stcat(former,latter)	char	*latter,	*former;
{
	register char	*fp;
	register char	*lp;
	for (fp = former; *fp++;);
	--fp;
	for (lp = latter; *fp++ = *lp++;);

	return fp-1;
}
/*

Name:
	arpadate

Function:
	to get an ARPA format date (format described in rfc 680)

Algorithm:
	Get the unix format date and jumble it around to be arpa format
	Unix format is:
			day mmm dd hh:mm:ss yyyy
	Arpa format is:
			dd mmm yyyy at hhmm-zzz

Parameters:
	None.
 
Returns:
	pointer to the arpa format date corresponding to that time

Files and Programs:
	None.

*/
char	*arpadate()
{
	register char	*p;
	register char	*t;
	register char	*q;
	extern  int	*localtime();
	int	tvec[2];

	extern char	dbuf[];	/* this is external only so that it can be initialized--it is used nowhere else in the program */

	time(tvec);
	t = ctime(tvec)+4;
	q = t+4;
	p = dbuf;

	*p++ = *q++;	/*	day	*/
	*p++ = *q++;	/*	day	*/
	p++;
	*p++ = *t++;	/*	month	*/
	*p++ = *t++;	/*	month	*/
	*p++ = *t++;	/*	month	*/
	p++;	t =+ 13;
	*p++ = *t++;	/*	year	*/
	*p++ = *t++;	/*	year	*/
	*p++ = *t++;	/*	year	*/
	*p++ = *t;	/*	year	*/
	p =+ 4;	q++;
	*p++ = *q++;	/*	hour	*/
	*p++ = *q++;	/*	hour	*/
		q++;
	*p++ = *q++;	/*	minute	*/
	*p++ = *q++;	/*	minute	*/
	if( localtime(tvec)[8] ) *(p+2) = 'D';	/* daylight savings */

	return t[-15]==' '? &dbuf[1] : &dbuf[0];
}
/*

Name:
	secure_open

Function:
	To check that a file may be opened by the user of sndmsg.  We are
	running as the root and thus can access any file, but the user must not
	be allowed to open files that are hidden from him.

Algorithm:
	let the system decide if the user may have access to the file by forking, seting uid and
	attempting to open the file.  This is the best approach since the system's idea of
	protection may change from time to time, and we will stay in step with the times.
	If no fork is allowed, we wait 10 seconds and try again, the user has the option of rubbing out sndmsg.

Parameters:
	pointer to filename
	function for "open" call

Returns:
	The file descriptor of the opened file or -1 if no permission to open

Files and Programs:
	None.


*/
secure_open(str,func)	char	*str;	int func;
{
	int status;

	status = -1;
	loop
	switch(str[0] == '\0' || fork())	/* if str[0]=='0' then we would open the directory */
	{
	case 0:
		setuid(myuid);
		exit(open(str,func));

	default:
		wait(&status);
		if (status>0)
			return open(str, func);
		else
			return -1;

	case -1:
		puts(1,"Waiting for process...\n");
		sleep(10);
		continue;
	}
}
/*

Name:
	netreceipt

Function:
	To create and send a receipt when mail is sent to the net and a receipt 
	is requested.

Algorithm:
	Fork off old mail program, send recipient, subject, and header down pipe.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	/bin/remindmail


*/

netreceipt()
{
	int	fildes[];
	register pidf,
		 i;

	seek (fd.th, 0, 0);	/*rewind file containing message */
	pipe (fildes);
	while ( (pidf = fork() ) == -1)	sleep(5);
	if (pidf == 0)
	{
		/*CHILD spawns remindmail*/
		close (0);
		dup (fildes[0]);
		close(fildes[1]);
		execl ("/bin/remindmail", "netreceipt(sndmsg)", from, 0);
		exit (0);
	}
	/*
	 * write explanatory message 
	 */
	write (fildes[1], "- - -\nTHIS MESSAGE WAS RECEIVED BY THE NET:\n",44);
	/*
	 * write the header of the message being receipted
	 */
	while ((i=read(fd.th, bigbuf, BUFLEN)) == BUFLEN)
		write (fildes[1], bigbuf, i);
	write (fildes[1], bigbuf, i);	

	write (fildes[1], "\n-----\n\0", 8);
	close (fildes[1]);
}
/*

Name:
	sigfnc1

Function:
	Go to function level when break (or del) occurs.

Algorithm:
	Reset signal, write break, reset to setexit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
sigfnc1()
{
	signal(2,&sigfnc1);
	puts(1,"-BREAK-\n");
	reset();
}
/*

Name:
	sigfnc2

Function:
	Tell user that break (or del) occurred.

Algorithm:
	Reset signal, display message.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
sigfnc2()
{
	signal(2,&sigfnc2);
	puts(1,"-BREAK-\n");
	return;
}
/*

Name:
	sigfnc3

Function:
	save body of message when hangup occurs.

Algorithm:
	Write body of message into "sndmsg-hup" file when hangup occurs
	during input.

Parameters:
	None.

Returns:
	EBRK	Break occurred.

Files and Programs:
	sndmsg-hup


*/
sigfnc3()
{
	int fdl;

	fdl = creat (HUPFILE,0666);
	chown("sndmsg-hup",myuid);
	write (fdl, lttr, ptend - lttr);
	close (fdl);

	unlink (htempfile);
	unlink (btempfile);
	normal();
	exit (EBRK);
}
/*

Name:
	sigfnc5

Function:
	Send editor a del when break ( or del) occurs.

Algorithm:
	Reset signal, pipe del to editor, reset.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
sigfnc5()
{

	signal (2,&sigfnc5);
	write (npipe[1], del, 1);
	reset ();
}
