#
#include <stdio.h>
#define RLEN	1024
#define FATAL	1
FILE *fp, *fopen();
FILE *fo, *fopen();

main(argc,argv)
int argc;
char **argv;
{
	register i,j;
	char ans[8];
	int authcnt,keycnt,errs;
	int cnt=0;
	char ref[RLEN], *dummy;
	if ((fp=fopen(*++argv,"r"))<=0)
		error("chkdb","Can't read database",FATAL);
	if ((fo=fopen("errs","w"))<=0)
		error("chkdb","Can't open error file",FATAL);
	printf("Note:  References will be printed if an error is found.\n");
	printf("       The line numbers of all the references as they are\n");
	printf("       checked will also be printed, for occasionally a\n");
	printf("       reference will be so corrupt it will crash rips.\n");
	printf("       If such is the case, edit the database and look\n");
	printf("       for errors a couple lines after the last line\n");
	printf("       printed before the crash.\n");
	printf("\nType \"go\" to continue....\b");
	scanf("%s",ans);
	while (fgets(ref,RLEN,fp) != NULL)
	{
		if (index(ref,"||")>=0) {
			printf("Check for error in:\n\n%s\n",ref);
			fprintf(fo,"%s",ref);
			++errs;
		}
		else {
			sscanf(ref,"%d|",&authcnt);
			for (j=i=0;i<=authcnt;i++)
				while (*(ref+j++) != '|');
			for (i=0;i<5;i++)
				while (*(ref+j++) != '|');
			i=0;
			while (*(ref+j) != '|')
			{
				if (*(ref+j)>='0' && *(ref+j)<='9')
					*(dummy+i++) = *(ref+j);
				j++;
			}
			*(dummy+i)='\0';
			sscanf(dummy,"%d",&keycnt);
			for(i=0;i<=keycnt;i++)
				while(*(ref+j++)!='|');
			if (*(ref+j+1)!='\0') {
				printf("Check for error in:\n\n%s\n",ref);
				fprintf(fo,"%s",ref);
				++errs;
			}
		}
		printf("%d\n",cnt++);
	}
	if (!errs) printf("\nNo errors");
	else printf("\n%d error",errs);
	if (errs>1) printf("s");
	printf(" in %s\n",*argv);
	if (errs) printf("Bad references stored in \"errs\"\n");
	fclose(fp);
	fclose(fo);
}
error(subr,mesg,type)	/*  Print the subroutine, error message, and bombout or return */
char subr[], mesg[RLEN];
int type;
{
	printf("RIPS-%s: %s\n",subr,mesg);
	if (type==FATAL) exit(0);
}
index(s,t)  /* NOTE 1 */
char *s, *t;
{
	int i,j,k;
	for (i=0;*(s+i) != '\0';i++) {
		for (j=i,k=0;*(t+k)!='\0' && *(s+j) == *(t+k);j++,k++) ;
		if(*(t+k) == '\0') return(i);
	}
	return(-1);
}
