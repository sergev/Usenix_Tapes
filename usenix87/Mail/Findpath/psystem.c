# include <signal.h>

int	psyspid;

psystem (com, fd0, fd1, fd2, child)
	char	*com;
	int	*fd0, *fd1, *fd2;
	void	(*child) ();
	{ int	p0 [2], p1 [2], p2 [2];
	  if (fd0 && pipe (p0) == -1)
		return (-1);
	  if (fd1 && pipe (p1) == -1)
	  { if (fd0)
	    { close (p0 [0]);
	      close (p0 [1]);
	    }
	    return (-1);
	  }
	  if (fd2 && pipe (p2) == -1)
	  { if (fd0)
	    { close (p0 [0]);
	      close (p0 [1]);
	    }
	    if (fd1)
	    { close (p1 [0]);
	      close (p1 [1]);
	    }
	    return (-1);
	  }
	  while ((psyspid = fork ()) == -1);
	  if (psyspid == 0)
	  { if (fd0)
	    { close (p0 [1]);
	      dup2 (p0 [0], 0);
	      close (p0 [0]);
	    }
	    if (fd1)
	    { close (p1 [0]);
	      dup2 (p1 [1], 1);
	      close (p1 [1]);
	    }
	    if (fd2)
	    { close (p2 [0]);
	      dup2 (p2 [1], 2);
	      close (p2 [1]);
	    }
	    signal (SIGQUIT, SIG_IGN);
	    if (child)
		(*child) ();
	    execl ("/bin/sh", "sh", "-c", com, 0);
	    _exit (127);
	  }
	  if (fd0)
	  { close (p0 [0]);
	    *fd0 = p0 [1];
	  }
	  if (fd1)
	  { close (p1 [1]);
	    *fd1 = p1 [0];
	  }
	  if (fd2)
	  { close (p2 [1]);
	    *fd2 = p2 [0];
	  }
	  return (0);
	}
