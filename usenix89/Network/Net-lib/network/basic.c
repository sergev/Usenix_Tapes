#ifndef lint
static char *RCSid = "$Header: basic.c,v 1.1 87/04/15 21:28:59 josh Exp $";
#endif

/*
 *------------------------------------------------------------------
 * Copyright 1987, Josh Siegel
 * All rights reserved.
 *
 * Josh Siegel (siegel@hc.dspo.gov)
 * Dept of Electrical and Computer Engineering,
 * University of New Mexico,
 * Albuquerque , New Mexico
 * (505) 277-2497
 *
 *------------------------------------------------------------------
 *
 * $Source: /usr/pcb/josh/other/network/RCS/basic.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:28:59 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	basic.c,v $
 * Revision 1.1  87/04/15  21:28:59  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


extern char *user_errlist[];
extern int errno;

#include "net.h"

pkill(fd)
int fd;
{
	shutdown(fd,2);
	if(close(fd)<0) 
		return(-1);
}

char *useport(port)
int port;
{
	static char buff[8];

	sprintf(buff,"#%d",port);

	return(buff);
}

/* Myport:  I wrote this so that if 100 students are using the routines,
	    they have a way of avoiding using the same socket address.
	    This uses getuid() to figure out what to choose as a 
	    port.

 Problems:  Could this be done cleaner ?
*/

char *myport()
{
	static char buff[8]; /* Its static! Don't try to free it */ 

	sprintf(buff,"#%d",MYPORT+getuid());
 
        return(buff); 
}

/*
attach:
	This is used to establish a port to which other processes
	can connect.  It is very standard code except for the
	useport() and myport() hacks.

*/

attach(s)
	char            s[];
{
	int             f,*p;
	struct sockaddr_in sin;
	struct servent *sp;
	char buff[255];

	bzero((char *) &sin,sizeof(sin));

        /* Is this something passed to us from useport and myport? */

	if(s[0]=='#') {
		sin.sin_port = htons(atoi(s+1));
	} else {
		sp = getservbyname(s, "tcp");
		if (sp == NULL)  {
			user_errlist[0]="service unknown";
			errno=255;
			return(-1);
		}
		sin.sin_port = sp->s_port;
	}

	f = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(f, (struct sockaddr *) & sin, sizeof(sin)) < 0) {
		return(-1);
	}
	listen(f, 5);
	return (f);
}


/* 
Answer: Excepts connection requests from clients.

Problem: I could make it make note of where the connection came
	 from but I never felt I had a need 

*/
	 
answer(s)
	int             s;
{
	int             tmp;
	struct sockaddr_in from;
	int             len = sizeof(from);

	bzero((char *) & from,len);

	tmp = accept(s, &from, &len);
	return (tmp);
}
/*
Phone:  Connects to a established port setup by accept().  Again,
	very clean code except for the useport() and myport()
	hacks.

Things to do:
	I could try to make this function call a interupt handler
	when connected so that it does not block for 30 or 40 seconds
	if the machine is down or something */

phone(service, host)
	char            service[], host[];
{

	struct sockaddr_in sin;
	struct servent *sp;
	struct hostent *hp;
	int             s,*p;
	char buff[255];

	bzero((char *) &sin, sizeof(sin));

	if(service[0]!='#') {
		sp = getservbyname(service, "tcp");
		if (sp == NULL)  {
			user_errlist[0]="service unknown";
			errno=255;
			return(-1);
		}
		sin.sin_port = sp->s_port;
	} else {
		/* Lets try to figure out something better... Ok? */

		sin.sin_port = htons(atoi(service+1));
	}

	hp = gethostbyname(host);
	if(hp==NULL) {
			errno=65;
			return(-1);
	}
	bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) 
		return(-1);

	if (connect(s, &sin, sizeof(sin)) < 0) 
		return(-1);
	

	return (s);
}

/* Give them users the ability to choose what they wish to block! */

addfd(x)
int x;
{
	naddfd(netblk.nmask,x);
}
delfd(x)
int x;
{
	ndelfd(netblk.nmask,x);
}
initfd()
{
	netblk.nmask[0]=0;
	netblk.nmask[1]=0;
}

/*
block:  Blocks until their is something in the Que waiting
	to be read in. Remembers old stuff and goes in a
	round robin mannor of handling stuff on sockets.
	This means that if the user reads from a socket
	that I have not returned but will it a little
	while, this program will be lying!  In general,
	the use of isignal() can over come many if not
	all of these problems. Infact,  block might not
	even be needed anymore.  I sure never use it.
*/

block()
{
	static          long flags[2] ;
	static int first=1;
	int             n;

	
	if(first) {
		flags[0]=0;
		flags[1]=0;
		first=0;
	}

	if (!flags[0] || !flags[1]) {
		flags[0] = netblk.nmask[0];
		flags[1] = netblk.nmask[1];
		select(64, flags, 0, 0, 0);
	}
	for (n = 0; n < 64; n++)
		if (nisset(flags,n)) {
			naddfd(flags,n);
			return (n);
		}
	return (-1);

}
