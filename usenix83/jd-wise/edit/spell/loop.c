#include <stdio.h>
#include <ctype.h>
char word[100];
main()
	{
	char line[512];

	while(gets(line)!=NULL){
		if(line[0]=='.') continue;	/* very crude deroff */
/*
		strcpy(saveword,word);
		if(find(1)){
			badword();
			continue;
			}
		strcpy(word,saveword);
		if(!find(0))
			badword();
*/		printf("%s\n",word);
		}
	}
