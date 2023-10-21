/* Written by Stephen J. Muir, Computing Dept., Lancaster University */

# include <sys/types.h>
# include <sys/dir.h>
# include <sys/file.h>
# include <stdio.h>
# include <strings.h>

typedef struct
	{ char	*dptr;
	  int	dsize;
	}	datum;

char	*filename, *mapdir = "/usr/spool/uucpmaps",
	*database = "/usr/local/lib/pfile", buf [4096];

struct direct	*curfile;

datum	key, content;

DIR	*mapdirp;

FILE	*ffd;

/*ARGSUSED*/
main (argc, argv, envp)
	char	*argv [], *envp [];
	{ register char	*cp;
	  register int	fd;
	  if (chdir (mapdir) == -1 || (mapdirp = opendir (".")) == 0)
	  { perror (mapdir);
	    exit (1);
	  }
	  strcpy (buf, database);
	  strcat (buf, ".pag");
	  if ((fd = open (buf, O_CREAT|O_TRUNC, 0666)) == -1)
	  { perror (buf);
	    exit (1);
	  }
	  close (fd);
	  strcpy (buf, database);
	  strcat (buf, ".dir");
	  if ((fd = open (buf, O_CREAT|O_TRUNC, 0666)) == -1)
	  { perror (buf);
	    exit (1);
	  }
	  dbminit (database);
	  while (curfile = readdir (mapdirp))
	  { filename = curfile->d_name;
	    if (*filename == '.' || index (filename, '.') == 0)
		continue;
	    if ((ffd = fopen (filename, "r")) == 0)
	    { perror (filename);
	      continue;
	    }
	    while (fgets (buf, sizeof (buf), ffd) != NULL)
	    { if (strncmp (buf, "#N", 2))
		continue;
	      for (cp = &buf [2]; *cp != '\n' && *cp != '\0'; ++cp);
	      while (*cp == '\n' || *cp == '\0' || *cp == ' ' || *cp == '\t')
		*cp-- = '\0';
	      for (cp = &buf [2]; *cp == ' ' || *cp == '\t'; ++cp);
	      key.dptr = cp;
	      key.dsize = strlen (cp);
	      content.dptr = filename;
	      content.dsize = strlen (filename) + 1;
	      if (store (key,content) == -1)
	      { perror ("store()");
		exit (1);
	      }
	    }
	    fclose (ffd);
	  }
	  exit (0);
	}
