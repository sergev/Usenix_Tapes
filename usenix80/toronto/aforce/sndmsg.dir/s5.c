#
#include	"./extern.h"
/*
Name:
	netpost

Function:
	to attempt to mail off a piece of network mail.

Algorithm:
	Contrive a unique name for a network mail file.
	Create it in the netmail daemon's mailbox
	put destination host, users and sender names into file.
	copy message into the file.
	Send receipt, if asked for.

Parameters:
	*char	pointer to null terminated user name
	*char	pointer to null terminated host name

Returns:
	1 = send successfully
	0 = unsuccessful

Files and Programs:
	/usr/lpd/netmail
	/dev/net


*/
netpost(uname,hname,s_flag)	char	*uname;	char	*hname; int s_flag;
{
	register char	*p1;
	register char	*p2;
	register int	i, j;

	if (stat(hname,&statb) < 0)
	{
		puts(1,"host "); puts(1,hname+9 /* jump over "/dev/net/" */);
		puts(1," is unknown\n");
		flag.unsent++;
		return false;
	}
	if(!s_flag)
		return true;

	p2 = &netfile[17/*sizeof "/usr/lpd/netmail" -1*/];
	for(p1=from; *p2++= *p1++;);
	--p2;
	for (i = 14+1+&netfile[17]-p2; --i;) *p2++ = '0';
	*p2-- = '\0';
	for(*p2 = 'a'; *p2 <= 'z'; (*p2)++)
	{
		if (stat(netfile,&statb) < 0)
		{
			/* p1 is now (poof!) an int which will forever (for the
			 * rest of its natural life until return does it part)
			 * contain the "constant" file descriptor for netfile
			 */
			p1 = creat(netfile, 0400);
			if (p1 < 0)
			{
				puts(1,"Can't create mailfile for ");
				puts(1,uname); puts(1,"@"); puts(1,hname); write(1,"\n",1);
				flag.unsent++;
				return false;
			}

			chown(netfile,myuid);
			puts(p1,hname);
			puts(p1,colon_lit);
			puts(p1,uname);
			puts(p1,colon_lit);
			if (findlocal(from)) puts(p1,mailbox);
			puts(p1, ":\n");

			seek(fd.th, 0, 0);
			seek(fd.tb, 0, 0);
			while((i=read(fd.th,bigbuf,BUFLEN))>0) j = write(p1,bigbuf,i);
			while((i=read(fd.tb,bigbuf,BUFLEN))>0) j =* write(p1,bigbuf,i);

			close(p1);
			puts(1,"Queued for transmission\n");
			if (receipt && j >= 0)	netreceipt();	/*send receipt to originator, saying 'sent to net' */
			return ++netmsgs;
		}
	}
	puts(1,"Too many messages awaiting transmission\n");
	flag.unsent++;
	return false;
}
/*

Name:
	getname

Function:
	To find out the name of the user who forked me.

Algorithm:
	Get my real user id and look it up in the password table
	If I'm Root, give me a chance to say who I'd rather be
	and check that I'd like to be a real user.

Parameters:
	pointer to buffer where name should be placed.

Returns:
	pointer to buffer where caller's name was placed.

Files and Programs:
	None.


*/
getname(caller)		char	*caller;
{
	register char	*cp;
	register char	*lp;
	register int	i;
	char linebuf[100];
	char c;

	myuid = getuid() & 0377;
	lp = &linebuf[0];
	if (pwuid(myuid, lp))
	{
		puts(2, "Your user id is not in the password file?!?\n");
		exit(1);
	}
	for(cp = caller; *lp != ':';) *cp++ = *lp++;
	*cp = '\0';

	if (strmatch (caller, "root"))	/* Don't want to send receipts to root's malbox- too messy */
		do
		{
			puts(1,"From (if other than 'root'): ");
			cp = &linebuf[0];
			do
			{
				read(0, &c, 1);
				*cp++ = c;
			} while(c != '\n');
			if (linebuf[0] != '\n')
			{
				for (cp = caller, lp = &linebuf[0]; (*cp++ = *lp++) != '\n';);
				*--cp = '\0';
			}
			else
			{
				caller[0] = 'r';caller[1] = caller[2] = 'o';
				caller[3] = 't';caller[4] = '\0';
			}
		}while (!validusr(caller));
	else
		validuser(caller);
					/* called for unsent_mail side effect */
	return caller;
}

/*

Name:
	validuser

Function:
	Look sender's name up, find homedir.

Algorithm:
	Scan passwd file for sender's name.  If found, create
	home directory / unsent_mail filename.

Parameters:
	Pointer to user name.

Returns:
	1 = user ok.
	0 = invalid user anem.

Files and Programs:
	homedir/unsent_mail.


*/
validusr(name)	char *name;
{
	register char *np;
	register char pwc;
	register char colon;
	int i;

	colon = ':';
	seek(fd.pw,0,0);	/* rewind password file */
	pwcount = 0;
	name[8] = '\0';

	do
	{
		/* search for a line starting with name */
		for (np = name; *np++ == (pwc = getpwchar()); );
		if (pwc == colon && *--np == '\0') {
			for(i=0; i<4; i++)
				while(getpwchar() != colon);
			for(np = &unsent_mail[0];(pwc = getpwchar()) != colon;*np++ = pwc);
			*np = '\0';
			np = stcat(&unsent_mail,"/unsent_mail");
			*np = '\0';
			return true;
		}
		while ((pwc = getpwchar()) != '\n' && pwc > 0);	/* Go to start of next line */
	}while (pwc > 0);
	return false;
}
/*

Name:
	slashc

Function:
	Given that the last input character seen was a backslash, this
	routine returns the character that is being escaped.

Algorithm:
	If terminal is half ascii
		If next char is lower case, return its upper case counterpart.
		If it is the preimage of a non-half ascii character, return
			that character.
	Else, return the next character.

Parameters:
	File descriptor for input

Returns:
	A character.

Files and Programs:
	None.


*/
slashc(infd)
int infd;
{
	register char c;
	extern  char   getchar();

	c = getchar(infd);
	if (ttys[2] & 04) /* Half-ASCII */
	{
		if (c >= 'a' && c <= 'z') return c&not 040; /* ascii dependent */
		else
		switch (c)
		{
		case'!':	return'|';
		case'\'':	return'`';
		case'^':	return'~';
		case'(':	return'{';
		case')':	return'}';
		default:	return c;
		}
	}
	else return c;
}
