#include <stdio.h>

char    buf[512];
long    nchars;
long    oldtime,newtime;
FILE    *mes;

main(argc,argv)
char **argv;
{
	int i,nblocks;
	mes = fopen("/dev/ttyw","w");
	nblocks = atoi(argv[1]);
	time(&oldtime);
	for(i=0; i<nblocks; i++) {
		write(1,buf,sizeof buf);
		nchars += sizeof buf;
	}
	time(&newtime);
	newtime -= oldtime;
	fprintf(mes,"%D bytes (%d blocks) in %D seconds=%D bytes/sec (%D baud)\n",
		nchars,nblocks,newtime,nchars/newtime,nchars/newtime*8);
}
