/*
 * a "fake" uux to replace the realone to allow uumail to intercept
 * mail at sites that mail not be able to recompile their mailers
 * Called via "uux - system!rmail user" from, normally, /bin/mail.
***************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of opath, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************
 * $Log:	uux.c,v $
 * Revision 4.0  86/11/17  16:02:42  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 3.0  86/03/14  12:05:06  sob
 * Release of 3/15/86 --- 3rd Release
 * 
 * Revision 1.6  86/03/14  11:57:51  sob
 * 
 * 
 * Revision 1.5  86/03/11  11:29:17  sob
 * Added Copyright Notice
 * 
 * Revision 1.4  86/02/17  18:07:48  sob
 * Moved REALUUX and UUMAIL definitions to the makefile
 * 
 * Revision 1.3  86/02/17  17:58:15  sob
 * Small syntax problem
 * 
 * Revision 1.2  86/02/17  17:55:45  sob
 * Corrected to remove parens from destbuf.
 * 
 * Revision 1.1  86/02/17  17:45:10  sob
 * Initial revision
 * 
 *
 */

#define _DEFINE

#include "uuconf.h"
static char rcsid[] = "$Header: uux.c,v 4.0 86/11/17 16:02:42 sob Exp $";

extern FILE *popen();
extern char *index();
extern struct passwd *getpwnam();

char sysbuf[BUFSIZ];
char destbuf[BUFSIZ];

main(argc, argv)
int argc;
char **argv;
{
	char *command;
	struct passwd *pwd;
	char cmd[BUFSIZ];
	char *system = sysbuf;
	char **psystem = &system;
	FILE *netf;
	int c;

	if ((argc != 4) || strcmp("-", argv[1]))  /* look for form 
							of uux command */
		realuux(argv);	

	strcpy(sysbuf, argv[2]);	/* save destination system */

	if ((command = index(sysbuf, '!')) == NULL)
		realuux(argv); 	
	*command++ = 0;
	if (strcmp("rmail", command))	/* look for rmail in command */
		realuux(argv);		

	mystrcpy(destbuf, argv[3]);      /*save destination path */
					/* but get rid of parens */
	/* become UUCP */
	setpwent();
	pwd = getpwnam("uucp");
	if (pwd == NULL) {
		fprintf(stderr, "Can't suid to \"uucp\" in %s\n", REALUUX);
		exit(1);	/* sigh */
	}
	endpwent();
	setuid(pwd->pw_uid);

	/* send the mail to uumail */
	sprintf(cmd, "uumail %s!%s", UUMAIL, sysbuf,destbuf);
	if ((netf = popen(cmd, "w")) == NULL)
		exit(EX_TEMPFAIL);	/* failure */

	/* send the actual mail */
	while ((c = getchar()) != EOF)
		putc(c, netf);
	fflush(netf);
	exit (pclose(netf)?1:0);	/* causes mail to do the right thing */
}

realuux(argv)
char **argv;
{
	int pid, sts;

	/* running suid root.  become us again */
	setuid(getuid());

        if ((pid = fork()) == -1) {
                fprintf(stderr, "uux: can't create proc for %s\n",REALUUX);
                exit(1);
        }
        if (pid) {
                while (wait(&sts) != pid) {
                        if (wait(&sts)==-1)
                                exit(1);
                }
                exit(sts?1:0);
        }
	execv(REALUUX, argv);
	fprintf(stderr, "uux: can't exec %s\n",REALUUX);
	exit (1);
}

/* remove parens for t and put what's left in s */
mystrcpy(s,t)
char * s, *t;
{
	int x;
	while (x = *t++){
		if ((x == '(') || (x == ')'))
			continue;
		*s++ = x;
	}
	
	*s = x;
}
