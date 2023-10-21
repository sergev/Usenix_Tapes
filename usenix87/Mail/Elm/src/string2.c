/**			string2.c		**/

/** This file contains string functions that are shared throughout the
    various ELM utilities...

    (C) Copyright 1986 Dave Taylor
**/

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

int 
in_string(buffer, pattern)
char *buffer, *pattern;
{
	/** Returns TRUE iff pattern occurs IN IT'S ENTIRETY in buffer. **/ 

	register int i = 0, j = 0;
	
	while (buffer[i] != '\0') {
	  while (buffer[i++] == pattern[j++]) 
	    if (pattern[j] == '\0') 
	      return(TRUE);
	  i = i - j + 1;
	  j = 0;
	}
	return(FALSE);
}

int
chloc(string, ch)
char *string, ch;
{
	/** returns the index of ch in string, or -1 if not in string **/
	register int i;

	for (i=0; i<strlen(string); i++)
	  if (string[i] == ch) return(i);
	return(-1);
}

int
occurances_of(ch, string)
char ch, *string;
{
	/** returns the number of occurances of 'ch' in string 'string' **/

	register int count = 0, i;

	for (i=0; i<strlen(string); i++)
	  if (string[i] == ch) count++;

	return(count);
}
