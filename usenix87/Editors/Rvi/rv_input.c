#include "rv.h"

#ifdef CRAY
#define cmd_countptr   c_countptr
#endif

/*
 * Read a command character, possibly prefixed with a count.
 *    c               - Command character
 *    cmd_count       - Command count, default 1.
 *
 *    Returns TRUE if an explicit count was specified
 */
boolean read_cmd(cptr, cmd_countptr)
INT  *cptr;
INT  *cmd_countptr;
{
	register	INT	c;
	register	INT	cmd_count;
	boolean		specified_count;

	cmd_count = 0;
	specified_count = FALSE;
	do { 
		c = getch();
		if (c == ERR || c == EOF)
			quit();
				
		if (c != '0') /* '0' is a command! */
			while (c >= '0' && c <= '9') {
				specified_count = TRUE;
				cmd_count = cmd_count * 10 + c - '0';
				c = getch();
			}
		if (c == '\"') {
			yank_cmd = getch();
			(void) char_to_yank(yank_cmd);
			if (errflag) /* If bad "x yank letter */
				break;
		}
	} while (c == '\"');

	if (!specified_count)
		*cmd_countptr = 1;  /* default */
	else
		*cmd_countptr = cmd_count;
	*cptr = c;
	return(specified_count);
}
