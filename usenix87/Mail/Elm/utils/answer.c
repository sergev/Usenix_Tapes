/**			answer.c			**/

/** This program is a phone message transcription system, and
    is designed for secretaries and the like, to allow them to
    painlessly generate electronic mail instead of paper forms.

    Note: this program ONLY uses the local alias file, and does not
	  even read in the system alias file at all.

    (C) Copyright 1986, Dave Taylor

**/

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include "defs.h"			/* ELM system definitions      */

#define  ELM		"elm"		/* where the elm program lives */

#define  answer_temp_file	"/tmp/answer."

static char ident[] = { WHAT_STRING };

struct alias_rec user_hash_table  [MAX_UALIASES];

int user_data;		/* fileno of user data file   */

char *expand_group(), *get_alias_address(), *get_token(), *strip_parens();

main()
{
	FILE *fd;
	char *address, buffer[LONG_STRING], tempfile[SLEN];
	char  name[SLEN], user_name[SLEN];
	int   msgnum = 0, eof;
	
	read_alias_files();

	while (1) {
	  if (msgnum > 9999) msgnum = 0;
	
	  printf("\n-------------------------------------------------------------------------------\n");

prompt:   printf("\nMessage to: ");
	  gets(user_name, SLEN);
	  if (user_name == NULL)
	    goto prompt;
	  
	  if ((strcmp(user_name,"quit") == 0) ||
	      (strcmp(user_name,"exit") == 0) ||
	      (strcmp(user_name,"done") == 0) ||
	      (strcmp(user_name,"bye")  == 0))
	     exit(0);

	  if (translate(user_name, name) == 0)
	    goto prompt;

	  address = get_alias_address(name, 1, 0);

	  printf("address '%s'\n", address);

	  if (address == NULL || strlen(address) == 0) {
	    printf("Sorry, could not find '%s' [%s] in list!\n", user_name, 
		   name);
	    goto prompt;
	  }

	  sprintf(tempfile, "%s%d", answer_temp_file, msgnum++);

	  if ((fd = fopen(tempfile,"w")) == NULL)
	    exit(printf("** Fatal Error: could not open %s to write\n",
		 tempfile));


	  printf("\nEnter message for %s ending with a blank line.\n\n", 
		 user_name);

	  fprintf(fd,"\n\n");

	  do {
	   printf("> ");
	   if (! (eof = (gets(buffer, SLEN) == NULL))) 
	     fprintf(fd, "%s\n", buffer);
	  } while (! eof && strlen(buffer) > 0);
	
	  fclose(fd);
 
	  sprintf(buffer, 
	     "((%s -s \"While You Were Out\" %s ; %s %s) & ) < %s > /dev/null",
	     ELM, strip_parens(address), remove, tempfile, tempfile);

	  system(buffer);
	}
}

int
translate(fullname, name)
char *fullname, *name;
{
	/** translate fullname into name..
	       'first last'  translated to first_initial - underline - last
	       'initial last' translated to initial - underline - last
	    Return 0 if error.
	**/
	register int i, lastname = 0;

	for (i=0; i < strlen(fullname); i++) {

	  if (isupper(fullname[i]))
	     fullname[i] = tolower(fullname[i]);

	  if (fullname[i] == ' ') 
	    if (lastname) {
	      printf(
	      "** Can't have more than 'FirstName LastName' as address!\n");
	      return(0);
	    }
	    else
	      lastname = i+1;
	
	}

	if (lastname) 
	  sprintf(name, "%c_%s", fullname[0], (char *) fullname + lastname);
	else
	  strcpy(name, fullname);

	return(1);
}

	    
read_alias_files()
{
	/** read the user alias file **/

	char fname[SLEN];
	int  hash;

	sprintf(fname,  "%s/.alias_hash", getenv("HOME")); 

	if ((hash = open(fname, O_RDONLY)) == -1) 
	  exit(printf("** Fatal Error: Could not open %s!\n", fname));

	read(hash, user_hash_table, sizeof user_hash_table);
	close(hash);

	sprintf(fname,  "%s/.alias_data", getenv("HOME")); 

	if ((user_data = open(fname, O_RDONLY)) == -1) 
	  return;
}

char *get_alias_address(name, mailing, depth)
char *name;
int   mailing, depth;
{
	/** return the line from either datafile that corresponds 
	    to the specified name.  If 'mailing' specified, then
	    fully expand group names.  Returns NULL if not found.
	    Depth is the nesting depth, and varies according to the
	    nesting level of the routine.  **/

	static char buffer[VERY_LONG_STRING];
	int    loc;

	if ((loc = find(name, user_hash_table, MAX_UALIASES)) >= 0) {
	  lseek(user_data, user_hash_table[loc].byte, 0L);
	  get_line(user_data, buffer, LONG_STRING);
	  if (buffer[0] == '!' && mailing)
	    return( (char *) expand_group(buffer, depth));
	  else
	    return( (char *) buffer);
	}
	
	return( (char *) NULL);
}

