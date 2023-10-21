#

/*
 *      Extraction program
 *
 * This program extracts the blocks of a file specified by inode number from
 * magnetic tape and creates a disc file containing these blocks.  The
 * syntax is:
 *
 * # trino [<device>] <inode>
 *
 * Only one file may be extracted per run.  The magnetic tape must have
 * been created with "dd".
 *
 * If the first character of the device name is "-", "trino" will
 * skip one file on the tape before starting in on the raw format.
 *
 */

#include "/usr/sys/filsys.h"
#include "/usr/sys/ino.h"

struct inode    node;

char *tape "/dev/rmt0";         /* Device containing filesystem */
int buffer[50][256];		/* Buffer for input of records */
unsigned blklst[256 * 8 + 1];   /* List of blocks in file */
int nrec -1;                    /* Number of records (50 blocks) read */
int flag 0;			/* Flag -- =1 if 1st file is ncheck */
int fd;                         /* File descriptor of tape */
int df;                         /* File descriptor for disc file */
char *fname;                    /* Points to ASCII inode number string */

main(argc, argv)
char **argv;
{
	register inum, j, k;
	int *p2;
	struct filsys *p;

	/* Get arguments to the program.  If less than 2 arguments
	 * (the first of which is actually the name of the program),
	 * exit with an error message.  Otherwise, examine the
	 * arguments to see if they match the desired syntax.
	 */

	switch(argc){
		case 2: if ((inum=atoi((fname=argv[1]))) == 0)
				goto case_bad;
			break;

		case 3: if ((inum=atoi((fname=argv[2]))) == 0)
				goto case_bad;
			if (argv[1][0] != '-') tape = argv[1];
			else {
				if (argv[1][1] != 0) tape = &argv[1][1];
				flag++;
			}
			break;

		case_bad:
		default:
			printf("Syntax is \"%s [<device>] <inumber>\"\n",
				argv[0]);
			exit();
	}

	if ((fd=open(tape, 0)) < 0){
		printf("ABORT: Can't open %s\n", tape);
		exit();
	}

	if (flag) skipf();

	for(j=0; j < (sizeof blklst)/2; j++) blklst[j] = 0;


	/* First step -- read in first 50 blocks.  Using the definition
	 * of the "super-block" as a prototype, determine the number of
	 * blocks of inodes that exist.  Check this value against the
	 * requested inode number.  If the requested inode number does
	 * not exist, print an error message and exit.
	 */

	read50();               /* First buffer-load */
	p = &buffer[1][0];
	if (p->s_isize * 16  <  inum) errxit("No such inode exists");


	/* Next, locate the desired inode.  Block 2 contains the first
	 * 16 inodes, etc.  Thus, calculate the block number the inode
	 * lies in.  If this is greater than 49, read the required
	 * number of records to locate the desired block.  Copy the
	 * inode into the structure "node".
	 */

	j = (inum - 1)/16 + 2;          /* Block number */
	while(j > 49){
		read50();
		j =- 50;
	}                               /* j now is block # within record */


	p2 = &node;
	for(k=0; k<16; k++)
		*(p2++) = buffer[j][k+16*((inum-1)%16)];
	prinode();

	/* Now that the inode has been obtained, check to make sure that
	 * it is allocated.  If not, take an error exit.  If so, determine
	 * if it is small, large, or huge.  Huge files cannot be handled,
	 * so if it is huge, take an error exit.
	 */

	if ((node.i_mode & IALLOC) == 0) errxit("Inode not allocated");
	if (node.i_mode & IFMT) errxit("Device or directory -- not a file");

	if ((df=creat(fname, node.i_mode & 0777)) < 0)
		errxit("Can't create disc file");
	j = node.i_uid0 & 0377 | (node.i_uid1 << 8);
	chown(fname, j);

	if ((node.i_mode & ILARG) == 0) {
		printf("Type: small\n");
		for(k=0; (blklst[k] = node.i_addr[k]) != 0; k++);
		getblks(8);
		done();

	} else {

		if (node.i_addr[7] != 0){
			unlink(fname);
			errxit("Huge file -- sorry");
		}
		printf("Type: large\n");

		/* The block list must now be obtained by reading the
		 * indirect blocks.  Then the file itself can be extracted.
		 */

		getindir();
		getblks((sizeof blklst)/2);
		done();

	}
}

