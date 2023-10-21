#include "uutty.h"
/* 
** The response string is believed to contain a login id.
** The id is copied to the global variable "userid", and
** its length returned.
*/
checkid(rp)
	char *rp;
{	char *p, *q;

	D4("checkid: r=\"%s\" ss=%d",rp,ss);
	for (p=rp; *p; ++p) {	/* Examine the chars for acceptability */
		switch(*p) {
		case 0x04:		/* EOT triggers a login prompt */
			Pwrite("login:");
			target = "?";
			ss = S_LOGIN;
			D4("State %d=%s",ss,gestate());
			return 1;
		case ' ':		/* Blanks aren't legal */
		case ':':		/* Colons aren't legal */
		case '\b':		/* Backspaces aren't legal */
		case '\t':		/* Tabs aren't legal */
			D3("ID with whitespace ignored.");
			ss = S_IDLE;
			D4("State %d=%s",ss,gestate());
			Fail;
		case '\0':		/* Assorted terminal chars */
		case '\r':
		case '\n':
			*p = 0;
			goto good;
		case '!':		/* Special goodie for killing daemon */
			if (p[1] == 'Q') {
				E("!Q in input; ");
				if (debug) P("%s: !Q in input, quitting [id]",getime());
				die(0);
			}
		default:
			continue;
		}
	}
good:			/* Make a copy of the supposed id */
	p = rp;
	q = userid;
	while (*p && q<userid+USERID)
		*q++ = *p++;
	*q = 0;
	if (debug >= 2) P("%s USERID=\"%s\"",getime(),userid);
	target = "logger";
	return (p-rp);
fail: return 0;
}
