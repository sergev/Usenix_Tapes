#
/*
 *	help -- print system documentation
 *
 */

/*	G L O B A L    D E C L A R A T I O N S	*/

#include	"/usr/sys/sbuf.h"
#define		ifdir	040000
#define WIDTH 3		/* num colums of directory output */

char		*root	"/usr2/news";
char		*man	"/usr2/news/manual/chapter1\0\0";
#define CHAPNUMPOS	25

/*char *erase "\033E\0\0\0";	/* super bee erase string */
char *erase "\032\0\0\0\0";	/* ADM-3 erase string */
#define ETEK	"\033\014"	/* tek 4010 erase string */
char elen 4;		/* len of erase string */

struct inode sbuf;

int bound	0;
int nroffpid	0;
/*int page 24;		/* super bee line len */
int page 23;		/* ADM-3 page length */
int pflg;		/* enable paging */
int fout;		/* output file descriptor */
int printfd;		/* fd for doc output */
int toplevel	0;	/* flag if help is shell */

#define ERASE(fd)	write(fd, erase, elen)

int (*iofcn)();		/* io fcn for print */

main(argc,argp)
int	argc;
char	**argp;
{
	register char *p;
	int cnt, errors, i;
	char *name;
	extern int write(), pause(), finis(), reset();

	iofcn = &write;		/* default output fcn */
	if(**argp == '-'){
		toplevel = 1;
		pflg++;		/* select pause mode */
	}
	if(fstat(2,&sbuf) == -1){
		open("/dev/tty", 2);	/* activate fd2 */
	}
	fout = open("/dev/tty",2);
	cnt = errors = 0;
	
	while(--argc){
		p = *++argp;
		if(*p == '-')
			switch(p[1]){
			case 'p':
				pflg++;
				break;

			case 'l':
				i = atoi(p+2);
				if(i > 0)
					page = i;
				pflg++;
				break;

			case 'e':
				erase = p+2;
				elen = len(erase);
				pflg++;
				break;

			default:
				printf("bad flag: %s\n", p);
				errors++;
			}
		else
			switch(cnt++){
			case 0:
				name = p;
				break;

			case 1:
				decodesect(p);
				break;

			case 2:
				printf("syntax: help [-p] [-lnnn] [-eccc] [name [chapter]]\n");
				errors++;
			}
	}
	if(errors){
		flush();
		exit();
	}
	signal(2, 1);
	tty(0);
	signal(2, finis);
	elen = len(erase);
	if(pflg)
		iofcn = &pause;
	if(cnt == 0)
		processinput(root);
	else
		searchdir(name);
	finis();
}
/**/
/*
 *	search various directories (starting at documentation "root")
 *	for the specified "file".
 */
searchdir(file)
char	*file;
{
	chdir(man);
	if(findfile(file)  ==  -1)  {
		chdir(root);
		if(findfile(file)  ==  -1)  {
			printf("Can't find documentation on \"%s\"\n",file);
			finis();
		}
	}
	finis();
}
/**/
/*
 *	If input is a file -- print it.  If it is a directory,
 *	pass control to processinput.  Otherwise, return an error code.
 */
findfile(file)
char	*file;
{

	if(stat(file,&sbuf)  !=  -1)  {
		if((ifdir & sbuf.i_mode)  ==  ifdir)
			processinput(file);	/*** never returns ***/
		print(file);
		return(1);
	} else
		return(-1);
}
/**/
/*
 *	list a directory, if there is only 1 active entry, determine
 *	its state (directory or file) and act accordingly.
 */
lsdir(dir_name)
char	*dir_name;
{
	char	*save,	*dptr;
	int	code;
	save	= dir_name;
	code	= 0;
	
	list(dir_name);
	while(bound  ==  1)  {
		dptr = scandir(1);
		stat(dptr,&sbuf);
		if((ifdir & sbuf.i_mode) == ifdir)  {
			chdir(dptr);
			list(".");
		} else	{
			print(dptr);
			code = 1;
			chdir(save);
			chdir("..");
			list(".");

		}
	}
	return(code);
}
/**/
/*
 *	list a directory's active entries (excluding entries
 *	beginning with ".").  If there are no active entries,
 *	output the fact and reprompt.
 */
