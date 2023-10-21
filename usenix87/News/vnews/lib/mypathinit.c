#include <stdio.h>
#include "config.h"
#include "defs.h"


#ifdef HOME
char SPOOL[FPATHLEN];
char LIB[FPATHLEN-15];
char CAESAR[FPATHLEN];
#else
char SPOOL[] = SPOOLDIR";
char LIB[] = LIBDIR";
char CAESAR[] = LIBDIR/caesar";
#endif HOME
#ifdef MYNAME
char FULLSYSNAME[] = MYNAME;
#else
#ifdef UNAME
#include <sys/utsname.h>
char FULLSYSNAME[9];
#else
char FULLSYSNAME[20];
#endif UNAME
#endif MYNAME
char DOMAIN[] = MYDOMAIN;
#ifdef SENDMAIL
char MAILPARSER[] = SENDMAIL;
#else
#ifdef HOME
char MAILPARSER[FPATHLEN];
#else
char MAILPARSER[] = LIBDIR/recmail";
#endif HOME
#endif SENDMAIL
char XINEWS[] = INEWS;
static char verpfx[4] = {'@', '(', '#', ')'};
char NEWS_VERSION[] = news_version;


pathinit() {
#ifdef HOME
      /* Relative to the home directory of user HOME */
      sprintf(SPOOL, "%s/%s", logdir(HOME), SPOOLDIR");
      sprintf(LIB, "%s/%s", logdir(HOME), LIBDIR");
#endif
      if (strlen(LIB) >= FPATHLEN-15)
            xerror("LIB too long");
      if (strlen(SPOOL) > FPATHLEN)
            xerror("SPOOL too long");
#if defined(HOME) && !defined(SENDMAIL)
      sprintf(MAILPARSER, "%s/recmail", LIB);
#endif
#ifndef MYNAME
#ifdef UNAME
      {
            struct utsname ubuf;
            uname(&ubuf);
            strcpy(FULLSYSNAME, ubuf.nodename);
      }
#else
#ifdef GHNAME
      gethostname(FULLSYSNAME, sizeof FULLSYSNAME);
#else
      whoami();
#endif GHNAME
#endif UNAME
#endif MYNAME
}


#if !defined(UNAME) && !defined(GHNAME) && !defined(MYNAME)
#define	HDRFILE "/usr/include/whoami.h"

whoami()
{
	char buf[BUFSIZ];
	FILE *fd;
	
	fd = fopen(HDRFILE, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open %s\n", HDRFILE);
		exit(1);
	}
	
	for (;;) {	/* each line in the file */
		if (fgets(buf, sizeof buf, fd) == NULL) {
			fprintf(stderr, "no sysname in %s\n", HDRFILE);
			fclose(fd);
			exit(2);
		}
		if (sscanf(buf, "#define sysname \"%[^\"]\"", FULLSYSNAME) == 1) {
			fclose(fd);
			return;
		}
	}
}
#endif
