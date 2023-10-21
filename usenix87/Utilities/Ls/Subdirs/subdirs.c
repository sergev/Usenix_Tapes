/* Written by Stephen J. Muir, Computing Dept., Lancaster University */

# include <errno.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/dir.h>
# include <sys/file.h>
# include <stdio.h>

extern	errno;
extern	char	*strcpy ();

struct	stat	state;

char	*dot = ".", *dotdot = "..", dir [4096], linkbuf [4096];

int	symlook;

DIR	*dirfd, *fdstack [32], **fdp = &fdstack [0];

error (name)
	char	*name;
	{ fflush (stdout);
	  perror (name);
	}

open_ (name)
	char	*name;
	{ *fdp++ = dirfd;
	  if ((dirfd = opendir (name)) != NULL)
		return (0);
	  dirfd = *--fdp;
	  return (1);
	}

close_ ()
	{ closedir (dirfd);
	  dirfd = *--fdp;
	}

ptree (ep)
	char	*ep;
	{ struct	direct	*curfile;
	  if (open_ (dot))
	  { if (errno == EMFILE)
		printf ("%s/...\n", dir);
	    else
		error (dot);
	    return;
	  }
	  *ep++ = '/';
	  while ((curfile = readdir (dirfd)) != NULL)
	  { if (strcmp (curfile->d_name, dot) == 0 ||
		strcmp (curfile->d_name, dotdot) == 0
	       )
		continue;
	    lstat (curfile->d_name, &state);
	    if ((state.st_mode & S_IFMT) == S_IFDIR)
	    { strcpy (ep, curfile->d_name);
	      if (state.st_ino == 2)
		printf ("%s: mounted file system\n", dir);
	      else
	      if (symlook == 0 && state.st_nlink == 2)
		printf ("%s\n", dir);
	      else
	      if (access (ep, R_OK) == -1 || chdir (ep) == -1)
		error (dir);
	      else
	      { char	*np;
		printf ("%s\n", dir);
		for (np = ep; *np++; );
		ptree (np - 1);
		if (chdir (dotdot) == -1)
			error (dotdot);
	      }
	    }
	    else
	    if ((state.st_mode & S_IFMT) == S_IFLNK)
	    { int	cc;
	      strcpy (ep, curfile->d_name);
	      printf ("%s", dir);
	      if ((cc = readlink (ep, linkbuf, sizeof (linkbuf))) == -1)
		error (": readlink");
	      else
	      { linkbuf [cc] = '\0';
		printf (" -> %s\n", linkbuf);
	      }
	    }
	  }
	  close_ ();
	}

/*ARGSUSED*/
main (argc, argv, envp)
	char	*argv [], *envp [];
	{ char	*ep = &dir [0], *ap;
	  close (0);
	  dup2 (1, 2);
	  if (argc > 1 && strcmp (argv [1], "-l") == 0)
	  { --argc;
	    ++argv;
	    ++symlook;
	  }
	  if (argc > 2)
	  { printf ("usage: subdirs [-l] [directory name]\n");
	    exit (1);
	  }
	  ap = argc == 2 ? argv [1] : dot;
	  if (access (ap, R_OK) == -1 || chdir (ap) == -1)
	  { error (ap);
	    exit (1);
	  }
	  if (strcmp (ap, "/") == 0)
		++ap;
	  while (*ep++ = *ap++);
	  ptree (ep - 1);
	  exit (0);
	}
