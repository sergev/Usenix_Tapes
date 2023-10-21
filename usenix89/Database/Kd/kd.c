/*
   Mark P.'s keyed database manager for miscellaneous notes and stuff

		October 25, 1986
		uucp: hplabs!ridge!valid!markp

   kd [-h]
		Prints help message

   kd -R
		Rebuilds the key database

   kd -x [key ...]
		Prints index of all stuff, or corresponding to keys

   kd -a [-f file] key ...
		Adds more stuff.  The given keys are attached to the entry.
		more keys may be added by prepending the special lines to
		the input.  If no file is specified, goes into vi.

   kd -q key ...
		Queries the database for the given keys (logical AND).

   kd -Q key ...
		Queries the database.  Prints full text of database entries.

   kd -e key ...
		Queries the database.  Enters vi on only 1 entry.
		Alters name of entry.

   kd -E key ...
		Queries the database.  Enters vi on several entries.
		Alters names of entries.

   kd -D key ...
		Deletes things (with confirmation).
*/

#include <stdio.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/dir.h>

struct	sgttyb tty;	/* Tty information */
int	ttyflags;	/* Tty flag image */
int	goodtty;	/* What it was upon entering */
int	iocount;	/* Characters read */
int	ospeed;		/* Baud rate */

#define savetty() (gtty(ttyflags,&tty), goodtty = tty.sg_flags)
#define fixtty() (tty.sg_flags = goodtty, stty(ttyflags,&tty))
#define cbreak() (tty.sg_flags |= CBREAK, stty(ttyflags,&tty))
#define nocbreak() (tty.sg_flags &= ~CBREAK, stty(ttyflags,&tty))

#define usage() \
   {fprintf(stderr, "\007usage: kd -R|-x [key ...]|-a [-f file] [key ...]|-{e|E|q|Q|D} key ...\n"); \
    exit(1);}

#define MAX_FN_LEN	40
#define KEYSTRING	"<$>"
#define KEYSTRING_LEN	3
#define KEY_FN		".keys"
#define NEW_KEY_FN	".keys.tmp"
#define BACKUP_KEY_FN	".keys.old"
#define KD_DIR		".kd"

typedef struct hts
{
   char *fn;
   struct hts *next;
} matchstruct;

int Rebuildflag = 0;		/* Command-line options */
int queryflag = 0;
int addflag = 0;
int fileflag = 0;
int indexflag = 0;
int printflag = 0;
int editflag = 0;
int editmanyflag = 0;
int Deleteflag = 0;
int allflags;

char *homedir;			/* HOME environment */
char keyfn[MAX_FN_LEN];		/* Key file name */
char newkeyfn[MAX_FN_LEN];	/* Newly generated key file name */
char backupkeyfn[MAX_FN_LEN];	/* Backup of old key file name */
char datadn[MAX_FN_LEN];	/* Data directory name */
char cshcommand[80];		/* Buffer for system() */
char addfn[MAX_FN_LEN];		/* File name for add */
matchstruct *basematch = NULL;	/* Beginning of list of matches */

/*
   Main program (finally)
*/
main(argc, argv)
int argc;
char *argv[];
{
   int i;

   argc--;			/* Process command-line arguments */
   argv++;
   while ((argc > 0) && (**argv == '-'))
   {
      (*argv)++;
      while (**argv)
	 switch (*(*argv)++)
	 {
	    case 'R' : Rebuildflag= 1; break;
	    case 'Q' : printflag= 1;
	    case 'q' : queryflag= 1; break;
	    case 'f' : fileflag= 1; break;
	    case 'a' : addflag= 1; break;
	    case 'x' : indexflag= 1; break;
	    case 'E' : editmanyflag= 1;
	    case 'e' : editflag= 1; break;
	    case 'D' : Deleteflag= 1; break;
	    case 'h' : break;
	    default:
	       usage();
	 }
      argc--;
      argv++;
   }

   /* Check consistency of command-line arguments */

   if (fileflag)
   {
      if (argc == 0)
      {
	 fprintf(stderr, "\007kd: no file specified with -f\n");
	 exit(1);
      }
      if (!addflag)
      {
	 fprintf(stderr, "\007kd: can only specify file with -a\n");
	 exit(1);
      }
      strcpy(addfn, *argv);
      argc--;
      argv++;
   }

   allflags= Rebuildflag+addflag+queryflag+indexflag+editflag+Deleteflag;
   if (allflags == 0)
   {
      Help();
      exit(0);
   }

   if (allflags != 1)
      usage();

   if (Rebuildflag && (argc > 0))
      usage();

   if ((queryflag || editflag || Deleteflag) && (argc == 0))
      usage();

   /* Generate constant file names */

   if ((homedir= (char *)getenv("HOME")) == (char *)NULL)
   {
      fprintf(stderr, "\007kd: can't get HOME environment\n");
      exit(1);
   }

   sprintf(keyfn, "%s/%s", homedir, KEY_FN);
   sprintf(newkeyfn, "%s/%s", homedir, NEW_KEY_FN);
   sprintf(backupkeyfn, "%s/%s", homedir, BACKUP_KEY_FN);
   sprintf(datadn, "%s/%s", homedir, KD_DIR);

   /* Dispatch the appropriate procedure */

   if (Rebuildflag)
      Rebuild();

   if (addflag)
      Add(argc, argv);

   if (queryflag)
      Query(argc, argv);

   if (indexflag)
      Index(argc, argv);

   if (editflag)
      Edit(argc, argv);

   if (Deleteflag)
      Delete(argc, argv);
}

