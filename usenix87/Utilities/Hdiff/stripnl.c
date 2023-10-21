/*
 * f=stripnl.c
 * author - dennis bednar 8 31 84
 *
 * this routine is designed to work in conjunction with fgets ().
 * In this case, the L_BUFOVER error should never be returned, but
 * the check is there anyway!
 *
 * strip new lines by converting the newline in the string to a null
 * returns
 *	L_BUFOVER = buffer is overfilled.  For example if bufsize = 10, then
 *		the string len could be at most 9 (so the NULL fits)
 *	L_TOOLONG = no newline was found (line too long) and buffer was filled.
 *		For a bufsize of 10, buffer[9] is the last char, and it's
 *		not the expected newline.
 *	L_BADFORM = no newline was found and buffer not filled, so bad format
 *	L_SUCCESS = success
 */

#include "stripnl.h"

stripnl (buffer, bufsize)
	char	*buffer;
	int	bufsize;
{
	char	*cp;
	int	len;	/* number of chars in string */


	/* get string length */
	len = strlen (buffer);

	/* make sure it's not already overrun */
	if (len >= bufsize)
		return (L_BUFOVER);

	/* if "" is passed, len is zero, and so 'last char' doesn't exist */
	if (len <= 0)
		return (L_BADFORM);

	/* now the string definitely fits in the buffer */

	/* get pointer to last char in string */
	cp = &buffer [len - 1];

	/* if the last char of the string is a newline, change it to a NULL */
	if (*cp == '\n')
		{
		*cp = '\0';
		return L_SUCCESS;
		}

	/* now the string fits in the buffer, and the last char != '\n' */

	/* line too long if the string length is the buffer size minus
	 * one for the null.
	 */
	else if (len >= bufsize-1)
		return L_TOOLONG;

	/* badly formatted if the line fits in the buffer, but
	 * contains no newline at the end.
	 * This happens on a file which is corrupted by, say,
	 * a transmission error.
	 */
	else
		return L_BADFORM;
}
