/* cwho.c :=: 9/25/85   By SLB */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

struct logs     lbuf;
struct stat	sbuf;

main()          /* Show who's logged into chat */
{
        int     lfile;

        if (stat(LOGFILE, &sbuf) == EOF || sbuf.st_size == 0) {
		puts("No users logged into Chat.");
		exit(0);
	}

	if ((lfile=open(LOGFILE, 0)) == EOF) {
		perror(LOGFILE);
                exit(1);
	}

        puts("Users In Chat        Tty    When");

        while (read(lfile, (char *) &lbuf, sizeof(lbuf))==sizeof(lbuf)) {
                if (lbuf.l_line[0]=='\0' || lbuf.l_name[0]=='\0' ||
			!strncmp(lbuf.l_name, "LOGIN", 5))
                        continue;       /* Skip blankies */

        printf("%-20.20s %-5.5s  %7.7s\n",lbuf.l_name,
                                                   lbuf.l_line,lbuf.l_time);

        }
        if (close(lfile) == EOF) {
                perror(LOGFILE);
                exit(1);
        }
}
