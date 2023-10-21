/*
 * rscs to sendmail mail interface
 * invoked by the rscs master control daemon when a file with filetype MAIL
 * is received over BITNET.
 *
 * The syntax is
 *     damail <mailfile> <fromsys> <fromuser> <tosys> <touser>
 *
 * 
 * Bill Nesheim
 * Cornell U Dept of Computer Science
 * August, 1983
 *
 * Rewritten by Jim Crammond
 * Weizmann Institute
 * Feb 1985
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sysexits.h>
#include "../common/rscs.h"
#include "../common/daemon.h"

#define MAILLOG "/usr/lib/rscs/maillog"

main(ac,av)
int ac;
char **av;
{
	FILE *mail, *outfile, *log;
	char line[BUFSIZ], From[BUFSIZ], To[BUFSIZ];
	char sender[20];
	char *actime, *err;
	long tm, time();
	int loc, smpid, ppe[2];
	union wait status;

	if(ac < 2) {
		logerror("DAMAIL: No Mail File??");
		return(TRUE);
	}

	if((log = fopen(MAILLOG, "a")) == (FILE *)NULL) {
		logerror("Can't open mail log file!");
		return(FALSE);
	}

	if((mail = fopen(av[1],"r")) == (FILE *)NULL) {
		sprintf(line,"MTU: Can't open %s", av[1]);
		logerror(line);
		return(FALSE);
	}                                   

	/* apparent source */
	sprintf(From,"%s@%s.BITNET", av[3], av[2]);
	/* apparent destination */
	sprintf(To,"%s@%s.BITNET", av[5], av[4]);

	/* sender flag to sendmail */
	convlower(av[2]);
	sprintf(sender,"-oMs%s.bitnet", av[2]);

	/* try to find real destination and source */
	while (fgets(line, BUFSIZ, mail) !=  NULL) 
	{	
		if (strncmp(line, "From:", 3) == 0) {
			register char *p, *e;
			char *index();
			char *addr;

			p = line + 5;

			while (*p == ' ')
				p++;

			addr = p;

			if ( (p=index(addr, '<')) && (e=index(p, '>')) )
			{	/*
				 *  address of the form:  comment <address>
				 */
				addr = ++p;
				*e = '\0';
			}
			else if ( (e=index(addr, '(')) )
			{	/*
				 *  address of the form:  address (comment) 
				 */
				while (*--e == ' ')
					;
				*++e = '\0';
			}
			else if ( (e=index(addr, '\n')) )
				*++e = '\0';

			strcpy(From, addr);
			break;
		}
	}

	rewind(mail);
	tm = time(0L); 
	actime = (char *)ctime(&tm);
	actime[19] = '\000';

	/* open pipe to sendmail */
	pipe(ppe);
	if((smpid=vfork()) == 0) {
		close(0);
		dup(ppe[0], 0);
		execl("/usr/lib/sendmail", "sendmail",
				sender, "-odi", "-oem", "-f", From, To, 0);

		logerror("MTU: exec failed");
		perror("reason");
		_exit(1);
	}

	if((outfile = fdopen(ppe[1], "w")) == (FILE *)NULL) {
		logerror("MTU: pipe fdopen failure!");
		return(FALSE);
	}

	loc = 1;

	while (fgets(line, BUFSIZ, mail) !=  NULL) {
		if (loc == 1) {
			loc++;
			if(strncmp(line, ":READ", 5) == 0)
				continue;
		}

		fputs(line, outfile);
	}

	fclose(outfile);
	fclose(mail);
	wait(&status);
	switch(status.w_retcode) {
	case EX_OK:
		err="sent";
		break;
	case EX_NOUSER:
		err="no such user";
		break;
	case EX_NOHOST:
		err="host unknown";
		break;
	case EX_UNAVAILABLE:
		err="service unavailable";
		break;
	default:
		err="other";
	}

	fprintf(log,"%s Received mail from %s %s (tag %s %s) \n",
		actime, av[2], av[3], av[4], av[5]);
	fprintf(log,"\t%s (%d) %s (%d) from %s to %s\n",
		status.w_retcode ? "NOT SENT" : "Sent" , smpid, err, status.w_retcode, From, To);
	fclose(log);

	unlink(av[1]);
	return(status.w_retcode);
}
