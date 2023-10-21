/**			strings.c		**/

/** This file contains all the string oriented functions for the
    ELM Mailer, and lots of other generally useful string functions! 

    For BSD systems, this file also includes the function "tolower"
    to translate the given character from upper case to lower case.

    (C) Copyright 1985, Dave Taylor
**/

#include <stdio.h>
#include "headers.h"
#include <ctype.h>

#ifdef BSD
#undef tolower
#undef toupper
#endif

/** forward declarations **/

char *format_long(), *strip_commas(), *tail_of_string(), *shift_lower(),
     *get_token(), *strip_parens(), *argv_zero(), *strcpy(), *strncpy();

#ifdef BSD

int
tolower(ch)
char ch;
{
	/** This should be a macro call, but if you use this as a macro
	    calls to 'tolower' where the argument is a function call will
	    cause the function to be called TWICE which is obviously the
	    wrong behaviour.  On the other hand, to just blindly translate
	    assuming the character is always uppercase can cause BIG
	    problems, so...
	**/

	return ( isupper(ch) ? ch - 'A' + 'a' : ch );
}

int
toupper(ch)
char ch;
{
	/** see comment for above routine - tolower() **/

	return ( islower(ch) ? ch - 'a' + 'A' : ch );
}

#endif

int
printable_chars(string)
char *string;
{
	/** Returns the number of "printable" (ie non-control) characters
	    in the given string... Modified 4/86 to know about TAB
	    characters being every eight characters... **/

	register int count = 0, index;

	for (index = 0; index < strlen(string); index++)
	  if (string[index] >= ' ') 
	    if (string[index] == '\t')
	      count += (7-(count % 8));
	    else
	      count++;

	return(count);
}

tail_of(from, buffer, header_line)
char *from, *buffer;
int   header_line;
{
	/** Return last two words of 'from'.  This is to allow
	    painless display of long return addresses as simply the
	    machine!username.  Alternatively, if the first three
	    characters of the 'from' address are 'To:' and 'header_line'
	    is TRUE, then return the buffer value prepended with 'To '. 

	    Mangled to know about the PREFER_UUCP hack.  6/86
	**/

	/** Note: '!' delimits Usenet nodes, '@' delimits ARPA nodes,
	          ':' delimits CSNet & Bitnet nodes, and '%' delimits
		  multiple stage ARPA hops... **/

	register int loc, i = 0, cnt = 0;
	char     tempbuffer[SLEN];
	
#ifdef PREFER_UUCP
	
	/** let's see if we have an address appropriate for hacking **/

	if (chloc(from,'!') != -1 && in_string(from, BOGUS_INTERNET))
	   from[strlen(from)-strlen(BOGUS_INTERNET)] = '\0';

#endif

	for (loc = strlen(from)-1; loc >= 0 && cnt < 2; loc--) {
	  if (from[loc] == BANG || from[loc] == AT_SIGN ||
	      from[loc] == COLON) cnt++;
	  if (cnt < 2) buffer[i++] = from[loc];
	}

	buffer[i] = '\0';

	reverse(buffer);

	if ((strncmp(buffer,"To:", 3) == 0) && header_line)
	  buffer[2] = ' ';
	else if ((strncmp(from, "To:", 3) == 0) && header_line) {
	  sprintf(tempbuffer,"To %s", buffer); 
	  strcpy(buffer, tempbuffer);
	}
	else if (strncmp(buffer, "To:", 3) == 0) {
	  for (i=3; i < strlen(buffer); i++)
	    tempbuffer[i-3] = buffer[i];
	  tempbuffer[i-3] = '\0';
	  strcpy(buffer, tempbuffer);
	}
}

