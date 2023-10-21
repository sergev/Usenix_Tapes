#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>
#include <pwd.h>

main(ac,av)
int ac;
char *av[];
{
	char *li, *findtty();
	if(ac!=2) {
		fprintf(stderr,"usage: findtty loginname\n");
		exit(1);
	}
	li = findtty(av[1]);
	if(li==NULL)
		fprintf(stderr,"not found\n");
	else
		printf("%s\n",li);
}

char *findtty(name)
char *name;
{
	struct utmp *u, *getutent();

	endutent();

	while((u = getutent())!=NULL) {
		if(u->ut_type!=USER_PROCESS) continue;
		if(strcmp(u->ut_user,name)!=0) continue;
		return u->ut_line;
	}
	return NULL;
}
