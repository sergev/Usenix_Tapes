#ifndef lint
static char rcsid[] = "$Header: do_mkprog.c,v 1.4 87/03/02 17:54:52 root Exp $";
static char rcswhere[] = "$Source: /usr/src/local/local/mkprog/RCS/do_mkprog.c,v $";
#endif

/*  do_mkprog.c contains those parts of the program that depend on
 *  the data representation used.  On the other hand, constant.c
 *  contains material that is independent of the data representation
 *  and, in almost all cases, of the actual data themselves.
 */

#include "mkprog.h"
#define MAX_OPTS 20

char b_opts[MAX_OPTS], i_opts[MAX_OPTS], s_opts[MAX_OPTS];

do_mk_prog(fp_h, fp_p)
FILE *fp_h, *fp_p;
{
    bool args;			/* any string or int args? */

    if (o_string)
	mk_arrays();		/* make b_opts[] etc and check on duplicates */

    top(fp_h, fp_p);		/* boiler plate at top */

    if (o_string)
	mk_decl(fp_h);		/* external declarations */

    if (i_opts[0] != '\0' || s_opts[0] != '\0')
	args = true;
    else
	args = false;
    middle(fp_p, args);		/* main(argc, argv) ... etc. */

    if (o_string)
	mk_switch(fp_p);	/* switch statement */

    if (!f_flag)
	bottom(n_string, fp_p);	/* file processing statements */
    else {
#ifdef undefined
	out(fp_p, 0, "\n");
#endif undefined
	out(fp_p, 1, "do_%s();\n", n_string);
	out(fp_p, 1, "exit(0);\n");
	out(fp_p, 0, "}\n");
    }
    mk_usage(fp_p);
}

/* break up the options in *o_string into three arrays: b_opt for
 * the binary options, i_opt for the integers, and s_opt for the strings,
 * then sort each of these arrays.
 */

mk_arrays()
{
    char *b_ptr = b_opts;
    char *i_ptr = i_opts;
    char *s_ptr = s_opts;
    char *cp;
    char *index();

    for (cp = o_string; *cp; cp++) {
	if ((index(b_opts, *cp) != (char *) 0)  ||
	    (index(i_opts, *cp) != (char *) 0)  ||
	    (index(s_opts, *cp) != (char *) 0)) {
		fprintf(stderr, "%s: '%c' found twice in options string\n",
		  progname, *cp);
		exit(1);
	}
	switch (*(cp + 1)) {
	case '$':
	    *s_ptr++ = *cp++;
	    *cp = ':';
	    break;
	case '#':
	    *i_ptr++ = *cp++;
	    *cp = ':';
	    break;
	default:
	    *b_ptr++ = *cp;
	    break;
	}
    }
    
    *b_ptr = *i_ptr = *s_ptr = '\0';

    /*  Then sort the strings made */

    strsort(b_opts);
    strsort(i_opts);
    strsort(s_opts);

    /* then put the options back into o_string in order:
     * binaries, then integers, then strings, each group
     * sorted.
     */

    cp = o_string;

    b_ptr = b_opts;
    while (*cp++ = *b_ptr++)
	;
    cp--;

    i_ptr = i_opts;
    while (*cp++ = *i_ptr++)
	*cp++ = ':';
    cp--;

    s_ptr = s_opts;
    while (*cp++ = *s_ptr++)
	*cp++ = ':';
    cp--;
}

/* use the sorted arrays to make the external declarations.  They
 * are arranged in order: booleans then integers than character pointers,
 * sorted alphabetically within each category.
 */

mk_decl(fp_h)
FILE *fp_h;
{
    char *cp;

    putc('\n', fp_h);

    for (cp = b_opts; *cp; cp++)
	out(fp_h, 0, "bool %c_flag;\n", *cp);
    for (cp = i_opts; *cp; cp++)
	out(fp_h, 0, "int %c_val;\n", *cp);
    for (cp = s_opts; *cp; cp++)
	out(fp_h, 0, "char *%c_string;\n", *cp);
}

mk_switch(fp_p)	/* make switch in same order, precede by initializing */
FILE *fp_p;
{
    char *cp;

    for (cp = i_opts; *cp; cp++)
	out(fp_p, 1, "%c_val = 0;\n", *cp);
    for (cp = s_opts; *cp; cp++)
	out(fp_p, 1, "%c_string = (char *) 0;\n", *cp);
    putc('\n', fp_p);

    out(fp_p, 1, "while ((c = getopt(argc, argv, \"%s\")) != EOF)\n", o_string);
    out(fp_p, 2, "switch(c) {\n");

    for (cp = b_opts; *cp; cp++) {
	out(fp_p, 2, "case '%c':\n", *cp);
	out(fp_p, 3, "%c_flag = true;\n", *cp);
	out(fp_p, 3, "break;\n");
    }

    for (cp = i_opts; *cp; cp++) {
	out(fp_p, 2, "case '%c':\n", *cp);
	out(fp_p, 3, "%c_val = atoi(optarg);\n", *cp);
	out(fp_p, 3, "break;\n");
    }

    for (cp = s_opts; *cp; cp++) {
	out(fp_p, 2, "case '%c':\n", *cp);
	out(fp_p, 3, "%c_string = optarg;\n", *cp);
	out(fp_p, 3, "break;\n");
    }

    out(fp_p, 2, "case '?':\n");
    out(fp_p, 3, "usage();\n");
    out(fp_p, 3, "break;\n");
    out(fp_p, 2, "}\n\n");
}

/*  Simple string sort, sorts a string in situ.  For current
 *  purpose simple interchange is all we need.
 */

strsort(s)
char *s;
{
    char *out_ptr, *in_ptr, tmp;

    if (s == (char *) 0 || *s == '\0')
	return;

    for (out_ptr = s + 1; *out_ptr; out_ptr++)
	for (in_ptr = out_ptr - 1; *in_ptr > *(in_ptr + 1) && in_ptr >= s; in_ptr--) {
	    tmp = *in_ptr;
	    *in_ptr = *(in_ptr + 1);
	    *(in_ptr + 1) = tmp;
	}
}

mk_usage(fp_p)	/* make usage() with options and files, as appropriate. */
FILE *fp_p;
{
    char *cp;

    out(fp_p, 0, "\nusage()\n{\n");
    out(fp_p, 1, "fprintf(stderr, \"Usage: %%s");

    if (*b_opts) {
	fputs(" [ -", fp_p);
	for (cp = b_opts; *cp; cp++)
	    putc(*cp, fp_p);
	fputs(" ]", fp_p);
    }

    if (*i_opts)
	for (cp = i_opts; *cp; cp++)
	    fprintf(fp_p, " [ -%c %c_val ]", *cp, *cp);

    if (*s_opts)
	for (cp = s_opts; *cp; cp++)
	    fprintf(fp_p, " [ -%c %c_string ]", *cp, *cp);

    if (!f_flag)
	fprintf(fp_p, " [ file ... ]");
    fprintf(fp_p, "\\n\", progname);\n");

    out(fp_p, 1, "exit(1);\n}\n");
}
