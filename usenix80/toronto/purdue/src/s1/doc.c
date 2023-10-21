/*
 *	doc -- prompt a user for documentation info.
 *
 *	% doc [-n] name [chapter]
 *
 *	-n:	produce nroff output
 *	name:	name of file (and name used in nroff header)
 *	chapter:	chapter for document (used ONLY in nroff header)
 *
 */

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

int ned_pid	0;	/* for quits */
int nroff	0;	/* nroff flag */
int fddoc;
int     i;

char *docname;
char *chapter	"1";
char *tempfile;

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
	int     pid,i1;

  char	buffer[132],
	*pntr1;
	extern quit();

	signal(2,1);  signal(3,quit);

/*
 *	parse args:
 */
	if(argv[1][0] == '-'){
		switch(argv[1][1]){
		case 'n':
			nroff++;
			break;
		case '\0':
			break;
		}
		argv++;
		argc--;
	}
	if(argc < 2){
		printf("Syntax:\n%% doc [-n] name [chapter]\n");
		exit();
	}
	docname = argv[1];
	if(argc > 2)
		chapter =argv[2];
/*
 *	open document file.
 */
	if((fddoc = creat(docname, 0644)) < 0){
		printf("can't create %s\n", docname);
		exit();
	}
/*
 *	get a temporary file to use as a work file.
 */
	tempfile = &("/tmp/d00000");
	pid = getpid();
	for (pntr1=tempfile+10, i1=1; i1<=5; i1++){
		(*pntr1--) = pid%10+'0';
		 pid =/ 10;
	}

	if(nroff){
		write(fddoc, ".th ", 4);
		docwrite(docname, 0);
		write(fddoc, "  ", 2);
		docwrite(chapter, 0);
		write(fddoc, "  ", 2);
		docwrite(pdate(), 1);
		printf("[Will produce NROFF formatted output]\n");
	}

	sp=section;
	while((i = *sp++) != 0) {
		if(docsect(i) == 0)
			continue;
		if(nroff)
			write(fddoc, ".sh ", 4);
		docwrite(i, 1);
		doccat(tempfile,fddoc);
		write(fddoc, "\n", 1);
	}
	unlink(tempfile);
}


doccat (file)
char	*file;
{
	int     fdtemp, rdcnt;
	char	buffer[512];

	if ( (fdtemp=open(file, 0)) < 0){
		printf("can't open %s\n", file);
		exit (1);
	}
	while (rdcnt=read(fdtemp, buffer, sizeof buffer)){
		if (rdcnt < 0){
			printf("ERROR READING WORK FILE\n");
			exit (2);
		}
		if (write(fddoc, buffer, rdcnt) < 0)
		{ printf ("ERROR WRITING ON DOCUMENT FILE\n");
		  exit (3);
		}
		}
	close (fdtemp);
}


docsect(message)
char	*message;
{
	int     i1;

	close(creat(tempfile,0666));
	printf ("enter %s section\n", message);
	ned_pid = fork();
	if (ned_pid == -1)
		return (0);
	if (ned_pid != 0)
		while ( (i1=wait()) != ned_pid );
	else {
		signal(2,0);
		execl ("/bin/ed", "ed", "-f", "-n", tempfile, 0);
		printf("[no editor!]\n");
		exit();
	}
	ned_pid = 0;
	if (stat(tempfile, &fst) || fst.size1 == 0)
		return (0);
	return(1);
}


docwrite(string, newline)
char	*string;
{
	int i1;
	char	*pntr1;

	i1 = len(string);
	if (i1 != 0){
		if(nroff)
			write(fddoc, "\"", 1);
		write(fddoc, string, i1);
		if(nroff)
			write(fddoc, "\"", 1);
	}
	if (newline)
		write (fddoc, "\n", 1);
}

quit()
{
	if(ned_pid)
		kill(ned_pid, 9);
	unlink(tempfile);
	printf("[doc exits per QUIT]\n");
	exit();
}

len(s)
char *s;
{
	register n;

	n = 0;
	while(*s++)
		n++;
	return(n);
}
