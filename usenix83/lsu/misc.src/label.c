#
#include <stdio.h>
/*  Label.c  a quicky routine to generate mailing labels - but see mlist.c
	S. Klyce - LSU/NO
*/
#define MAXL 5
#define LLEN 32
FILE *fp, *fopen();
char name[LLEN];
char addr[MAXL][LLEN];
char ans[5];
int eflag = 0;
int goflag = 1;
main()
{
	char c;
	goflag = 1;
	printf("\n\tLABEL MAKER\n\n");
	printf("Please enter name and address lines with <return>\nMaximum line length and number of lines will be enforced silently\n\n");
	printf("                              | <-- line length\n");
	go();
	if (goflag)
	{
		printf("Your labels are stored in \"/tmp/labels\".\nAre you ready to print them? (yes) ");
		c=getchar();
		getline(ans,5);
		if (strcmp(ans,"\n\0") == 0)
		{
			system("cat /tmp/labels | lpr -inf");
			system("rm -f /tmp/labels");
		}
		else exit(0);
	}
	main();
}
go()
{
	int runflag;
	int num;
	register i,j;
	register k;
	runflag=1;
	while (runflag && !eflag)
	{
		getline(name,LLEN);
		if (strcmp(name,"\n\0") != 0)
		{
			for (i = 0;i < MAXL ;i++)
			{
				getline(addr[i],LLEN);
				if (strcmp(addr[i],"\n\0") == 0) break;
			}
			printf("\n%s",name);
			for (j=0;j<i-1,j<MAXL-1;j++)
				printf("%s",addr[j]);
			for (i=0;i<MAXL-j;i++) printf("\n");
			printf("\nIs this correct? (yes) ");
			getline(ans,5);
			if (strcmp(ans,"\n\0")==0)
			{
				printf("How many labels of this type do you want? ");
				scanf("%d",&num);
				printf("\nHold on for processing...");
				fp=fopen("/tmp/labels","w");
				for (k=0;k<num;k++)
				{
					j=0;
					fprintf(fp,"%s",name);
					while (strcmp(addr[j],"\n\0") != 0)
					{
						fprintf(fp,"%s",addr[j]);
						j++;
					}
					for (i=0;i<MAXL-j;i++) fprintf(fp,"\n");
				}
				fclose(fp);
				runflag=0;
			}
			else
			{
				goflag=0;
				break;
			}
		}
		else
		{
			runflag = 0;
			go();
		}
	}
}
getline(s,lim)
char s[];
int lim;
{
	int c;
	int i;
	i=eflag=0;
	while (--lim > 0 && (c=getchar()) != EOF && c != '\n' && c != '' && c != '\016' && c != '\005')
		/* check for backspace, ctrl n, ctrle */
		s[i++] = c;
	if (c == '' || c == '\005' || c == '\016')
	{
		eflag = 1;
		printf("\nERROR\n");
		c=getchar();
	}
	s[i++] = '\n';
	s[i] = '\0';
	return(i);
}
