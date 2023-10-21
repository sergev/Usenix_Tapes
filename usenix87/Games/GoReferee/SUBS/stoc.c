#include	"../godef.h"

/*
**	STOC, CTOS, LTON -- Miscellaneous conversions
*/

char	*letters	= "?ABCDEFGHJKLMNOPQRST???";

extern	char    *lfs();
struct mvstr	{
	int	m_code;
	char	*m_text;
};
static	struct	mvstr	mv[]	= {
	PASS,		"pass\n",
	RESIGN,		"resign\n",
	BEBLACK,	"black\n",
	BEWHITE,	"white\n",
	BYOROMI,	"byo-romi\n",
	0
};

char	*
stoc(s) /* Turn the spot number form of a move into the character form */
{
	static char buf[32];
	register int i;

	for (i = 0; mv[i].m_code; i++)
	    if (s == mv[i].m_code)
		return(lfs(mv[i].m_text));
	sprintf(buf, "%c%d", letters[s % BSZ2], s / BSZ2);
	return(buf);
}

ctos(mp) /* Turn the character form of a move into the spot number form */
char	*mp;
{
	register int i;

	for (i = 0; mv[i].m_code; i++)
	    if (strcmp(mp, mv[i].m_text) == 0)
		return(mv[i].m_code);
	return(lton(mp[0]) + BSZ2 * atoi(&mp[1]));
}

lton(c)		/* turn a letter into a number */
char	c;
{
	register int i;

	if (c < 'A')
	    return(0);
	for (i = 1; letters[i] && letters[i] != c; i++);
	return(i);
}
