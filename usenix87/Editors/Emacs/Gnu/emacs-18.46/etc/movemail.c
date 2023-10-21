/* movemail foo bar -- move file foo to file bar,
   locking file foo the way /bin/mail respects.
   Copyright (C) 1986 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but without any warranty.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
document "GNU Emacs copying permission notice".   An exact copy
of the document is supposed to have been given to you along with
GNU Emacs so that you can know how you may redistribute it all.
It should be in a file named COPYING.  Among other things, the
copyright notice and this notice must be preserved on all copies.  */

/*
 * Modified January, 1986 by Michael R. Gretzinger (Project Athena)
 *
 * Added POP (Post Office Protocol) service.  When compiled -DPOP
 * movemail will accept input filename arguments of the form
 * "po:username".  This will cause movemail to open a connection to
 * a pop server running on $MAILHOST (environment variable).  Movemail
 * must be setuid to root in order to work with POP.
 * 
 * New module: popmail.c
 * Modified routines:
 *	main - added code within #ifdef MAIL_USE_POP; added setuid(getuid())
 *		after POP code. 
 * New routines in movemail.c:
 *	get_errmsg - return pointer to system error message
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#define NO_SHORTNAMES   /* Tell config not to load remap.h */
#include "../src/config.h"

#ifdef USG
#include <fcntl.h>
#endif /* USG */

/* Cancel substitutions made by config.h for Emacs.  */
#undef open
#undef read
#undef write

char *concat ();
extern int errno;

main (argc, argv)
     int argc;
     char **argv;
{
  char *inname, *outname;
  int indesc, outdesc;
  char buf[1024];
  int nread;

#ifndef MAIL_USE_FLOCK
  struct stat st;
  long now;
  int tem;
  char *lockname, *p;
  char tempname[40];
  int desc;
#endif /* not MAIL_USE_FLOCK */

  if (argc < 3)
    fatal ("two arguments required");

  inname = argv[1];
  outname = argv[2];

#ifdef MAIL_USE_POP
  if (!bcmp (inname, "po:", 3))
    {
      int status; char *user;

      user = (char *) rindex (inname, ':') + 1;
      status = popmail (user, outname);
      exit (status);
    }

  setuid (getuid());
#endif /* MAIL_USE_POP */

#ifndef MAIL_USE_FLOCK
  /* Use a lock file named /usr/spool/mail/$USER.lock:
     If it exists, the mail file is locked.  */
  lockname = concat (inname, ".lock", "");
  strcpy (tempname, inname);
  p = tempname + strlen (tempname);
  while (p != tempname && p[-1] != '/')
    p--;
  *p = 0;
  strcpy (p, "EXXXXXX");
  mktemp (tempname);
  unlink (tempname);

  while (1)
    {
      /* Create the lock file, but not under the lock file name.  */
      /* Give up if cannot do that.  */
      desc = open (tempname, O_WRONLY | O_CREAT, 0666);
      if (desc < 0)
	fatal ("error creating lock file");
      close (desc);

      tem = link (tempname, lockname);
      unlink (tempname);
      if (tem >= 0)
	break;
      sleep (1);

      /* If lock file is a minute old, unlock it.  */
      if (stat (lockname, &st) >= 0)
	{
	  now = time (0);
	  if (st.st_ctime < now - 60)
	    unlink (lockname);
	}
    }
#endif /* not MAIL_USE_FLOCK */

#ifdef MAIL_USE_FLOCK
  indesc = open (inname, O_RDWR);
#else /* if not MAIL_USE_FLOCK */
  indesc = open (inname, O_RDONLY);
#endif /* not MAIL_USE_FLOCK */
  if (indesc < 0)
    pfatal_with_name (inname);
#ifdef BSD
  /* In case movemail is setuid to root, make sure the user can
     read the output file.  */
  /* This is desirable for all systems
     but I don't want to assume all have the umask system call */
  umask (umask (0) & 0333);
#endif /* BSD */
  outdesc = open (outname, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (outdesc < 0)
    pfatal_with_name (outname);
#ifdef MAIL_USE_FLOCK
  (void) flock (indesc, LOCK_EX);
#endif /* MAIL_USE_FLOCK */

  while (1)
    {
      nread = read (indesc, buf, sizeof buf);
      if (nread != write (outdesc, buf, nread))
	{
	  int saved_errno = errno;
	  (void) unlink (outname);
#ifndef MAIL_USE_FLOCK
	  (void) unlink (lockname);
#endif /* not MAIL_USE_FLOCK */ 
	  errno = saved_errno;
	  pfatal_with_name (outname);
	}
      if (nread < sizeof buf)
	break;
    }

#ifdef MAIL_USE_FLOCK
#ifdef STRIDE
   /* Stride has flock(), but no ftruncate().  This mess will do. */
  (void) close (open (inname, O_CREAT | O_TRUNC | O_RDWR, 0666));
#else
  (void) ftruncate (indesc, 0L);
#endif /* STRIDE */
#endif /* MAIL_USE_FLOCK */
  close (indesc);
  close (outdesc);
#ifndef MAIL_USE_FLOCK
  /* Delete the input file; if we can't, at least get rid of its contents.  */
  if (unlink (inname) < 0)
    if (errno != ENOENT)
      creat (inname, 0666);
  unlink (lockname);
#endif /* not MAIL_USE_FLOCK */
  exit (0);
}

/* Print error message and exit.  */

fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (1);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

error (s1, s2)
     char *s1, *s2;
{
  printf ("movemail: ");
  printf (s1, s2);
  printf ("\n");
}

pfatal_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  fatal (s, name);
}

