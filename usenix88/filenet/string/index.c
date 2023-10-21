#include <stream.h>
#include "string.h"

extern void	bcopy (char*, char*, int);
extern int	strlen (char*);

const char	EOS = '\0';


int
string.index (char c)
{
    register char	*cpt;
    
    if ((cpt = ::index (pt, c)) == 0)
	return -1;
    else
	return (int) (cpt - pt);
}

int
string.rindex (char c)
{
    register char	*cpt;
    
    if ((cpt = ::rindex (pt, c)) == 0)
	return -1;
    else
	return (int) (cpt - pt);
}
