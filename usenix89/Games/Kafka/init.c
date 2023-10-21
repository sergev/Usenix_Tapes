
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include <stdio.h>

FILE *textp, *specp, *inp;
extern char *sourcefile;

init()
{
	/* These should be only defaults here... */

	if ((textp = fopen("kaf.text.c", "w")) == NULL) {
		perror("kaf.text.c");
		exit(1);
	}
	if ((specp = fopen("kaf.spec.c", "w")) == NULL) {
		perror("kaf.spec.c");
		exit(1);
	}
	if (!sourcefile)
		inp = stdin;
	else
		if ((inp = fopen(sourcefile, "r")) == NULL) {
			perror(sourcefile);
			exit(1);
		}
	fprintf(textp, "/* The user routine file. */\n\n");
	fprintf(specp, "/* The specification file. */\n\n");
	fprintf(specp, "#include \"kafgraf.h\"\n\n");
	return (0);
}

