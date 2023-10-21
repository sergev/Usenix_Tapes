struct  {
	char        minor;
	char        major;
	int         inumber;
	int         flags;
	char        nlinks;
	char        uid;
	char        gid;
	char        size0;
	int         size1;
	int         addr[8];
	int         actime[2];
	int         modtime[2];
} fst;
int     i;
char    **sp;
char    *section[] {
	"NAME",
	"SYNOPSIS",
	"DESCRIPTION",
	"DIAGNOSTICS",
	"FILES",
	"SOURCE LANGUAGE",
	"LOCATION",
	"SEE ALSO",
	"BUGS",
	"AUTHOR",
	0
};

main(argc,argv)
int   argc;           /* number of arguments including command name */
char  **argv;         /* pointer to argument list */
{
	int     pid,i1,fddoc;

  char	buffer[132],
	*docname,
	*pntr1,
	*tempfile;
	signal(2,1);  signal(3,1);

/*
	get document filename from argument list.
*/
  if (argc <= 1)
	{ printf ("NO DOCUMENT FILENAME SPECIFIED\n");
	  exit (1);
	}
  docname = argv[1];
  /* check to see if document file already exists */
  if (!stat(docname, &fst))
	{ printf ("DOCUMENT FILE %s ALREADY EXISTS\n",
		docname);
	  exit (2);
	}
/*
	open document file.
*/
  if ( (fddoc=creat(docname, 0640)) < 0)
	{ printf ("UNABLE TO CREATE DOCUMENT FILE %s\n",
		docname);
	  exit (3);
	}
/*
	get a temporary file to use as a work file.
*/
  tempfile = &("/tmp/d00000");
  pid = getpid();
  for (pntr1=tempfile+10, i1=1; i1<=5; i1++)
	{ (*pntr1--) = pid%10+'0';
	  pid =/ 10;
	}

	sp=section;
	while((i = *sp++) != 0) {
		if(docsect(tempfile,i) == 0) continue;
		docwrite(fddoc,i,1);
		doccat(tempfile,fddoc);
		docwrite(fddoc, "\n", 0);
	}
/*
	insure that temporary work file is erased.
*/
  unlink (tempfile);
}


doccat (tempfile, fddoc)
  int	fddoc;
  char	*tempfile;
{
int     fdtemp,
	rdcnt;
  char	buffer[512];
  if ( (fdtemp=open(tempfile, 0)) < 0)
	{ printf ("UNABLE TO OPEN WORK FILE\n");
	  exit (1);
	}
  while (rdcnt=read(fdtemp, buffer, sizeof buffer))
	{ if (rdcnt < 0)
			{ printf ("ERROR READING WORK FILE\n");
		  exit (2);
		}
	  if (write(fddoc, buffer, rdcnt) < 0)
		{ printf ("ERROR WRITING ON DOCUMENT FILE\n");
		  exit (3);
		}
		}
  close (fdtemp);
}


docsect (tempfile, message)
  char	*message,
	*tempfile;
{
int     i1,
	pid;
	close(creat(tempfile,0666));
  printf ("enter ");
  printf ("%s section\n", message);
  pid = fork();
  if (pid == -1)
	return (0);
  if (pid != 0)
	while ( (i1=wait()) != pid );
  else
	execl ("/bin/ed", "ed", "-f", "-n", tempfile, 0);
  if (stat(tempfile, &fst) || fst.size1 == 0)
	return (0);
  return(1);
}


docwrite (fddoc, string, newline)
  int	fddoc,
	newline;
  char	*string;
{
int i1;
  char	*pntr1;
  for (i1=0, pntr1=string; (*pntr1++)!='\0'&&i1<512; i1++);
  if (i1 != 0)
	if (write(fddoc, string, i1) < 0)
		{ printf ("ERROR WRITING ON DOCUMENT FILE\n");
		  exit (1);
		}
  if (newline)
	write (fddoc, "\n", 1);
}
