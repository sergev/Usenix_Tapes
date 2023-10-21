#include "rv.h"
/*
 *	Rvi - Portable distributed screen editor (DSE).
 *	86/07/16.  Alan Klietz
 *	Copyright (c) 1986, Research Equipment Incorporated
 *			    Minnesota Supercomputer Center
 *
 * Permission is hereby granted to use this software on any computer system
 * and to copy this software, including for purposes of redistribution, subject
 * to the conditions that 
 *
 *  o  The full text of this copyright message is retained and prominently
 *      displayed
 *
 *  o  No misrepresentation is made as to the authorship of this software
 *
 *  o  The software is not used for resale or direct commercial advantage
 *
 *  By copying, installing, or using this software, the user agrees to abide
 *  by the above terms and agrees that the software is accepted on an "as is"
 *  basis, WITHOUT WARRANTY expressed or implied, and relieves Research Equip-
 *  ment Inc., its affiliates, officers, agents, and employees of any and all
 *  liability, direct of consequential, resulting from copying, installing
 *  or using this software.
 */
#ifndef lint
static char *copyright = "Copyright (c) 1986, Research Equipment Incorporated.";
#endif

/*
 * Make initial program,
 * Alan Klietz, Nov 11, 1985.
 */

/*
 * Declare global externals
 */
struct	fi_file		file;
struct	li_line		*line_array;
struct  ya_yank		yank_array[NUM_YANK_BUFS];
struct	un_undo		undo;
struct	wi_window	window;
struct	sc_screen	screen;

INT	errflag;	/* 1 if error in command line parsing */
INT	nwin_lines;	/* Number of lines in window */
char    editlist[256];	/* List of files to edit */
char	**Argv;
boolean	use_linemode = 1 ;/* TRUE to set linemode on tty during shell escapes */

/*
 * Parse command line arguments
 * Call major routines
 */
main(argc, argv)
int argc;
char **argv;
{
	void  rv_cmd(), usage(), initialize();

	if (argc < 4)
		usage();
	Argv = argv;
	strncpy(editlist, argv[3], 255);
	if (argc == 5)
		use_linemode++;
	initialize();
	rv_cmd();
	/*NOTREACHED*/
}


void
usage()
{
	fprintf(stderr, "Usage: rvi in_fd out_fd \"files\"\n");
	exit(1);
}
