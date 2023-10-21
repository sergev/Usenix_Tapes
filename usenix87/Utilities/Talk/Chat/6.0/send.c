/* send.c :=: 9/25/85   By SLB */

#include "chat.h"

send(o)		/* Send	messages, 0=input, 1=login, 2=logout */
int o;
{
	int	i = 0,
		t, ch,
		nusers = 0;
	char	line[512],
		*sname,
		shrtname[5],
		fname[64];

	sname = mytty;
	if ((!strncmp(sname, "tty", 3)) || (!strncmp(sname, "   ", 3)))
		sname =	sname+3;

	if (o == 0) {
		fputs("\r\nTEXT> ",stdout);
		while (ON) {
			ch =  getchar();
			if ((i + 1 > INLEN) && ((ch != '\r') && (ch != '\n') && (ch != '\b'))) {
				putchar('\007');
				continue;
			}
			if ((ch == '\b') && i > 0) {
				i--;
				fputs("\b \b", stdout);
				continue;
			} else if (isprint(ch) || (isspace(ch) && ch != '\t')) {
				line[i]	= ch;
				putchar(ch);
				if (ch == '\n' || ch == '\r')
					if (i > 0) {
						line[i]	= '\0';
						putchar('\n');
						break;
					} else {
						zapline();
						puts("Aborted\r");
						return;
					}
				i++;
			}
		}
	} else if (o ==	1)
		sprintf(line, "** %s logged in on %s **", myname, sname);
	else
		sprintf(line, "** %s logged out on %s **", myname, sname);

/* File	routine	- appends message to all tty's except mine */

/* Note:  This system is not perfect.  All devicenames throughout
the program get truncated to 5 chars which could cause trouble with
systems that have long and similar devicenames.                 */


	if (o == 0)
		fputs("Sent to ",stdout);

	lseek(lfile, 0L, 0);			/* Rewind */
	while(read(lfile, (char	*) &lbuf, sizeof(lbuf))	== sizeof(lbuf)) {

		if (!strncmp(lbuf.l_line, mytty, length))
			continue;

		strcpy(fname, PREFIX);
		strncat(fname, lbuf.l_line, 5);

		strncpy(shrtname, lbuf.l_line, 5);
		if ((!strncmp(shrtname,"tty",3))||(!strncmp(shrtname,"   ", 3)))
			strcpy(shrtname, &shrtname[3]);

		if ((wfd=fopen(fname, "a")) == FERROR) {
			if (o == 0)
				printf("*%-2.2s* ", shrtname);
			continue;
		} else
			if (o != 0)
				fprintf(wfd, "%s\n", line);
			else {
				printf("%-.5s ", shrtname);
				fprintf(wfd, "%s %s= %s<\n",
							 sname, mixname, line);
			}
		nusers++;

		if (fclose(wfd)	== ERR)	{
			error("( send()	)", OFF);
			quit();
		}
	}
	if (o == 0) {
		if (nusers == 0)
			fputs("-Nobody", stdout);
		puts("\r");
	}
}