/*
   outputxlate(string)
   Takes file name of a data file, and prints it out in an understandable
   form.  Conveniently, the file name is just the ASCII version of time(0),
   which is readily converted to ASCII date and time of entry/last change
   of this datum.
*/
outputxlate(s)
char *s;
{
   long val;

   val= atoi(s);
   printf("%.24s", ctime(&val));
}

/*
   Rebuild()
   Rebuilds the key file out of all the data files.  This is not necessarily
   the most efficient way to do this, but it is extremely robust.  The new
   key file is built in a separate file, then the old key file is moved to
   a backup, and the new key file is moved to its place.  Directory
   operations here are 4.1c/4.2-specific.  Each line in the key file is
   just a file name, space, and one key.
*/
Rebuild()
{
   DIR *dirp;			/* Directory stream */
   struct direct *dirent;	/* Directory entry for current data file */
   char datafn[MAX_FN_LEN];	/* Data file name */
   FILE *dataf;
   FILE *newkeyf;
   char c;
   int i;

   fprintf(stderr, "Rebuilding...");

   /* Create the new key file */

   if ((newkeyf= fopen(newkeyfn, "w")) == NULL)
   {
      fprintf(stderr, "kd: can't create %s\n", newkeyfn);
      exit(1);
   }

   /* Open up the data file directory */

   if ((dirp= opendir(datadn)) == NULL)
   {
      fprintf(stderr, "kd: can't open directory %s\n", datadn);
      exit(1);
   }

   /* Find keys in each file in directory, excluding . and .. */

   while ((dirent= readdir(dirp)) != NULL)
   {
      if (dirent->d_name[0] != '.')
      {
	 sprintf(datafn, "%s/%s", datadn, dirent->d_name);
	 if ((dataf= fopen(datafn, "r")) == NULL)
	 {
	    fprintf(stderr, "kd: can't open %s\n", datafn);
	    exit(1);
	 }
	 else
	 {
	    for(;;)
	    {
	       for (i= 0; i < KEYSTRING_LEN; i++)
		  if (getc(dataf) != KEYSTRING[i])
		     goto endloop;
	       fprintf(newkeyf, "%s", dirent->d_name);
	       while ((c= getc(dataf)) != '\n')
		  putc(c, newkeyf);
	       putc('\n', newkeyf);
	    }
endloop:    fclose(dataf);
	 }
      }
   }
   closedir(dirp);
   fclose(newkeyf);
   unlink(backupkeyfn);			/* May or may not incur error */
   link(keyfn, backupkeyfn);		/* Make backup */
   unlink(keyfn);			/* Kill old key file */
   link(newkeyfn, keyfn);		/* Brand new one now */
   unlink(newkeyfn);			/* Kill temporary */
   putc('\n', stderr);
}