/* Return a newly-allocated string whose contents concatenate those of s1, s2, s3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

int
xmalloc (size)
     int size;
{
  int result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}

/* This is the guts of the interface to the Post Office Protocol.  */

#ifdef MAIL_USE_POP

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#ifdef USG
#include <fcntl.h>
/* Cancel substitutions made by config.h for Emacs.  */
#undef open
#undef read
#undef write
#endif /* USG */

#define NOTOK (-1)
#define OK 0
#define DONE 1

char *progname;
FILE *sfi;
FILE *sfo;
char Errmsg[80];

static int debug = 0;

popmail(user, outfile)
char *user;
char *outfile;
{
    char *host;
    int nmsgs, nbytes;
    char response[128];
    register int i;
    int mbfi;
    FILE *mbf;
    char *getenv();
    int mbx_write();
    char *get_errmsg();

    host = getenv("MAILHOST");
    if (host == NULL) {
	fatal("no MAILHOST defined");
    }

    if (pop_init(host) == NOTOK) {
	error(Errmsg);
	return(1);
    }

    if (getline(response, sizeof response, sfi) != OK) {
	error(response);
	return(1);
    }

    if (pop_command("USER %s", user) == NOTOK || 
	pop_command("RPOP %s", user) == NOTOK) {
	error(Errmsg);
	pop_command("QUIT");
	return(1);
    }

    if (pop_stat(&nmsgs, &nbytes) == NOTOK) {
	error(Errmsg);
	pop_command("QUIT");
	return(1);
    }

    if (!nmsgs)
      {
	pop_command("QUIT");
	return(0);
      }

    mbfi = open (outfile, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (mbfi < 0)
      {
	pop_command("QUIT");
	error("Error in open: %s, %s", get_errmsg(), outfile);
	return(1);
      }
    fchown(mbfi, getuid(), -1);

    if ((mbf = fdopen(mbfi, "w")) == NULL)
      {
	pop_command("QUIT");
	error("Error in fdopen: %s", get_errmsg());
	close(mbfi);
	unlink(outfile);
	return(1);
      }

    for (i = 1; i <= nmsgs; i++) {
	mbx_delimit_begin(mbf);
	if (pop_retr(i, mbx_write, mbf) != OK) {
	    error(Errmsg);
	    pop_command("QUIT");
	    close(mbfi);
	    return(1);
	}
	mbx_delimit_end(mbf);
	fflush(mbf);
    }

    for (i = 1; i <= nmsgs; i++) {
	if (pop_command("DELE %d", i) == NOTOK) {
	    error(Errmsg);
	    pop_command("QUIT");
	    close(mbfi);
	    return(1);
	}
    }

    pop_command("QUIT");
    close(mbfi);
    return(0);
}

pop_init(host)
char *host;
{
    register struct hostent *hp;
    register struct servent *sp;
    int lport = IPPORT_RESERVED - 1;
    struct sockaddr_in sin;
    register int s;
    char *get_errmsg();

    hp = gethostbyname(host);
    if (hp == NULL) {
	sprintf(Errmsg, "MAILHOST unknown: %s", host);
	return(NOTOK);
    }

    sp = getservbyname("pop", "tcp");
    if (sp == 0) {
	strcpy(Errmsg, "tcp/pop: unknown service");
	return(NOTOK);
    }

    sin.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = sp->s_port;
    s = rresvport(&lport);
    if (s < 0) {
	sprintf(Errmsg, "error creating socket: %s", get_errmsg());
	return(NOTOK);
    }

    if (connect(s, (char *)&sin, sizeof sin) < 0) {
	sprintf(Errmsg, "error during connect: %s", get_errmsg());
	close(s);
	return(NOTOK);
    }

    sfi = fdopen(s, "r");
    sfo = fdopen(s, "w");
    if (sfi == NULL || sfo == NULL) {
	sprintf(Errmsg, "error in fdopen: %s", get_errmsg());
	close(s);
	return(NOTOK);
    }

    return(OK);
}

pop_command(fmt, a, b, c, d)
char *fmt;
{
    char buf[128];
    char errmsg[64];

    sprintf(buf, fmt, a, b, c, d);

    if (debug) fprintf(stderr, "---> %s\n", buf);
    if (putline(buf, Errmsg, sfo) == NOTOK) return(NOTOK);

    if (getline(buf, sizeof buf, sfi) != OK) {
	strcpy(Errmsg, buf);
	return(NOTOK);
    }

    if (debug) fprintf(stderr, "<--- %s\n", buf);
    if (*buf != '+') {
	strcpy(Errmsg, buf);
	return(NOTOK);
    } else {
	return(OK);
    }
}

    
pop_stat(nmsgs, nbytes)
int *nmsgs, *nbytes;
{
    char buf[128];

    if (debug) fprintf(stderr, "---> STAT\n");
    if (putline("STAT", Errmsg, sfo) == NOTOK) return(NOTOK);

    if (getline(buf, sizeof buf, sfi) != OK) {
	strcpy(Errmsg, buf);
	return(NOTOK);
    }

    if (debug) fprintf(stderr, "<--- %s\n", buf);
    if (*buf != '+') {
	strcpy(Errmsg, buf);
	return(NOTOK);
    } else {
	sscanf(buf, "+OK %d %d", nmsgs, nbytes);
	return(OK);
    }
}

pop_retr(msgno, action, arg)
int (*action)();
{
    char buf[128];

    sprintf(buf, "RETR %d", msgno);
    if (debug) fprintf(stderr, "%s\n", buf);
    if (putline(buf, Errmsg, sfo) == NOTOK) return(NOTOK);

    if (getline(buf, sizeof buf, sfi) != OK) {
	strcpy(Errmsg, buf);
	return(NOTOK);
    }

    while (1) {
	switch (multiline(buf, sizeof buf, sfi)) {
	case OK:
	    (*action)(buf, arg);
	    break;
	case DONE:
	    return (OK);
	case NOTOK:
	    strcpy(Errmsg, buf);
	    return (NOTOK);
	}
    }
}

getline(buf, n, f)
char *buf;
register int n;
FILE *f;
{
    register char *p;
    int c;

    p = buf;
    while (--n > 0 && (c = fgetc(f)) != EOF)
      if ((*p++ = c) == '\n') break;

    if (ferror(f)) {
	strcpy(buf, "error on connection");
	return (NOTOK);
    }

    if (c == EOF && p == buf) {
	strcpy(buf, "connection closed by foreign host");
	return (DONE);
    }

    *p = NULL;
    if (*--p == '\n') *p = NULL;
    if (*--p == '\r') *p = NULL;
    return(OK);
}

multiline(buf, n, f)
char *buf;
register int n;
FILE *f;
{
    if (getline(buf, n, f) != OK) return (NOTOK);
    if (*buf == '.') {
	if (*(buf+1) == NULL) {
	    return (DONE);
	} else {
	    strcpy(buf, buf+1);
	}
    }
    return(OK);
}

char *
get_errmsg()
{
    extern int errno, sys_nerr;
    extern char *sys_errlist[];
    char *s;

    if (errno < sys_nerr)
      s = sys_errlist[errno];
    else
      s = "unknown error";
    return(s);
}

putline(buf, err, f)
char *buf;
char *err;
FILE *f;
{
    fprintf(f, "%s\r\n", buf);
    fflush(f);
    if (ferror(f)) {
	strcpy(err, "lost connection");
	return(NOTOK);
    }
    return(OK);
}

mbx_write(line, mbf)
char *line;
FILE *mbf;
{
    fputs(line, mbf);
    fputc(0x0a, mbf);
}

mbx_delimit_begin(mbf)
FILE *mbf;
{
    fputs("\f\n0,unseen,,\n", mbf);
}

mbx_delimit_end(mbf)
FILE *mbf;
{
    putc('\037', mbf);
}

#endif /* MAIL_USE_POP */
