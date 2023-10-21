/* Written by Stephen J. Muir, Computing Dept., Lancaster University */

# include <stdio.h>
# include <strings.h>

typedef struct
	{ char	*dptr;
	  int	dsize;
	}	datum;

extern datum	fetch ();

char	*findpath = "exec /usr/local/bin/findpath -", buf [4096];

int	interactive, wpfd = -1, rpfd, newline;

FILE	*frpfd;

datum	key, value;

char	*
getline (prompt)
	char	*prompt;
	{ register char	c, *cp;
	  register int	i;
	  cp = &buf [0];
	  if (interactive && prompt)
	  { printf (prompt);
	    fflush (stdout);
	  }
	  while ((i = getchar ()) != EOF)
	  { if ((c = i & 0377) == '\n' || c == '\0')
	    { newline = c == '\n';
	      *cp = '\0';
	      return (buf);
	    }
	    *cp++ = c;
	  }
	  return (0);
	}

/*ARGSUSED*/
main (argc, argv, envp)
	char	*argv [], *envp [];
	{ register char	*cp;
	  if (argc == 1)
		++interactive;
	  dbminit ("/usr/local/lib/pfile");
	  while ((key.dptr = getline ("USENET site: ")) != 0)
	  { key.dsize = strlen (key.dptr);
	    value = fetch (key);
	    if (value.dsize)
	    { if (interactive)
		printf ("%s\n", value.dptr);
	      else
	      { if (newline)
			*(value.dptr + value.dsize - 1) = '\n';
		write (1, value.dptr, value.dsize);
	      }
	    }
	    else
	    { if (interactive)
		printf ("site may be hidden ... ");
	      if (wpfd == -1)
	      { if (interactive)
		{ printf ("calling up \"findpath\" ... ");
		  fflush (stdout);
		}
		if (psystem (findpath, &wpfd, &rpfd, 0, 0) == -1)
		{ perror (findpath + 5);
		  exit (1);
		}
		frpfd = fdopen (rpfd, "r");
	      }
	      write (wpfd, key.dptr, key.dsize + 1);
	      cp = &buf [0];
	      while ((*cp++ = (fgetc (frpfd) & 0377)) != '\0');
	      if (buf [0])
	      { if (interactive)
			printf ("returns: %s\n", buf);
		do
		{ if (cp = rindex (buf, '!'))
		  { *cp++ = '\0';
		    if (!strcmp (cp, "%s"))
			continue;
		    if (!strncmp (cp, "%s@", 3))
			cp += 3;
		  }
		  else
			cp = &buf [0];
		  if (interactive)
			printf ("looking up: %s\n", cp);
		  key.dsize = strlen (key.dptr = cp);
		  value = fetch (key);
		  if (value.dsize)
		  { if (interactive)
			printf ("%s\n", value.dptr);
		    else
		    { if (newline)
			*(value.dptr + value.dsize - 1) = '\n';
		      write (1, value.dptr, value.dsize);
		    }
		    break;
		  }
		}
		while (cp != &buf [0]);
		continue;
	      }
	      fflush (stdout);
	      if (interactive)
		fprintf (stderr, "No such site\n");
	      else
		write (1, newline ? "\n" : "\0", 1);
	    }
	  }
	  if (interactive)
		putchar ('\n');
	  exit (0);
	}
