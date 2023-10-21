#include "uutty.h"
/* 
** We seem to have gotten a shell prompt, but it's kinda hard to be sure.
** This routine is somewhat of a relic in uutty, but has been left here
** because you might want to deal with this case.  What we try to do
** basically is to get the shell to logout.
*/
shprompt(rp) char *rp;
{
	D5("shprompt(\"%s\")",rp);
	target = "shell";
	switch(ss) {
	case S_INIT:
	case S_PASSWD:
	default:
		E("****Can't handle shell prompt \"%s\" in state %d=%s.",rp,ss,gestate());
		Awrite("exit\r");		/* This terminates most shells */
		Awrite("logout\r");		/* This terminates other shells */
		Awrite("\4\3");			/* This works with still others */
		if (m_exit) Awrite(m_exit);	/* Other optional exit message */
		if (m_init) Awrite(m_init);	/* Try to tell the modem to quit */
		ss = S_IDLE;
		D4("State %d=%s",ss,gestate());
	}
}
