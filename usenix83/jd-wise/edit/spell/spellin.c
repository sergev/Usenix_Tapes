#include "dict.h"
#include <stdio.h>
/* add entries to hash table for use by spell
   preexisting hash table is first argument
   words to be added are standard input
   if no hash table is given, create one from scratch
*/
int traceflag;

main(argc,argv)
char **argv;
{
	register i, j;
	long h;
	register long *lp;
	char word[NW];
	register char *wp;
	char *dfile=0, *sfile=0;
	int tabno=0;
	short *table;
	long *p;
	unsigned size;

	if(--argc && **++argv=='-')
		switch(*++*argv){
		case 'd':
			if(--argc)
				dfile=argv[1];
			tabno=0;
			break;
		case 's':
			if(--argc)
				sfile=argv[1];
			tabno=1;
			break;
		case 't':
			traceflag++;
			if(--argc)
				sfile=argv[1];
			tabno=1;
			break;
		default:
			fprintf(stderr,"bad flag %c\n",**argv);
			exit(1);
			}
	p=P[tabno];
	table=tab[tabno];
	size=tabno?STOPSIZE:DICTSIZE;
if(traceflag){
fprintf(stderr,"table=%o stoptab=%o size=%d p[0]=%D\n",
	table,stoptab,size,p[0]);
}

	if(!prime(dfile,sfile)) {
		fprintf(stderr,
		    "spellin: cannot initialize hash table\n");
		exit(1);
		}
if(traceflag){
fprintf(stderr,"after prime table=%o stoptab=%o size=%d p[0]=%D\n",
	table,stoptab,size,p[0]);
}
	while (fgets(word, sizeof(word), stdin)) {
		for (i=0; i<NP; i++) {
			for (wp = word, h = 0, lp = POW2[tabno][i];
				 (j = *wp) != '\0'; ++wp, ++lp)
				h += j * *lp;
			h %= p[i];
if(traceflag)
fprintf(stderr,"i=%d p[i]=%D *lp=%D h=%D\n",i,p[i],*lp,h);
			set(table,h);
			}
		}
	if (fwrite(table, sizeof(short), size, stdout) != size) {
		fprintf(stderr,
		    "spellin: trouble writing hash table\n");
		exit(1);
		}
	return(0);
	}
