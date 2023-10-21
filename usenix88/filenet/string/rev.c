#include <stream.h>
#include "string.h"

main (int argc, char *argv[])
{
    if (argc != 2) {
	cerr << "usage: rev {file}\n";
	exit (1);
    }

    filebuf	f;
    if (f.open (argv[1], input) == NULL) {
	cerr << "Cannot open " << argv[1] << "\n";
	exit (1);
    }
    istream	ins = &f;

    string	in, out;
    char	inbuf[200];
    
    while ( ins.get (inbuf, sizeof inbuf) ) {
	f.snextc ();		/* skip the \n at the end of the line */
	in = inbuf;
	out = "";
	for (register int i = in.strlen () - 1; i >= 0; i--) {
	    out += in (i, i);
	}
	cout << out << "\n";
    }

    f.close ();
    exit (0);
}