/*
   Add(integer, array of strings)
   The arguments are as argc and argv, and are interpreted as a list
   of keys.  If a user-specified file exists (i.e. -f file), then
   write the keys to the new data file and append the user's file.
   If the user didn't specify a file, then write the keys to a temporary
   file, call up vi on this file, and copy the result to the new data
   file (deleting temporary when done).  Finally, rebuild the key file.
*/
Add(ac, av)
int ac;
char *av[];
{
   FILE *f;				/* User file or temporary */
   char datafn[MAX_FN_LEN];		/* Data file name */
   FILE *dataf;
   int i;
   int charsread;			/* Number of characters from read() */
   char buf[1024];			/* Buffer for copying */

   if (!fileflag)			/* No user-specified file */
   {
      sprintf(addfn, "/tmp/kd%010d.tmp", time(0));
      if ((f= fopen(addfn, "a")) == NULL)
      {
	 fprintf(stderr, "\007kd: couldn't create %s\n", addfn);
	 exit(1);
      }
      for (i= 0; i < ac; i++)		/* Make header and call up vi */
      {
	 fprintf(f, "%s %s\n", KEYSTRING, *av);
	 av++;
      }
      fprintf(f, "*** Replace this line with your text ***\n");
      fclose(f);
      sprintf(cshcommand, "/usr/ucb/vi +%d %s", ac+1, addfn);
      system(cshcommand);
   }

   /* In either case, addfn should be the user's file now */

   if ((f= fopen(addfn, "r")) == NULL)
   {
      fprintf(stderr, "\007kd: %s not found\n", addfn);
      exit(1);
   }

   /* Magically generate the new data file name using time(0) */

   sprintf(datafn, "%s/%010d", datadn, time(0));

   if ((dataf= fopen(datafn, "w")) == NULL)
   {
      fprintf(stderr, "\007kd: couldn't create %s\n", datafn);
      exit(1);
   }

   /* Write header if user-specified file */

   if (fileflag)
      for (i= 0; i < ac; i++)
      {
	 fprintf(dataf, "%s %s\n", KEYSTRING, *av);
	 av++;
      }

   /* Copy textual stuff to file */

   while ((charsread= read(fileno(f), buf, 1024)) == 1024)
      write(fileno(dataf), buf, 1024);

   write(fileno(dataf), buf, charsread);
   fclose(f);
   fclose(dataf);

   /* Remove temporary */

   if (!fileflag)
      unlink(addfn);

   /* And rebuild... */

   Rebuild();
}

