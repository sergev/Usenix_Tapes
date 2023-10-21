
/* wtmpclose - close off a wtmp login entry.
 * David Sherman, The Law Society of Upper Canada
 * dave@lsuc.UUCP
 */
#include <utmp.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define TTYMAX	41	/* highest tty number on the system */

char buffer[100];
char name[100];
time_t offtime;


time_t getdate();

main()
{
	register wtmp;
	struct utmp utmp;

	printf("wtmp file to patch? ");
	gets(name);
	if((wtmp = open(name, 1)) < 0)
		error("can't open file %s", name);

	reask:
	printf("signoff time to patch closing entries? ");
	gets(buffer);
	offtime = getdate(buffer, (struct timeb *)NULL);
	/* ctime puts in its own \n */
	printf("signoff time will be %sIs this correct? ", ctime(&offtime));
	gets(buffer);
	if(buffer[0] != 'y' && buffer[0] != 'Y')
		goto reask;

	utmp.ut_name[0] = 0;	/* null entry for signoff */
	utmp.ut_time = offtime;	/* assignment works because this is a "long" */

	while(1)
	{
		printf("\ntty line to close off (press RETURN if no more)? ");
		gets(buffer);
		if(buffer[0] == 0)
		{
			printf("All done. Bye-bye.\n");
			close(wtmp);
			exit(0);
		}
		if(strcmp(buffer, "console"))	/* if NOT "console" */
		{
			int tty;
			if(strncmp(buffer, "tty", 3))
			{
				printf("Invalid. Valid entries are either \"console\" or begin with \"tty\".\n");
				continue;
			}
			if(!isdigit(buffer[3]) || !isdigit(buffer[4]) || buffer[5])
			{
				printf("Invalid - tty must be followed by two digits.\n");
				continue;
			}
			tty = atoi(buffer+3);
			if(tty < 2 || tty > TTYMAX)
			{
				printf("Invalid. Valid tty numbers are from 02 to %d.\n", TTYMAX);
				continue;
			}
		}
		strcpy(utmp.ut_line, buffer);
		lseek(wtmp, 0L, 2);	/* to end of the file */
		if(write(wtmp, (char *)&utmp, sizeof(utmp)) < 0)
			perror("write");
		else
			printf("Done.\n");
	}
}
