/* x -- print out text with funny characters escaped (ie '\006')
 *	Written at CSRG, U of Toronto by Henry Spencer.
 *	modified for version 7 by Dave Galloway
 */

#include <stdio.h>

main(argc, argv)
	int argc;
	char *argv[];
	{
	int argindex;

	if( argc == 1 ){
		dodesc(stdin, "standard input");
		exit(0);
	}

	for(argindex=1; argindex<argc; argindex++) 
		dofile(argv[argindex]);
	exit(0);
	}

dofile(name)
	char *name;
	{
	int save;
	FILE *desc;

	desc = fopen(name, "r");
	if( desc == NULL ){
		fprintf(stderr, "Can't open file %s.\n", name);
		return;
	}

	dodesc(desc, name);

	fclose(desc);
	}

dodesc(desc, name)
	register FILE *desc;
	char *name;
	{
	register int c;

	while((c=getc(desc)) != EOF)
		output(c);
	}

output(c)
	char c;
	{
	if( (c&0200) != 0 ){
		putchar('`');
		output(c&0177);
	}else if( c == '\t' ){
		putchar('>');
	}else if( c == '\b' ){
		putchar('<');
	}else if( c < ' ' && c != '\n' ){
		putchar('\\');
		putchar('0');
		putchar((c>>3)+'0');
		putchar((c&07)+'0');
	}else if( c == '\177' ){
		putchar('\\');
		putchar('1');
		putchar('7');
		putchar('7');
	}else{
		putchar(c);
	}
	}
