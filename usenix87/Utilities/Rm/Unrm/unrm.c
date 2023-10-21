/**			unrm.c			**/

/** This is the companion program to the rm.c program, and will extract
    files from the RM_DIR/login directory if they exist.  It checks to see 
    that you are indeed the owner of the file before it'll let you copy
    it AND it ensures that the file doesn't already exist in the current
    directory (makes sense, eh?).

    This will not allow unrm'ing files that aren't owned by you, nor 
    will it allow restores that replace a file of the same name in the
    current directory (unless '-f' is specified, which will cause the
    file to be replaced regardless).

    (C) Copyright 1986, Dave Taylor, Hewlett-Packard
**/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define  RM_DIR		"/tmp/rm"

/** access modes for calls to 'access()' **/

#define  DIRACCESS	02 & 04
#define  TOCOPY		04
#define  TOREPLACE	02 & 04

#define  SLEN		80

int      force_overwrite = 0;		/* replace current regardless! */

char     *getenv(), *getlogin();

main(argc, argv)
int argc;
char **argv;
{
	extern int optind;	/* for getopt */
	char   buffer[SLEN], login_name[SLEN], dirname[SLEN], *cp;
	int    c;

	while ((c = getopt(argc, argv, "f")) != EOF) {
	  switch (c) {
	    case 'f' : force_overwrite++;	break;
	    case '?' : exit(fprintf(stderr,"Usage: unrm [-f] files\n"));
	  }
	}

	if (argv[optind] == 0 || strlen(argv[optind]) == 0)
	  exit(0);

	if (access(RM_DIR, DIRACCESS)) {
	  fprintf(stderr,"Error: Directory %s doesn't exist!\n", RM_DIR);
	  exit(0);
	}

	if ((cp = getenv("LOGNAME")) == NULL)
	  strcpy(login_name, getlogin());
	else
	  strcpy(login_name, cp);

	sprintf(dirname, "%s/%s", RM_DIR, login_name);

	if (access(dirname, DIRACCESS)) {
	  fprintf(stderr,"Error: Directory %s doesn't exist!\n", dirname);
	  exit(0);
	}


	while (argv[optind] && strlen(argv[optind]) > 0) {
	  restore(dirname, argv[optind]);
	  optind++;
	}

	exit(0);
}

restore(directory, filename)
char *directory, *filename;
{
	/** Try to link RM_DIR/filename to current directory.  If that
	    fails, try to copy it byte by byte... **/

	struct stat buffer;
	char newfname[80], answer[80];

	sprintf(newfname,"%s/%s", directory, filename);

	if (access(newfname,TOCOPY) != 0)
	  return(fprintf(stderr,"Error: Can't find old copy of '%s'!\n", 
		 filename));
	
	if (stat(newfname, &buffer) != 0)
	  return(fprintf(stderr,"Error: Can't stat old copy of '%s'!\n", 
		 filename));

	if (buffer.st_uid != getuid())
	  return(fprintf(stderr,"Error: File '%s' isn't yours to restore!\n", 
		 filename));

	/** now we're ready to start some REAL work... **/

	if (access(filename,TOREPLACE) == 0) {	/* it exists! */
	  if (! force_overwrite)
	    printf(
	    "File %s already exists in this directory!  Replace it? (y/n) ",
	           filename);
	    gets(answer, 1);
	    if (tolower(answer[0]) != 'y') {
	      fprintf(stderr,"Restore of file %s cancelled\n", filename);
	      return;
	    }
	}

	(void) unlink(filename);	/* blow it away, if it's here! */

	if (link(newfname, filename) != 0) {
	  FILE *infile, *outfile;	
	  int   c;
	  
	  if ((infile = fopen(newfname, "r")) == NULL)
	    exit(fprintf(stderr, 
                 "Error: Can't read file '%s' to restore from!\n", 
		 newfname));

	  if ((outfile = fopen(filename, "w")) == NULL)
	    exit(fprintf(stderr, "Error: Can't write to file '%s'!\n",
		 filename));

	  while ((c = getc(infile)) != EOF)
	    putc(c, outfile);
	  
	  fclose(infile);
	  fclose(outfile);
	}

	unlink(newfname);

	fprintf(stderr,"Restored file '%s'\n", filename);
}
