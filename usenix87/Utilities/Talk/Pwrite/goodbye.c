
#include "header.h"

goodbye()
{
	int	a;

	for (a=0; a < NUM_T; a++)
	{
		if (T[a].__sock >= 0 && !T[a].__dead)
			s_delete(T[a].__sock);
	}

	unlink(T[0].__file);
	do_line("");
	nocrmode(); echo();
	endwin();

	exit(1);
}

