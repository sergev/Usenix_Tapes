#
#include <stdio.h>
FILE *fp, *fopen();
char name[120];
char ylabel[120];
char xlabel[120];

main()
{

	printf("Please enter title of experiment =? ");
	getline(name,120);
	printf("Enter the vertical axis label =? ");
	getline(ylabel,120);
	printf("Enter the horizontal axis label =? ");
	getline(xlabel,120);
	fp = fopen("/tmp/glabel","w");
	fprintf(fp,"%s\n%s\n%s\n",name,ylabel,xlabel);
	fclose(fp);
}
getline(s,lim)
char s[];
int lim;
{
	int c,i;
	int ii,j;
	i=0;
	while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		s[i++] = c;
	s[i++]='\0';
	return(i);
}
