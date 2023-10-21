#include "dispd.h"
#include <stdio.h>
#include <sys/file.h>
struct user_data user_info[50];

main()
{
	int mf;
	FILE *fd;
	struct user_data *ud;
	char mess[100];
	
	if ((mf = open(MFILE, O_RDONLY, 0)) > 0) {
		if (read(mf, (char *) user_info, 
			  sizeof(user_info)) < sizeof(user_info)) {
			perror("can't read message file");
			exit(1);
		}
		(void) close(mf);
	}
	ud = user_info;
	printf("User               Message\n\n");
	while (ud->user_name[0] != '\0') {
		printf("%-18s%-61s\n", ud->real_name, ud->user_message);
		ud++;
	}
	if ((fd = fopen(GLOB_MESS, "r")) == NULL) exit(1);
	printf("\n\n");
	while (fgets(mess,80,fd) != NULL) printf("%s",mess);
	(void) fclose(fd);
}
