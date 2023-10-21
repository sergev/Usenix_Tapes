/**				editor.c			**/

/** This is a very simple editor designed for general purpose use.
    
    This program is a sorta subset of the 'editor=none' option in the
    Elm program & the Berkeley mailer, etc, etc.

   (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>

#define  SLEN	100		/* length of a line of input */

#define  temp	"/tmp/pe"	/* temp file for pipe stuff  */
#define  SHELL	"sh"		/* default shell    	     */

#ifdef BSD
# include <signal.h>
#endif

char shell[SLEN]; 
char *getenv(), *strcpy();

main(argc, argv)
int argc;
char *argv[];
{
	/** assume we're invoked as "<editor> <filename>" **/
	FILE *fd;
	char buffer[SLEN];
#ifdef BSD
	int  dumb_continue();	/* says "(continue)" when return from ^Z */
#endif

	if (argc == 1) {
	  printf("Usage: %s filename\n", argv[0]);
	  exit(1);
	}

	if (access(argv[1], 00) != -1) {
	  printf("Refuse to edit an existing file!  Try a \"real\" editor!!\n");
	  exit(1);
	}

	if ((fd = fopen(argv[1], "w")) == NULL) {
	  printf("Can't open %s as the temp file!\n", argv[1]);
	  exit(1);
	}

	if (getenv("SHELL") != NULL)
	  strcpy(shell, getenv("SHELL"));
	else
	  strcpy(shell, SHELL);

	printf("Enter message, '^D' to end, or ~? for help;\n");

#ifdef BSD
	signal(SIGCONT, dumb_continue);
#endif

	while (gets(buffer) != NULL) {
	  if (strcmp(buffer, ".") == 0) break;	/* outta here! */
	  else if (buffer[0] == '~') {
	    switch (buffer[1]) {
	      case '\0' : printf("(Huh?  Try \"~?\" for help!)\n");	break;
	      case '?'  : help();					break;
	      case 'v'  : invoke("vi", fd, argv[1]);			break;
	      case 'e'  : invoke("emacs", fd, argv[1]);			break;
	      case 'o'  : invoke("", fd, argv[1]);			break;
	      case 'r'  : read_in(buffer, fd);         			break;
	      case 'w'  : write_out(buffer, fd, argv[1]);		break;
	      case 'p'  : fclose(fd);
		          fd = fopen(argv[1], "r");
			  while (fgets(buffer, SLEN, fd) != NULL)
	                    printf("%s", buffer);
		   	  printf("(continue)\n");
			  fd = fopen(argv[1], "a");			break;
	      case '!'  : if (strlen(buffer) > 2) 
			    system((char *) buffer + 2);
			  else
			    system(shell);				break;
	      case '|'  : pipe_it(buffer, fd, argv[1]);			break;
	      default   : printf("(don't know what \"~%c\" means!)\n", 
			  buffer[1]);
			  break;
	    }
	  }
	  else
	    fprintf(fd, "%s\n", buffer);
	}

	fclose(fd);
	
	exit(0);
}

#ifdef BSD

dumb_continue()
{
	/** just to make it all look nice.  We don't really DO much here! */

	signal(SIGCONT, dumb_continue);

	printf("(continue)\n");
}

#endif

help()
{
	/** list the possible commands! **/

	printf("(The commands available from here are;\n\
    ~?    list this help menu\n\
    ~!    either give you a shell, or execute the specified command\n\
    ~|    pipe the message written so far through the specified command\n\
    ~e    invoke \"emacs\" on the response so far\n\
    ~o    invoke the specified editor on the response\n\
    ~p    print what we've entered so far\n\
    ~r    read in the specified file\n\
    ~v    invoke \"vi\" on the response so far\n\
    ~w    write out the specified file\n\
\nto end the editor, use <control>-D or a '.' on a line by itself)\n");
}

