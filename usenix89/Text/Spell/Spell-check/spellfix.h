
/* RCS Info: $Revision: 1.4 $ on $Date: 86/04/02 10:33:39 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/spellfix.h,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include <stdio.h>
#include <ctype.h>

#define isvalid(ch)	(isupper(ch) || islower(ch) || ((ch) == '\'') || \
				isdigit(ch))

#define DICTFILE 	"/usr/dict/words"
#define BINDIR		"/usr/public/lib/spellfix"

#define SIZE		128	/* The maximum size of a word. */
#define NLETTERS	26	/* # letters in the alphabet. */
#define NOCHANCE	100000	/* No chance of a match between words. */
#define NSAVE		16	/* Keep the top NSAVE choices... */
#define NALTS		256	/* Max # of decompositions given by baseword */
#define NDICTS		8	/* How many dictionaries we can search. */

extern char *malloc();

