#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

 /* for either the old directory struct or new, but not emulation*/

#include <sys/dir.h>

#include "vmstape.h"

struct dirlis {
	char *names;	/* newline sep'd list of files */
	int n;		/* number of files there */
};

struct carray {
	char *block;	/* block w/text stored in it */
	char **array;	/* block of pointers into above */
	int num;	/* number of 'valid' pointers */
};

int open_file();

/* makerec assumes normal textual files
 *	note the check for zero byte means such is not valid to have
 *	in textual files.  All material following the zero byte is
 *	lost.
 *
 *	fgets has filled in the variable "in" with an asciz string
 *	and so the variable "in" is null-terminated.
 */

makerec(in, out, max)
	char    *in;
	char    *out;
{
	register        char    *p;
	register	int	count;

	count	= 0;
	for(p = in ; *p != '\n' ; p++){
		count++;
		if( count + 4> max ){  /* we need the 4 to have control info*/
			printf("GASP!  More than %d characters in the file not seperated by a newline.\nGASP!  File Incomplete.\n",max-4);
			return 0; /* error condition */
			}
		if( !(*p) )
			break;
		}

	*p = '\0';
	sprintf(out, "%04d%s", strlen(in) + 4, in);
}

bflush(seen)
register        int     seen;
{
	if(seen == 0)
		return;
	for( ; seen < blocksz ; seen++)
		databuf[seen] = FILLCHAR;
	if(write(magtape, databuf, blocksz) != blocksz) {
		fprintf(stderr, "FATAL WRITE ERROR\n");
		exit(FAILURE);
		}
	databuf[0] = '\0';              /* for strcat() */
}

flush_blk(buf,charcount)
char *buf;
long charcount;
{
	int i;
	if (charcount)
		for (i = charcount;i<blocksz;i++) buf[i] = FILLCHAR;
	if (write(magtape,buf,blocksz) != blocksz)
	 {
		fprintf(stderr,"FATAL WRITE ERROR\n");
		exit (FAILURE);
	 }
}

vt_create(argv, device, mode)

char    **argv, *device;
int mode;

