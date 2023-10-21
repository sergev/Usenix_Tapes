/* page.c :=: 9/25/85  by SLB */

/* Page	another	user...for use with the	chat system.  */
/* calling this	routine	with a 1 will activate "page" */
/* calling with	a 0 will only call the print routine  */

#include "chat.h"

page(c)
{


	struct	utmp	ubuf;

	FILE	*fp;
	char	hisname[32],
		_name[20][11],
		*timein	= "00:00am",
		inp[256];
	int fd;
	int i =	0, ttyn;


	clear();				/* Clear Screen */
	printf("\r\n\r\n\r\nUSERS LOGGED IN->\r\n");
	if ((fd=open(FNAME, 0))== ERR) {
		error("Page can't open /etc/utmp", ON);
		quit();
	}

	while (read(fd,	(char *) &ubuf,	sizeof(ubuf))==sizeof(ubuf)) {
		if (ubuf.ut_line[0]=='\0' || ubuf.ut_name[0]=='\0'
			|| !strcmp(ubuf.ut_name, "LOGIN"))
			continue;	/* skip	unused entries */
		strcpy(_name[i],ubuf.ut_name);
		prtime(ubuf.ut_time, timein);
		printf("Job%d %-8.8s %-7.7s in \@ %s\r\n", i+1, 
					ubuf.ut_name, ubuf.ut_line, timein);
		i++;
	}

	if(close(fd) ==	ERR)
	{
		error("Page can't close	utmp.", ON);
		quit();
	}

					/* page	another	user routine */
	if (c == 0)
		return(OK);

	fputs("\nPage which job? [CR exits] ", stdout);
	ttyset(OFF,ON);			/* Turn on echo for input */

	gets(inp);
	ttyn = atoi(inp);

	ttyset(ON,OFF);			/* All good things must	be undone */

	if (ttyn <= i && ttyn >= 1)
	{
		strcpy(hisname, PAGER);
		strcat(hisname, _name[ttyn-1]);

		if ((fp=popen(hisname,"w")) == FERROR)
		{
			error("Unable to page user.", ON);
			return(ERR);
		}

		fprintf(fp,"\n>>YOU ARE BEING REQUESTED TO<<\n");
		fprintf(fp,"      >>ENTER CHAT BY<<\n");
		fprintf(fp,"--->%s -%s-\n", mixname, mytty);

		if (fclose(fp) == ERR)
		{
			error("Unable to close PAGER.", ON);
			quit();
		}
		printf("Page on it\'s way.  Please give %s enough time to respond.\r\n",_name[ttyn-1]);
	} else
		puts("No page sent.\r");
	return(0);
}


