/**		newalias.c		**/

/** (C) Copyright 1986 Dave Taylor      **/

/** Install a new set of aliases for the 'Elm' mailer. 

	If invoked with a specific filename, it assumes that
  it is working with an individual users alias tables, and
  generates the .alias.hash and .alias.data files in their
  home directory.
	If, however, it is invoked with no arguments, then
  it assumes that the user is updating the system alias
  file and uses the defaults for everything.

  The format for the input file is;
    alias1, alias2, ... : username : address
or  alias1, alias2, ... : groupname: member, member, member, ...
                                     member, member, member, ...

  "-q" flag added: 6/17/86
**/

#ifdef BSD
#  include <sys/file.h>
#else
#  include <fcntl.h>
#endif

#include <stdio.h>
#include "defs.h"		/* ELM system definitions */

static char ident[] = { WHAT_STRING };

#ifndef TAB
# define TAB	 	'\t'	/* TAB character!         */
#endif
	
#define alias_hash	".alias_hash"
#define alias_data	".alias_data"
#define alias_text	".alias_text"

#define group(string)		(strpbrk(string,", ") != NULL)

struct alias_rec
shash_table[MAX_SALIASES];	/* the actual hash table     */

struct alias_rec
uhash_table[MAX_UALIASES];	/* the actual hash table     */

int  hash_table_loaded=0;	/* is system table actually loaded? */

int  buff_loaded;		/* for file input overlap... */
int  error= 0;			/* if errors, don't save!    */
int  system=0;			/* system file updating?     */
int  count=0;			/* how many aliases so far?  */
long offset = 0L;		/* data file line offset!    */

main(argc, argv)
int argc;
char *argv[];
{
	FILE *in, *data;
	char inputname[SLEN], hashname[SLEN], dataname[SLEN];
	char home[SLEN], buffer[LONG_STRING];
	int  hash, count = 0, owner, quiet = 0;

	if (argc != 1)
	  if (strcmp(argv[1], "-q") == 0)
	    quiet++;
	  else
	    exit(printf("Usage: %s\n", argv[0]));

	owner = getuid();

	if (owner == 0 && ! quiet) {	/* being run by root! */
	  printf("Would you like to update the system aliases? (y/n)");
	  gets(buffer, 2);
	  if (buffer[0] == 'y' || buffer[0] == 'Y') {
	    printf("Updating the system alias file...\n");

	    sprintf(inputname, "%s/%s", mailhome, alias_text);
	    sprintf(hashname, "%s/%s", mailhome, alias_hash);
	    sprintf(dataname, "%s/%s", mailhome, alias_data);
	    system++;
	    init_table(shash_table, MAX_SALIASES); 
	  }
	  else 
	    printf("Updating your personal alias file...\n");
	}
	
	if (! system) {
	  if (strcpy(home, getenv("HOME")) == NULL)
	    exit(printf("Confused: No HOME variable in environment!\n"));

	  sprintf(inputname, "%s/%s", home, alias_text);
	  sprintf(hashname,  "%s/%s", home, alias_hash); 
	  sprintf(dataname,  "%s/%s", home, alias_data); 

	  init_table(uhash_table, MAX_UALIASES); 

	  read_in_system(shash_table, sizeof shash_table);
	}

	if ((in = fopen(inputname,"r")) == NULL)
	  exit(printf("Couldn't open %s for input!\n", inputname));

	if ((hash = open(hashname, O_WRONLY | O_CREAT, 0644)) == -1)
	  exit(printf("Couldn't open %s for output!\n", hashname));

	if ((data = fopen(dataname,"w")) == NULL)
	  exit(printf("Couldn't open %s for output!\n", dataname));

	buff_loaded = 0; 	/* file buffer empty right now! */

	while (get_alias(in, buffer) != -1) {
	  if (system)
	    put_alias(data, buffer, shash_table, MAX_SALIASES);	
	  else
	    put_alias(data, buffer, uhash_table, MAX_UALIASES);	
	  count++;
	}

	if (error) {
	  printf("\n** Not saving tables!  Please fix and re-run %s!\n",
		 argv[0]);
	  exit(1);
	}
	else {
	  if (system)
	    write(hash, shash_table, sizeof shash_table);
	  else
	    write(hash, uhash_table, sizeof uhash_table);

	  close(hash);
	  fclose(data);
	  close(in);
	
	  printf("Processed %d aliases\n", count);
	  exit(0);
	}
}

