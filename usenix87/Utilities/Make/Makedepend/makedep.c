/* -- Make dependency generator
 *
 * $File: makedep.c  1.3  1985-03-29  14:46:31 $
 *
 *  Copyright (c) 1983  Kim Walden
 *      ENEA DATA, Sweden and    
 *      SYSLAB, Department of Computer Science,
 *      University of Stockholm, Sweden.
 *
 *	This software may be used and copied freely,
 *	provided that proper credit is given to the originator
 *	by including the above text in each copy.
 *
 *	Descr:	This program implements part of the algorithm described
 *      	in K. Walden, "Automatic Generation of Make Dependencies",
 *      	Softw. Practice & Exper., vol. 14, no. 6, June 1984 
 * $Log:	makedep.c,v $
 * 
 * Revision 1.3  1985-03-29  14:46:31  kim
 * New source format
 * 
 * Revision 1.3  1985-03-29  14:11:11  kim
 * New source format
 * 
 * Revision 1.2  1985-03-08  01:18:53  kim
 * Added support for parallell transfer rules.
 * Added -1 option to makedep and depend.
 * 
 * Revision 1.1  1985-02-25  19:37:39  kim
 * Initial revision
 */

# define CMDSIZ  10000
# define OPTSIZ  1000
# define CNULL  '\0'

# include <strings.h>

static char usage[]=
"Usage: makedep [-1] [-rfile] [-Rrules] [-Ilist] [-alist] [-i] sourcefiles";

main(argc, argv)
	int		argc;
	char	*	argv[];
{
	char	*	n,
		*	d,
		*	s;
	char		buf[CMDSIZ],
			srcs[CMDSIZ],
			norm[OPTSIZ],
			dep[OPTSIZ];
	int 		vflag = 0;

	setprogname(argv[0]);

	s = srcs;
	n = norm;
	d = dep;
	*s = *n = *d = CNULL;

	while (argc > 1 && argv[1][0] == '-')
	{
		switch (argv[1][1])
		{
		 case 'v':
			vflag++;
			break;

		 case 'r':
		 case 'R':
			sprintf(d, "-%c\"%s\" ", argv[1][1], &argv[1][2]);
			d += strlen(d);
		 case 'I':
		 case 'a':
			sprintf(n, "-%c\"%s\" ", argv[1][1], &argv[1][2]);
			n += strlen(n);
			break;

		 case '1':
			sprintf(d, "-%c ", argv[1][1]);
			d += strlen(d);
			break;

		 case 'i':
			sprintf(n, "-%c ", argv[1][1]);
			n += strlen(n);
			break;

		 default:
			error("unknown option %s", argv[1]);
			fatalmesg(usage);
		}

		argv++;
		argc--;
	}

	if (argc < 2)
		fatal("no source files specified");

	while (--argc)
	{
		sprintf(s, "%s ", argv++[1]);
		s += strlen(s);
	}

	s = buf;
	sprintf(s, "grep '^#[ \t]*include' /dev/null %s | ", srcs);
	s += strlen(s);

	sprintf(s, "makenorm %s `pwd` %s %s | ", norm, srcs);
	s += strlen(s);

	sprintf(s, "depend %s\n", dep);

	if (vflag)
		printf("%s", buf);

	system(buf);
}