invoke(editor, file_descriptor, filename)
char *editor, *filename;
FILE *file_descriptor;
{
	/** invokes the specified editor, closing the file and opening it
	    again when we're done.  If editor = NULL ask the user! **/

	char buffer[SLEN];

	if (strlen(editor) == 0) {
	  printf("Enter the name of the editor to use: ");
	  fflush(stdout);
	  gets(editor);
	  if (strlen(editor) == 0) goto end_it;
	}
	else {
	  printf("(invoking %s) ", editor);
	  fflush(stdout);
	}

	fclose(file_descriptor);

	sprintf(buffer, "%s %s", editor, filename);

	system(buffer);

	file_descriptor = fopen(filename, "a");

end_it:
	printf("(continue)\n");

	return;
}

pipe_it(command, file_descriptor, filename)
char *command, *filename;
FILE *file_descriptor;
{
	/** this will either accept a previously entered pipe command,
	    as in "~|fmt", or if none, will prompt for one.  It's really
	    quite a simple routine!
	**/

	char buffer[SLEN];

	fclose(file_descriptor);

	if (strlen(command) > 2) {
	  printf("(piping response through \"%s\")\n", 
		  (char *) command + 2);
	  sprintf(buffer, "cat %s | %s > %s.%d; mv %s.%d %s",
		  filename, (char *) command + 2, temp, getpid(),
		  temp, getpid(), filename);
	}
	else {
	  printf("Pipe message through: ");
	  fflush(stdout);
	  gets(command);
	  if (strlen(command) == 0) goto end_it;
	  sprintf(buffer, "cat %s | %s > %s.%d ; mv %s.%d %s",
		  filename, command, temp, getpid(),
		  temp, getpid(), filename);
	}
	
	system(buffer);

end_it:

	file_descriptor = fopen(filename, "a");

	printf("(continue)\n");

	return;
}

read_in(fname, fd)
char *fname;
FILE *fd;
{
	/** read the specified file in, continuing when done.  **/

	FILE     *newfd;
	register int i, j, lines = 0;
	char     filename[SLEN], buffer[SLEN];

	if (strlen(fname) < 3) {
	  printf("Enter name of file to read in: ");
	  gets(filename);
	  if (strlen(filename) == 0) goto end_it;
	}
	else {
	  for (i=2; fname[i] == ' '; i++) 
	    /* count up! */ ;
	  for (j = 0; i < strlen(fname);)
	    filename[j++] = fname[i++];
	  filename[j] = '\0';
	}

	if ((newfd = fopen(filename, "r")) == NULL) {
	  printf("(can't open file \"%s\" for reading!  Continue...)\n", 
		 filename);
	  return;
	}

	while (fgets(buffer, SLEN, newfd) != NULL) {
	  fprintf(fd, "%s", buffer);
	  lines++;
	}

	fclose(newfd);

	printf("(read in %d line%s from file \"%s\"   Continue...)\n",
		lines, lines == 1? "" : "s", filename);
	return;

end_it:
	printf("(continue)\n");
	return;
}

write_out(fname, fd, base_filename)
char *fname, *base_filename;
FILE *fd;
{
	/** write a copy of the current message to the specified file! **/

	FILE     *newfd;
	register int i, j, lines = 0;
	char     filename[SLEN], buffer[SLEN];

	if (strlen(fname) < 3) {
	  printf("Enter name of file to write to: ");
	  gets(filename);
	  if (strlen(filename) == 0) goto end_it;
	}
	else {
	  for (i=2; fname[i] == ' '; i++) 
	    /* count up! */ ;
	  for (j = 0; i < strlen(fname);)
	    filename[j++] = fname[i++];
	  filename[j] = '\0';
	}

	if ((newfd = fopen(filename, "w")) == NULL) {
	  printf("(Can't open file \"%s\" for writing!  Continue...)\n", 
		 filename);
	  return;
	}

	fclose(fd);	/* close the current file... */

	fd = fopen(base_filename, "r");	/* and open for reading */

	while (fgets(buffer, SLEN, fd) != NULL) {
	  fprintf(newfd, "%s", buffer);
	  lines++;
	}

	fclose(newfd);
	fclose(fd);	/* close the current file... */

	fd = fopen(base_filename, "a");	/* and open back up for appending */

	printf("(saved %d line%s to file \"%s\"   Continue...)\n",
		lines, lines == 1? "" : "s", filename);
	return;

end_it:
	printf("(continue)\n");
	return;
}
