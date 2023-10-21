
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Assorted things.
 */

#include <stdio.h>

extern FILE *specp, *textp;

/* This should exist somewhere. It should be first in the file... */

char *itoa(num)
{
	static char buf[32];
	int p;

	p = 31;
	do {
		buf[p] = (char) num % 10 - '\0';
		num /= 10;
		p--;
	} while (num);
	return (buf + p);
}

/* The hashing function. Note that hashtables have to be of size 256. 
 * They probably don't need to be very big, and the fn is simple...
 */

phash(string)
char *string;
{
	unsigned char rv = 0;
	while (*string) 
		rv += *(string++);
	return (rv);
}


/* This function reads a C function from the input file. It is kept in
 * the file pointed to by outp. It takes the function number as an argument
 * (the function will not have arguments). Note that this will be messed
 * up by unbalanced brackets in comments.
 */ 

transcribe(num)
int num;
{

	/* Note that we must add the first bracket... */

	extern FILE *textp;
	extern char input();
	char c;
	int brct;

	fprintf(textp, "\n_kkFunc%d ()\n{\n", num);
	brct = 1;
	while (c = input()) {	/* This is the lex input function. */
		if (c == '{') brct++;
		if (c == '}') brct--;
		putc(c, textp);
		if (!brct) return (0);
	}
	/* Reached EOF. */
	return (1);
}

/* "Gensym" for user supplied C code names. */

newnum()
{
	static int n = 0;

	n++;
	return (n);
}

/* "Gensym" for arcnumbers and nodenumbers. */

newnum2()
{
	static int n = 0;

	n++;
	return (n);
}

