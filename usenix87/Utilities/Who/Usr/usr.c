/*
 *  usr [... chars- ... username ...]
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/dir.h>
/*
 * use the include or the structs that follow
 *
 * #include ".../etc/rwhod/rwhod.h"
 */

struct	outmp {
	char	out_line[8];
	char	out_name[8];
	long	out_time;
};
struct	whod {
	char	wd_vers;
	char	wd_type;
	char	wd_pad[2];
	int	wd_sendtime;
	int	wd_recvtime;
	char	wd_hostname[32];
	int	wd_loadav[3];
	int	wd_boottime;
	struct	whoent {
		struct	outmp we_utmp;
		int	we_idle;
	} wd_we[1024 / sizeof (struct whoent)];
};

struct	whod wd;
struct outmp outmp;

#define	RWHODIR "/usr/spool/rwho"
#define	WHDRSIZE (sizeof (wd) - sizeof (wd.wd_we))
#define NMAX sizeof (outmp.out_name)
#define HMAX sizeof (wd.wd_hostname)
#define MAXSTRING NMAX + 1
#define MAXARGNAMES 30			/* number of args to usr	*/
#define MAXNAMES 100			/* number of names per machine	*/
#define REST '-'
#define TRUE 1
#define FALSE 0

char *malloc ();
char argnames [MAXARGNAMES] [MAXSTRING];
char *names [MAXNAMES];
int now, nwlnflg;
DIR *etc;

main (argc, argv)
  int argc;
  char *argv[];
  {
   struct direct *dp;
   int argnct, f;
   argnct = 0;
   if (argc < 2)
     {
      argnames [argnct][0] = REST;		/* no arg default */
      argnames [argnct++][1] = '\0';
     }
   for (; (--argc > 0) && (argnct < MAXARGNAMES); argnct++)
     (void) strcpy (argnames [argnct], *++argv);
   if (argnct == MAXARGNAMES)
     fatal ("Error: MAXARGNAMES %d reached\n", MAXARGNAMES);
   if (chdir(RWHODIR) < 0)
     fatal ("%s: Permission denied\n", RWHODIR);
   if ((etc = opendir (".")) == NULL)
     fatal ("%s/.: Can't access\n", RWHODIR);
   (void) time (&now);
   nwlnflg = FALSE;
   while (dp = readdir (etc))
     {
      if (dp->d_ino == 0)
        continue;
      if (strncmp(dp->d_name, "whod.", 5))
        continue;
      if ((f=open (dp->d_name, 0)) < 0)
        continue;
      process_rwhod (argnct, f);
      (void) close (f);
     }
  }

scmp (p, q)
  char **p, **q;
  {
   return (strcmp (*p, *q));
  }

process_rwhod (argnct, f)
  int argnct, f;
  {
   int cc, n, found, i;
   register struct whod *w = &wd;
   register struct whoent *we;
   char host [HMAX], name [MAXSTRING];
   char **namp = names;
   cc = read (f, (char *)&wd, sizeof (struct whod));
   if (cc < WHDRSIZE)
     return;
   if (now - w->wd_recvtime > 5 * 60)
     return;
   cc -= WHDRSIZE;
   we = w->wd_we;
   (void) strncpy (host, w->wd_hostname, HMAX);
   host [HMAX-1] = '\0';
   for (n = cc / sizeof(struct whoent); n > 0; n--, we++)
     {
      (void) strncpy (name, we->we_utmp.out_name, NMAX);
      name [NMAX] = '\0';
      for (i=0, found=FALSE; (i < argnct) && !found; i++)
        if (found = match (name, argnames[i]))
          {
           *namp = malloc (strlen (name) + 1);
           (void) strcpy (*namp++, name);
	   if ((namp - names) > MAXNAMES) 			    /* abort */
	      fatal ("Error: MAXNAMES %d reached\n", MAXNAMES); 
          }
     }
   if (namp > names)
     print_out (host, namp);
  }

print_out (host, namp)
  char *host;
  char **namp;
  {
   register char **base, **p;
   int i, formlen, offset, prcol, tabcol;
   qsort (names, namp - names, sizeof names[0], scmp);
   if (nwlnflg)
       printf ("\n");
     else nwlnflg = TRUE;
   for (formlen=i=strlen (host)+4; i-- > 0 ; putchar (':'));
   printf ("\n: %s :\n", host);
   while (++i < formlen)
     printf (":");
   printf ("\n\n");
   offset = (namp - names) / 5 + 1;
   base = names;
   do {
        tabcol = prcol = 1;
	p = base;
	do {
	    for (; prcol < tabcol; prcol += 8, printf ("\t"));
	    printf ("%s", *p);
	    prcol = tabcol + strlen (*p);
	    tabcol += 16;
	   }
	  while ((p += offset) < namp);
	printf ("\n");
      }
	while (++base < (names + offset));
  }

match (name, request)
  char name[], request[];
  {
   int i;
   for (i=0; (name[i] == request[i]) && (name[i] != '\0'); i++);
   if (request[i] == REST) 
     return (TRUE);
   if (((name[i] == '\0') && (request[i] == '\0'))
       || ((name[i] == ' ') && (request[i] == '\0')))
     return (TRUE);
   return (FALSE);
  }

fatal (format, argument)
  char format[], argument[];
  {
   fprintf (stderr, format, argument);
   exit (1);
  }
