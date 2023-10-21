#

/*
 * add-users  -  add user accounts
 *
 *
 * This program adds accounts to UNIX by making entries in the
 * accounting files shown below.
 *
 * Syntax:    # add-users <filename> [xx | -lp | -ep | -it | -67 | ...]
 *
 * where <filename> is the name of a file containing lines of the form:
 *
 * <login name>TAB<true name>TAB<classification>TAB<password>
 *
 * where <login name> is the proposed login name for the new user,
 * <true name> is his true name, <classification> is the user's status
 * and <password> is the initial password.
 *
 * Second argument is optional -- if it begins with a "-" it
 * specifies the site for creation reports (if omitted, site = -ep).
 * If it doesn't start with "-", no creation reports will be printed.
 * Changed to run on EE UNIX:
 * added accounting files for 2nd Houston and Centronics
 * --ghg 01/09/77
 *
 */
char    *PASSWORD       "/etc/6passwd";
char    *USERFILE       "/etc/user-file";
char    *SEEKFILE       "/etc/u-seek";
char    *DISKLIM        "/usr2/sys-usage/disk-limit";
char    *CCFILE         "/usr/ccd/account";
char    *EPFILE         "/usr/adm/pdufile.ep";
char	*MOWLEFILE	"/usr/adm/pdufile.67";
char	*LPFILE		"/usr/adm/pdufile.lp";

char    *STRIP          "/etc/strippw";
char    *MSKF           "/etc/mk-u-seek";
char    *USERTEMP       "/etc/temp-user";
char    *LOCK           "/tmp/ptmp";
char    *FSTMAIL        "/usr2/sys-usage/first_mail";
char    *USRDIR         "/usr";
char    *CRTRPT         "/usr2/sys-usage/creat_text";
char    *CRTEMP         "/tmp/crtemp";

int	MODE            0600;
#define	NUM             50              /* Maximum number of users to add */

struct {
	char lname[9];          /* Login name */
	char tname[30];         /* True name */
	char status[30];        /* Classification */
	char passwd[9];         /* Password */
	int  uid;               /* User-id */
} user[NUM];

char    buf[512];               /* Big buffer for various routines */
int     zero    0;
char    *printer        "-ep";  /* Default printer name */


