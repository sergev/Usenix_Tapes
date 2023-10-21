#include "tu58.h"
#include "rtdir.h"
#include <stdio.h>
#define FBLOCKIO
#include "debug.h"

 int file[NTU58];			/* file descriptors */
 char *fname[NTU58];			/* file names */
 int fflags[NTU58];
 int fmode[NTU58];
 int fpt;				/* number of active file descriptors */
 char ronly[NTU58];			/* write permission */
 char cflag[NTU58];			/* create file */
 char iflag[NTU58];			/* init RT-11 Dir */

#ifdef FBLOCKIO
#define FSIZE 512			/* must be integral number of DATALEN */
 char fobuf[FSIZE];			
 char fibuf[FSIZE];			
 char *wpt;				/* pointer to data quanta. len DATALEN*/
 char *rpt;
#endif

#define fileck(dev)	( dev <= NTU58 || file[dev] != -1 )

fileinit()
{
	register int i;

	for (i = 0; i < NTU58; i++) {
		file[i] = -1;
		ronly[i] = 0;
		cflag[i] = 0;
		iflag[i] = 0;
	}
	fpt = 0;
#ifdef FBLOCKIO
	fwclear();	frclear();
#endif
}

readonly(index)
{
	setoption(index, ronly);
}

createflag(index)
{
	setoption(index, cflag);
}

initrtflag(index)
{
	setoption(index, iflag);
}

setoption(index, option)
register char *index; register char *option;
{
	register int i;

	if (index[2] == 0)
		for (i = 0; i < NTU58; i++)
			option[i] = 1;
	else {
		i = index[2] - '0';
		if (0 <= i && i < NTU58)
			option[i] = 1;
		else
			fprintf(stderr,"Bad option: %s", index);
	}
}

fileopen(name,flags,mode)
char *name; int flags, mode;
{
	register int fd;

	if ( ronly[fpt] )
		fd = open(name, 0);
	else {
		fd = open(name, flags, mode);
		if( flags > 0 && fd < 0 ) {
			fd = open(name, 0);
			if( fd >= 0 )
				ronly[fpt] = 1;
		}
	}

	if (fd < 0 && cflag[fpt])
		fd = creat(name, mode);

	if (fd < 0) {
		fprintf(stderr, "Cannot open or create: %s", name);
		exit(-1);
	}

	file[fpt] = fd;
	fname[fpt] = name;
	fflags[fpt] = flags;
	fmode[fpt] = mode;

	if (iflag[fpt])
		frt11_init(file[fpt], name);

	fpt++;
	return 1;
}

frt11_init(fd, name)
int fd; char *name;
{
	char buf[512];
	register int i;
	register char *cp;

	fprintf(stderr, "tu58: Initialize RT-11 directory on %s ?", name);
	if (gets(buf, 80) == NULL || !(*buf == 'y' || *buf == 'Y')) 
			return -1;

	for( cp=buf, i=0; i<512; i++)
		*cp++=0;

	for( i=0; i<RTFS; i++)
		if( 512 != write(fd, buf, 512)) 
			goto quit;
	lseek(fd, RTBBPOS, 0);
	if( RTBB != write(fd, (char *)bootbuf, RTBB))
		goto quit;
	lseek(fd, RTDIRPOS, 0);
	if( RTDIR != write(fd, (char *)dirbuf, RTDIR))
		goto quit;
	lseek(fd, RTDIR2POS, 0);
	if( RTDIR2 != write(fd, (char *)dir2buf, RTDIR2))
		goto quit;
	return(1);
		
quit:	fprintf(stderr, "frt11_init: Can't\n");
	return -1;
}

fposition(dd,mod,block)			/* seek file (tape) position */
int dd, mod, block;
{
	if( !fileck(dd) ) {
		fprintf( stderr, "position on bad unit\n");
		sendend( TUOP_END, TUE_BADU, 0, 0);
		return 0;
	}
	if( mod & TUMD_128 ) {
		if(   lseek( file[dd], block*128, 0) < 0   ) {
			fprintf( stderr, "position bad \n");
			sendend( TUOP_END, TUE_BADB, 0, 0);
			return 0;
		}
	}
	if( block == 6 )
		fupdate(dd);		/* allow for rtpip insert */
	if(  lseek( file[dd], block*512, 0) < 0   ) {
		fprintf( stderr, " position bad \n");
		sendend( TUOP_END, TUE_BADB, 0, 0);
		return 0;
	}
	debugo("position",block);
	debugo("mod",mod);
	return 1;
}

#ifdef FBLOCKIO
char *fread(dd)			/* get pointer to data from file */
int dd;
{
	int i;
		char *cp; 
	rpt += DATALEN;
	if( rpt >= &fibuf[FSIZE]) {
		if( FSIZE != (i=read(file[dd],fibuf,FSIZE)) )
			return 0;
		rpt = fibuf; 
	}
	return rpt;
}

char *fwrite(dd)		/* get pointer to write buf */
int dd;
{
	register char *cp;

	if(wpt == &fobuf[FSIZE - DATALEN]) 
		if( !fwflush(dd) )
			return 0;

	if(wpt)
		wpt += DATALEN;
	else
		wpt = fobuf;

	return wpt;
}

fwflush(dd)			/* flush write buf to file. return 1 or 0 */
int dd;
{
		register char *cp;

		for(cp = wpt + DATALEN; cp < &fobuf[FSIZE]; cp++)
			*cp = 0;
		wpt = 0;
		if( FSIZE != write(file[dd],fobuf,FSIZE) )
			return 0;
		return 1;
}

fwclear()			/* init write buf */
{
	wpt = 0;
}

frclear()			/* init read buf */
{
	rpt = &fibuf[FSIZE];
}
#endif

fupdate(dev)
int dev;
{
	close(file[dev]);
	fileopen(fname[dev],fflags[dev],fmode[dev]);
}
