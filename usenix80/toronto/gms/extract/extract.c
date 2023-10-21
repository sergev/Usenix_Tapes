/*
 * This file contains the source for the tape extraction program (see
 * EXTRACT(VIII)).
 * compile: cc -n -v -O extract.c -lt -lS -lg -lS -o extract
 */

/*
 * Include files
 */

#include <ino.h>	/* structure of inodes on the disk */
#include <path.h>	/* defines maximum length of system pathname */
#include <stdio.h>	/* structures and defines for standard i/o library */
#include <struct.h>	/* standard PDP-11 structures */
#include <tapio.h>	/* structures and defines for tape i/o library */

/*
 * Parameters used by routines (should NOT be changed)
 */

#define	BLCKSIZ	512		/* number of bytes in a block */
#define	OFF	0		/* flag is off */
#define	ON	1		/* flag is on */

/*
 * Parameters for tuning (may be changed)
 */

#define	RECSIZ	(33 * BLCKSIZ)	/* number of bytes in a tape record */
/* NOTE: this should agree with the record size in the dump program */

/*
 * Structures
 */

/* structure to hold a line from an ncheck */

struct	ncheck {
	unsigned n_ino;		/* the inode number */
	char	n_name[PATHLEN];/* the file pathname */
};

/* structure for sorting block addresses */

struct	blkadd {
	unsigned b_block;	/* the block number in the file */
	unsigned b_addr;	/* the block address on the tape */
};

/*
 * Externals
 */

/* program pathnames */

static	char	*ddname = {	/* pathname of DD(I) program */
	"/bin/dd"
};

/* raw magtape device externals */

static	char	*protodev = {	/* prototype name for raw magtape device */
	"/dev/rmt"
};

/* temporary file externals */

static	char	*tempdir = {	/* name of directory for temporary files */
	"/tmp"
};
static	char	*temptr0 = {	/* pointer to pathname of temporary file 0 */
	NULL
};
static	char	*temptr1 = {	/* pointer to pathname of temporary file 1 */
	NULL
};

/* misc. externals */

static	struct	ncheck	ncheck;	/* holds a line from an ncheck */
static	TAPE	*tp;		/* tape i/o pointer */

/*
 * Functions
 */

/*
 * This is the main program for EXTRACT(VIII).
 */