int
get_alias(file, buffer)
FILE *file;
char *buffer;
{
	/* load buffer with the next complete alias from the file.
	   (this can include reading in multiple lines and appending
	   them all together!)  Returns EOF after last entry in file.
	
	Lines that start with '#' are assumed to be comments and are
 	ignored.  White space as the first field of a line is taken
	to indicate that this line is a continuation of the previous. */

	static char mybuffer[SLEN];
	int    done = 0, first_read = 1;

	/** get the first line of the entry... **/

	buffer[0] = '\0';			/* zero out line */

	do {
	  if (get_line(file, mybuffer, first_read) == -1) 
	    return(-1);
	  first_read = 0;
	  if (mybuffer[0] != '#')
	    strcpy(buffer, mybuffer);
	} while (strlen(buffer) == 0);	

	/** now read in the rest (if there is any!) **/

	do {
	  if (get_line(file, mybuffer, first_read) == -1) {
	    buff_loaded = 0;	/* force a read next pass! */
	    return(0);	/* okay. let's just hand 'buffer' back! */
	  }
	  done = (mybuffer[0] != ' ' && mybuffer[0] != TAB);
	  if (mybuffer[0] != '#' && ! done)
	    strcat(buffer, mybuffer);
	  done = (done && mybuffer[0] != '#');
	} while (! done);
	
	return(0);	/* no sweat! */
}

put_alias(data, buffer, table, size)
FILE *data;
char *buffer;
struct alias_rec table[];
int  size;
{
	/** break buffer down into three pieces: aliases, comment, and address.
	    Make the appropriate entries in the table (size) 
	**/

	char aliases[LONG_STRING], address[LONG_STRING];
	char comment[LONG_STRING];
	int  first, last, i = 0, j = 0;

	remove_all(' ', TAB, buffer);

	for (i=0; buffer[i] != ':' && i < LONG_STRING; i++)
	  aliases[i] = buffer[i];
	aliases[i] = '\0';

	for (i=strlen(buffer)-1; buffer[i] != ':' && i > 0; i--)
	  address[j++] = buffer[i];
	address[j] = '\0';

	comment[0] = '\0';	/* default to nothing at all... */

	if ((first=strlen(aliases)+1) < (last=(strlen(buffer) - j))) {
	  extract_comment(comment, buffer, first, last); 
	}

	reverse(address);

	add_to_table(data, aliases, comment, address, table, size);
}

int
get_line(file, buffer, first_line)
FILE *file;
char *buffer;
int  first_line;
{
	/** read line from file.  If first_line and buff_loaded, 
	    then just return! **/
	int stat;

	if (first_line && buff_loaded) {
	  buff_loaded = 1;
	  return;
	}

	buff_loaded = 1;	/* we're going to get SOMETHING in the buffer */

	stat = fgets(buffer, SLEN, file) == NULL ? -1 : 0;

	if (stat != -1)
	  no_ret(buffer);

	return(stat);
}

reverse(string)
char *string;
{
	/** reverse the order of the characters in string... 
	    uses a bubble-sort type of algorithm!                 **/
	
	register int f, l;
	char     c;
	
	f = 0;
	l = strlen(string) - 1;
	
	while (f < l) {
	  c = string[f];
 	  string[f] = string[l];
	  string[l] = c;
	  f++;
	  l--;
	}
}

add_to_table(data, aliases, comment, address, table, size)
FILE *data;
char *aliases, *comment, *address;
struct alias_rec table[];
int  size;
{
	/** add address + comment to datafile, incrementing offset count 
	    (bytes), then for each alias in the aliases string, add to the
	    hash table, with the associated pointer value! **/

	static char buf[SLEN], *word;
	long additive = 1L;

	word = buf;	/* use the allocated space! */

	if (group(address)) {
	  check_group(address, aliases);
	  if (error) return;	/* don't do work if we aren't to save it! */
	  fprintf(data, "!%s\n", address);
	  additive = 2L;
	}
	else {
	  if (error) return;	/* don't do work if we aren't to save it! */
	  if (strlen(comment) > 0) {
	    fprintf(data, "%s (%s)\n", address, comment);
	    additive = (long) (strlen(comment) + 4);
	  }
	  else
	    fprintf(data, "%s\n", address, comment);
	}

	while ((word = (char *) strtok(aliases,", ")) != NULL) {
	  add_to_hash_table(word, offset, table, size);
	  aliases = NULL;	/* let's get ALL entries via 'strtok' */
	  count++;
	}

	if ( system ? count > MAX_SALIASES-35 : count > MAX_UALIASES-21) {
	  printf("** Too many aliases in file! **\n");
	  error++;
	}

	offset = (offset + (long) strlen(address) + additive);
}	

remove_all(c1, c2, string)
char c1, c2, *string;
{
	/* Remove all occurances of character 'c1' or 'c2' from the string.
	   Hacked (literally) to NOT remove ANY characters from within the
	   colon fields.  This will only be used if the line contains TWO
	   colons (and comments with colons in them are the kiss of death!)
	 */

	char buffer[LONG_STRING];
	register int i = 0, j = 0, first_colon = -1, last_colon = -1;
	
	for (i = 0; string[i] != '\0' && i < LONG_STRING; i++) {
	  if (string[i] != c1 && string[i] != c2)
	    buffer[j++] = string[i];

	  if (first_colon == -1 && string[i] == ':') {
	    first_colon = i;
	    for (last_colon=strlen(string);string[last_colon] != ':'; 
		last_colon--) ;
	  }
	  else if (i > first_colon && i < last_colon)
	   if (string[i] == c1 || string[i] == c2)
	     buffer[j++] = string[i];
	}
	
	buffer[j] = '\0';
	strcpy(string, buffer);
}

