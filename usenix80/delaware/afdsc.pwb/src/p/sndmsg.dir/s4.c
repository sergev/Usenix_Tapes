#
#include	"./extern.h"
#include	<error.h>
/*
Name:
	send

Function:
	To take a list of recipients and take steps to verify their
	correctness or to  deliver the
	message (now in the temp file) to all of them.

Algorithm:
	parse off a name.
	If it is a name, open that file and read from it for a while.
	Try locally, then centrally, to open this file.
	If it is a local user id, call post
	If it is a network user id, call netpost
	If receipt is asked for, and recipient is not sender, set receipt flag to 1.
	If send flag is not set, stop short of actual delivery.

Parameters:
	Pointer to a null terminated character string which is a list
	of recipients.
	Flag indicating whether to send mail (=1) or just check 
	for legal addressee (=0);

Returns:
	None.

Files and Programs:
	/usr/office
	homedir/mailbox

*/
send(list,s_flag)	char	*list;	int s_flag;
{
#define getnext		(file? getchar(namefd): *curp++)
#define skipfill	{while(!namestr(c)) c = getnext;}
#define USIZ		8	/* size of a user's name in passwd file */
	register char	*name;
	register char	*curp;
	register int	c;
	extern  char   getchar();

	int	atflag;
	char	temp;
	char	user[STRINGLEN];
	char	distrib[30];
	char	file;	/* flag for file open */
	int	namefd;	/* file descriptor of open file */
	char	*stringend, *p;

	c = ' ';
	curp = list;	file = false;	name = &user[0];	atflag = false;
	do
	{
		receipt = receiptall;	/* binary flag for receipt tagging */
		while (valid(c)) /* copy chars (!terminators and namestrs) either into user[] or hostname[] */
		{
			if(name <= &user[STRINGLEN]) *name++ = c;
			c = getnext;
		}
		stringend = name;	/* preserve the end of the string */
		*name++ = '\0';		/* terminate name string */
		*name = '\0';		/* extra null in case ':' is added */
		name = &user[0];
		skipfill;
		switch (c)
		{
		case ':':
			if (atflag)
			{
				atflag = false; puts(1,"ignoring @\n");
			}
			if (file)
			{
				*stringend = ':';	/* fake a name terminated by a colon */
				send(name,s_flag);	/* recursive lists! (up to 15 open files that is.) */
			}
			else
			{
				/* try to open local or full pathname distrib list */
				if ((namefd = secure_open(name,0)) < 0) {
					/* possibly too many open files */
					/* try to open central distrib list */
					distrib[0] = '\0';
					p = stcat(stcat(&distrib,OFFICE),&user[0]);
					*p++ = '\0';
					*p++ = '\0';
					if((namefd = secure_open(&distrib,0)) < 0) {
						puts(1,"Cannot find distribution list\n");
						flag.unsent++;
					}
					else
						file++;
				}
				else
					file++;
			}
			c = getnext;
			continue;


		case '@':
			c = getnext;
			if(atflag)
			{
				atflag=false; puts(1,"Ignoring @\n");
			}
			else
			{
				atflag=true;
				name = &hostname[9/* sizeof "/dev/net/" -1*/];
				skipfill;
			}
			continue;

		case IOERR: if(file)
			{
				close(namefd);
				file = false;
				c = ' ';
			}

		default:
			if (c<' ' && c!=IOERR)
			{
				if(!ask("Control character after name\nContinue? ")) return;
				flag.unsent++;
				c = getnext;
			}

		 case '\0':
			if (*name=='\0') continue;
			for (p=name; valid(*p); p++);
			if (*--p == ';')
			{
				receipt = 1;	/* tag it for a receipt */
				*p = '\0';	/* delete semicolon */
			}
			if (strmatch(from,name))
				receipt = 0;	/* don't receipt when origionator = recipient */
			if (atflag)
			{
				for (p=hostname; valid(*p); p++);
				if (*--p == ';')
				{
					receipt = 1;	/* tag it for a receipt */
					*p = '\0';	/* delete semicolon */
				}
				if(s_flag) {
					puts(1,"@");
					puts(1,&hostname[9]);
					puts(1,"...");
				}
				netpost(name,hostname,s_flag);
				atflag = false;
			}
			else {
				if (findlocal(name))
				{
					if(s_flag) {
						stcat(mailbox, "/mailbox");
						post(mailbox);
					}
				}
				else
				{
					puts(2,name);
					puts(2,"...Not a known user\n");
					flag.unsent++;
				}
			}
		}
	}
	while (file || c>0);
}

