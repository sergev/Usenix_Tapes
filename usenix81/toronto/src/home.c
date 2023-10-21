#include <stdio.h>
#include <pwd.h>

struct passwd *getpwuid(), *getpwnam();

main(argc, argv)
int argc;
char *argv[];
{
	struct passwd *p;
	int i;
	int errs = 0;

	if(argc==1){
		if((p=getpwuid(getuid())) == NULL){
			printf("Unknown user id\n");
			errs++;
		}
		else
			printf("%s\n", p->pw_dir);
	}
	else{
		for(i=1; i<argc; i++){
			if((p=getpwnam(argv[i])) == NULL){
				printf("No user %s\n", argv[i]);
				errs++;
			}
			else{
				if(argc==2)
					printf("%s\n", p->pw_dir);
				else
					printf("%s: %s\n", argv[i], p->pw_dir);
			}
		}
	}
	exit(errs==0 ? 0 : 1);
}
