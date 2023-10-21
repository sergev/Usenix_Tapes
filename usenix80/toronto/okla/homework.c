/* copyright June 15, 1979  University of Oklahoma
 *
 *	homework [-class] aname file
 *
 *	homework takes a file and inserts it into the instructor's
 *	directory for assignment named aname unless:
 *		the file does not belong to the user invoking homework,
 *		the assignment is past due,
 *		the assignment has already been submitted.
 *
 *	The due-date file has the following format:
 *
 *	  aname<space>due-date[<space>comments]<nl>
 *
 *		A assignment name, less than 14 characters.
 *		A blank (space) or a tab.
 *		The last date that the assignment is to be accepted,
 *			in the form MM/DD/YY.  Assignments will be
 *			accepted up to midnight of that date.
 *		   optionally:
 *			White space - blank(s) and/or tab(s).
 *			Comments.
 *		NEW-LINE.
 *
 *	There must be a directory with the same name as each assignment
 *	These directories must be sub-directories of the instructor
 *	directory. ie: /u0/class/instructor/aname
 *
 *	The homework file will be copied to:
 *
 *	/u0/class/instructor/aname/user-name
 *
 *	This file will have mode 600 and belong to the same user
 *	that ownes the instructor directory.
 *
 *		Suzanne P. O'Dell
 */


#include <stat.h>
#include <stdio.h>
#include <owner.h>
#define NBYTES 512
#define TAB '	'
#define BLANK ' '
#define YES 0
#define NO -1
#define NL '\n'
#define debug 0

FILE *fopen(), *fd;
FILE *fopen(), *fp;

struct inode ibuf;			/* stat buffer */
char *ip &ibuf;		/* pointer to stat buffer */

char aname[50] "/u0/";	/*  complete file name  */
char dname[25] "/u0/";	/*  due-date file name  */
char due[] "/instructor/due-dates";
char inst[] "/instructor/";
char ubuf[NBYTES],uname[9];
char buff[NBYTES];
char dclass[14];	/* default class */
char date[10];		/* due-date */
char name[15];		/* name for assignment found */
char *assign;		/* assignment name */
char *class 0;		/* class */
char *file;		/*  input file  */
char *dpt;
int uid, instid;


main(argc,argv)  int argc; char **argv; {

char c;
int p,q,parm;


if(argc < 3 || argc > 4) {
	printf("usage: %s [-class] assignment file\n",argv[0]);
	exit();
	}
parm = 0;
for(p=1;p < argc;p++) {
	switch(argv[p][0]) {
	case '-':
		if(class == 0) {
			class = &argv[p][1];
			}
		else    erin("too many flagged arguments");
		continue;
	default : switch(parm) {
			case 0 :
				assign = argv[p];
				parm++;
				continue;
			case 1 :
				file = argv[p];
				parm++;
				continue;
			default : erin("too many arguments");
			}
		}
	}
if(debug) {
	printf("class %s\n",class);
	printf("assign %s\n",assign);
	printf("file %s\n",file);
	}
uid = getuid();
if(stat(file,ip) == -1)  erin("File not found");
if(ownerc(ip) != uid && uid != 0) erin("not your file");
getpw(uid,ubuf);
for(p = 0;ubuf[p] != ':';p++)		/* get user name */
	uname[p] = ubuf[p];
uname[p] = 0;
if(debug) printf("uname %s\n",uname);
if(class == 0) {		/* find default class */
	class = dclass;
	for(q = 1;q < 5;q++)
		for(p++;ubuf[p] != ':';p++) ;	/* first 5 fields */
	for((p =+ 2);ubuf[p] != '/';p++) ;  /* first level of path */
	q = 0;
	for(p++;ubuf[p] != '/' && ubuf[p] != ':';p++)	/* second level = class */
		class[q++] = ubuf[p];
	class[q] = 0;
	if(debug) printf("dclass %s\n",class);
	}
strcat(dname,class);
strcat(dname,due);
if(debug) printf(" due-date - %s\n",dname);

if((dpt = finddd(name)) == NO ) erin("no due-date for assignment");
if(ontime(dpt) == NO) erin("late - not accepted");
if((fd = fopen(file,"r")) == NULL) erin("empty file");
strcat(aname,class);
strcat(aname,inst);
if(debug) printf("instr dir - %s\n",aname);
if(stat(aname,ip) == -1)  erin("bad class");
instid = ownerc(ip);
strcat(aname,assign);
strcat(aname,"/");
strcat(aname,uname);
if(debug) printf("aname - %s\n",aname);
if(stat(aname,ip) != NO) erin("already submitted");
if((fp = fopen(aname,"w")) == NULL) erin("can't create file");

/*  fd is file discriptor for input file  */
/*  fp is the file discriptor for the target file  */

for((p = fread(buff,1,NBYTES, fd));p > 0;p = fread(buff,1,NBYTES,fd))
	fwrite(buff, 1,p,fp);
chown(aname,instid);
chmod(aname,0600);
fclose(fd);
fclose(fp);
printf("assignment submitted\n");
exit();
}			/* end of main */




