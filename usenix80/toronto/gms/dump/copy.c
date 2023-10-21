/*
 * This file contains the source for the copy program.  Copy copies its
 * standard input to its standard output in RECSIZ byte records.  The
 * last record is null padded to RECSIZ bytes.
 * compile: cc -n -v -O copy.c -lS -o copy
 */

/*
 * Include files
 */

#include <stdio.h>	/* structures and defines for standard i/o library */

/*
 * Parameters used by routines (should NOT be changed)
 */

#define	OK	0		/* non-error return status */
#define ERR	(-1)		/* error return status */
#define	BLCKSIZ	512		/* number of bytes in a block */

/*
 * Parameters for tuning (may be changed)
 */

#define	RECSIZ	(33 * BLCKSIZ)	/* number of bytes in a record */

/* NOTE: this should agree with the record size in the dump program */

/*
 * Functions
 */

/*
 * This is the main program for copy.
 */

main() {
	register char	*p;
	register unsigned nbytes;
	char	buf[RECSIZ];

/* copy full records from standard input to standard output */
	while((nbytes = fread(buf,1,RECSIZ,stdin)) == RECSIZ) {
		if(write(1,buf,RECSIZ) != RECSIZ) {
			fprintf(stderr,"write error\n");
			exit(ERR);
		}
	}
/* check for read error */
	if(ferror(stdin)) {
		fprintf(stderr,"read error\n");
		exit(ERR);
	}
/* if there is nothing in the buffer, all done */
	if(nbytes == 0) {
		exit(OK);
	}
/* pad the record to RECSIZ bytes with nulls */
	for(p = &buf[nbytes];p < &buf[RECSIZ];) {
		*p++ = '\0';
	}
/* write out the final record */
	if(write(1,buf,RECSIZ) != RECSIZ) {
		fprintf(stderr,"write error\n");
		exit(ERR);
	}
/* all done */
	exit(OK);
}
