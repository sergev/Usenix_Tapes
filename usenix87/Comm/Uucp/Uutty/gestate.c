#include "uutty.h"
/* 
** Look up the current state and return its symbolic name.
**
** The state[] array is the list of known states.  Note that 
** we assume that states are non-negative, and use -1 as an 
** end-of-array flag.  Note also that zero is a highly-likely 
** state, and should be included as a distinct state (probably 
** S_INIT).  The order is irrelevant, except for speed; a linear 
** lookup is done.
*/
struct state {int ssval; char *ssnam; } state[] = {
	{S_EXIT,	"EXIT"},
	{S_IDLE,	"IDLE"},
	{S_INIT,	"INIT"},
	{S_LOGIN,	"LOGIN"},
	{S_PASSWD,	"PASSWD"},
	{-1,		"UNKNOWN"}
};
char *
gestate()
{	int i;

	for (i=0; state[i].ssval >= 0; i++)
		if (state[i].ssval == ss)
			return state[i].ssnam;
	return state[i].ssnam;			/* Final "unknown" state */
}
