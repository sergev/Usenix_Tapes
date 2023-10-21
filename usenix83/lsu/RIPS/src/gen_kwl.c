#include <stdio.h>

#define	RLEN		1024
#define	MAXKEYS		100
#define KLEN		64
#define FATAL		7
#define NONFATAL	8
FILE *fp, *fopen();
FILE *fq, *fopen();

char *mesg[] = {

	"Huh ?",
	"database busy",
	"Can't open input file",
	"format error",
	"Can't create output file",
	"system error",
	"Permission to alter file denied",
	"syntax error",
	"Correct ? (yes) ",
	"working ...\b",
	"Instructions ? (no) ",
	"******************************\n",
	"*** REPRINT REQUEST ",
	" ***\n",
	"*** LSU/LIONS EYE RESEARCH"
};
int debug 0;
main(argc,argv)
int argc; char **argv;
{
	char keytab[MAXKEYS][KLEN];
	char ref[RLEN];
	int auth_cnt;
	char s[128];
	char c;
	char sref[RLEN];
	int key_cnt,i,j,k,ii,err=0;
	int tw_cnt;
	printf("%s",*(mesg+9));
	*++argv;
	if ((fp=fopen(*argv,"r"))<=0)
		error("gen_kwl",2,FATAL);
	*++argv;
	sprintf(s,"/usr/bin/kwsort %s&",*argv);
	if ((fq=fopen(*argv,"w"))<=0)
		error("gen_kwl",4,FATAL);
	if (**++argv=='1') debug = 1;
	while (fgets(ref,RLEN,fp) != NULL)
	{
		sscanf(ref,"%d|",&auth_cnt);
		slowcpy(sref,ref);
		j=3;
		for (i=0;i<auth_cnt;i++)
		{
			k=0;
			while (sref[j++]!='|') keytab[i][k++]=sref[j-1];
			keytab[i][k++]='\n';
			keytab[i][k]='\0';
		}
		while (sref[j++]!='|');
		tw_cnt=0;
		while (sref[j]!='|')
		{
			k=0;
			while (sref[j++]!=' ')
			{
				if (issalpha(sref[j-1]))
					keytab[i][k++]=sref[j-1];
				if (sref[j]=='|') break;
			}
			keytab[i][k++]='\n';
			keytab[i++][k]='\0';
			tw_cnt++;
		}
		k++;
		i--;
		keytab[i][k++]='\n';
		keytab[i++][k]='\0';
		for (ii=0;ii<4;ii++) while (sref[j++]!='|');
		j++;
		key_cnt= sref[j] - '0';
		j += 2;
		for (;i<key_cnt+auth_cnt+tw_cnt;i++)
		{
			k=0;
			while(sref[j++]!='|')
				if (issalpha(sref[j-1]))
					keytab[i][k++]=sref[j-1];
			keytab[i][k++]='\n';
			keytab[i][k]='\0';
		}
		for (i=auth_cnt;i<key_cnt+auth_cnt+tw_cnt;i++)
		{
			fprintf(fq,"%s",keytab[i]);
			if (debug) printf("%s",keytab[i]);
			if (index(keytab[i],"fefe")>=0 || index(keytab[i],"fezfez")>=0) err=1;
				/*  Generally if incorrect fields are present */
				/*  in a reference the garbage out here has   */
				/*  mysterious strings like fefe fezfez aaamp */
		}
		if (debug) printf("%s\n",ref);
		 if (err) {
			printf("Check possible error in\n%s\n",ref);
			err=0;
		}
	}
	fclose(fp);
	fclose(fq);
	system(s);
	printf("\nRegenerating keyword list...\n");
	exit();
}
issalpha(c)  /* return true for lower case or 0-9, false for all else */
char c;
{
	if ((c < '0' || c > '9') && (c < 97 || c > 122)) return(0);
	return(1);
}
index(s,t)
char s[RLEN], t[RLEN];
{
	int i,j,k;
	for (i=0;*(s+i) != '\0';i++) {
		for (j=i,k=0;*(t+k)!='\0' && *(s+j) == *(t+k);j++,k++) ;
		if(*(t+k) == '\0') return(i);
	}
	return(-1);
}
slowcpy(s,t)  /* copy lower case t to s  */
char *s, *t;
{
	while(*s++ = lower(*t++));
}
error(s,m,type)	/* Print the subroutine, error message and bombout or return */
int  m, type;
char s[64];
{
	printf("RIPS-%s: %s\n",s,*(mesg+m));
	if (type==FATAL) exit(0);
}
lower(c)	/* Local version to convert to lower case.  Could be	*/
		/* site dependent!					*/
int c;
{
	if (c>=65 && c<=90)
		return(c+32);
	else return (c);
}
