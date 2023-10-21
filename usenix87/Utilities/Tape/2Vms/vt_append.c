#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include "vmstape.h"

int open_file();

vt_append(argv, device, mode)

char **argv, *device;
int mode;

{
	char buf[RECSIZE];
	int  i,offset;
	int  nblocks;

	if ( !(*argv) ) {
		printf("r: no arguments??\n");
		return;
	}

	/* open tape file */

	magtape = open_file (device, mode);

	/* read the volume label */
	r_record(buf);

 	offset = 1;
	while (r_record(buf)) 
	 {
		++offset;	/* counting files */
		skiptm(3);
	 }

	if (section_flag) offset = 1; /* if we want to make sections ... */
	/* back up one tape mark */
	backtm(); 

	for(i = 0 ; argv[i] ; i++) {
		if(verbose)
			printf("r %s\n", argv[i]);
		if( subdir(argv[i]) ) 
			continue;
		if( size0(argv[i]) )
			continue;
		if ( isdir(argv[i]) ) {
		    printf("'%s' is a directory...writing all files within\n", argv[i]);
		    w_dir(argv[i],  &offset);
		}
		else {
			w_file(argv[i], offset);
			++offset;
		}
	   }
	w_tapemark();
}

backtm()
{
	int i;
	struct mtop 	m;

	m.mt_count	= 1;
	m.mt_op		= MTBSF;

	i = ioctl(magtape, MTIOCTOP, &m);
}