read50(){

	/* This routine reads a 50-block record from the tape device
	 * and will update the record counter.  A tape error will cause
	 * a -1 return from the read, which will cause an error exit.
	 */

	if (read(fd, buffer, sizeof buffer) < sizeof buffer)
		errxit("TAPE ERROR");
	nrec++;
}

errxit(str)
char *str;
{

	/* This routine prints the error message passed as an argument
	 * on the standard output and then closes the tape fd and
	 * exits.
	 */

	printf("ABORT: %s\n", str);
	exit();
}

getblks(length){

	register i;
	unsigned register *k;


	/* This routine will search through the magnetic tape for
	 * the desired blocks, extracting them in numerical order
	 * and placing them in the output disc file.
	 *
	 * The algorithm is rather inefficient -- the entire list is
	 * scanned for the lowest block number.  The index into the
	 * list is saved and then the block number is zeroed.  When
	 * only zeros remain in the list, the search terminates.
	 */

	printf("Getting data blocks\n");
	while ((i = nxtblk(blklst, length)) >= 0){
		seek(df, i, 3);         /* Position disc file */
		k = &blklst[i];
		while (*k >= (nrec+1)*50) read50();
		write(df, &buffer[*k%50][0], 512);
		*k = 0;
	}

}

nxtblk(list, length)
int *list;
{
	register i, j;
	unsigned register k;


	i = -1;
	k = 65535;
	for(j=0; j<length; j++){
		if (list[j] == 0 || list[j] > k) continue;
		k = list[(i=j)];
	}

	return(i);
}

prinode(){

	register k;

	printf("Inode statistics:\n");
	printf("Mode: %o\nLinks: %d\n", node.i_mode, node.i_nlink);
	k = node.i_uid0 & 0377 | (node.i_uid1 << 8);
	printf("Owner: ");
	usrname(k);
	putchar('\n');
}
getindir(){

	unsigned register *k;
	register i,j;
	unsigned int j2;

	/* This routine gets the indirect blocks specified by the
	 * inode for a large file and then rewinds the tape (via
	 * a close/open) for the next pass, if necessary.  This
	 * routine is not used for small files since they have no
	 * indirect blocks.
	 */

	printf("Locating indirect blocks\n");
	while((i=nxtblk(node.i_addr, 8)) >= 0){
		k = &node.i_addr[i];
		while(*k >= (nrec+1)*50) read50();
		for(j=0; j<256; j++) blklst[j+256*i]  = buffer[*k%50][j];
		*k = 0;
	}


	/* No rewind is necessary if the address of the lowest block of
	 * the file is either within the current buffer-load in core
	 * or has not yet been read from the tape.
	 */

	j2 = blklst[nxtblk(blklst, (sizeof blklst)/2)];  /* Lowest block # */

	if (j2 < nrec*50){
		printf("Pass 2 rewind required\n");
		nrec = -1;
		close(fd);
		if ((fd=open(tape, 0)) < 0){
			printf("ABORT: Can't re-open %s for pass 2\n", tape);
			exit();
		}
		if (flag) skipf();
		read50();               /* Prime the buffer */

	} else printf("No pass 2 rewind required\n");

}

done(){

	printf("File extraction complete.\n");
}
usrname(uid){

	/* Prints user name for "uid" or prints "uid" if no existing
	 * user has that numeric id.
	 */

	register j;
	char pwbuf[64];

	if (getpw(uid, pwbuf) < 0){
		printf("%d", uid);
		return;
	}

	for(j=0; j<64; j++){
		if (pwbuf[j] != ':') continue;
		pwbuf[j] = 0;
		printf("%s", pwbuf);
		return;
	}
}
skipf(){

	/* Skip one file on the magnetic tape */

	printf("Skipping \"ncheck\" file\n");
	while(read(fd, buffer, sizeof buffer) > 0);
}