main(ac,av)
char	**av; {
	register char	*p;
	register unsigned i;
	char	*file;
	char	tapdev[PATHLEN],temp0[PATHLEN],temp1[PATHLEN];
	char	ddbs[BLCKSIZ],ddif[BLCKSIZ],ddskip[BLCKSIZ],ddcount[BLCKSIZ];
	int	first,dflag,nflag,small,large,huge,contig;
	unsigned count,inum,iobuf[BLCKSIZ / 2];
	long	block,bytes,lastblock,lastbyte,offset,size;
	FILE	*ip,*op;
	struct	blkadd	blkadd,sortbuf[7 + 256];
	struct	inode	inode;
	extern	char	*fsortdir;
	extern	compare();

/* check the argument count */
	if(ac < 3) {
		cleanup(ERR,"usage: extract tapedrive [-d] [-n] [-t directory] file\n");
	}
/* set up the raw magtape device name */
	strcpy(tapdev,protodev);
	strcat(tapdev,av[1]);
/* decode the remainder of the arguments */
	dflag = OFF;
	nflag = OFF;
	inum = 0;
	file = NULL;
	for(i = 2;i < ac;i++) {
/* decode flags */
		if(av[i][0] == '-') {
			if(av[i][2] != '\0') {
				cleanup(ERR,"bad flag: %s\n",av[i]);
			}
			switch(av[i][1]) {
/* bad flag */
			default:
				cleanup(ERR,"bad flag: %s\n",av[i]);
/* -d (data) flag */
			case 'd':
				dflag = ON;
				break;
/* -n (ncheck) flag */
			case 'n':
				nflag = ON;
				break;
/* -t (temporary directory) flag */
			case 't':
				if(++i < ac) {
					tempdir = av[i];
				}
				else {
					cleanup(ERR,"no directory specified for -t flag\n");
				}
				break;
			}
		}
/* decode file name or inode number */
		else {
			if((inum != 0) || (file != NULL)) {
				cleanup(ERR,"more than one file specified\n");
			}
			if(isdigit(av[i][0])) {
				sscanf(av[i],"%d",&inum);
			}
			else {
				file = av[i];
			}
		}
	}
/* open the tape for reading */
	tp = topen(tapdev,RECSIZ);
	if(tp == NULL) {
		cleanup(ERR,"cannot open %s for reading\n",tapdev);
	}
/* read through the ncheck lines */
	do {
		rncheck();
/* if file name was specified, get inode number from ncheck */
		if((inum == 0) &&
		   (file != NULL) &&
		   (strcmp(file,ncheck.n_name) == 0)) {
			inum = ncheck.n_ino;
		}
	}  while(ncheck.n_ino != 0);
/* calculate the offset of the file system */
	if((offset = ttell(tp)) == ERR) {
		cleanup(ERR,"tape tell error\n");
		exit(ERR);
	}
	offset += RECSIZ - 1;
	offset -= (offset % RECSIZ);
/* special processing for -d and -n flags */
	if((dflag == ON) || (nflag == ON)) {
/* close the tape (causes a rewind) */
		if(tclose(tp) == ERR) {
			cleanup(ERR,"tape close error\n");
			exit(ERR);
		}
/* set up arguments for dd */
		sprintf(ddbs,"bs=%u",RECSIZ);
		sprintf(ddif,"if=%s",tapdev);
		block = offset;
		block /= RECSIZ;
		sprintf(ddskip,"skip=%ld",block);
		sprintf(ddcount,"count=%ld",block);
	}
/* dump the whole tape (both ncheck and data) */
	if((dflag == ON) && (nflag == ON)) {
		fprintf(stderr,"WARNING: tape will give i/o error on end of file\n");
		fprintf(stderr,"please check count to see if error has occured\n");
		fprintf(stderr,"the ncheck is %ld record(s) long\n",block);
		execl(ddname,"extract-dd",ddbs,ddif,0);
		cleanup(ERR,"cannot execl %s\n",ddname);
	}
/* dump just the data */
	if(dflag == ON) {
		fprintf(stderr,"WARNING: tape will give i/o error on end of file\n");
		fprintf(stderr,"please check count to see if error has occured\n");
		execl(ddname,"extract-dd",ddbs,ddif,ddskip,0);
		cleanup(ERR,"cannot execl %s\n",ddname);
	}
/* dump just the ncheck */
	if(nflag == ON) {
		execl(ddname,"extract-dd",ddbs,ddif,ddcount,0);
		cleanup(ERR,"cannot execl %s\n",ddname);
	}
/* extract a specific file */
/* see if a file or inode was specified */
	if(inum == 0) {
		if(file == NULL) {
			cleanup(ERR,"file not specified\n");
		}
		else {
			cleanup(ERR,"%s is not on the tape\n",file);
		}
	}
/* set up the temporary file names */
	strcpy(temp0,tempdir);
	for(p = temp0;*p != '\0';p++);
	*p++ = '/';
	tmpnam(p);
	temptr0 = temp0;
	strcpy(temp1,tempdir);
	for(p = temp1;*p != '\0';p++);
	*p++ = '/';
	tmpnam(p);
	temptr1 = temp1;
/* set up the temporary file directory for fsort */
	fsortdir = tempdir;
/* calculate the block offset for the inode */
	block = inum;
	block += 31;
	block /= 16;
	block *= BLCKSIZ;
/* calculate the byte offset within the block for the inode */
	bytes = inum;
	bytes += 31;
	bytes %= 16;
	bytes *= 32;
/* seek to the inode's location */
	if(tseek(tp,offset + block + bytes) == ERR) {
		cleanup(ERR,"tape seek error\n");
	}
/* read in the inode */
	if(tread(&inode,sizeof(struct inode),1,tp) != 1) {
		cleanup(ERR,"tape read error\n");
	}
/* store the size as a long */
	size.hiint.hibyte = 0;
	size.hiint.lobyte = inode.i_size0;
	size.loint = inode.i_size1;
/* print a readable dump of the inode */
/* if inode is unallocated, exit */
	if((inode.i_mode & IALLOC) != IALLOC) {
		cleanup(ERR,"inode %u is unallocated\n",inum);
	}
/* if inode is a character special file, exit */
	if((inode.i_mode & IFMT) == IFCHR) {
		cleanup(ERR,"inode %u is a character special file\n",inum);
	}
/* if inode is a block special file, exit */
	if((inode.i_mode & IFMT) == IFBLK) {
		cleanup(ERR,"inode %u is a block special file\n",inum);
	}
/* if inode is a directory or a plain file, go on */
	if((inode.i_mode & IFMT) == IFDIR) {
		fprintf(stderr,"inode %u is a directory\n",inum);
	}
	else {
		fprintf(stderr,"inode %u is a plain file\n",inum);
	}
/* check if file is contiguous */
	if(inode.i_addr[CN_FLAG] == 1) {
		contig = ON;
		first = CN_TOP;
		fprintf(stderr,"file is contiguous\n");
	}
	else {
		contig = OFF;
		first = 0;
	}
/* determine file type (small, large, or huge) */
	small = OFF;
	large = OFF;
	huge = OFF;
	if((inode.i_mode & ILARG) == ILARG) {
		if(inode.i_addr[7] != 0) {
			huge = ON;
			fprintf(stderr,"file is huge (3 passes)\n");
		}
		else {
			large = ON;
			fprintf(stderr,"file is large (2 passes)\n");
		}
	}
	else {
		small = ON;
		fprintf(stderr,"file is small (1 pass)\n");
	}
/* print useful information from the remainder of the inode */
	fprintf(stderr,"mode (see CHMOD(II)): %o\n",inode.i_mode & 07777);
	fprintf(stderr,"number of links: %u\n",inode.i_nlink & 0377);
	fprintf(stderr,"user id, group id of owner: %u, %u\n",inode.i_uid & 0377,inode.i_gid & 0377);
	fprintf(stderr,"size of file (in bytes): %ld\n",size);
	fprintf(stderr,"modification time: %s",ctime(inode.i_mtime));
/* determine the last block and the last byte count to write */
	lastbyte = size;
	lastbyte %= BLCKSIZ;
	lastblock = size;
	lastblock -= lastbyte;
/* write to a temporary file */
	op = fopen(temp0,"w");
	if(op == NULL) {
		cleanup(ERR,"cannot create temporary file\n");
	}
/* initialize the sort buffer */
	for(i = 0;i < (7 + 256);i++) {
		sortbuf[i].b_block = i;
		sortbuf[i].b_addr = 0;
	}
/* interpret the inode block addresses */
	for(i = first;i < 8;i++) {
/* if the file is small write out the inode data block addresses */
		if(small == ON) {
			if(fwrite(&inode.i_addr[i],2,1,op) != 1) {
				cleanup(ERR,"file write error\n");
			}
		}
/* otherwise put the inode indirect block addresses into the sort buffer */
		else {
			sortbuf[i - first].b_addr = inode.i_addr[i];
		}
	}
/* if the file is huge, retrieve the double indirect block */
	if(huge == ON) {
		block = (unsigned)inode.i_addr[7];
		block *= BLCKSIZ;
		if(tseek(tp,offset + block) == ERR) {
			cleanup(ERR,"tape seek error\n");
		}
		if(tread(iobuf,1,BLCKSIZ,tp) != BLCKSIZ) {
			cleanup(ERR,"tape read error\n");
		}
/* copy the indirect addresses into the the sort buffer */
		for(i = 0;i < 256;i++) {
			sortbuf[i + (7 - first)].b_addr = iobuf[i];
		}
/* close and reopen the tape (causes a rewind) */
		if(tclose(tp) == ERR) {
			cleanup(ERR,"tape close error\n");
		}
		tp = topen(tapdev,RECSIZ);
		if(tp == NULL) {
			cleanup(ERR,"tape open error\n");
		}
	}
/* if the file is large or huge, retrieve the indirect blocks */
	if((large == ON) || (huge == ON)) {
/* sort the sort buffer in core */
		qsort(sortbuf,7 + 256,sizeof(struct blkadd),compare);
		for(i = 0;i < (7 + 256);i++) {
/* ignore blocks with 0 addresses */
			if(sortbuf[i].b_addr == 0) {
				continue;
			}
/* read from the tape */
			block = sortbuf[i].b_addr;
			block *= BLCKSIZ;
			if(tseek(tp,offset + block) == ERR) {
				cleanup(ERR,"tape seek error\n");
			}
			if(tread(iobuf,1,BLCKSIZ,tp) != BLCKSIZ) {
				cleanup(ERR,"tape read error\n");
			}
/* write to the output file */
			block = sortbuf[i].b_block;
			block *= BLCKSIZ;
			if(fseek(op,block,0) == ERR) {
				cleanup(ERR,"file seek error\n");
			}
			if(fwrite(iobuf,1,BLCKSIZ,op) != BLCKSIZ) {
				cleanup(ERR,"file write error\n");
			}
		}
/* close and reopen the tape (causes a rewind) */
		if(tclose(tp) == ERR) {
			cleanup(ERR,"tape close error\n");
		}
		tp = topen(tapdev,RECSIZ);
		if(tp == NULL) {
			cleanup(ERR,"tape open error\n");
		}
	}
/* the temporary file contains a list of all the data blocks */
	fclose(op);
/* set up the blocks to be sorted */
	ip = fopen(temp0,"r");
	if(ip == NULL) {
		cleanup(ERR,"cannot open temporary file\n");
	}
	op = fopen(temp1,"w");
	if(op == NULL) {
		cleanup(ERR,"cannot create temporary file\n");
	}
/* if the file is contiguous copy those block numbers first */
	i = 0;
	if(contig == ON) {
		for(;i < inode.i_addr[CN_LEN];i++) {
			blkadd.b_addr = i + inode.i_addr[CN_BEG];
			blkadd.b_block = i;
			if(fwrite(&blkadd,sizeof(struct blkadd),1,op) != 1) {
				cleanup(ERR,"file write error\n");
			}
		}
	}
/* copy from one temporary file to the other */
	for(;fread(&blkadd.b_addr,2,1,ip) == 1;i++) {
/* eliminate blocks with 0 addresses */
		if(blkadd.b_addr == 0) {
			continue;
		}
/* fill in the file block numbers */
		blkadd.b_block = i;
		if(fwrite(&blkadd,sizeof(struct blkadd),1,op) != 1) {
			cleanup(ERR,"file write error\n");
		}
	}
	fclose(ip);
	fclose(op);
/* sort the indirect blocks (in the file) */
	fsort(temp1,sizeof(struct blkadd),compare);
/* reopen the sorted temporary file */
	ip = fopen(temp1,"r");
	if(ip == NULL) {
		cleanup(ERR,"cannot open temporary file\n");
	}
/* copy from the tape to the file */
	while(fread(&blkadd,sizeof(struct blkadd),1,ip) ==1) {
/* read from the tape */
		block = blkadd.b_addr;
		block *= BLCKSIZ;
		if(tseek(tp,offset + block) == ERR) {
			cleanup(ERR,"tape seek error\n");
		}
		if(tread(iobuf,1,BLCKSIZ,tp) != BLCKSIZ) {
			cleanup(ERR,"tape read error\n");
		}
/* write to the file */
		block = blkadd.b_block;
		block *= BLCKSIZ;
		if(fseek(stdout,block,0) == ERR) {
			cleanup(ERR,"file seek error\n");
		}
/* check for partial write on last block */
		count = (block == lastblock) ? lastbyte : BLCKSIZ;
		if(fwrite(iobuf,1,count,stdout) != count) {
			cleanup(ERR,"file write error\n");
		}
	}
/* all done */
	if(tclose(tp) != OK) {
		cleanup(ERR,"tape close error\n");
	}
	cleanup(OK,"completed successfully\n");
}

