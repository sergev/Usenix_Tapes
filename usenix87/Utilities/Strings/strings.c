#include <stdio.h>
#define	MINLEN	3
#define	MAXLEN	64

char	instr[65];
int	strpos;
int	filed;

main(argc,argv)
int argc;
char *argv[];
{
	int i;
	if(argc<2) {
		fprintf(stderr,"usage: strings file [file...]\n");
		exit(1);
	}
	for(i=1; i<argc; i++) {
		filed = open(argv[i],0);
		if(filed<0) {
			fprintf(stderr,"strings: file %s error ",argv[i]);
			perror();
			continue;
		}
		strings();
		close(filed);
	}
	exit(0);
}

strings()
{
	char c;
	strpos = 0;
	while(read(filed,&c,1)==1) {
		if(c!=NULL && (c<' ' || c>'~') && c!='\n' && c!='\t') {
			strpos = 0;
			continue;
		}
		if(c==NULL || c=='\n') {
			if(strpos>=MINLEN) {
				instr[strpos++] = '\0';
				printf("%s\n",instr);
			}
			strpos = 0;
			continue;
		}
		instr[strpos++] = c;
		if(strpos>MAXLEN) {
			printf("%s\n",instr);
			strpos = 0;
		}
	}
}
