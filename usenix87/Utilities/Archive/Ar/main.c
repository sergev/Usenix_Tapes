/*
 *		COPYRIGHT (c) HEWLETT-PACKARD COMPANY, 1984
 *
 *	Permission is granted for unlimited modification, use, and
 *	distribution except that this software may not be sold for
 *	profit.  No warranty is implied or expressed.
 *
 *Author:
 *	Paul Bame, HEWLETT-PACKARD LOGIC SYSTEMS DIVISION - Colorado Springs
 *	{ ihnp4!hpfcla | hplabs | harpo!hp-pcd }!hp-lsd!paul
 */
#include <stdio.h>

extern	int	arwrite();
extern		arhdwrite();

main(argc, argv)
    int argc;
    char *argv[]; {
/*
 *	Write the named files (in argv) to stdout in portable ar format.
 */
	int i;

	if( argc < 2 ) {
		fprintf(stderr,"Usage: %s file1 [file...]\n",argv[0]);
		exit(1);}

	/* Write the header */
	arhdwrite(stdout);

	/* and the files */
	for( i = 1 ; i < argc ; i++ ) {
		if( arwrite(argv[i],stdout) < 0 ) {
			perror(argv[i]);
			exit(1);}}
	exit(0);}