char *format_long(inbuff, init_len)
char *inbuff;
int   init_len;
{
	/** Return buffer with \n\t sequences added at each point where it 
	    would be more than 80 chars long.  It only allows the breaks at 
	    legal points (ie commas followed by white spaces).  init-len is 
	    the characters already on the first line...  Changed so that if 
            this is called while mailing without the overhead of "elm", it'll 
            include "\r\n\t" instead.
	    Changed to use ',' as a separator and to REPLACE it after it's
	    found in the output stream...
	**/

	static char ret_buffer[VERY_LONG_STRING];
	register int index = 0, current_length = 0, depth=15, i;
	char     buffer[VERY_LONG_STRING];
	char     *word, *bufptr;

	strcpy(buffer, inbuff);

	bufptr = (char *) buffer;

	current_length = init_len + 2;	/* for luck */

	while ((word = get_token(bufptr,",", depth)) != NULL) {

	    /* first, decide what sort of separator we need, if any... */

	  if (strlen(word) + current_length > 80) {
	    if (index > 0) {
	      ret_buffer[index++] = ',';	/* close 'er up, doctor! */
	      if (mail_only)
	        ret_buffer[index++] = '\r';
	      ret_buffer[index++] = '\n';
	      ret_buffer[index++] = '\t';
	    }
	    
	    /* now add this pup! */

	    for (i=(word[0] == ' '? 1:0); i<strlen(word); i++)
	      ret_buffer[index++] = word[i];
	    current_length = strlen(word) + 8;	/* 8 = TAB */
	  }

	  else {	/* just add this address to the list.. */

	    if (index > 0) {
	      ret_buffer[index++] = ',';	/* comma added! */
	      ret_buffer[index++] = ' ';
	      current_length += 2;
	    }
	    for (i=(word[0] == ' '? 1:0); i<strlen(word); i++)
	      ret_buffer[index++] = word[i];
	    current_length += strlen(word);
	  }
	
	  bufptr = NULL;
	}
	
	ret_buffer[index] = '\0';

	return( (char *) ret_buffer);
}

char *strip_commas(string)
char *string;
{
	/** return string with all commas changed to spaces.  This IS
	    destructive and will permanently change the input string.. **/

	register int i;

	for (i=0; i < strlen(string); i++)
	  if (string[i] == COMMA)
	    string[i] = SPACE;

	return( (char *) string);
}