main(argc, argv)
int argc;
char **argv;
{

	register j, k;
	extern fin, fout;
	int pid, num, fd, fd2, fd3, n;
	int fd4, fd5;
	char *pwd;

	if (getuid() != 0){
		printf("Not super-user.\n");
		exit();
	}

	if (argc < 2){
		printf("Syntax is %s <filename>\n", argv[0]);
		exit();
	}


	/* If there are three arguments, and the first character of
	 * the third argument (second parameter from shell) is "-",
	 * it is interpreted as the printer location.
	 */

	if (argc > 2) printer = (*argv[2] == '-') ? argv[2] : 0;


	/* Interlock with other password programs */

	if (stat(LOCK, buf) >= 0){
		printf("Password file busy--try again.\n");
		exit();
	}

	signal(1,1);		/* Ignore hangups */
	signal(2,1);		/* Ignore interrupts */
	signal(3,1);		/* Ignore quits */

	if(close(creat(LOCK,0)) < 0){
		printf("Can't create lock file %s\n", LOCK);
		exit();
	}

	runl(MSKF,MSKF,0);      /* regenerate seek file */

	fin=openfl(argv[1],0);
	num=getnew();           /* read in 'user' structure */
	close(fin);

	/* The next task to be implemented is the assignment of
	 * user id's and the comparison of the proposed login names
	 * with existing login names.  The seek-format file will be
	 * searched, and uids of vacant records (starting with '\0')
	 * will be assigned to each entry in the "user structure".
	 * At the same time, the login names in the seek-file will
	 * be compared with those in the structure.  If a match is
	 * found, the uid will be set to -1, a flag which will inhibit
	 * further processing of that entry (a message will also be
	 * printed).
	 */

	fd=openfl(SEEKFILE,2);
	assign(fd, num);
	close(fd);

	fd=openfl(PASSWORD,2);
	seek(fd,0,2);           /* Find end of file */
	printf("appending to %s\n",PASSWORD);
	flush();
	fout=fd;
	for(j=0; j<num; j++){
		if (user[j].uid == -1) continue;
		if (user[j].passwd[0] == 0) pwd = "";
			else pwd = crypt(user[j].passwd);
		printf("%s:%s:%d:1::%s/%s:\n",user[j].lname,pwd,
			user[j].uid,USRDIR,user[j].lname);
	}
	flush();
	close(fd);
	fout = 1;

	/* Next, directories must be created for each new user.
	 * Mail must be send and mailboxes set up.  To perform
	 * this, change directory the user directory "/usr",
	 * create the subdirectory, and then change directory to
	 * it.  Create ".mail" and "mbox" mode 600 and copy 
	 * the first mail message to ".mail".
	 */

	fd = openfl(FSTMAIL,0);
	for(k=0; k<num; k++){
		if (user[k].uid < 0) continue;
		chdir(USRDIR);
		if(
			runl("/bin/mkdir","mkdir",user[k].lname,0) ||
			chdir(user[k].lname) ||
			close(creat("mbox",0600)) ||
			(fd2 = creat(".mail", 0600)) < 0
		) {
			printf("problems with user dir %s\n",user[k].lname);
			printf("abort\n");  done();
		} else {
			chown(".",user[k].uid);
			chown("mbox", user[k].uid);
			chown(".mail", user[k].uid);
			seek(fd, 0, 0);
			while((j=read(fd, buf, 512)) > 0)
				write(fd2, buf, j);
		}
		close(fd2);
	}
	close(fd);

	/* Now the file of user-names must be updated.  Since this
	 * file is order-dependant, it must be edited to a temporary
	 * file and then copied back.  Fortunately, due to the method
	 * chosen for assigning UID's, the new entries will be added
	 * in the order in which they occur in the structure "user".
	 * Entries with a UID < 0 will be ignored.
	 */

	fd=openfl(USERFILE,2);
	fd2=creat(USERTEMP,MODE);
	flush();
	fout = fd2;

	j = 0;
	n = 1;
	for(k=0; k<num; k++){
		if (user[k].uid < 0) continue;
		if (j == 0) j = copyline(&n, user[k].uid);
		printf("%s\t%s\t%s\n", user[k].lname,
			user[k].tname, user[k].status);
	}
	if (j == 0) copyline(&n, 32767);
	flush();

	close(fd2);
	fout=1;

	fd2=openfl(USERTEMP,0);
	seek(fd, 0, 0);
	printf("%s modified\n",USERFILE);
	while((j=read(fd2, buf, 512)) > 0) write(fd, buf, j);
	close(fd);
	close(fd2);


	/* Now an entry must be prepared in the disk-limit file.
	 * Also, the line-printer accounting file must be reset
	 * for this uid.
	 */

	flush();
	fd = openfl(DISKLIM, 2);
	fout = fd;
	seek(fd, 0, 2);
	fd2 = open(EPFILE, 2);
	fd3 = open(CCFILE, 2);
	fd4 = open(MOWLEFILE, 2);
	fd5 = open(LPFILE, 2);
	for(k=0; k<num; k++){
		if (user[k].uid < 0) continue;
		if (fd >= 0) printf("200\t%s/%s\n", USRDIR, user[k].lname);
		if (fd2 >= 0){
			seek(fd2, user[k].uid*2, 0);
			write(fd2, &zero, 2);
		}
		if (fd4 >= 0){
			seek(fd4, user[k].uid*2, 0);
			write(fd4, &zero, 2);
		}
		if (fd5 >= 0){
			seek(fd5, user[k].uid*2, 0);
			write(fd5, &zero, 2);
		}
		if(fd3 >= 0) {
			seek(fd3, user[k].uid*16, 0);
			write(fd3, &zero, 2);
		}
	}

	flush();
	if (fd >= 0) close(fd);
	if (fd2 >= 0) close(fd2);
	if (fd3 >= 0) close(fd3);
	if (fd4 >= 0) close(fd4);
	if (fd5 >= 0) close(fd5);

	/* Generate creation reports, if desired (printer != 0) */

	if (printer){
		fout = creat(CRTEMP, MODE);
		fd = open(CRTRPT, 0);
		for(k=0; k<num; k++){
			if (user[k].uid < 0) continue;
			printf("\014UNIX Account Creation Report\n\n");
			printf("User: %s    %s\n\n", user[k].tname,user[k].status);
			printf("Login name: %s\n\n", user[k].lname);
			printf("Password: %s\n\n\n\n\n", user[k].passwd);
			flush();
			if (fd >= 0){
				seek(fd, 0, 0);
				while((j=read(fd, buf, 512)) > 0)
					write(fout, buf, j);
			}
		}
		write(fout, "\014", 1);
		close(fout);
		close(fd);
		fout = 1;
		runl("/bin/opr","opr",printer,CRTEMP,0);
	}
	runl("/bin/sh",STRIP,STRIP,0);
	runl(MSKF,MSKF,0);
	done();

}