finddd(name) char *name;  {
	char c;
	int p;

	if((fd = fopen(dname,"r")) == NULL) erin("no due-date file");
	for(c = getc(fd);;)  {	/* forever */
		if(c == EOF) return(NO);
		p = 0;
		while(c != TAB && c != BLANK)  {
			if(c == EOF || c == NL)  erin("bad due-date file 1");
			name[p++] = c;
			c = getc(fd);
			}
		name[p] = 0;
		if(debug) printf("name %s\n",name);
		if(strcmp(name,assign) == YES) {
			for(c = getc(fd);c == TAB || c == BLANK;c = getc(fd));
			p =0;
			for(;c != TAB && c != BLANK && c != NL;c = getc(fd)) {   /* due-date */
				if(c == EOF) erin("bad due-date file 2");
				date[p++] = c;
				}
			date[p] = 0;
			return(date);
			}
		while(c != EOF && c != NL) c = getc(fd);
		if(c == NL)  c = getc(fd);
		}
	}

ontime(dpt) char *dpt; {
	extern int dmsize[];
	int tvec[2];
	int *ptr;
	int q,mo,dy,yr,jdate;

	time(tvec);
	ptr = localtime(tvec) ;		/* get today's date */
	mo = dy = yr = jdate = 0;
	if(debug) printf("date %s\n",dpt);
	for(q = 0; isdigit(dpt[q]) != 0;q++)
		mo = mo * 10 + dpt[q] - '0';
		if(debug) printf("mo %d\n",mo);
	for(;isdigit(dpt[q]) == 0;q++)
		if(dpt[q] == 0 ) erin("bad due date in file");
	for(;isdigit(dpt[q]) != 0;q++)
		dy = dy * 10 + dpt[q] -'0';
		if(debug) printf("dy %d\n",dy);
	for(;isdigit(dpt[q]) == 0;q++)
		if(dpt[q] == 0 ) erin("bad due date in file");
	for(;isdigit(dpt[q]) != 0;q++)
		yr = yr * 10 + dpt[q] - '0';
		if(debug) printf("yr %d\n",yr);
	dy--;		/* convert day to 0 - 365 */
	mo--;		/* convert month to 0 - 11 */
	for(q = 0;q < mo;q++)
		jdate =+ dmsize[q];
	jdate =+ dy;
	if((yr%4) == 0 && mo > 1) {		/* leap year */
		jdate++;
		if((yr%100) == 0 && (yr%400) != 0)
			jdate--;
		}
	if(debug) {
		printf("due-date %d  ",jdate);
		printf("  today %d\n",ptr[7]);
		}
	if(yr == ptr[5] && jdate >= ptr[7]) return(YES);
	if(yr > ptr[5]) return(YES);
	printf("due-date %s   ",dpt);
	return(NO);
	}


erin(str) char *str; {
	printf("%s\n",str);
	exit();
	}