/*

Name:
	strmatch

Function:
	Compare for equal strings

Algorithm:
	Compare 2nd string against 1st string until end of 2nd string.

Parameters:
	Two string pointers.

Returns:
	1 = match
	0 = no match

Files and Programs:
	None.


*/
strmatch (buf,str)
char	buf[],
	str[];
{
	int	i;
	for (i=0; str[i] != '\0' && str[i] == buf[i]; i++);
	if (str[i] == '\0')
		return(1);
	else
		return (0);
}
/*

Name:
	findlocal

Function:
	To ascertain whether or not someone is a local user, and
	if they are, to find the name of their root directory.

Algorithm:
	Search the password file for a line starting with the user name.
	If found:
		extract all the info about uid, gid mailbox, etc.
		return.
	Else, continue the search.
	Announce that he isn't a known user.

Parameters:
	pointer to user name.

Returns:
	1 = she was found
	0 = invalid user name.

Files and Programs:
	None.


*/
findlocal(name)		char	*name;
{
	register char	*np;
	register char	pwc;
	register char	colon;

	colon = ':';
	seek(fd.pw,0,0);	/*	rewind the password file	*/
	pwcount = 0;
	name[8] = '\0';	/* truncate name after 8th char as the name in the
			 * passwd file is currently 8 chars max
			 */

	do
	{
		/* search for a line starting with name */
		for (np = name; *np++ == (pwc = getpwchar()); );
		if (pwc == colon && *--np == '\0')
		{
			/* extract all neat info from pw entry */
			while(getpwchar() != colon);

			/* we have skipped over the password */
			/* good candidates for subroutines */
			for(uid = 0; (pwc = getpwchar()) != colon; uid =+ (9*uid) + pwc - '0');	/* how many words? */
			for(gid = 0; (pwc = getpwchar()) != colon; gid =+ (9*gid) + pwc - '0');
			while(getpwchar() != colon);

			/* we have skipped over the default proj */
			for (np = &mailbox[0]; (pwc = getpwchar()) != colon; *np++ = pwc);
			*np = '\0';
			return true;
		}
		while( pwc != '\n' && pwc > 0 ) pwc = getpwchar();
	}
	while (pwc > 0);
	return false;
}
/*

Name:
	tmpfile

Function:
	To select a unique name for, then creat a temporary file; write it out.

Algorithm:
	concatenate the process id to the filename,
	write out either header or body text, as per first two parameters.
	Header label must be interspersed with header text.

Parameters:
	Pointer to beginning of message body buffer.
	Pointer to end of message body buffer.
	Pointer to beginning of temp file name.

Returns:
	None.

Files and Programs:
	/tmp/hsndxxxxx
	/tmp/sendxxxxx


*/
tmpfile(lttr, pend, tempfile)	char	*pend, *lttr, *tempfile;
{
	register int i;
	register char	*p1;
	register char	*p2;

	p1 = &tempfile[9/*==length("/tmp/send")-1; (for the null)*/];
	p2 = getpid();
	for(i=5+1; --i; p2 =>> 3)
		*p1++ ='0'+ (p2 & 07);

	p1 = &tempfile[0];	/* p1 pointer to tempfile string */
	/* poof! p2 is now a register which contains a file descriptor */
	p2 = creat(p1,PROT);
	if (p2 < 0)
	{
		puts(1,"Can't create temp file\n");
		exit(EACCES);
	}

	chown(p1, myuid);
	if (lttr == '\0' && pend == '\0')
	{
		puts(p2, header);
		puts(p2, to);
		if (*cc!='\0')
		{
			puts(p2, "\nCc: "); puts(p2, cc);
		}
		puts(p2, "\nSubject: ");
		puts(p2, subject);
		puts(p2, "\n- - -\n");
		close(p2);
		fd.th = open(p1,0);
		if(fd.th < 0)
		{
			puts(1,"Can't open temp header file\n");
			exit(EACCES);
		}
		htempfile = tempfile;
	}
	else
	{
		write(p2, lttr, pend-lttr);
		/* whole message is now in temp file */
		close(p2);
		fd.tb = open(p1,0);
		if(fd.tb < 0)
		{
			puts(1,"Can't open temp body file\n");
			exit(EACCES);
		}
		btempfile = tempfile;
	}
}
/*

Name:
	post

Function:
	To mail a letter (in tempfile) to some individual unix user.

Algorithm:
	Create a mailbox if he doesn't have one.
	If he had one make sure it is not in use.
	Append the contents of the tempfile to it, putting
	control-a before header and control-b before body.
	Make recipient the owner if he wasn't already.
	If a receipt is asked for, turn on bit 7 of control-a at beginning of message.

Parameters:
	none

Returns:
	1 = sent successfully
	0 = unsuccessful

Files and Programs:
	homedir/mailbox


*/
post(box)	char	*box;
{
	register int	i;
	register int	mbfd;
	register char	*mbox;
	char mbhd;

	mbox = box;
	if (stat(mbox,&statb) < 0)
	{
		mbfd = creat(mbox,PROT);
		if (mbfd < 0)
		{
			puts(2,"Can't create ");
			puts(2,mbox);
			puts(2,"\n");
			flag.unsent++;
			return false;
		}
	}
	else
	{
		if (statb.s_nlinks > 1)
		{
			puts(2,mbox);
			puts(2," is busy\n");
			flag.unsent++;
			return false;
		}
		mbfd = open(mbox,2);
		if (mbfd < 0)
		{
			puts(2,"Can't open ");
			puts(2,mbox);
			puts(2,"\n");
			flag.unsent++;
			return false;
		}
		seek(mbfd,0,2);	/* seek end of file */
	}

	seek(fd.th,0,0);
	seek(fd.tb,0,0); /* rewind the file containing the message */
	/* a subroutine? */
	mbhd = '\001';
	if(receipt)
		mbhd =| 0200;
	write(mbfd,&mbhd,1);
	while((i = read(fd.th,bigbuf,BUFLEN))>0) write(mbfd,bigbuf,i);
	write(mbfd,"\002",1);
	while((i = read(fd.tb,bigbuf,BUFLEN))>0) write(mbfd,bigbuf,i);

	close(mbfd);
	chown(mbox,uid);
	return true;
}
int pip[2];
quepost() {
	char **queargs, *nr;
	char unit;
	char *ptr, *delta;
	int i, qi;
	char c;
	int quefd;
	if(fork()) {		/* parent */
		wait();
		return;
	}
	setuid(myuid);		/* child */
	ptr = bigbuf;
	puts(1,"What is the (first) delivery date (ddmmyy)? ");
	while((c = getchar(0)) != '\n')
		*ptr++ = c;
	*ptr++ = '\0';
	delta = ptr;
	puts(1,"Is this to be delivered repeatedly (y or n)");
	while((*ptr++ = getchar(0)) != '\n');
	ptr = delta;
	qi = 0;
	queargs[qi++] = &"queue";
	queargs[qi++] = &"-q";
	queargs[qi++] = &"mail";
	queargs[qi++] = &"-t";
	queargs[qi++] = bigbuf;
	if(*ptr == 'y') {
		puts(1,"What is to be the interval between delivery?");
		while((*ptr++ = getchar(0)) != '\n');
		ptr = delta;
puts(1,"about to scan\n");
		for(;;) {
			nr = ptr;
			while(*ptr != ' ' && *ptr != '\n')
				ptr++;
			unit = ptr[-1];
			switch(unit) {
			case 'd':
				queargs[qi++] = &"-s";
				break;
			case 'w':
				queargs[qi++] = &"-w";
				break;
			case 'm':
				queargs[qi++] = &"-m";
				break;
			default:
				puts(1,"Illegal unit\n");
				continue;
			}
			ptr[-1] = '\0';
			queargs[qi++] = nr;
puts(1,"done with scan\n");
			if(*ptr == '\n')
				break;
		}
	}
	else {
		queargs[qi++] = &"-s";
		queargs[qi++] = &"0";
	}
	queargs[qi++] = 0;
	pipe(pip);
	if((fork()) == 0) {	/* child */
		close(0);
		dup(pip[0]);
		close(pip[1]);
		execv("/bin/queue",queargs);
		puts(1,"Can't find /bin/queue\n");
		exit(ENOEXEC);
	}
	seek(fd.th,0,0);
	for(i=0;i<2;) {
		read(fd.th,&c,1);
		if(c == '\n')
			i++;
	}
	/*
	 * Strip header of message headers
	 */
	striphdr();
	quefd = open(btempfile,0);
	while((i = read(quefd,bigbuf,BUFLEN)) > 0)
		write(pip[1],bigbuf,i);
	close(quefd);
	close(pip[0]);
	close(pip[1]);
}
/*
 * Strip messagge of programmed headers
 */
striphdr() {
	int sstat;
	char buf[256];
	int gotc,gots;
	char *ptr;
	char c;
	gotc = gots = 0;
	for(;;) {
			ptr = &buf;
		for(;;) {
			sstat = read(fd.th,&c,1);
			if(sstat != 1) {
puts(1,"read not one\n");
				if(!gotc)
					write(pip[1],"\n",1);
				if(!gots)
					write(pip[1],"\n",1);
				return;
			}
			if((*ptr++ = c) == '\n')
				break;
		}
		*ptr++ = '\0';
		switch(buf[0]) {
		default:
			puts(pip[1],&buf);
			break;
		case 'C':
			gotc++;
		case 'T':
			puts(pip[1],&buf[4]);
		case '-':
			break;
		case 'S':
			gots++;
			if(!gotc) {
					gotc++;
					write(pip[1],"\n",1);
				}
			puts(pip[1],&buf[9]);
			break;
		}
	}
}