list(dir_name)
char	*dir_name;
{
	int	fddir,  inc,  ctr,  red;
	struct {
		int ino;
		char name[16];
	} dir;

	if(pflg)
		ERASE(1);
	dir.name[14] = 0;
	red 	= 16;
	ctr	= 1;

	chdir(dir_name);
	printf("\nCurrent topics are:\n\n");
	if((fddir = open(".",0))  !=  -1)  {
		while(red == 16)  {
			inc = 0;
			while(inc < WIDTH  && (red = read(fddir,&dir,16)) > 0)  {
				if(dir.ino != 0 && dir.name[0] != '.'){
					printf("%3d:%-14s ",ctr++,dir.name);
					inc++;
				}
			}
			putchar('\n');
		}
		close(fddir);
		if(ctr == 1)  {
			printf("directory %s has no entries\n",dir_name);
			chdir("..");
		} else	{
			bound = ctr -1;
		}
	}  else	{
		printf("cant read %s\n",dir_name);
		ctr = 0;
	}
	return;
}
/**/
/*
 *	decode the optional chapter specification of the argument
 *	string.  An optional argument that can be passed to help is
 *	the chapter to consider when searching for a command.  The
 *	UNIX manual chapter consist of a string of digits, with "1"
 *	being the default.  Chapter 1 details shell commands.
 */
decodesect(num)
char	*num;
{

	if(*num){
		man[CHAPNUMPOS] = num[0];
		if(num[1])
			man[CHAPNUMPOS+1] = num[1];
		return(0);
	} else
		return(1);	/* error */
}
/**/
/*
 *	prompt the user for input.  Valid input is one of the
 *	following:
 *	<file>		a file listed by the previous list;
 *			implies <file> in the present working
 *			directory.
 *	<number>	shorthand notation for <file>
 *	/		reset the present working directory at
 *			the documentation root and continue
 *			prompting and accepting input.
 *	..		move toward the root one level;
 *			equivalently, change the present
 *			working directory to the parent
 *			directory of the present working
 *			directory.
 *	ctrl-d		stop this program immediately.
 */
char *input()
{
	static char inbuf[70];
	int i;
	char	*inptr;
	int	offset;
	extern	int  fin;
	inptr = inbuf;

	printf("\n\n\nselect topic ('?' for help): ");
	flush();
	if((i = read(fin,inbuf,70))  <=  0)  {
		putchar('\n');
		finis();
	}
	if(inbuf[0] == '$') {
		putchar('\n');
		finis();
	}
	inbuf[i-1] = 0;
	inptr = inbuf;
	if(*inptr >= '0' && *inptr <= '9') {
		offset = atoi(inbuf);
		return(scandir(offset));
	} else	if(compstr("/",inbuf))  {
			return(root);
	} else if(compstr("?",inbuf)) {
		help();
		return(".");
	} else	{
		return(inptr);
	}
}
/*  */
/*
 *	output the contents of a file.
 */

int pcnt;
int bscnt;
int nlcnt;
#define TOOMANY 4	/* too many consecutive '\n' */

print(file)
char	*file;
{	
	register int cnt;
	char buf[512];
	extern nget(), read();

	pcnt = nlcnt = bscnt = 0;
	if((printfd = open(file,0))  !=  -1)  {
		if(cnt = nroff(printfd, file)){
			close(printfd);
			printfd = cnt;
		} else
			seek(printfd, 0, 0);
		if(pflg)
			ERASE(1);
		while((cnt = read(printfd, buf, 512)) > 0)
			(*iofcn)(1, buf, cnt);
		if(pflg)
			hold(1);
		close(printfd);
	}  else	{
		printf("cannot open %s for reading\n",file);
		sleep(2);
	}
	printfd = 0;
}

pause(fd, buf, cnt)
char *buf;
{
	register char *p;
	register i,n;

	p = buf;
	i = 0;
	while(i++ < cnt  &&  pcnt < page){
		switch(*p){
		case '\b':
			bscnt++;
			*p = 0;
			nlcnt = 0;
			break;
		case '_':
			if(bscnt){
				bscnt--;
				*p = 0;
			}
			nlcnt = 0;
			break;
		case '\014':
			pcnt = page;
			break;
		case '\n':
			bscnt = 0;
			if(nlcnt++ > TOOMANY)
				*p = 0;
			else
				pcnt++;
			break;
		default:
			bscnt = 0;
			nlcnt = 0;
		}
		p++;
	}
	i--;
	if(i == 0)
		return;
	write(fd, buf, i);
	if(pcnt >= page){
		hold(fd);
		pcnt = 0;
		if(cnt > i)
			pause(fd, p, cnt-i);
	}
}

hold(fd)
{
	char buf[100];
	extern fin;

	flush();
	read(fin, buf, 100);
	write(fd, erase, elen);
}

nroff(fd, file)
{
	char buf[20];
	int pp[2];
	int cnt;

	if(read(fd, buf, 20) < 1 || buf[0] != '.')
		return(0);
	pipe(pp);
	if(nroffpid=fork()){
		if(nroffpid == -1)
			return(0);
		close(pp[1]);
		write(2, "[Nroffing...]\n", 14);
		sleep(1);
		return(pp[0]);
	} else {
		close(1);
		dup(pp[1]);
		close(pp[0]);
		execl("/usr/bin/nroff", "nroff", "-mmm", file, 0);
		write(1, "can't exec nroff\n", 17);
		exit();	/* can't exec nroff */
	}
}

