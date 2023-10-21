#include <stream.h>
#include <strings.h>		/*  the old 4.2 BSD string subroutines */

#ifndef string_X

#define string_X


class string {
    char	    *pt;
    int		    len;
public:
			string (string&);
			string (int);
			string (char*);
			string (double);
			string () { pt = 0; len = 0; }
    inline		~string () { delete pt; }

    void		operator= (string&);
    void		operator+= (string&);
    string		operator() (int, int =-1);
    void		operator() (int, int, string&);
    friend string	operator+ (string&, string&);
    friend ostream&	operator<< (ostream&, string&);
    friend istream&	operator>> (istream&, string&);
    friend int		operator== (string&, string&);
    friend int		operator!= (string&, string&);
    friend int		operator> (string&, string&);
    friend int		operator< (string&, string&);
    friend int		operator>= (string&, string&);
    friend int		operator<= (string&, string&);
    
    inline int		strlen () { return len; }
    inline char*	charptr () { return pt; }
    int			index (char);
    int			rindex (char);
    string		left (int);
    string		right (int);
};

#endif string_X
