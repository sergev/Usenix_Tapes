#include <strings.h>
#include <stream.h>
#include "string.h"

extern void	bcopy (char*, char*, int);
extern int	strlen (char*);

const char	EOS = '\0';


void string.operator= (string& s)
{
    delete pt;
    pt = new char[s.len + 1];
    len = s.len;
    bcopy (s.pt, pt, len);
    pt[len] = EOS;
}

string operator+ (string& a, string& b)
{
    string  x;

    x.len = a.len + b.len;
    x.pt = new char[x.len + 1];
    bcopy (a.pt, x.pt, a.len);
    bcopy (b.pt, x.pt + a.len, b.len);
    x.pt[x.len] = EOS;
    return x;
}

void string.operator+= (string& s)
{
    register string	    temp = *this;

    len = temp.len + s.len;
    delete pt;
    pt = new char[len + 1];
    bcopy (temp.pt, pt, temp.len);
    bcopy (s.pt, pt + temp.len, s.len);
    pt[len] = EOS;
}

ostream& operator<< (ostream& o, string& s)
{
    return o << s.pt;
}

istream& operator>> (istream& i, string& s)
{
    char	temp[128];

    cin >> temp;
    s = string (temp);
    return i;
}

int
operator== (string& a, string& b)
{
    return strcmp (a.pt, b.pt) == 0;
}

int
operator!= (string& a, string& b)
{
    return strcmp (a.pt, b.pt) != 0;
}

int
operator> (string& a, string& b)
{
    return strcmp (a.pt, b.pt) > 0;
}

int
operator< (string& a, string& b)
{
    return strcmp (a.pt, b.pt) < 0;
}

int
operator>= (string& a, string& b)
{
    return strcmp (a.pt, b.pt) >= 0;
}

int
operator<= (string& a, string& b)
{
    return strcmp (a.pt, b.pt) <= 0;
}
