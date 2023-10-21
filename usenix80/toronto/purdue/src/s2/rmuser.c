#

/*
 *      remove user
 *
 *      # rmuser <filename>
 *
 *      where "filename" contains the login names of the
 *      accounts to be deleted, one per line.
 *
 *      Accounts are removed by editing the group of system
 *      accounting files shown below; during editing they
 *      are copied to temporary files.  An interlock file
 *      (/tmp/ptmp) is used to prevent interference with /bin/passwd.
 *
 *      After completion the system manager should: (1) run /etc/strippw
 *      and /etc/mk-u-seek  (2) build a shell file to remove directories
 *      using /etc/rmall, watching the link count carefully.
 *
 */

#define	PASSWORD	"/etc/6passwd"
#define	USERFILE	"/etc/user-file"
#define DISKLIM         "/usr2/sys-usage/disk-limit"

#define	PASSTEMP	"/etc/temp-pass"
#define	USERTEMP	"/etc/temp-user"
#define LIMTEMP         "/etc/temp-lim"

#define	LOCK		"/tmp/ptmp"
#define	MODE	0600
#define	NUM	500	/* Maximum number of users per pass */


/*      The following structure contains the names and flags
 *      for each account to be deleted.  It is read as the first
 *      action of this program.  The "flags" field is currently
 *      not used, but will eventually contain information as to
 *      whether or not the specified user could not be located in
 *      the password or user-name files.
 */

struct {
	char name[10];
	char flags;
} names[NUM];

char buf[512];          /* Buffer */
int num;
int     fd;
extern  fin;


main(argc, argv)
char **argv;
{
	register n, j;

	if (getuid() != 0){
		printf("Not super-user.\n");
		exit();
	}

	if (argc < 2){
		printf("Syntax is %s <filename>\n", argv[0]);
		exit();
	}

	/* Check to see if modification of the password
	 * file is underway (someone running "passwd")
	 * and if so, terminate this process.
	 */

	if (stat(LOCK,buf) == 0) {
		printf("Password file is busy -- try again.\n");
		exit();
	}


	/* Turn off signals 1, 2, and 3 so that the program won't be
	 * interrupted while changing critical files.
	 */

	signal(1,1);
	signal(2,1);
	signal(3,1);


	/* Create lock file to prevent other use of the
	 * password program while this is running.
	 */

	if (close(creat(LOCK, 0)) < 0){
		printf("Unable to create %s\n", LOCK);
		exit();
	}


	/* Read in names of users to delete from system */

	if ((fin=open(argv[1],0)) < 0){
		printf("Unable to open %s\n", argv[1]);
		done();
	}

	getnames();
	close(fin);

	openfs(PASSWORD,PASSTEMP);

	/* Copy all lines in password file except those with matching
	 * names.  This deletes the old users from the password file
	 */

	while(n=getline()) if (!match(0,1)) write(fd, buf, n);

	/* Edit the user-name file */

	openfs(USERFILE,USERTEMP);
	while(n=getline())
		if (!match(0,2)) write(fd, buf, n);
			else write(fd, "\n", 1);

	/* Edit the disk-limit file to the temporary file, removing
	 * all entries with matching login-names.
	 */

	openfs(DISKLIM,LIMTEMP);
	while(n=getline()){
		j=0;
		while(buf[j++] != '/');
		while(buf[j++] != '/');
		if (!match(j,4)) write(fd,buf,n);
	}
	close(fd);  close(fin);

	/* At this point, the updated files are in the temporary files.
	 * Now copy them back to the originals.
	 */

	copy(USERTEMP, USERFILE);
	copy(LIMTEMP, DISKLIM);
	copy(PASSTEMP, PASSWORD);
	printf("%s done - run mk-u-seek,strippw,rmall\n", argv[0]);
	done();
}

openfs(old,temp)
char *old,*temp;
{
	close(fd);  close(fin);
	if((fin=open(old,0)) < 0) {
		printf("can't open %s\n",old);
		done();
	}
	if((fd=creat(temp,MODE)) < 0) {
		printf("can't creat %s\n");
		done();
	}
}

copy(from,to)
char *from,*to;
{
	int fda,fdb,n;
	if ((fda=open(from,0)) < 0){
		printf("Can't open %s, abort\n", from);
		done();
	}

	if ((fdb=creat(to,MODE)) < 0){
		printf("Can't create %s, abort\n", to);
		done();
	}

	printf("%s modified\n", to);
	while((n=read(fda,buf,512)) > 0) write(fdb,buf,n);
	close(fda);
	close(fdb);
}

done(){
	unlink(PASSTEMP);
	unlink(USERTEMP);
	unlink(LIMTEMP);
	unlink(LOCK);
	exit();
}

getnames(){

	register j, flag;
	register char c;

	/* This routine reads lines via "getchar" and takes the first
	 * string on the line as a login name of an account to be
	 * deleted.  This name is then placed in the "names" structure.
	 */

	flag = j = 0;
	num = 0;
	while((c=getchar()) != 0){
		if (c == '\n'){
			names[num++].name[j] = 0;
			flag = 0;
			j = 0;
			if(num < NUM)
				continue;
			else {
				printf("only removed first %d names\n",NUM);
				return;
			}
		}

		if (flag || c == ' ' || j > 7) flag = 1;
		if (flag) continue;

		names[num].name[j++] = c;
	}
}

getline(){

	register j;

	/* This routine reads lines via "getchar" and places an entire
	 * line into the buffer "buf".  It returns the number of
	 * characters (including the '\n'), so 0 means the end-of-file
	 * was reached.
	 */

	j = 0;
	while(buf[j] = getchar())
		if (buf[j++] == '\n') return(j);

	return(0);
}

match(col, arg){

	register j, k, n;

	/* This routine compares the string in "buf" (starting at "col" and
	 * terminated by either a space, tab, newline, or colon) with the
	 * array of accounts to delete.  It sets the appropriate flag
	 * in the structure "names" and returns 1 if a match was
	 * found; otherwise it returns 0.
	 */

	for (j=0; j < num; j++){
		k = 0;
		n = col;
		while(names[j].name[k++] == buf[n++]);
		if (names[j].name[--k] != 0) continue;
		if (buf[--n] == '\t' || buf[n] == ' ' ||
			buf[n] == ':' || buf[n] == '\n' || k > 7){

			names[j].flags =| arg;
			return(1);
		}
	}
	return(0);
}
