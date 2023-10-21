#include "stdio.h"

/* rnd.c :
	written by Marcus J Ranum.
	All rights reserved. You may freely copy, modify, distribute,
	and delete this program, but any use that will cause you to 
	gain money is prohibited unless we work out a deal, or you
	ask me nicely. Any nifty ideas/modifications, please let me
	know.
		-mjr

	New Features added by:  Dale Salyer
	       E-Mail Address:	ihnp4!druhi!des

	CHG1) Default of # of iterations.
	CHG2) Rolling of dice takes best of 3 tries.
	CHG3) Nesting of subtables.
	CHG4) Alternate format of stat command.

*/

main(argc,argv)
int argc;
char *argv[];
{
	int a;
	char seed[100];
	char word[200];
	int	iterations;

	/* If you can't do this, change back to old way */
	long t;
	time(&t);
	srand(getpid() + (int)((t>>16) + t));

	/* CHG1) Default is 10 iterations if C/R pressed */

	fprintf(stderr,"How many runs do you want[10]?");
	iterations=atoi(gets(word));
	if( iterations == 0 ) iterations = 10;

	while(iterations--) {
		for(a=1 ; a < argc; a++) {
			dotable(argv[a]);
		}
	}
}

/* roll a <sides> sided die <num> times [best of three tries] */

roll(num,sides)
int num;
int sides;
{
	int a=0, m=0;
	int i, j;
	for(i=1,j=num;i<=3;i++) {
	    while (j-- >0)
		a += rnd(sides);
	/* CHG2) Rolling of dice takes best of 3 tries. */
	    if(a > m) m = a;
	}
	return(m);
}

/* roll a <die> sided die once */

rnd(die)
register die;
{
	return(((rand()>>3) % die)+1);
}

/* reads a table. calls doparse for every line in the table.
	note, if doparse hits another table name, this will
	be recursively called. max recursions seems to be
	around 10....
*/

dotable(table)
char table[];
{
	FILE *file;
	char junk[BUFSIZ];

	if((file = fopen(table,"r")) ==NULL) {
		fprintf(stderr,"can't open %s\n",table);
		exit(1);
	}

	while((fgets(junk,BUFSIZ-3,file)) !=NULL){
		doparse(junk);
	}
	if(fclose(file) == EOF) {
		fprintf(stderr,"unusual file-close error!\n");
		exit(1);
	}
}

/* reads a subtable, and looks for a numbered entry that matches the
	given selection criterion. <stat> is the table name, <num> is
	the number of dice, and <die> is the number of sides on the
	die. the subtables must be in the form of 
<#> text entry (text string terminated with newline)
<#> table entry ...
<#> subtable entry ...
<#> stat entry ...
*/

dosubtable(stat,num,die)
char stat[];
int	num;
int	die;
{

	FILE *tbl;

	char junk[BUFSIZ];
	char word[100];
	int old;
	int new;
	int pick;
	int a;

	old=0;
	if((tbl=fopen(stat,"r")) == NULL){
		fprintf(stderr,"can't open subtable %s\n",stat);
	} else {
		pick = roll(num,die);

		while((fgets(junk,BUFSIZ-3,tbl)) != NULL){
			if(sscanf(junk,"%d",&new)) {
				if( pick > old &&  pick <= new) {
					if (sscanf(junk,"%s",word)==0) {
						fprintf(stderr, "invalid format\n");
						exit(1);
					}
					junk[strlen(junk)-1] = ' ';

				/* CHG3) Nesting of subtables.
				 *       Skip over number, then call
				 *       doparse() to re-parse the rest.
				 */
					for(a=0;junk[a] != ' ';a++);
					a++;      /* skip over leading space */
					doparse(&junk[a]);
					break;
				}
				old = new;
			}
		}
	}
	if(fclose(tbl)== EOF) {
		fprintf(stderr,"unusual file-close error\n");
		exit(1);
	}
}

/* prints a stat <stat> is statname, <num> is number of dice, and 
	<die> is how many sides the die has. */

dostat(stat,num,die)
char stat[];
int	num;
int	die;
{
	/* CHG4) Alternate format of stat command. */

	if( stat[0] == '*' ) {
		printf("%d %s", roll(num,die), &stat[1]);
	} else {
		printf("%s:  %d",stat,roll(num,die));
	}
}

/* doparse looks for the magic words "table" "subtable" "\t" or "\l"
	and does the appropriate thing if it encounters them. if it
	does not, it dumps it as text */

doparse(junk)
char junk[];
{
	char stat[200];
	char word[200];
	int die;
	int num;

	if( junk[0] == '\\') {
		switch(junk[1]) {
		case 'l':
			putchar(12);
			break;
		case 't':
			printf("\t");
			break;
		}
	} else {
		if((sscanf(junk,"%s %s %d %d",
		    word,stat,&num,&die))!= 4) {
			dotext(junk);
		} else {
			if(!strcmp(word,"subtable"))
				dosubtable(stat,num,die);
			else if(!strcmp(word,"stat"))
				dostat(stat,num,die);
			else if(!strcmp(word,"table"))
				dotable(stat);
			else
				dotext(junk);
		}
	}
}

/* dohack takes a string and dumps all of it except the FIRST word.
	this is basically useful for printing subtable entries.
	while there are more elegant ways of doing this in C,
	my Lattice compiler dies when I try it. */

dohack(string)
char string[BUFSIZ];
{
	int a = 0;
	int b = 0;

	while(b == 0)
		if(string[a++] == ' ')
			b++;
	for(b = a; b < strlen(string); b++)
		putchar(string[b]);
}

/* dotext basically dumps a text string.
	if the 2nd to last char (newline is last) is a ":" 
	it does NOT put a newline on. this is to allow you to 
	have items from subtables appear after a :	*/

dotext(string)
char string[BUFSIZ];
{
	int c;
	
	c = (strlen(string)-2);
	if (string[c] == ':')
		string[++c] = ' ';
	printf("%s",string);
}
