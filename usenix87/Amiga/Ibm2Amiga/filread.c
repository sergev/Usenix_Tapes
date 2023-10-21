/* 

toamiga.c

A program to transfer ASCII files from a IBM PC to the Amiga, 
for use with the Amiga READ command.

This filter ignores CR, passes LFs, and expands TABs to spaces.
Stdin should be a ASCII text file.
Stdout gets a stream of hex nybbles, with end of stream marked by 'Q'.

V1.0:  John Foust, October 26, 1985 with Lattice C v2.15 under PC-DOS.
This is public domain software.  If want to send me money, go for it.
*/


#include "stdio.h"

/* TAB stop size for the output file.  Any TABs are expanded to spaces. */
#define TABSIZE 8

/* a convenient buffer size for level 2 i/o */
#define BIGSIZE 512*32

/* input buffer */
char bigbuf[BIGSIZE+1];

main()
{
char c;
int x,y,
    linepos;	/* current column number in virtual output file */

    do {
	x = fread(&bigbuf,1,BIGSIZE,stdin);
	for (y=0;y<x;y++) {
	    c = bigbuf[y];
	    switch (c) {
		case 13:
		    linepos = 0;
		    break;
		case 9:		/* change TAB to TABSIZE spaces */
		    /* if at left edge, or on a TAB stop, do a TAB */
		    if (!(linepos%8)) {
			printf("20");
			linepos++;
		    }
		    while (linepos%TABSIZE) {
			printf("20");
			linepos++;
		    }
		    break;
		/* MS-DOS uses CR/LF pairs for end of line. */
		/* Ignore the CR, and use the LF the Amiga needs. */
		case 10:
		    linepos = 0;
		    printf("0A");
		    break;
		default:
		    linepos++;
		    if (c<10)
			printf("0");
		    printf("%x",c);
	    }
	}
    } while (x==BIGSIZE);

    printf("Q");
    exit(0);
}