getnew(){

	register j, k;

	for(j=0; j<NUM; j++){
		if(parse(user[j].lname)) return(j);
		if(parse(user[j].tname)) continue;
		if(parse(user[j].status)) continue;
		parse(user[j].passwd);
		user[j].passwd[8] = 0;
	}
	return(NUM);
}
parse(array)
char array[];
{

	register char c;
	register j;

	j=0;
	while(1) switch(c=getchar()){
		case 0: return(-1);
		case '\t':
		case '\n': array[j]=0;
			   return(c == '\t' ? 0 : 1);
		default: array[j++]=c;
	};
}
assign(fd, num){

	register j, k, count;
	int dupid;

	seek(fd, 0, 0);
	dupid=count=0;
	for(j=0; j<=num; j++){
		while(1){
			if(read(fd, buf, 64) < 64){
				while(j<num) if(user[j++].uid == 0)
					user[j-1].uid = count++;
				if(dupid) {
					printf("no accounts generated\n");
					done();
				}
				return;
			}
			for(k=0; k<num; k++) if (cmpar(user[k].lname,buf)){
				dupid=1;
				user[k].uid = -1;
				printf("Login name \"%s\" already in use\n",
					user[k].lname);
			}
			if (j == num) continue;
			if (buf[0] == 0 && user[j].uid == 0){
				user[j].uid = count++;
				break;
			}
			count++;
		}
	}
}

cmpar(str1, str2)
char *str1, *str2;
{

	register j;

	for(j=0; j<8; j++){
		if (str1[j] == 0 && str2[j] == ':') break;
		if (str1[j] != str2[j]) return(0);
	}

	return(1);
}
done(){
	unlink(CRTEMP);
	unlink(LOCK);
	unlink(USERTEMP);
	flush();
	exit();
}
copyline(n1, n2)
int *n1;
{

	register char c;

	/* This routine will copy lines, incrementing the count in "n1"
	 * until it either reaches "n2" or the EOT is reached.  A 0 is
	 * returned if the EOT was not read, a 1 if it was.
	 */

	while((*n1)++ < n2) while(1){
		if ((c=getchar()) == 0) return(1);
		putchar(c);
		if (c == '\n') break;
	}


	/* Having copied all lines up to the desired one, merely
	 * ignore the next line.
	 */

	while((c = getchar()) != '\n') if (c == 0) return(1);
	return(0);
}
runl(xa,xb,xc,xd,xe)
char *xa,*xb,*xc,*xd,*xe;
{
	extern errno;
	int pid;
	char **ps,*s;
	while((pid=fork()) < 0) sleep(5);
	if(pid == 0) {
		ps = &xa;
		while(s = *ps++)
			printf("%s ",s);
		printf("\n");
		flush();
		execl(xa,xb,xc,xd,xe);
		printf("can't exec, errno %d\n",errno);
		flush();
		exit(0123);
	}
	while(wait(&s) != pid);
	if((s>>8) == 0123)done();       /* exec fail */
	return(0);
}

openfl(s,m)
char *s;
{
	int i;
	if((i=open(s,m)) < 0){
		printf("can't open %s, abort\n",s);
		done();
	}
	return(i);
}
