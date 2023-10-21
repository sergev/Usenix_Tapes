#ifndef lint
static char rcsid[] = "$Header: mkprog.c,v 1.3 87/03/02 17:55:39 root Exp $";
static char rcswhere[] = "$Source: /usr/src/local/local/mkprog/RCS/mkprog.c,v $";
#endif

#include "mkprog.h"

char h_file[256];	/* name of .h file, used in constant.c */

main(argc, argv)
int argc;
char *argv[];
{
    int c;
    FILE *fp_p, *fp_h, *efopen();
    char p_file[256];	/* name of .c file */

    /*  The following symbols are defined in the Makefile */

    t_val = TABS;
    R_flag = RCS;
    S_flag = SCCS;

    n_string = "prog";
    progname = argv[0];
    opterr = 1;
    while ((c = getopt(argc, argv, "RSfhn:o:p:t:")) != EOF)
	switch(c) {
	case'R':
	    R_flag = true;
	    break;
	case'S':
	    S_flag = true;
	    break;
	case'f':
	    f_flag = true;
	    break;
	case'h':
	    h_flag = true;
	    break;
	case'n':
	    n_string = optarg;
	    break;
	case'o':
	    o_string = optarg;
	    break;
	case't':
	    t_val = atoi(optarg);
	    break;
	case '?':
	    usage();
	    break;
	}

    argc -= optind;
    argv += optind;

    if (argc != 0)
	usage();

    (void) sprintf(p_file, "%s%s", n_string, ".c");
    fp_p = efopen(p_file, "w");

    if (h_flag) {
	(void) sprintf(h_file, "%s%s", n_string, ".h");
	fp_h = efopen(h_file, "w");
    }
    else
	fp_h = fp_p;	/* "header" stuff gets written into .c file */

    do_mk_prog(fp_h, fp_p);
}

usage()
{
    fprintf(stderr, "Usage: %s [ -RSfh ] [ -t t_val ] [ -n n_string ] [ -o o_string ]\n", progname);
}
