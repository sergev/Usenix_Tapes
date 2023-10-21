/*	 "$Header: mkprog.h,v 1.1 86/05/19 16:32:46 root Exp $"	*/
#include <stdio.h>
#include <ctype.h>

typedef enum {false=0, true=1} bool;

int optind;
int opterr;
char *optarg;

char *progname;

bool f_flag;	/* true if program does not read files */
bool h_flag;	/* true if want to generate .h file */
bool R_flag;	/* true if want RCS header, default set in Makefile */
bool S_flag;	/* true if want SCCS header, default set in Makefile */

int t_val;	/* spaces indented or size of tab, default set in Makefile */

char *h_string;	/* pointer to name of .h file */
char *n_string;	/* pointer to name of .c file */
char *o_string;	/* string of options */
