#include <stdio.h>
/*
 * Routines to send and receive on sockets.  Four bytes of length are
 * sent, followed by the null terminated string.
 *
 */
void sockread(s, buf)
int s;				/* socket to talk on */
char *buf;			/* string to send */
{
	int nbytes;

	(void) read(s, (char *) &nbytes, sizeof(int));
	nbytes = htons(nbytes);
	(void) read(s, buf, nbytes);
}


void sockwrite(s, buf)
int s;				/* socket to talk on */
char *buf;			/* string to read on */
{
	int nbytes;

	nbytes = htons(strlen(buf) + 1);
	(void) write(s, (char *) &nbytes, sizeof(int));
	(void) write(s, buf, nbytes);
}
