#
/*
 *  Disk-usage accounting program
 */

#define	USERS	1500
#define LIMFILE         "/usr2/ghg/sys-usage/disk-limit"
#define USRFILE         "/usr2/ghg/sys-usage/usr-usage"
#define USR2FILE        "/usr2/ghg/sys-usage/usr2-usage"
#define EXCEED          "/usr2/ghg/sys-usage/exceed"

int mflg;
int limit[USERS];
char name[USERS][15];
int exfd;
int m20	-20;

struct {
	char name[15];
	char over;
	int consec;
	int total;
} exrec;


main(argc, argv)
int 	argc;
char	**argv;
{
	extern fin, fout;
	int fd, j, k, jk;
	int usage, pass, lim;
	char id[15];

	mflg=0;
	if (argc > 1 && argv[1][0] == '-') switch(argv[1][1]){
		case 's': summary();
		case 'z': zero();
		case 'm': mflg++;
	}


	fout = dup(1);

	/* Read in file containing limits for each login name */

	if ((fd=open(LIMFILE,0)) < 0){
		printf("Can't open file %s\n", LIMFILE);
		exit();
	}

	/* Open file containing historical data (login names and
	 * counts of excesses by all users who have exceeded their
	 * quota.  The records contain both consecutive and total
	 * excesses.  If the file cannot be opened, it will not
	 * be created.  An error message is issued only for "root"
	 * and the program will continue.
	 */

	if(newday() < 0 && mflg && (getuid() == 0))
		printf("Can't open %s\n", EXCEED);

	fin=fd;
	for (k=0; scan(&limit[k], name[k]) == 0; k++);

	/* Pass 1:  Use file for "usr"
	 * Pass 2:  Use file for "usr2"
	 * 
	 * Read in disk usage and login-directory name.
	 * Compare with internally-stored allotment and
	 * call function "exceed" if limit was exceeded.
	 * Function "notfnd" is found if the limit is undefined.
	 */
	for(pass=1; pass < 3; pass++){
		if (fd != -1) close(fd);
		if ((fd=open(USRFILE,0)) < 0){
			printf("Can't open file %s\n", USRFILE);
			goto endlp;
		}
	
		if (pass == 2 && (fd=open(USR2FILE,0)) < 0){
			printf("Can't open file %s\n", USR2FILE);
			goto endlp;
		}
		fin=fd;
	
		while(scan(&usage, id) == 0){
			lim=0;
			for(j = 0; j < k; j++){
				if (cmpar(id, name[j]) == 1) lim=limit[j];
			}
			if (lim == 0) notfnd(id, usage);
			else if (usage > lim) exceed(id, usage, lim);
		}
	endlp:;
	}

	flush();
	if (fd != -1) close(fd);
	cleanup();
}
notfnd(id, usage)
char id[15];
int usage;
{
	/* This routine merely prints out an error message on the
	 * standard output if no limit was found for an account
	 */
	printf("User \"%s\" -- no limit defined -- using %d\n", id, usage);
}
exceed(id, usage, lim)
char id[15];
int usage, lim;
{
	/* This routine prints out on the standard output
	 * the pathnames, usage, and quotas for users exceeding
	 * their quota.  In addition, it will attempt to send
	 * a warning message to that user's mailbox.  An error
	 * message is output if the mailbox cannot be written
	 * (will not create a mailbox).
	 */

	int fd, count;
	register int i, j;
	char	mailname[32];
	int tvec[2];
	extern fout;

	if (lim == 32767) return; /* 32767 designates no limit */
	printf("User \"%s\" exceeds limit:  %d > %d", id, usage, lim);
		if ((count=update(id)) > 2)
		printf("  ** %d consecutive days **", count);
	putchar('\n');
	if (! mflg) return;
	if (getuid() != 0) return;	/* Only "root" can send mail */
	for (i = 0; id[i] != 0; i++) mailname[i] = id[i];
	for (j = 0; j < 7; j++) mailname[i+j] = ("/.mail")[j];
	time(tvec);
	if((fd=open(mailname,1)) < 0){
		printf("Can't open %s\n",mailname);
		return;
	}
	if(seek(fd,0,2) < 0){
		printf("Seek on %s failed -- no mail sent\n", mailname);
		close(fd);
		return;
	}

	flush();	/* Flush output on previous file descriptor */
	fout = fd;	/* Set output to mail file */
	printf("\nMESSAGE FROM SYSTEM STAFF:\n");
	printf("%s\n",ctime(tvec));
	printf("Our latest disk accounting shows that your account,");
	printf(" using\n");
	printf("%d blocks ", usage);
	if (usage - lim > 100) printf("greatly ");
	printf("exceeds your quota of %d blocks.\n", lim);
	if (count >= 2){
		printf("You have been over for %d", count);
		printf(" consecutive days\n");
	}
	flush();
	close(fd);
	fout = 1;	/* Return to standard output */
}
cmpar(name1, name2)
char name1[15], name2[15];
{
	/* This routine compares two 15-character strings.  If they
	 * are identical a 1 is returned, if unequal a 0 is returned.
	 */
	int jk;

	for (jk = 0; jk < 15; jk ++) if (name1[jk] != name2[jk]){
		return(0);
		}
	return(1);
}
scan(lim, id)
int *lim;
char id[15];
{


	/* This routine clear out the array "id" (passed as an argument)
	 * and then fills it and the variable "lim" with values
	 * obtained by a "scanf" call.  ("scanf" alone could be
	 * used except that the character array for the directory
	 * name must be zeroed first.
	 */
	int k;

	for (k=0; k < 15; k++) id[k] = 0;
return(scanf("%d %*[ \t] %[^\n] %*[\n]", lim, id));
}


