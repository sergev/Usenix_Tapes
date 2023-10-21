#include <ctype.h>
#include <stdio.h>

/* #define EOF -1	/**/

FILE *fdi;
FILE *fdo;

main(argc,argv)
int argc;
char **argv;
{
	char tmpname[40],line[256];
	register int i,j;

	system("rm -f /tmp/zork*.f");
	for(j = 1; j != argc; j++) {
		fdi = fopen(argv[j],"r");
		sprintf(tmpname,"/tmp/zork%d.f",time(0) & 0177777);
		fdo = fopen(tmpname,"w+");
		while(input(line) != EOF){
			switch(line[0]){
			case 'C':
			case 'D':
			case 'c':
			case 'd':
			case '!':
				fputs("\n");
				break;
			default:
				if(all_white(line))break;
				for(i=0;line[i];i++){
					switch(line[i]){
					case '!':
						line[i] = '\0';
						break;
					}
					if(line[i] == '\''){
						for(i++;line[i];i++){
							if((line[i] == '\'') && (line[i+1] != '\''))
								break;
						}
					}
				}
				for(i=strlen(line);i;i--)
					if(!isspace(line[i-1]))break;
				line[i++] = '\n';
				line[i] = '\0';
				fputs(line);
				break;
			}
		}
		close_io();
	}
	printf("Done copying.\n");
/*	system("f77 -Nn2047 -w /tmp/zork* -o zork");	/**/
/*	system("rm -f /tmp/zork*.f");	/**/
}

input(line)
char line[];
{
	register int i,c;

	i = 0;
	for(;;){
		if((c = getc(fdi)) == EOF)break;
		if(c == '\f')continue;
		line[i++] = c;
		if(c == '\n')break;
	}
	if((i == 0) && (c == EOF))return(EOF);
	line[i] = '\0';
	return(0);
}

close_io()
{
	fclose(fdo);
	fclose(fdi);
}

all_white(line)
char *line;
{
	while(*line){
		if(!isspace(*line))return(0);
		line++;
	}
	return(1);
}
