#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "vmstape.h"

static	int	nomore = 0;
int open_file();

vt_extract(files, device, mode)

char	**files, *device;
int mode;

{
	register        int     blocksize;
			int     nblocks;
			int	i;
			char    buf[RECSIZE];
			char    filename[RECSIZE];
			char	recformat;
			char	formc;
	/* open tape file */

	magtape = open_file (device, mode);

	/* get volume label */
	r_record(buf);

	while( r_record(buf) ){
		rawflag	= 0;
		binflag	= 0;

		strcpy(filename, field(buf, H1_FNAME));
		strip(filename);
		r_record(buf);
		blocksize = atoi(field(buf, H2_BSZ));

		/* set flags for extraction of the file.
		 *
		 * variable length crlf form control --> no flags set
		 *	this is your normal text file on VMS
		 *
		 *	|length|record|  --> |record|'\n'
		 *
		 * fixed length --> binflag set
		 *	file records extracted as is.  no interpretation
		 *	done on blocks of file.  (ie, file is full of records
		 *	and no information about record length).  This type
		 *	of file is an array of records which have no control
		 *	(record length) info).
		 *
		 *	|record|  --> |record|
		 *
		 * variable length no form control --> raw flag set
		 *
		 *	|length|record|  --> |length|record|
		 */
		recformat = *field(buf, H2_FMT);
		fixreclen = atoi(field(buf, H2_RECLEN));
		if( recformat == 'F' )	/* is fixed length */
			binflag	= 1;
		else {
			if( recformat != 'D' ){
				printf("UNKNOWN RECORD FORMAT <%c>\n",
					recformat);
				exit(FAILURE);
				}
			/* if recformat == 'D', then
			 * is variable length.  set no flags */
			}

		formc	= *field(buf, H2_FORMC);
		if( formc == 'M')
			rawflag	= 1;	/* stuff record onto disk with
					 * its control area defining its
					 * length.
					 */
		else {
			if( formc != ' '){
				printf("UNKNOWN FORM CONTROL <%c>\n",
					formc);
				exit(FAILURE);
				}
			/* is the default whith crlf
			 * stuff done at the end of each record.
			 */
			}

		if( header3() )
			r_tapemark();

		if( isarg(filename, files) )
			r_data(blocksize, filename, files);
		else{
			/* skip tape mark at end of data area and
			 * after Trailers.
			 */
			skiptm(2);
			continue;
			}

		if( nomore )	/* got all files wanted */
			break;

		/* grap Trailer areas and tape mark separating files
		 */
		r_record(buf);
		r_record(buf);
		if( trailer3() )
			r_tapemark();
	}

	for( i=0; files[i]; i++)
		if (files[i] != (char *)(-1))
		    printf("'%s' not found on tape\n", files[i]);

}

r_data(blksize,  filename, argv)
register        int     blksize;
		char    *filename;
		char    **argv;
{
	register        int     i;
			FILE    *outfile;
			char	*pathname;

	if(blksize > MAXBLOCKSZ) {
		fprintf("BUFFER SIZE EXCEEDED\n");
		exit(FAILURE);
		}

	if(exists(filename)) {
		fprintf(stderr, "x: %s already exists.  Not extracted.\n",
			filename);
		while( read(magtape, databuf, blksize) > 0)
			continue;
		return;
	}

	outfile = fopen(filename, "w");
	if(outfile == NULL) {
		fprintf(stderr, "x: cannot create %s\n", filename);
		while( read(magtape, databuf, blksize) > 0)
			continue;
		return;
	}

	if (verbose) printf("Extracting %s\n", filename);

	while( (i = read(magtape, databuf, blksize)) > 0 )
		ext(databuf, i, outfile);

	fclose(outfile);
}

exists(filename)
	char    *filename;
{
	struct	stat	statbuf;

	if( stat(filename, &statbuf) < 0 )
		return(FALSE);

	return(TRUE);
}

ext(p, blksize, outfile)
register        char    *p;
		int     blksize;
register        FILE    *outfile;
{
	register        int     i;
	register        int     seen;
			int     thisline;
			char    nbuf[5];
			char	outbuf[MAXFIXRECLEN];
			int	filler = 1;	

/* if binflag, then global fixreclen is set to record size */
	if(binflag) {
	   while (blksize>0) {
		for(i = 0 ; i < fixreclen ; i++) {
			outbuf[i] = *p;
			if (*p++ != FILLCHAR) filler = 0;
			}
		if (filler) return;  
		filler = 1;
		for(i = 0;i<fixreclen;i++) {	
			putc(outbuf[i], outfile);
			}
		blksize -= fixreclen;
		}
	   }
	else
		for(seen = 0 ; seen < blksize ; ) {
			for(i = 0 ; i < 4 ; i++)
				nbuf[i] = *p++;
			nbuf[4] = '\0';
			seen += 4;
			if(!isdigit(nbuf[0]))
				break;
			thisline = atoi(nbuf) - 4;

			if(rawflag)
				putw(thisline, outfile);

			for(i = 0 ; i < thisline ; i++)
				fputc(*p++, outfile);

			seen += thisline;

			if(!rawflag)
				putc('\n', outfile);
			}
}

r_record(p)
	char    *p;
{
	int     n_read;

	n_read = read(magtape, p, RECSIZE);
	return(n_read);
}

isarg(str, argv)
		char    *str;
		char    **argv;
{
	int     i;
	int	count;	/* of number left */
	int	retval;

	/* argv vector is null terminated.
	 * if no files listed, then extract all
	 */
	if( !(*argv) )
		return(1);

	count	= 0;
	retval	= 0;

	for(i = 0 ; argv[i] ; i++){
		if( argv[i] == ((char *)(-1)) )
			continue;
		if( streq(str, argv[i]) ) {
			argv[i] = ((char *)(-1));
			retval	= 1;
			}
		count++;
		}

	if( (!count) && (!retval) )
		nomore	= 1;	/* all done */
	return(retval);
}

strip(p)
register        char    *p;
{
	if(*p == '\0')
		return;
	while(*p != '\0')
		p++;
	p--;
	while(*p == ' ')
		p--;
	*++p = '\0';
}

r_tapemark()
{
	if(read(magtape, databuf, MAXBLOCKSZ) != 0) {
		fprintf(stderr, "MISSING TAPE MARK??\n");
		exit(FAILURE);
		}
}