newday(){

	/* This routine opens the file containing the names and
	 * disc excess counts.  It clears the byte which indicates
	 * to "cleanup" that the account was in excess today 
	 */


	if (! mflg || (exfd = open(EXCEED,2)) < 0) return(exfd= -1);
	while(1){
		if (read(exfd, &exrec, 20) < 20 ||
			exrec.name[1] == 0) return(0);
		exrec.over=0;
		seek(exfd, m20, 1);
		write(exfd, &exrec, 20);
	}
}
update(id)
char id[15];
{
	register k;

	/* This routine, called by "exceed", searches through the
	 * file containing the users who have exceeded their quota
	 * at some past time.  It sets a flag to indicate that the
	 * user exceeded the quota during this accounting session,
	 * and increments the total count and consecutive count
	 * of excesses for each user.
	 */

	if (exfd < 0) return(-1);

	seek(exfd, 0, 0);


	while(1){
		if (read(exfd, &exrec, 20) < 20 ||
			exrec.name[0] == 0) break;

	if (cmpar(id, exrec.name)){
			seek(exfd, m20, 1);
			exrec.over++;
			exrec.consec++;
			exrec.total++;
			write(exfd, &exrec, 20);
			return(exrec.consec);
		}
	}

	for (k=0; k < 15; k++) exrec.name[k] = id[k];
	exrec.consec = exrec.total = 1;
	exrec.over = 1;
	write(exfd, &exrec, 20);
	return(1);
}
cleanup(){


	/* This routine finishes the operations on the file containing
	 * excessive users' names.  It checks to see if the user
	 * exceeded his quota during this accounting run.  If not,
	 * it clears the tally of consecutive excesses.  After
	 * processing all entries in the file, it closes it.
	 */
	if (exfd < 0) return;


	seek(exfd, 0, 0);
	while(1){
		if (read(exfd, &exrec, 20) < 20 ||
			exrec.name[0] == 0){
				close(exfd);
				return;
		}

		seek(exfd, m20, 1);

		if(exrec.over == 0) exrec.consec = 0;
		write(exfd, &exrec, 20);
	}
}

summary(){

	register j, k, fd;

	if ((fd=open(EXCEED,0)) < 0){
		printf("Can't open file %s\n", EXCEED);
		exit();
	}

	printf("\t\tDisk Usage Summary\n\n");
	printf("Directory Name  \t Consec.\t Total\n");
	printf("--------------  \t -------\t -----\n\n");

	while(read(fd, &exrec, 20) == 20) print();

	close(fd);
	printf("\n\n");
	exit();
}
zero(){

	register j, k, fd;

	if ((fd=open(EXCEED,2)) < 2){
		printf("Can't open file %s for read/write\n", EXCEED);
		exit();
	}

	while(read(fd, &exrec, 20) == 20){
		if (exrec.consec <= 3) continue;
		exrec.total = 0;
		seek(fd, m20, 1);
		write(fd, &exrec, 20);
	}

	close(fd);
	exit();
}
print(){

	if (exrec.total == 0) return;
	printf("%-15s \t%5d\t\t%5d", exrec.name, exrec.consec,
	  exrec.total);
	if (exrec.consec > 2) printf("  ** VIOLATION **");
	putchar('\n');
}
