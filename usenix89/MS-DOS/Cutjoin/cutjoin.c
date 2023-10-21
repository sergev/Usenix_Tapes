#include <stdio.h> /* cut.c  */
long i,j,k,l;
int c,c1,c2;
char str1[55],str2[55],str3[55];
FILE *f1,*f2,*f3;
main(argc,argv)
int argc;
char *argv[];
{
	i = 0;
	j = 2;
	f1 = fopen(argv[1],"rb");
	f2 = fopen("part1","wb");
	while((c = fgetc(f1)) != EOF)
	{
		if(i++ >= 350000)
		{
			i = 0;
			fclose(f2);
			strcpy(str1,"part");
			sprintf(str2,"%d",j++);
			strcat(str1,str2);
			f2= fopen(str1,"wb");
		}
		fputc(c,f2);
	}
}

#include <stdio.h>  /* join.c */
long i,j,k,l;
int ii,jj,kk;
int c,c1,c2;
char str1[55],str2[55],str3[55];
FILE *f1,*f2,*f3;
main(argc,argv)
int argc;
char *argv[];
{
	j = 1;
	k = 0;
	f2 = fopen(argv[1],"wb");
	ii = atoi(argv[2]);
	for(i=0;i<ii;i++)
	{
		strcpy(str1,"part");
		sprintf(str2,"%d",j++);
		strcat(str1,str2);
		if(k == 1)fclose(f1);
		k = 1;
		f1 = fopen(str1,"rb");
		while((c = fgetc(f1)) != EOF)fputc(c,f2);
	}
}
