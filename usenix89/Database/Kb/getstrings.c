
#define SIZE	128
#include <stdio.h>
#include <ctype.h>

main()
{ int ch;

  /* skip to next word */
  nextword: ch = getc(stdin);
  	    if ( EOF == ch ) exit(0);
	    ch = tolower(ch);
	    if ( isalnum(ch) || (ch == '_') ) {	/* 0-9 a-z  _ */
     	      putchar(ch);
	      goto word;
	    } else {
	      goto nextword;
	    }
  word:    ch = getc(stdin);
  	   if ( EOF == ch ) exit(0);
	   ch = tolower(ch);
	   if ( isalnum(ch) || (ch == '_') ) {	/* 0-9 a-z  _ */
     	     putchar(ch);
	     goto word;
	   } else {
	     putchar(' ');
	     goto nextword;
	   }
}/*main*/