/*
   int search(integer, array of strings, integer)
   Does most of the work.  Passed the keys and a flag which selects either
   silent lookup (for query) or a report (for index).  Compares keys against
   the entries in the central key file.  Matches occur on exact compares
   or when the specified key is a shortened version of the stored key.  A
   linked list of matches is built, consisting solely of the file names
   which contained matches.  If indexstyle is off, search() does no
   reporting.  If it is on, search() prints one line for each file that
   matched with all the keys occurring in that file.  If there are no keys
   specified and indexstyle is on, then the entire index is printed.  This
   is really just a nicer way of looking at the key file.  At any rate,
   search() returns a value equal to the number of files which contained
   matches.
*/
int
search(ac, av, indexstyle)
int ac;
char *av[];
int indexstyle;
{
   FILE *keyf;
   int i;
   char currentfn[MAX_FN_LEN];
   char fnread[MAX_FN_LEN];
   char keyread[MAX_FN_LEN];
   char c;
   int matches;
   int lengths[MAX_FN_LEN];
   matchstruct *oldbasematch;
   int foundcount;
   int firstline;
   char datafn[MAX_FN_LEN];
   FILE *dataf;
   int keysinthisfile;

   /* Initialize some things */

   strcpy(currentfn, " ");
   matches= 0;
   foundcount= 0;
   firstline= 1;
   keysinthisfile= 0;
   for (i= 0; i < ac; i++)
      lengths[i]= strlen(av[i]);

   /* Open up the key file */

   if ((keyf= fopen(keyfn, "r")) == NULL)
   {
      fprintf(stderr, "\007kd: can't find %s\n", keyfn);
      exit(1);
   }

   /* For each line in the key file, get the file name and key. */
   /* Keep a count of the number of matches for each given file name, */
   /* and add that file name to the list of matches if the count is */
   /* equal to the number of user-specified keys.  Also, report in */
   /* the appropriate fashion if indexstyle is on. */

   while ((c= getc(keyf)) != EOF)
   {
      ungetc(c, keyf);
      fscanf(keyf, "%s %s", fnread, keyread);

      /* Check to see if we have advanced to a new file name */

      if (strcmp(fnread, currentfn))
      {
	 /* Indexstyle is on and no keys- every file matches */

	 if (indexstyle && (ac == 0))
	 {
	    if (firstline)
	       firstline= 0;
	    else
	       putc('\n', stdout);
	    outputxlate(fnread);
	    printf(" - ");
	    foundcount++;
	 }

	 /* Perfect match?  Add to list and report if indexstyle */

	 if (matches == ac)
	 {
	    oldbasematch= basematch;
	    basematch= (struct hts *)malloc(sizeof(matchstruct));
	    basematch->fn= (char *)malloc(strlen(currentfn)+1);
	    strcpy(basematch->fn, currentfn);
	    basematch->next= oldbasematch;
	    foundcount++;
	    if (indexstyle && (ac > 0))
	    {
	       outputxlate(currentfn);
	       printf(" - ");
	       sprintf(datafn, "%s/%s", datadn, currentfn);
	       if ((dataf= fopen(datafn, "r")) == NULL)
	       {
		  fprintf(stderr, "\007kd: can't find %s\n", datafn);
		  exit(1);
	       }
	       for (i= 0; i < keysinthisfile; i++)
	       {
		  while ((c= getc(dataf)) != ' ')
		     ;
		  do
		  {
		     putc(c, stdout);
		     c= getc(dataf);
		  }  while (c != '\n');
	       }
	       putc('\n', stdout);
	       fclose(dataf);
	    }
	 }
	 if (matches > ac)
	 {
	    fprintf(stderr, "\007:kd duplicate keys in %s\n", currentfn);
	    exit(1);
	 }
	 strcpy(currentfn, fnread);
	 matches= 0;
	 keysinthisfile= 0;
      }
      keysinthisfile++;
      if (indexstyle && (ac == 0))
	 printf(" %s", keyread);

      /* Do the actual key comparisons */

      for (i= 0; i < ac; i++)
      {
	 if (!strncmp(keyread, av[i], lengths[i]))
	    matches++;
      }

      /* Get rest of line from key file */

      while ((c= getc(keyf)) != '\n')
	 ;
   }
   if (indexstyle && (ac == 0))
      putc('\n', stdout);
   if (matches == ac)
   {
      oldbasematch= basematch;
      basematch= (struct hts *)malloc(sizeof(matchstruct));
      basematch->fn= (char *)malloc(strlen(currentfn)+1);
      strcpy(basematch->fn, currentfn);
      basematch->next= oldbasematch;
      foundcount++;
      if (indexstyle && (ac > 0))
      {
	 outputxlate(currentfn);
	 printf(" - ");
	 sprintf(datafn, "%s/%s", datadn, currentfn);
	 if ((dataf= fopen(datafn, "r")) == NULL)
	 {
	    fprintf(stderr, "\007kd: can't find %s\n", datafn);
	    exit(1);
	 }
	 for (i= 0; i < keysinthisfile; i++)
	 {
	    while ((c= getc(dataf)) != ' ')
	       ;
	    do
	    {
	       putc(c, stdout);
	       c= getc(dataf);
	    } while (c != '\n');
	 }
	 putc('\n', stdout);
	 fclose(dataf);
      }
   }
   if (matches > ac)
   {
      fprintf(stderr, "\007kd: duplicate keys in %s\n", currentfn);
      exit(1);
   }
   fclose(keyf);
   return(foundcount);
}

/*
   Query(integer, array of strings)
   Does the -q (terse) or -Q command.  Calls search() with the number of
   keys and the keys, and either summarizes its results or dumps the full
   contents of all the files that matched.
*/
Query(ac, av)
int ac;
char *av[];
{
   char matchfn[MAX_FN_LEN];
   FILE *dataf;
   int i;
   int found;
   matchstruct *traverse;
   char buf[1024];
   int charsread;

   found= search(ac, av, 0);
   if (found == 0)
   {
      printf("No matches found\n");
      exit(0);
   }
   if (!printflag)
   {
      printf("%d %s found:\n", found,
	 (found == 1) ? "match" : "matches");
      for (traverse= basematch; traverse != NULL; traverse= traverse->next)
	 printf("   %s\n", traverse->fn);
   }
   else
   {
      for (traverse= basematch; traverse != NULL; traverse= traverse->next)
      {
	 printf("---------- %s ----------\n", traverse->fn);
	 sprintf(matchfn, "%s/%s", datadn, traverse->fn);
	 if ((dataf= fopen(matchfn, "r")) == NULL)
	    fprintf(stderr, "\007kd: can't find %s\n", matchfn);
	 else
	 {
	    while ((charsread= read(fileno(dataf), buf, 1024)) == 1024)
	       write(fileno(stdout), buf, 1024);
	    write(fileno(stdout), buf, charsread);
	 }
	 fclose(dataf);
      }
   }
}

