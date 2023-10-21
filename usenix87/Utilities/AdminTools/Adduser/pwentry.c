/*	pwentry.c			Lory Molesky (root!sbcs)

	Automatic account generator.  

	options:	
	-uUID	use this as the lower bound uid to start search.
	-d	Do not call the shell file to make the users directory.
	-n	Do not enter users in /etc/group file
	-gGID	Use this number as the group id for all entries
	-hDIR	Use this directory as the home directory for all entries.


*/
	

#include <stdio.h>
#include <pwd.h>
#define max_count 40	/* max entries per invocation */

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fopen(), *fp;
	char name[max_count][12],home[max_count][40];
	char firstname[20],lastname[20],shell[20],homename[35],newdir[55],groupie[70];
	char uid_s[5],gid_s[5],c,*x;
	int i,j,debug,uid,count,makedir,entgroup,home_ask,group_ask;
	int gid[max_count];
	
	debug = 0;
	uid = 1000;
	count = 0;
	home_ask = 1;
	group_ask = 1;
	makedir = 1;
	entgroup =1;
	for (i=1; i<argc; i++)
	{
		if (argv[i][0] == '-')
		{
			c = argv[i][1];
			switch(c)
			{
			case 'u':
				j=0;
				while ((uid_s[j] = argv[i][j+2]) != '\0') j++;
				uid = atoi(uid_s);
				break;

			case 'd':
				makedir = 0;
				break;

			case 'n':
				entgroup = 0;
				break;

			case 'g':
				group_ask = 0;
				j=0;
				while ((gid_s[j] = argv[i][j+2]) != '\0') j++;
				gid[0] = atoi(gid_s);
				break;

			case 'h':
				home_ask = 0;
				j=0;
				while ((home[0][j] = argv[i][j+2]) != '\0') j++;
				home[0][j] = '\0';
				break;

			default : 
				printf("*** illegal option ***\n");
				exit();
			}
		}
	}

	if ((fp = fopen("/etc/tempfile","w")) == NULL) {
		printf("File tempfile does not exist\n");
		exit();
	}

	printf("Automatic passwd entry program.\n");
	sprintf(shell, "/bin/csh");
	sprintf(name[count], "nothing");

	/* Loop until user enters the name 'exit' on the first prompt */

	while (strcmp(name[count],"exit") != 0) {
		if (count == (max_count - 1)) {
			printf("*** Maximum number of names reached, exiting ***\n");
			sprintf(name[count],"exit");
		}
newname:	if (count < (max_count - 1)) {
			printf("\nLogin name:\t\t");
			scanf("%s",name[count]);
		}
		if (strcmp(name[count],"exit") != 0) {

			if (strlen(name[count]) > 8) {
				printf("*** Login name limited to eight chararacters ***\n");
				goto newname;
			}

			if ((strcmp(name[count],"?") == 0) || (strcmp(name[count],"help") == 0)) {
				printf("*** Enter exit to finish ***");
				goto newname;
			}
			if ((x = (char *) getpwnam(name[count])) != NULL)
			{ 
				printf("*** Duplicate login name ***\n");
				goto newname;
			}
		
			printf("full name:\t\t");
			c = getchar();
			i = 0;
			while ((c = getchar()) != '\n') {
				firstname[i++] = c;
			}
			firstname[i] = '\0';

			if (home_ask) {
				printf("home directory:\t\t");
				scanf("%s",home[count]);
			}
			else strcpy(home[count],home[0]);
			sprintf(homename, "%s/%s",home[count],name[count]);

			if (group_ask) {
				printf("groupid:\t\t");
				scanf("%d",&gid[count]);
			}
			else gid[count] = gid[0];
		
			while ((x = (char *) getpwuid(uid)) != NULL) uid++;
			if (debug) printf("uid is :%d",uid);

			fprintf(fp,"%s::%d:%d:%s %s:%s:%s\n",name[count],uid,gid[count],firstname,lastname,homename,shell);

			count++;
			uid++;	/* to allow multiple entries per invocation */
		}
	}

	fclose(fp);
	system("cp /etc/passwd /etc/passwd.OLD");
	system("sort -t: +2n /etc/tempfile -o /etc/tempfile");
	if (debug) system("cat /etc/tempfile");
	system("sort -t: +2n /etc/tempfile /etc/passwd -o /etc/passwd");
	printf("sorted, merged tempfile\n");
	/* pwmakedir is a shell script to make the new users home directory,
	/* with correct ownership, .login and .cshrc	*/
	if (makedir) {
		for (j=0; j<count; j++)
		{
			sprintf(newdir, "pwmakedir %s %s",home[j],name[j]);
			system(newdir);
			if (entgroup) {
				system("cp /etc/group /etc/group.OLD");
				sprintf(groupie,"sed s/%d:/%d:%s,/ /etc/group > /etc/group.old",gid[j],gid[j],name[j]);
				system(groupie);
				system("mv /etc/group.old /etc/group");
			}
		}
	}

}
