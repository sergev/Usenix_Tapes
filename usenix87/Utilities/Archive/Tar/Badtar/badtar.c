/* read tar tapes with bad blocks MWS */
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#define TBLOCK 512
#define NAMSIZ 100
#define TAPEBLOCK 10240

/* see tar (5) in the manual */
union hblock {
     char dummy[TBLOCK];
     struct header {
          char name[NAMSIZ];
          char mode[8];
          char uid[8];
          char gid[8];
          char size[12];
          char mtime[12];
          char chksum[8];
          char linkflag;
          char linkname[NAMSIZ];
     } dbuf;
};
char pad[TBLOCK];

/* check the check sum for the header block */
check_sum(c)
union hblock *c;
{
	int i,j;
	char *cp;
	cp = c->dummy;
	i = 0;
	for (j = 0; j < TBLOCK; j++) i += *cp++;
	for (j = 0; j < 8; j++) i -= c->dbuf.chksum[j];
	for (j = 0; j < 8; j++) i += ' ';
	return(i);
}

char buf[TAPEBLOCK];
char xbuf[TAPEBLOCK];
int bpos = TAPEBLOCK - TBLOCK, eot, bad, fd1;
FILE *logf;

/* get the next TBLOCK chars from the tape */
char *get_next()
{
	int res;
	if (bpos == TAPEBLOCK - TBLOCK) {
		bcopy(xbuf, buf,TAPEBLOCK);
		res = read(fd1,buf,TAPEBLOCK);
		if (res == 0) eot = 1;
		if (res < TAPEBLOCK) {
			bad = 1; 
			fprintf(logf, "***Bad block on tape!!\n");
		}
		else {
			if (bad) fprintf(logf, "***End of bad block(s) on tape\n");
			bad = 0;
		}
		bpos = 0;
	}
	else bpos += TBLOCK;
	return(&buf[bpos]);
}

main(argc, argv)
char **argv;
{
	int i, size, chksum, fblocks, eot_block;
	union hblock *hp;
	char tape[20];
	strcpy(tape, "/dev/rmt8"); /* default */
	logf = stderr;
	i = 1;
	
	/* get arguments */
	while (argc > i && argv[i][0] == '-') {
		switch (argv[i][1]) {
			case 'f' : 
				if (argc > i +1) {
					strcpy(tape,argv[++i]);
					}
				else {
					fprintf(stderr, "No tape drive name given\n");
					exit(10);
				}
				break;
			case 'l':
				if (argc > i + 1) {
					if ((logf = fopen(argv[++i], "w")) == NULL) {
						perror("Can't open log file\n");
						exit(11);
					}
				}
				else {
					fprintf(stderr, "No log file name given\n");
					exit(12);
				}
				break;
			default: 
				fprintf(stderr, "usage %s [-l logfile] [-f tape drive]\n",
				  argv[0]);
				exit(13);
		}
		i++;
	}
	
	/* first char cannot be a 0 */
	pad[0]  = 'x';
	
	/* don't quite know what the tape driver will return, so fill buffer
	   with non zero rubbish
	 */
	for (i = 0; i < TAPEBLOCK; i++) xbuf[i] = 'x';
	
	/* open the tape drive */
	if ((fd1 = open(tape, O_RDONLY,0)) < 0) {
		perror("can't open tape");
		exit(1);
	}
	while (1) {
		hp = (union hblock *) get_next();
		/* tar tests the first char to see if it is an end of tape (eot) block
		   or not. Can't see why it doesn't use tape marks. Maybe they weren't
		   invented when it was written?
	 	*/
	 	/* get a tar block */
		
		if (hp->dbuf.name[0] == '\0' && !bad && !fblocks) {
			/* skip possible eot block (there are two of them) */
			fprintf(logf, "***End of tape block\n");
			eot_block++;
			if (eot_block < 2) continue;
			eot = 1;
		}
		/* note if the last block read is bad there may be rubish
		  (old info from the last write at the end of it) this may cause
		  some of the last files which are extracted to be partially
		  overwritten. There is very little one can do about this
		  (except pray)
		*/
		
		/* end of tape ?? */
		if (eot) {
			if (fblocks) {
				fprintf(logf,"***Last file Truncated. File padded!!\n");
				while (fblocks--) write(1, pad, TBLOCK);
			}
			/* write two blank (eot) blocks */
			pad[0] = '\0';
			write(1, pad, TBLOCK);
			write(1, pad, TBLOCK);
			exit(1);
		}
		
		eot_block = 0;
		
		if (fblocks && !bad) { /* all ok in the middle of a file */
			write(1, hp, TBLOCK);
			fblocks--;
			continue;
		}
		
		/* have we got a header ?? */
		sscanf(hp->dbuf.size, "%lo", &size);
		sscanf(hp->dbuf.chksum, "%o", &chksum);
		if(check_sum(hp) == chksum && hp->dbuf.name[0] != '\0') {
		
			/* we have a header */
			if (fblocks) {
				fprintf(logf, "***Truncated!! File padded!!\n");
				while (fblocks--) write(1, pad, TBLOCK);
			}
			fblocks = (size%TBLOCK) ? size/TBLOCK + 1 : size/TBLOCK;
			fprintf(logf,"%s\n", hp->dbuf.name);
			write(1,hp,TBLOCK);
			continue;
		}
		
		/* not a header */
		if (fblocks == 0) {
			/* throw it away! */
			fprintf(logf, "***Deleted block!!\n");
			continue;
		}
		fblocks--;
		write(1,hp,TBLOCK);
	}
}
