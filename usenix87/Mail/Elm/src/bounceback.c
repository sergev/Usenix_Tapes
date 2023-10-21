/**			bounceback.c			**/

/** This set of routines implement the bounceback feature of the mailer.
    This feature allows mail greater than 'n' hops away (n specified by
    the user) to have a 'cc' to the user through the remote machine.  

    Due to the vagaries of the Internet addressing (uucp -> internet -> uucp)
    this will NOT generate bounceback copies with mail to an internet host!

    (C) Copyright 1986 by Dave Taylor
**/

#include "headers.h"

char *bounce_off_remote(),		/* forward declaration */
     *strcat(), *strcpy();

int
uucp_hops(to)
char *to;
{	
	/** Given the entire "To:" list, return the number of hops in the
	    first address (a hop = a '!') or ZERO iff the address is to a
  	    non uucp address.
	**/

	register int hopcount = 0, index;

	for (index = 0; ! whitespace(to[index]) && to[index] != '\0'; index++) {
	  if (to[index] == '!')
	    hopcount++;
	  else if (to[index] == '@' || to[index] == '%' || to[index] == ':')
	    return(0);	/* don't continue! */
	}

	return(hopcount);
}
	
char *bounce_off_remote(to)
char *to;
{
	/** Return an address suitable for framing (no, that's not it...)
	    Er, suitable for including in a 'cc' line so that it ends up
	    with the bounceback address.  The method is to take the first 
	    address in the To: entry and break it into machines, then 
	    build a message up from that.  For example, consider the
	    following address:
			a!b!c!d!e!joe
	    the bounceback address would be;
			a!b!c!d!e!d!c!b!a!ourmachine!ourname
	    simple, eh?
	**/

	static char address[LONG_STRING];	/* BEEG address buffer! */

	char   host[MAX_HOPS][SHORT_SLEN];	/* for breaking up addr */
	register int hostcount = 0, hindex = 0, 
	       index;

	for (index = 0; !whitespace(to[index]) && to[index] != '\0'; index++) {
	  if (to[index] == '!') {
	    host[hostcount][hindex] = '\0';
	    hostcount++;
	    hindex = 0;
	  }
	  else 
	    host[hostcount][hindex++] = to[index];
	}

	/* we have hostcount hosts... */

	strcpy(address, host[0]);	/* initialize it! */

	for (index=1; index < hostcount; index++) {
	  strcat(address, "!");
	  strcat(address, host[index]);
	}
	
	/* and now the same thing backwards... */

	for (index = hostcount -2; index > -1; index--) {
	  strcat(address, "!");
	  strcat(address, host[index]);
	}

	/* and finally, let's tack on our machine and login name */

	strcat(address, "!");
	strcat(address, hostname);
	strcat(address, "!");
	strcat(address, username);

	/* and we're done!! */

	return( (char *) address );
}
