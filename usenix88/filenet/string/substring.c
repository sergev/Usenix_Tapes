#include <stream.h>
#include "string.h"

extern void	bcopy (char*, char*, int);
extern int	strlen (char*);

const char	EOS = '\0';


string string.operator() (int left, int right)
{
    string	sub;
    
    if (right > len  ||  right == -1)
	right = len - 1;
    if (left < 0  ||  left > right)
	left = 0;
    sub.len = right - left + 1;
    sub.pt = new char[sub.len + 1];
    bcopy (&pt[left], sub.pt, sub.len);
    sub.pt[sub.len] = EOS;
    return sub;
}

void string.operator() (int left, int right, string& s)
{
    /* 
     * Assigning to a substring.
     */
    if (this -> strlen () != s.strlen ()) {
	string	    temp = *this;

	*this = temp (0, left - 1)
		+ s
		+ temp (right + 1, temp.strlen () - 1);
    }
    else {
	if (right > len  ||  right == -1)
	    right = len - 1;
	if (left < 0  ||  left > right)
	    left = 0;
	bcopy (s.pt, &pt[left], right - left + 1);
    }
}