/*
   Index(integer, array of strings)
   Prints a synopsis of the key file, either the whole thing if no keys
   are specified, or reducing it by matching the specified keys.  The
   search() function does all the work for us.
*/
Index(ac, av)
int ac;
char *av[];
{
   int found;

   found= search(ac, av, 1);
   if (found == 0)
   {
      printf("No matches found\n");
      exit(0);
   }
}

/*
   Edit(integer, array of strings)
   Implements -e and -E.  Matches the keys against the key file.  If
   -e was specified and there is exactly one match, calls up vi on that
   file.  If -E was specified, calls up vi on each matching file in
   turn.  Each file edited has its name changed to correspond to the
   current date and time.  Notice the sleep(1) hack to prevent the
   not-particularly-nice possibility of two data files being created
   with the same name.  Rebuilds the key file when done.
*/
Edit(ac, av)
int ac;
char *av[];
{
   int found;
   matchstruct *traverse;

   found= search(ac, av, 0);
   if (found == 0)
   {
      printf("No matches found\n");
      exit(0);
   }

   if ((found != 1) && (!editmanyflag))
   {
      fprintf(stderr, "\007kd: -e specified but %d matches\n", found);
      exit(1);
   }

   for (traverse= basematch; traverse != NULL; traverse= traverse->next)
   {
      sprintf(cshcommand, "/usr/ucb/vi %s/%s", datadn, traverse->fn);
      system(cshcommand);
      sprintf(cshcommand, "/bin/mv %s/%s %s/%010d",
	 datadn, traverse->fn, datadn, time(0));
      system(cshcommand);
      sleep(1);
   }

   Rebuild();
}

/*
   Delete(integer, array of strings)
   Deletes all data files which match the specified keys.  Each deletion
   is confirmed before executing.  Rebuilds the key file when done.
   Note that, in event of an interrupt, the user's terminal will be left
   in cbreak mode.
*/
Delete(ac, av)
int ac;
char *av[];
{
   int i;
   int found;
   char matchfn[MAX_FN_LEN];
   FILE *dataf;
   matchstruct *traverse;
   int charsread;
   char buf[1024];
   char c;

   found= search(ac, av, 0);
   if (found == 0)
   {
      printf("No matches found\n");
      exit(0);
   }
   for (traverse= basematch; traverse != NULL; traverse= traverse->next)
   {
      printf("---------- %s ----------\n", traverse->fn);
      sprintf(matchfn, "%s/%s", datadn, traverse->fn);
      if ((dataf= fopen(matchfn, "r")) == NULL)
	 fprintf(stderr, "\007kd: can't find %s\n", matchfn);
      else
      {
	 while ((charsread= read(fileno(dataf), buf, 1024)) == 1024)
	    write(fileno(stdout), buf, 1024);
	 write(fileno(stdout), buf, charsread);
      }
      fclose(dataf);
      fprintf(stderr, "\nDo you really want to delete this <y/n>? ");
      savetty();
      ospeed= tty.sg_ospeed;
      cbreak();
      while (c= getc(stdin), (c != 'y') && (c != 'n'))
	 ;
      fixtty();
      fprintf(stderr, "\n");
      if (c == 'y')
      {
	 unlink(matchfn);
	 fprintf(stderr, "Deleted.\n");
      }
      else
	 fprintf(stderr, "Left alone.\n");
   }
   Rebuild();
}

/*
   Help()
   Prints help information.
*/
Help()
{
   fprintf(stderr, "kd -a key ... : add a new entry with optional keys\n");
   fprintf(stderr, "                '-f file' after -a for existing file\n");
   fprintf(stderr, "kd -x         : prints index of whole database\n");
   fprintf(stderr, "kd -x key ... : prints index of selected items\n");
   fprintf(stderr, "kd -q key ... : quick query of selected items\n");
   fprintf(stderr, "kd -Q key ... : full print of selected items\n");
   fprintf(stderr, "kd -e key ... : edit a single selected item\n");
   fprintf(stderr, "kd -E key ... : edit many selected items\n");
   fprintf(stderr, "kd -D key ... : deletes selected items (confirms) \n");
   fprintf(stderr, "kd -R         : rebuilds the key database\n");
   fprintf(stderr, "kd [-h]       : this message\n");
}