char *strip_parens(string)
char *string;
{
	/** Return string with all parenthesized information removed.
	    This is a non-destructive algorithm... **/

	static char  buffer[VERY_LONG_STRING];
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

move_left(string, chars)
char string[];
int  chars;
{
	/** moves string chars characters to the left DESTRUCTIVELY **/

	register int i;

	chars--; /* index starting at zero! */

	for (i=chars; string[i] != '\0' && string[i] != '\n'; i++)
	  string[i-chars] = string[i];

	string[i-chars] = '\0';
}

remove_first_word(string)
char *string;
{	/** removes first word of string, ie up to first non-white space
	    following a white space! **/

	register int loc;

	for (loc = 0; string[loc] != ' ' && string[loc] != '\0'; loc++) 
	    ;

	while (string[loc] == ' ' || string[loc] == '\t')
	  loc++;
	
	move_left(string, ++loc);
}

split_word(buffer, first, rest)
char *buffer, *first, *rest;
{
	/** Rip the buffer into first word and rest of word, translating it
	    all to lower case as we go along..
	**/

	register int i, j = 0;

	/** skip leading white space, just in case.. **/

	for (i=0; whitespace(buffer[i]); i++)	;

	/** now copy into 'first' until we hit white space or EOLN **/

	for (j=0; i < strlen(buffer) && ! whitespace(buffer[i]); )
	  first[j++] = tolower(buffer[i++]);

	first[j] = '\0';
	
	while (whitespace(buffer[i])) i++;

	for (j=0; i < strlen(buffer); i++)
	  rest[j++] = tolower(buffer[i]);

	rest[j] = '\0';

	return;
}

char *tail_of_string(string, maxchars)
char *string;
int  maxchars;
{
	/** Return a string that is the last 'maxchars' characters of the
	    given string.  This is only used if the first word of the string
	    is longer than maxchars, else it will return what is given to
	    it... 
	**/

	static char buffer[SLEN];
	register int index, i;

	for (index=0;! whitespace(string[index]) && index < strlen(string); 
	     index++)
	  ;

	if (index < maxchars) {
	  strncpy(buffer, string, maxchars-2);	/* word too short */
	  buffer[maxchars-2] = '.';
	  buffer[maxchars-1] = '.';
	  buffer[maxchars]   = '.';
	  buffer[maxchars+1] = '\0';
	} 
	else {
	  i = maxchars;
	  buffer[i--] = '\0';
	  while (i > 1) 
	    buffer[i--] = string[index--];
	  buffer[2] = '.';
	  buffer[1] = '.';
	  buffer[0] = '.';
	}

	return( (char *) buffer);
}

reverse(string)
char *string;
{
	/** reverse string... pretty trivial routine, actually! **/

	char buffer[SLEN];
	register int i, j = 0;

	for (i = strlen(string)-1; i >= 0; i--)
	  buffer[j++] = string[i];

	buffer[j] = '\0';

	strcpy(string, buffer);
}

int
get_word(buffer, start, word)
char *buffer, *word;
int start;
{
	/**	return next word in buffer, starting at 'start'.
		delimiter is space or end-of-line.  Returns the
		location of the next word, or -1 if returning
		the last word in the buffer.  -2 indicates empty
		buffer!  **/

	register int loc = 0;

	while (buffer[start] == ' ' && buffer[start] != '\0')
	  start++;

	if (buffer[start] == '\0') return(-2);	 /* nothing IN buffer! */

	while (buffer[start] != ' ' && buffer[start] != '\0')
	  word[loc++] = buffer[start++];

	word[loc] = '\0';
	return(start);
}

char *shift_lower(string)
char *string;
{
	/** return 'string' shifted to lower case.  Do NOT touch the
	    actual string handed to us! **/

	static char buffer[LONG_SLEN];
	register int i;

	for (i=0; i < strlen(string); i++)
	  if (isupper(string[i]))
	    buffer[i] = tolower(string[i]);
	  else
	    buffer[i] = string[i];
	
	buffer[strlen(string)] = 0;
	
	return( (char *) buffer);
}

Centerline(line, string)
int line;
char *string;
{
	/** Output 'string' on the given line, centered. **/

	register int length, col;

	length = strlen(string);

	if (length > COLUMNS)
	  col = 0;
	else
	  col = (COLUMNS - length) / 2;

	PutLine0(line, col, string);
}

char *argv_zero(string)
char *string;
{
	/** given a string of the form "/something/name" return a
	    string of the form "name"... **/

	static char buffer[NLEN];
	register int i, j=0;

	for (i=strlen(string)-1; string[i] != '/'; i--)
	  buffer[j++] = string[i];
	buffer[j] = '\0';

	reverse(buffer);

	return( (char *) buffer);
}

#define MAX_RECURSION		20		/* up to 20 deep recursion */

char *get_token(source, keys, depth)
char *source, *keys;
int   depth;
{
	/** This function is similar to strtok() (see "opt_utils")
	    but allows nesting of calls via pointers... 
	**/

	register int  last_ch;
	static   char *buffers[MAX_RECURSION];
	char     *return_value, *sourceptr;

	if (depth > MAX_RECURSION) {
	   error1("get_token calls nested greater than %d deep!", 
		  MAX_RECURSION);
	   emergency_exit();
	}

	if (source != NULL)
	  buffers[depth] = source;
	
	sourceptr = buffers[depth];
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we hit end-of-string last time!? */

	sourceptr += strspn(sourceptr, keys);	  /* skip the bad.. */
	
	if (*sourceptr == '\0') {
	  buffers[depth] = sourceptr;
	  return(NULL);			/* we've hit end-of-string   */
	}

	last_ch = strcspn(sourceptr, keys);   /* end of good stuff   */

	return_value = sourceptr;	      /* and get the ret     */

	sourceptr += last_ch;		      /* ...value            */

	if (*sourceptr != '\0')		/** don't forget if we're at end! **/
	  sourceptr++;			      
	
	return_value[last_ch] = '\0';	      /* ..ending right      */

	buffers[depth] = sourceptr;	      /* save this, mate!    */

	return((char *) return_value);	     /* and we're outta here! */
}
