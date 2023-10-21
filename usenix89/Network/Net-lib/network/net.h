/*
 * $Header: net.h,v 1.1 87/04/15 21:29:42 josh Exp $
 *
 * $Log:	net.h,v $
 * Revision 1.1  87/04/15  21:29:42  josh
 * Initial revision
 * 
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

struct {
	int imask[2],nmask[2];
	/* imask: mask of pipes that are interupt handled */
	/* nmask: mask of pipes handled by the user */

	int (*protab[64])();
	/* protab: The table of processes that handle pipes */

	} _netblk;

#define netblk _netblk

#define MYPORT	3000


#define naddfd(x,y)  (x[y>>5] |=  (1<< (y>31 ? y-32 : y)))
#define ndelfd(x,y)  (x[y>>5] &= ~(1<< (y>31 ? y-32 : y)))
#define nisset(x,y)  (x[y>>5] &   (1<< (y>31 ? y-32 : y)))
