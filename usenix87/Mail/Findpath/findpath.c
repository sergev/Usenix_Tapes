/* Written by Stephen J. Muir, Computing Dept., Lancaster University */

# include <stdio.h>
# include <strings.h>

typedef struct
	{ char	*dptr;
	  int	dsize;
	}	datum;

extern datum	fetch ();

char	buf [4096];

int	interactive, newline;

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
	{ if (argc == 1)
		++interactive;
	  dbminit ("/usr/local/lib/palias");
	  while ((key.dptr = getline ("USENET site: ")) != 0)
	  { key.dsize = strlen (key.dptr) + 1;
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
		fprintf (stderr, "No such site\n");
	      else
		write (1, newline ? "\n" : "\0", 1);
	    }
	  }
	  if (interactive)
		putchar ('\n');
	  exit (0);
	}
