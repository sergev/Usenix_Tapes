
#include "header.h"

#define	VERSION			"2.0"

/***********************************************************************
 *
 *	____
 *	|   \               o   |	     A multi-user communication
 *	|___/  _  _  _ __  _.  _|_    ___    program by:
 *	|      |  |  |/     |   |    /___>
 *	|      |/\|  |     _|_  |_/  \___	Robert Scott Puchyr,
 *						Dalhousie University,
 *						Halifax, Nova Scotia,
 *						Canada,
 *						April 9, 1985
 *
 ***********************************************************************/

main(argc,argv)
int	argc;
char	**argv;
{
	printf("Pwrite. version %s.\nPress ESC ? for help.\n\n",VERSION);
	hello();
	do_pwrite();
	goodbye();
}

do_pwrite()
{
	while (1)
	{
		sock_io();
		if (NUM_C < NUM_T)
			p_beg();
	}
}

sock_io()
{
	char	S[BUFSIZ];
	int a, i, m;

	m = s_select(ACTIVE_MASK);

	if (m & 1)
	{
		read(0,&a,1);
		eval_key(a);
	}

	if (m <= 1)
		return(0);

	if (m & (1 << T[0].__sock))
	{
		p_answer();
	}

	for (a=1; a < NUM_T; a++)
	{
		if (m & (1 << T[a].__sock) && T[a].__connected)
		{
			i = read(T[a].__sock,S,BUFSIZ);
			if (i <= 0)
			{
				sprintf(STRING,"%c\n\"%s\" LOS \"%s\".%c",BEGIN_SECRET,T[0].__tty,T[a].__tty,END_SECRET);
				p_delete(a);
				p_sendstr(STRING);
			}
			else
			{
				S[i] = NULL;
				w_writestr(a,S);
			}
		}
	}
}

