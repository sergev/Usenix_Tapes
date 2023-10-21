#include <stream.h>
#include "string.h"

extern void	bcopy (char*, char*, int);
extern int	strlen (char*);

const char	EOS = '\0';


string.string (char* value)
{
    len = ::strlen (value);
    pt = new char[len+1];
    bcopy (value, pt, len);
    pt[len] = EOS;
}

string.string (string& s)
{
    len = s.len;
    pt = new char[len + 1];
    bcopy (s.pt, pt, len);
    pt[len] = EOS;
}

string.string (int a)
{
    len = ::strlen (dec (a));
    pt = new char[len+1];
    bcopy (dec (a), pt, len);
    pt[len] = EOS;
}

string.string (double a)
{
    len = ::strlen (form ("%g", a));
    pt = new char[len+1];
    bcopy (form ("%g", a), pt, len);
    pt[len] = EOS;
}