{
	int     i;
	int	offset;	/* offset for filenumbers if any arguments */
			/* are directory names instead of just files */

	if( !(*argv) ){
		printf("c: no args?\n");
		return;
		};

	/* open tape file */

	magtape = open_file (device, mode);

	w_label();	 /* volume label */

	offset = 1; /* first file sequence number */

	for(i = 0 ; argv[i] ; i++) {
		if(verbose)
			printf("c %s\n", argv[i]);
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

w_dir(dirname,  offset)
char *dirname;
int *offset;
{
	struct carray files, gath();
	char *p, *malloc();
	int i;

	p = malloc(256);

	files = gath(dirname);
	
	for(i = 0; i < files.num; i++) {
	    if(*files.array[i] == '.')
		continue;

	    if(verbose)
		printf("%s: %s\n", dirname, files.array[i]);

	    strcpy(p, dirname);
	    strcat(p, "/");
	    strcat(p, files.array[i]);

	    if ( isdir(p) ) {
		printf("%s is a subdirectory. Not added to tape\n", p);
		continue;
	    }
	    if( size0(p) )
		continue;
	    w_file(p, *offset );
	    ++*offset;	/* inc file sequence number */
	}
}

int w_data(filename)
char    *filename;
{
	FILE    *ioptr;
	char    c,buf[MAXBLOCKSZ+1];
	struct stat filestat;
	long totalchar=0,charcount=0;
	long nrecs;
	int nblocks = 0;
	int length,outcount;
	char rec[MAXBLOCKSZ+1];

	ioptr = fopen(filename, "r");
	if(ioptr == NULL) {
		fprintf(stderr, "Cannot open %s\n", filename);
		}
	else {

	if (fixed_length_flag) {
		stat(filename,&filestat);
		nrecs = (filestat.st_size%fixreclen ) 
					? filestat.st_size/fixreclen +1
					: filestat.st_size/fixreclen ;
		totalchar = fixreclen * nrecs; /*this will be an even rec size*/
		while (charcount < totalchar)
		 {
			c = getc(ioptr);
			buf[charcount++ % blocksz] = c;
			if ((charcount % blocksz) == 0 ) flush_blk(buf,0);
		 }
		if ((charcount % blocksz) != 0 ) 
				flush_blk(buf,charcount % blocksz);
		fclose(ioptr);
	nblocks = nrecs/(blocksz/fixreclen);
	if (nrecs % (blocksz/fixreclen)) nblocks++;
	}
	else {  /* variable length records */

		outcount = 0;
		for(;;){
			length = (int)fgets(buf, blocksz, ioptr);
			/* scratch variable for the moment */

			if(length == NULL)
				break;

			if (makerec(buf, rec, blocksz) == 0) 
			 {
				nblocks = 0;
				break; /* error */
			 }
			length = strlen(rec);
			if(outcount + length > blocksz) {
				bflush(outcount);
				nblocks++;
				outcount = 0;
				}
			strcat(databuf, rec);
			outcount += length;
			}

		if( outcount ){
			bflush(outcount);
			nblocks++;
			}

		fclose(ioptr);
		}
}
	return(nblocks);
}


w_file(path, number)
char *path;
int number;
{
	int nblocks;
	char *file, *malloc();

	file = malloc(256);
	strcpy(file, path);

	while(*file != '\0' && *file !='/') file++;
	if (*file == '\0') file = path;
	else file++;
	w_hdr1(file, number);
	w_hdr2();
	w_tapemark();
	nblocks = w_data(path);
	w_tapemark();
	w_eof1(file, number, nblocks);
	w_eof2();
	w_tapemark();
	if( nblocks == 0 ){
	    fprintf(stderr,"%s caused no data area to be written on tape??\n",
		file);
	    w_tapemark(); /* make greaceful crash (all other files are okay )*/
	    exit(FAILURE);
	}
}

w_eof1(filename, filenum, blkcount)
char    *filename;
int     filenum;
int	blkcount;
{
	char    buf[81];

	sprintf(buf,
		"EOF1%-17s%-6s0001%04d000101%-6s%-6s %06dDECFILE11A          ",
		filename, vol_label, filenum, CREATION, EXPIRATION, blkcount);

	if(write(magtape, buf, 80) != 80) {
		fprintf(stderr, "WRITE -- FATAL ERROR\n");
		exit(FAILURE);
		}
}

w_eof2()
{
	char buf[82];
	sprintf(buf,
		 "EOF2%c%05d%05d                     %c             00                             ",(fixed_length_flag)?'F':'D',
			blocksz,(fixed_length_flag)?fixreclen:blocksz-4,
			(fixed_length_flag) ? 'M' : ' ');
	if(write(magtape, buf, 80) != 80) {
		fprintf(stderr, "WRITE -- FATAL ERROR\n");
		exit(FAILURE);
		}
}

w_hdr1(filename, filenum)
	char    *filename;
	int     filenum;
{
	char    buf[81];
	
	sprintf(buf,
	    "HDR1%-17s%-6s0001%04d000101%-6s%-6s 000000DECFILE11A          ",
	    filename, vol_label, filenum, CREATION, EXPIRATION);

	if(write(magtape, buf, 80) != 80) {
		fprintf(stderr, "WRITE -- FATAL ERROR\n");
		exit(FAILURE);
		}
}

w_hdr2()
{
	char	hdr2_label[RECSIZE];
	int	i;

	sprintf( hdr2_label, "HDR2%c%05d%05d",(fixed_length_flag)?'F':'D',blocksz,(fixed_length_flag) ? fixreclen: MAXFIXRECLEN);
	i	= strlen(hdr2_label);
	for( ; i<RECSIZE ; i++){

		hdr2_label[i]	= ' '; /* By default */

		/* Form control info */
		if (i == 36 && (fixed_length_flag)) hdr2_label[i] = 'M';

		/* buffer offset = "00" */
		if( i == 50 || i == 51 ) hdr2_label[i]	= '0';

		}

	if(write(magtape, hdr2_label, RECSIZE) != RECSIZE) {
		fprintf(stderr, "WRITE -- FATAL ERROR\n");
		exit(FAILURE);
		}
}

w_label()
{
	char    buf[RECSIZE];
	int	i;

	sprintf(buf,"VOL1%-6s", vol_label);

	i	= strlen(buf);
	for( ; i<RECSIZE ; i++){

		/* DIGITAL standard version */
		if( i == 50 ){
			buf[i]	= '1';
			continue;
			}

		/* label standard version */
		if( i == 79 ){
			buf[i]	= '3';
			continue;
			}

		buf[i]	= ' ';
		}

	if(write(magtape, buf, RECSIZE) != RECSIZE) {
		fprintf(stderr, "WRITE -- FATAL ERROR\n");
		exit(FAILURE);
		}
}
size0(filename)
	char	*filename;
{
	struct	stat	statbuf;

	if( stat(filename, &statbuf) < 0 ){
		printf("'%s' does not exist...not added to tape\n", filename);
		return(1);
		}
	if( statbuf.st_size == 0 ){
		printf("'%s' is of 0 size...skipped\n", filename);
		return(1);
		}

	return(0);
}

subdir(filename)
char *filename;
{
	register char *p;

	for(p = filename; *p != '\0'; p++) 
	    if (*p == '/') {
		printf("File '%s' from a subdirectory...not added to tape\n", filename);
		return(1);
	    }
	return(0);
}
	
isdir(file) 
char *file;
{
	struct stat sb;
	register int t;

	stat(file, &sb);
	t = sb.st_mode & S_IFMT;

	return(t == S_IFDIR);
}

/*	DIRSIZ, to return the size of the (directory) passed it.
 *	used as an estimate for the amount of space to keep for 
 *	matches.  ERROR if longer than 16 bit's worth.
 *      From Dave Brownell's help program
 */

int dirsiz(dir) char *dir;
{
	struct stat entry;

	if (stat(dir,&entry) == EOF) {
		printf("Can't stat() %s\n", dir);
		exit(1);
	}
	if (entry.st_size > 65000L) {	/* lazy */
		printf("Directory exceeds 64K bytes in length\n");
		exit(1);
	}
	else return((int) entry.st_size);
}

/*	NAMGET, to get the names in a directory and put them in
 *	a string, separated by newlines.  will also count the number
 *	of files. From Dave Brownell's help program.
 *	Modified to reflect 4.2 file changes as of 6/13/84. -- S.K.
 */

struct dirlis namget(dir) char *dir;
{	
#ifdef OLD_DIR_STRUCT
	struct direct entry;
#else
	struct direct *entry;
	DIR *dp;
#endif
	struct dirlis ret;
	register int i;
	register char *t;
	register FILE *file;
	extern char *malloc();

	ret.n = 0;

	t = ret.names = malloc(dirsiz(dir));
#ifdef OLD_DIR_STRUCT
	if ((file = fopen(dir,"r")) == NULL) return (ret);
	while (read(file,&entry,sizeof(struct direct))) {
	    if (entry.d_ino != 0) {
		for (i = 0 ; entry.d_name[i] != 0 && i < MAXNAMLEN ; i++)
			*t++ = entry.d_name[i]; /* copy the name */
		*t++ = '\n';
		ret.n++;
		}
	 }
	*t++ = '\0';
	fclose(file);
	return(ret);
}
#else
	if ((dp = opendir(dir)) == NULL) return (ret);
	while( entry = readdir(dp)) {
	    if (entry->d_ino != 0) {
		for (i = 0 ; entry->d_name[i] != 0 && i < MAXNAMLEN ; i++)
			*t++ = entry->d_name[i]; /* copy the name */
		*t++ = '\n';
		ret.n++;
	    }
	    
	}

	*t++ = '\0';
	closedir(dp);
	return(ret);
}
#endif

/*	GATH, to return an array of strings representing the names
 *	of the files in the directory.  This is not sorted.
 *	Note that a 'struct carray' has two pointers to blocks which
 *	must be free()ed.
 *	The newlines in the block returned by namget() are changed
 *	to nulls. From Dave Brownell's help program.
 */

struct carray gath(dir) char *dir;
{
	struct carray ret;
	struct dirlis names;
	char *calloc();

	names = namget(dir);
	ret.block = names.names;
	ret.array = (char **) calloc(names.n + 1, sizeof(char *));
	for (ret.num = 0; ret.num < names.n; ret.num++) {
		(ret.array)[ret.num] = names.names;
		while (*(names.names) != '\n') (names.names)++;
		*(names.names)++ = '\0';
	}
	ret.array[ret.num] = NULL;
	return(ret);
}
