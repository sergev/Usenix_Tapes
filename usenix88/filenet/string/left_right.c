#include <stream.h>
#include "string.h"

extern void	bcopy (char*, char*, int);
extern int	strlen (char*);


string
string.left (int newlen)
{
    string	    newstr;

    if (newlen <= len)
	return *this;

    newstr.len = newlen;
    newstr.pt = new char[newlen + 1];
    strcpy (newstr.pt, pt);

    for (char* ptr = &newstr.pt[len]; ptr < &newstr.pt[newstr.len]; ptr++) {
	*ptr = ' ';
    }
    return newstr;
}

string
string.right (int newlen)
{
    string	    newstr;
    register char   *ptr;

    if (newlen <= len)
	return *this;

    newstr.len = newlen;
    newstr.pt = new char[newlen + 1];
    for (ptr = newstr.pt; ptr < &newstr.pt[newlen - len]; ptr++)
	*ptr = ' ';
    strcpy (ptr, pt);
    return newstr;
}
