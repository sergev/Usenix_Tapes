/*
 * wmi - who am I, anyway?
 *
 * RR Gomes -- 26 September 1979
 */

#include <stdio.h>
#include <pwd.h>
#include <grp.h>

#define streq(a, b) (strcmp((a), (b)) == 0)

char *uname(), *gname(), *number();
struct passwd *getpwuid();
struct group *getgrgid();
int nflag = 0;
int gflag = 0;

main(argc, argv)
int argc;
char *argv[];
{
	int ruid, euid, rgid, egid;

	while(argc > 1){
		if(streq(argv[1], "-n"))
			nflag++;
		else if(streq(argv[1], "-g") || streq(argv[1], "-"))
			gflag++;
		else{
			printf("wmi: bad option %s\n", argv[1]);
			exit(0);
		}
		--argc;
		argv++;
	}
	ruid = getuid();
	euid = geteuid();
	printf("%s", nflag ? number(ruid) : uname(ruid));
	if(ruid != euid)
		printf("(%s)", nflag ? number(euid) : uname(euid));
	if(gflag){
		rgid = getgid();
		egid = getegid();
		printf("\t%s", nflag ? number(rgid) : gname(rgid));
		if(rgid != egid)
			printf("(%s)", nflag ? number(egid) : gname(egid));
	}
	printf("\n");
	exit(0);
}

char *
uname(uid)
int uid;
{
	struct passwd *pw;
	static char name[33];

	if((pw=getpwuid(uid)) == NULL)
		strcpy(name, number(uid));
	else
		strcpy(name, pw->pw_name);
	return(name);
}

char *
gname(gid)
int gid;
{
	struct group *gp;
	static char name[33];

	if((gp=getgrgid(gid)) == NULL)
		strcpy(name, number(gid));
	else
		strcpy(name, gp->gr_name);
	return(name);
}

char *
number(n)
int n;
{
	static char num[12];

	sprintf(num, "#%d", n);
	return(num);
}