add_to_hash_table(word, offset, table, size)
char *word;
long  offset;
struct alias_rec table[];
int   size;
{
	/** add word and offset to current hash table. **/
	register int loc;
	
	if (strlen(word) > 20)
	  exit(printf("Bad alias name: %s.  Too long.\n", word));

	loc = hash_it(word, size);

	while (table[loc].name[0] != '\0' && strcmp(table[loc].name, word) != 0)
	  loc = loc + 1 % size; 

	if (table[loc].name[0] == '\0') {
	  strcpy(table[loc].name, word);
	  table[loc].byte = offset;
	}
	else 
	  printf("** Duplicate alias '%s' in file.  Multiples ignored.\n",
	         word);
}

int
hash_it(string, table_size)
char *string;
{
	/** compute the hash function of the string, returning
	    it (mod table_size) **/

	register int i, sum = 0;
	
	for (i=0; string[i] != '\0'; i++)
	  sum += (int) string[i];

	return(sum % table_size);
}

init_table(table, size)
struct alias_rec table[];
int size;
{
	/** initialize hash table! **/

	register int i;

	for (i=0; i < size; i++)
	  table[i].name[0] = '\0';
}

read_in_system(table, size)
struct alias_rec table[];
int size;
{
	/** read in the system hash table...to check for group aliases
	    from the user alias file (to ensure that there are no names
	    in the user group files that are not purely contained within
	    either alias table) **/
	
	int  fd;
	char fname[SLEN];

	sprintf(fname, "%s/%s", mailhome, alias_hash);

	if ((fd = open(fname, O_RDONLY)) == -1)
	  return;	/* no sweat: flag 'hash_table_loaded' not set! */

	(void) read(fd, table, size);
	close(fd);
	hash_table_loaded++;
}
	
check_group(names, groupname)
char *names, *groupname;
{
	/** one by one make sure each name in the group is defined
	    in either the system alias file or the user alias file.
	    This search is linearly dependent, so all group aliases
	    in the source file should appear LAST, after all the user
	    aliases! **/

	char *word, *bufptr, buffer[LONG_STRING];

	strcpy(buffer, names);
	bufptr = (char *) buffer;

	while ((word = (char *) strtok(bufptr,", ")) != NULL) {
	  if (! can_find(word)) 
	    if (! valid_name(word)) {
	      error++;
	      printf("** Alias %s in group %s is bad!\n", word, groupname);
	    }
	  bufptr = NULL;
	}
}

int
can_find(name)
char *name;
{	
	/** find name in either hash table...use 'system' variable to
	    determine if we should look in both or just system....    **/

	register int loc;
	
	if (strlen(name) > 20) {
	  error++;
	  printf("** Bad alias name: %s.  Too long.\n", name);
	  return(1);	/* fake out: don't want 2 error messages! */
	}

	/** system alias table... **/
	if (hash_table_loaded || system) {
	  loc = hash_it(name, MAX_SALIASES);

	  while (strcmp(name, shash_table[loc].name) != 0 && 
                 shash_table[loc].name[0] != '\0')
	    loc = (loc + 1) % MAX_SALIASES; 
  
	  if (strcmp(name, shash_table[loc].name) == 0)
	    return(1);	/* found it! */
	}

	if (! system) {	/* okay! Let's check the user alias file! */
	  loc = hash_it(name, MAX_UALIASES);

	  while (strcmp(name, uhash_table[loc].name) != 0 && 
                 uhash_table[loc].name[0] != '\0')
	    loc = (loc + 1) % MAX_UALIASES; 

	  if (strcmp(name, uhash_table[loc].name) == 0)
	    return(1);	/* found it! */
	}

	return(0);
}

extract_comment(comment, buffer, first, last)
char *comment, *buffer;
int first, last;
{
	/** Buffer contains a comment, located between the first and last
	    values.  Copy that into 'comment', but remove leading and
	    trailing white space.  Note also that it doesn't copy past
	    a comma, so `unpublishable' comments can be of the form;
		dave: Dave Taylor, HP Labs : taylor@hplabs
	    and the output will be "taylor@hplabs (Dave Taylor)".
	**/

	register int loc = 0; 

	/** first off, skip the LEADING white space... **/

	while (buffer[first] == ' ') first++;
	
	/** now let's backup the 'last' value until we hit a non-space **/

	last -= 2;	/* starts at ch AFTER colon.. */
	while (buffer[last] == ' ') last--;

	/** now a final check to make sure we're still talking about a 
	    reasonable string (rather than a "joe :: joe@dec" type string) **/

	if (first < last) {
	  /* one more check - let's find the comma, if present... */
	  for (loc=first; loc < last; loc++)
	    if (buffer[loc] == ',') {
	      last = loc-1;
	      break;
	  }
	  loc = 0;
	  while (first <= last)
	    comment[loc++] = buffer[first++];
	  comment[loc] = '\0';
	}
}