/*  */
/*
 *	given a number, find that corresponding entry in the
 *	current working directory.  When "lsdir" outputs the direc-
 *	tory's active entries, it prints a number beside each entry.
 *	This number may be input by the user instead of the file 
 *	name, if desired.  If a number is input to "helpin", "scandir"
 *	will scan the current working directory and find the correspond-
 *	ing entry, if there is one.
 */
char *scandir(offset)
int	offset;
{
	int	ctr,  fddir;
	static struct{
		int ino;
		char name[16];
	} dir;

	dir.name[14] = 0;
	ctr = 1;

	if((fddir = open(".",0))  !=  -1)  {
		while(read(fddir,&dir,16) > 0)  {
			if(dir.ino != 0 && dir.name[0] != '.'){
				if(ctr++ == offset)  {
					close(fddir);
					return(dir.name);
				}
			}
		}
		printf("%d too large\n",offset);
	} else	{
		printf("cant open '.'\n");
	}
	reset();
}
/**/
/*
 *	coordinate the input and processing routines of help.
 *	This routine, once entered, will never return.
 */
processinput(start)
char *start;
{
	char	*in;
	int	listed;
	extern int reset();
	
	chdir(start);
	list(start);
	setexit();
	signal(2, reset);
	if(printfd){
		close(printfd);
		printfd =0;
	}
	while(1)  {
		in = input();
			if(in[0] == '!'){
				if(toplevel)
					printf("sorry Charlie -- can't call shell\n");
				else
					run(&in[1]);
				continue;
			}
			if(stat(in,&sbuf)  !=  -1)  {
				if((ifdir & sbuf.i_mode) == ifdir)  {
					listed = lsdir(in);
				} else	{
					print(in);
					lsdir(".");	
				}
			} else	{
				printf("can't find: %s\n",in);
				sleep(2);
				lsdir(".");
			}
	}
	finis();
}
/*
 *	compare two strings for equivalent values.
 */
compstr(first,second)
char	*first,	*second;
{
	int	i,	size;
	size  =  i  =  0;

	size = sizstr(first);
	if(size != (sizstr(second)))
		return(0);
	while(*first != '\0'  &&  *first++  ==  *second++)
		i++;
	return(i == size ? 1:0);
}
/*
 *	return the size of a string (number of bytes).
 */
sizstr(str)
char	*str;
{
	int	size;
	size	= 0;

	while(*str++  !=  '\0')  {
		size++;
	}
	return(size);
}
/*  */
/*
 * help -- help the user
 */

help()
{
	printf("\n\nenter a name or a number to see more documentation\n");
	printf("type '..' to return to previous documentation level\n");
	printf("type '/' to return to documentation root\n");
	if(!toplevel)
		printf("type '!' followed by shell command\n");
	printf("press 'esc' key to suspend output\n");
	printf("type '?' for this message\n\n\n");
}

len(s)
char *s;
{
	register n;
	register char *p;

	p = s;
	n = 0;
	while(*p++)
		n++;
	return(n);
}

#define FF1 040000
#define BS1 0100000

int ntty[3], otty[3];

tty(n)
{
	if(n){
		/* shutdown */
		stty(1, otty);
		exit();
	} else {
		/* startup */
		gtty(1, otty);
		ntty[0] = otty[0];
		ntty[1] = otty[1];
		ntty[2] = otty[2];
		ntty[2] =& ~BS1;
		if(ntty[2] & FF1){
			erase = ETEK;
			page = 32;
		}
		stty(1, ntty);
	}
}

finis()
{
	flush();
	tty(1);
	exit();
}
/*	Subroutine "run":
 *
 *
 *	This subroutine will perform a combination of a 
 *	"fork" and "execl" system call to execute system
 *	calls from C in an easy manner.
 *
 *	Modified for use with "help" -- disables interrupts
 *	before the parent waits, renamed from "shell" to "run".
 *
 *
 */
run(cmd)
char	cmd[];
{
	register int pid, a;

	if ((pid=fork()) < 0){
		write(2,"Couldn't fork--try again\n",25);
		return(-1);
	}

	if (pid != 0) {
		a = signal(2, 1);
		while(wait() != pid);
		signal(2, a);
	} else {
		execl("/bin/sh","sh","-c",cmd,0);
		write(2,"Can't execute \"sh\"\n",19);
		exit();
		}
	return(0);
}