char *expand_group(members, depth)
char *members;
int   depth;
{
	/** given a group of names separated by commas, this routine
	    will return a string that is the full addresses of each
	    member separated by spaces.  Depth is the current recursion
	    depth of the expansion (for the 'get_token' routine) **/

	char   buffer[VERY_LONG_STRING];
	char   buf[LONG_STRING], *word, *address, *bufptr;

	strcpy(buf, members); 	/* parameter safety! */
	buffer[0] = '\0';	/* nothing in yet!   */
	bufptr = (char *) buf;	/* grab the address  */
	depth++;		/* one more deeply into stack */

	while ((word = (char *) get_token(bufptr, "!, ", depth)) != NULL) {
	  if ((address = (char *) get_alias_address(word, 1, depth)) == NULL) {
	    fprintf(stderr, "Alias %s not found for group expansion!", word);
	    return( (char *) NULL);
	  }
	  else if (strcmp(buffer,address) != 0) {
	    sprintf(buffer,"%s %s", buffer, address);
	  }

	  bufptr = NULL;
	}

	return( (char *) buffer);
}

int
find(word, table, size)
char *word;
struct alias_rec table[];
int size;
{
	/** find word and return loc, or -1 **/
	register int loc;
	
	if (strlen(word) > 20)
	  exit(printf("Bad alias name: %s.  Too long.\n", word));

	loc = hash_it(word, size);

	while (strcmp(word, table[loc].name) != 0) {
	  if (table[loc].name[0] == '\0') 
	    return(-1);
	  loc = (loc + 1) % size; 
	}

	return(loc);
}

int
hash_it(string, table_size)
char *string;
int   table_size;
{
	/** compute the hash function of the string, returning
	    it (mod table_size) **/

	register int i, sum = 0;
	
	for (i=0; string[i] != '\0'; i++)
	  sum += (int) string[i];

	return(sum % table_size);
}

get_line(fd, buffer)
int fd;
char *buffer;
{
	/* read from file fd.  End read upon reading either 
	   EOF or '\n' character (this is where it differs 
	   from a straight 'read' command!) */

	register int i= 0;
	char     ch;

	while (read(fd, &ch, 1) > 0)
	  if (ch == '\n' || ch == '\r') {
	    buffer[i] = 0;
	    return;
	  }
	  else
	    buffer[i++] = ch;
}

print_long(buffer, init_len)
char *buffer;
int   init_len;
{
	/** print buffer out, 80 characters (or less) per line, for
	    as many lines as needed.  If 'init_len' is specified, 
	    it is the length that the first line can be.
	**/

	register int i, loc=0, space, length; 

	/* In general, go to 80 characters beyond current character
	   being processed, and then work backwards until space found! */

	length = init_len;

	do {
	  if (strlen(buffer) > loc + length) {
	    space = loc + length;
	    while (buffer[space] != ' ' && space > loc + 50) space--;
	    for (i=loc;i <= space;i++)
	      putchar(buffer[i]);
	    putchar('\n');
	    loc = space;
	  }
	  else {
	    for (i=loc;i < strlen(buffer);i++)
	      putchar(buffer[i]);
	    putchar('\n');
	    loc = strlen(buffer);
	  }
	  length = 80;
	} while (loc < strlen(buffer));
}

/****
     The following is a newly chopped version of the 'strtok' routine
  that can work in a recursive way (up to 20 levels of recursion) by
  changing the character buffer to an array of character buffers....
****/

#define MAX_RECURSION		20		/* up to 20 deep recursion */

#undef  NULL
#define NULL			(char *) 0	/* for this routine only   */

extern int strspn();
extern char *strpbrk();

char *get_token(string, sepset, depth)
char *string, *sepset;
int  depth;
{

	/** string is the string pointer to break up, sepstr are the
	    list of characters that can break the line up and depth
	    is the current nesting/recursion depth of the call **/

	register char	*p, *q, *r;
	static char	*savept[MAX_RECURSION];

	/** is there space on the recursion stack? **/

	if (depth >= MAX_RECURSION) {
	 fprintf(stderr,"Error: Get_token calls nested greated than %d deep!\n",
			MAX_RECURSION);
	 exit(1);
	}

	/* set up the pointer for the first or subsequent call */
	p = (string == NULL)? savept[depth]: string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if (*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if ((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept[depth] = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept[depth] = ++r;
	}
	return(q);
}

char *strip_parens(string)
char *string;
{
	/** Return string with all parenthesized information removed.
	    This is a non-destructive algorithm... **/

	static char  buffer[LONG_STRING];
	register int i, depth = 0, buffer_index = 0;

	for (i=0; i < strlen(string); i++) {
	  if (string[i] == '(')
	    depth++;
	  else if (string[i] == ')') 
	    depth--;
	  else if (depth == 0)
	    buffer[buffer_index++] = string[i];
	}
	
	buffer[buffer_index] = '\0';

	return( (char *) buffer);
}