/*
 * Cleanup removes the temporary files if necessary, prints the specified
 * message, and exits with status.
 */

cleanup(status,mess)
register status;
char	*mess; {
	if(temptr0 != NULL) {
		unlink(temptr0);
	}
	if(temptr1 != NULL) {
		unlink(temptr1);
	}
	fprintf(stderr,"extract: %r\n",&mess);
	exit(status);
}

/*
 * Rncheck reads in an ncheck line from the tape into the ncheck buffer.
 */

rncheck() {
	register char	*p;
	register c;
	register unsigned i;

/* read in the inode number */
	i = 0;
	while((c = tgetc(tp)) != '\t') {
		if(c == ERR) {
			cleanup(ERR,"tape read error\n");
		}
		i *= 10;
		i += c - '0';
	}
	ncheck.n_ino = i;
/* read in the pathname */
	p = ncheck.n_name;
	while((c = tgetc(tp)) != '\n') {
		if(c == ERR) {
			cleanup(ERR,"tape read error\n");
		}
		if(p < &ncheck.n_name[PATHLEN - 1]) {
			*p++ = c;
		}
	}
	*p = '\0';
}

/*
 * This is the comparison routine for sorting.
 */

compare(ap,bp)
register struct blkadd *ap,*bp; {
/* this routine compares tape block addresses */
	if(ap->b_addr > bp->b_addr) {
		return(1);
	}
	if(ap->b_addr < bp->b_addr) {
		return(-1);
	}
	return(0);
}
