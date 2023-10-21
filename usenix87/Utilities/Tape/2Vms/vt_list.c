#include "vmstape.h"

int open_file();

vt_list(device, mode)

char *device;
int mode;

{
	register        int     blocksize;
	register        int     recformat;
	register        int     formcntrl;
			int     nblocks;
			char    buf[RECSIZE];
			char    filename[RECSIZE];
			char	format[40];
			char	formc[40];
			int	n_read;
			int	reclen;
	/* open tape file */

	magtape = open_file (device, mode);

	/* get volume label */

	r_record(buf);
	printf("\nVolume Label: %s\n\n", field(buf, VLABEL));
	if( verbose )
		printf("%-20s%-15s %5s %6s %7s %7s\n",
			"filename", "record format", "bsize",
			"reclen", "formc", "nblocks"
			);
	else
		printf("%-20s\n",
			"filename"
			);

	while( r_record(buf) ){

		strcpy(filename, field(buf, H1_FNAME));
		strip(filename);

		r_record(buf);
		blocksize = atoi(field(buf, H2_BSZ));
		if( blocksize > MAXBLOCKSZ ){
			printf("blocksize %d too large for program !\n",
				blocksize);
			exit(-1);
			}

		recformat = *field(buf, H2_FMT);
		if( recformat == 'D' )
			strcpy(format,"var length");
		else if( recformat == 'F' )
			strcpy(format,"fixed length");
		else
			strcpy(format,"unknown");

		reclen	= atoi(field(buf, H2_RECLEN));
		if( recformat == 'D' )
			reclen -= 4;	/* 4 = size of control area
					 * of for variable length records
					 *
					 * the control area defines the length
					 * of the record, including the control
					 * area.
					 */

		formcntrl = *field(buf, H2_FORMC);
		if( formcntrl == 'A' )
			strcpy(formc, "fortran");
		else if( formcntrl == 'M' )
			strcpy(formc, "none");
		else if( formcntrl == ' ' )
			strcpy(formc, "crlf"); /* to be put between records */
		else
			strcpy(formc,"unknown");


		if( header3() )
			r_tapemark();

		if( verbose )
			printf("%-20s%-15s %-5d %-6d %7s ",
				filename, format, blocksize, reclen, formc);
		else
			printf("%-20s\n",
				filename);
		nblocks = 0;
		while( read(magtape, databuf, blocksize) > 0 )
			nblocks++;

/*
		if (recformat == 'F') nblocks = (nblocks % (blocksize/reclen))
						? nblocks/(blocksize/reclen)+1
						: nblocks/(blocksize/reclen);
*/
				/* Because when we were reading in 
				blocksize chunks of data, we
				were only getting reclen size peices. */

		if( verbose )
			printf("%-7d\n", nblocks);

		/* the condition that causes the exit from the while loop
		 * reads in the tape mark after the data area
		 */
		r_record(buf);
		r_record(buf);
		if( trailer3() )
			r_tapemark();
	}
}
