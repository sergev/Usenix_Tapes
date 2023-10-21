/* ruser.c :=: 9/25/85   By SLB */

/*  Remove a pesky luser from the chat system.  */
/*  To use type: riduser ttyname                */

#include <stdio.h>
#include "defs.h"

struct logs     lbuf;

main(argc, argv)                /* Get's rid of user, on mytty, in logfile */
int argc;
char *argv[];
{
        char    *buffer = "\0";
        int     lfile;
        FILE    *fp;

        if (argc != 2) {
                fputs("usage: ruser <ttyname>\n", stderr);
                exit(1);
        }

        if ((lfile=open(LOGFILE, 0)) == EOF) {
                puts("No users in logfile.");
                exit(0);
        }

        while(read(lfile, (char *) &lbuf, sizeof(lbuf)) == sizeof(lbuf)) {
                if (strncmp(lbuf.l_line, argv[1], 5) == NULL) 
                        continue;

                strncat(buffer, lbuf.l_line, sizeof(lbuf.l_line));
                strncat(buffer, lbuf.l_name, sizeof(lbuf.l_name));
                strncat(buffer, lbuf.l_time, sizeof(lbuf.l_time));

        }

        if (close(lfile) == EOF) {
                perror(LOGFILE);
                exit(1);
        }

	if (strlen(buffer) == 0) {
        	unlink(LOGFILE);
		exit(0);
	}

	if ((fp=fopen(LOGFILE, "w")) == NULL) {
                perror(LOGFILE);
                exit(1);
        }

        fputs(buffer, fp);

        if (fclose(fp) == EOF) {
                perror(LOGFILE);
                exit(1);
        }
}
