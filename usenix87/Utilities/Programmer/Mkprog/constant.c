#ifndef lint
static char rcsid[] = "$Header: constant.c,v 1.3 87/03/02 17:51:28 root Exp $";
static char rcswhere[] = "$Source: /usr/src/local/local/mkprog/RCS/constant.c,v $";
#endif

/*  This file contains top(), middle() and bottom().
 *  The first two print out constant boilerplate, while bottom()
 *  prints out constant stuff plus one line with the name of
 *  the program being made.
 *
 *  It also contains out(), the function that actually prints the
 *  lines with a suitable indent.  It, too, is independent of the
 *  options and of the data structure used for them
 */

#include "mkprog.h"

top(fp_h, fp_p)
FILE *fp_h, *fp_p;
{
    extern char h_file[];

    if (R_flag || S_flag) {		/* rcsid and sccsid headers in prog */
	out(fp_p, 0, "#ifndef lint\n");
	if (R_flag)
	    out(fp_p, 0, "static char rcsid[] = \"$Header$\";\n");
	if (S_flag)
	    out(fp_p, 0, "static char sccsid[] = \"$Header$\";\n");
	out(fp_p, 0, "#endif\n\n");

	if (fp_h != fp_p) {		/* and in header */
	if (R_flag)
	    out(fp_h, 0, "/*\t \"$Header$\"\t*/\n");
	if (S_flag)
	    out(fp_h, 0, "/*\t \"$Header$\"\t*/\n");
	}
    }

    if (h_flag)
	out(fp_p, 0, "#include \"%s\"\n", h_file);

    out(fp_h, 0, "#include <stdio.h>\n");
    out(fp_h, 0, "#include <ctype.h>\n");
    out(fp_h, 0, "\n");
    if (o_string) {
	out(fp_h, 0, "typedef enum {false=0, true=1} bool;\n");
	out(fp_h, 0, "\n");
    }
    out(fp_h, 0, "char *progname;\n");
}

middle(fp_p, args)
bool args;
FILE *fp_p;
{
    out(fp_p, 0, "\nmain(argc, argv)\n");
    out(fp_p, 0, "int argc;\n");
    out(fp_p, 0, "char *argv[];\n");
    out(fp_p, 0, "{\n");
    if (!f_flag)
	out(fp_p, 1, "extern int optind;\n");
    out(fp_p, 1, "extern int opterr;\n");
    if (args)
	out(fp_p, 1, "extern char *optarg;\n");
    if (o_string)
	out(fp_p, 1, "int c;\n");
    if (!f_flag) {
	out(fp_p, 1, "int i;\n");
	out(fp_p, 1, "bool used_stdin = false;\n");
	out(fp_p, 1, "FILE *fp, *efopen();\n");
    }
    out(fp_p, 0, "\n");
    out(fp_p, 1, "opterr = 1;\n");
    out(fp_p, 1, "progname = argv[0];\n\n");
}

bottom(progname, fp_p)
char *progname;
FILE *fp_p;
{
    if (o_string) {
	out(fp_p, 1, "argc -= optind;\n");
	out(fp_p, 1, "argv += optind;\n");
    }
    else {
	out(fp_p, 1, "argc--;\n");
	out(fp_p, 1, "argv++;\n");
    }
    out(fp_p, 0, "\n");
    out(fp_p, 1, "for (i = 0; i < argc; i++) {\n");
    out(fp_p, 2, "if (strcmp(argv[i], \"-\") == 0) {\n");
    out(fp_p, 3, "if (used_stdin) {\n");
    out(fp_p, 4, "fprintf(stderr, \"standard input used twice\\n\");\n");
    out(fp_p, 4, "exit(1);\n");
    out(fp_p, 3, "}\n");
    out(fp_p, 3, "else {\n");
    out(fp_p, 4, "used_stdin = true;\n");
    out(fp_p, 3, "}\n");
    out(fp_p, 2, "}\n");
    out(fp_p, 0, "#ifdef unix\n");
    out(fp_p, 2, "else if (access(argv[i], 4) == -1) {\n");
    out(fp_p, 3, "fprintf(stderr, \"%%s: cannot access %%s: \", progname, argv[i]);\n");
    out(fp_p, 3, "perror(\"\");\n");
    out(fp_p, 3, "exit(1);\n");
    out(fp_p, 2, "}\n");
    out(fp_p, 0, "#endif unix\n");
    out(fp_p, 1, "}\n");
    out(fp_p, 0, "\n");
    out(fp_p, 1, "fp = stdin;\n");
    out(fp_p, 1, "i = 0;\n");
    out(fp_p, 0, "\n");
    out(fp_p, 1, "do {\t\t\n");
    out(fp_p, 2, "if (argc > 0)\n");
    out(fp_p, 3, "fp = efopen(argv[i], \"r\");\n");
    out(fp_p, 0, "\n");
    out(fp_p, 2, "do_%s(fp);\n", progname);
    out(fp_p, 2, "(void) fclose(fp);\n");
    out(fp_p, 1, "} while (++i < argc);\n");
    out(fp_p, 1, "exit(0);\n");
    out(fp_p, 0, "}\n");
}

	/* VARARGS3 */

out(fp, n, s1, s2, s3, s4, s5)
FILE *fp;
int n;
char *s1, *s2, *s3, *s4, *s5;
{
    int n_tabs, n_spaces;

    n_spaces = n*t_val;

    n_tabs = n_spaces/8;
    while (n_tabs--)
	putc('\t', fp);

    n_spaces %= 8;
    while (n_spaces--)
	putc(' ', fp);
    fprintf(fp, s1, s2, s3, s4, s5);
}
