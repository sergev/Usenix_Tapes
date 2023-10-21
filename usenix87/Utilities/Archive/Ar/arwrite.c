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
#include <sys/types.h>
#include <sys/stat.h>

#define ARFMAG "`\n"
#define ARMAG "!<arch>\n"

int
arwrite(filename, stream)
    char *filename;
    FILE *stream; {
/*
 *	Write the file denoted by 'filename' to 'stream' in portable ar format.
 *	Any stream positioning is assumed to already be done.
 *
 *	arwrite() normally returns 0 but returns -1 in case of errors.  errno
 *	may be inspected to determine the cause of the error.
 *
 *	If necessary, the stream is padded with \n's to match the even boundary
 *	condition.
 */
	struct stat statbuf;
	FILE *instream;
	long i;

	/* stat() the file to get the stuff for the header */
	if( stat(filename,&statbuf) < 0 ) {
		/* error! */
		return -1;}

	/* Open file for reading */
	if( (instream = fopen(filename,"r")) == NULL ) {
		/* error! */
		return -1;}

	/* Now write the header */
	/* This information gleaned from ar(4) in V.2 */
	fprintf(stream,
	    "%-16s%-12ld%-6d%-6d%-8o%-10ld%2s",
	    filename,
	    statbuf.st_mtime,
	    statbuf.st_uid,
	    statbuf.st_gid,
	    statbuf.st_mode,
	    statbuf.st_size,
	    ARFMAG
	    );

	/* And copy the file */
	/*   Note that there is no error recovery here! */
	for( i = 0 ; i < statbuf.st_size ; i++ ) {
		fputc(fgetc(instream),stream);}

	/* and close */
	close(instream);

	/* and pad output stream */
	if( (ftell(stream) & 0x1) != 0 ) {
		/* if offset is odd, pad with a nuline */
		fputc('\n',stream);}
	return 0;}

arhdwrite(stream)
    FILE *stream; {
/*
 *	Write the archive header onto 'stream'
 */
	fprintf(stream,"%s",ARMAG);
	return;}
