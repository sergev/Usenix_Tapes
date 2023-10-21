#include <stdio.h>

main()
	{
	int slot;
	FILE *ttysfile;
	char line[128];
	register char *cp;

	if((slot=ttyslot())<=0)
		exit(1);
	ttysfile=fopen("/etc/ttys","r");
	while(slot--)
		if(fgets(line,128,ttysfile)==NULL)
			exit(1);
/*
	while((c=getc(ttysfile))!=' ' && c!='\t')
		;
	while((c=getc(ttysfile))==' ' || c=='\t')
		;
	do
		putchar(c);
		while((c=getc(ttysfile))!=' ' && c!='\t' && c!='\n');
*/
	for(cp=line; *cp!=' ' && *cp!='\t'; cp++)
		;
	while(*cp==' ' || *cp=='\t')
		cp++;
	while(*cp!=' ' && *cp!='\t' && *cp!='\n')
		putchar(*cp++);
	putchar('\n');
	}
