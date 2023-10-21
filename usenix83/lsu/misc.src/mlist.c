#
#include <stdio.h>
/* Mlist.c - quick & dirty mail list label generator.
	S. Klyce - LSU/NO
*/
#define MAXL	5
#define ZLEN	7
#define LLEN	32
#define ILEN	512
FILE *fp, *fopen();
FILE *fo, *fopen();
char name[LLEN];
char addr[MAXL][LLEN];
char zip[ZLEN];
char item[ILEN];
char fname[LLEN];
char fpath[LLEN];
char *fdir "/usr/mailist/";  /* Do a mkdir /usr/mlist to set up */
char *ans;
int done 1;

main()
{
	printf("\t\t\tMAILING LIST\n\n");
	getdb();
	getmode();
}

getdb()
{
	printf("Current lists:\n");
	system("lss /usr/mailist");
	*fname='\0';
	while (*fname=='\0') {
		strinp("\nWhich list (or quit) ? ",fname,14);
		if (*fname=='q') exit(0);
		squeeze(fname,'\n');
	}
	strcpy(fpath,fdir);
	strcat(fpath,fname);
}

getmode()
{
	printf("Options:\n\n");
	printf("\ta - add names\n");
	printf("\tl - print labels\n");
	printf("\tq - exit to shell\n");
	strinp("\n? ",ans,8);
	switch (*ans) {

		case 'a':
			ad_name();
			break;

		case 'l':
			pr_label();
			break;

		case 'q':

			exit(0);
			break;

		default:
			printf("Huh?\n");
			getmode();
			break;
	}
}

ad_name()
{
	register i,j,k;
	int runflag=1;
	int zerr=0;
	if ((fp=fopen(fpath,"a"))<=0) {
		printf("Can't open mail list\n");
		main();
	}
	else {
		strinp("Instructions (no) ? ",ans,8);
		switch (*ans) {

			case 'n':
			case '\n':
				break;

			default:
				helper();
				break;
		}
		while (runflag) {
			printf("Enter name and address (d for done):\n");
			printf("Line length ");
			for (i=0;i<LLEN-6;i++) printf("-");
			printf("|");
			strinp("\nl1\t",name,LLEN);
			switch (*name) {

				case 'd':
					fclose(fp);
					runflag=0;
					break;

				default:
					for (i=0;*(addr[i-1])!='\n';i++) {
						if (i==MAXL-1)
							sprintf(addr[i],"\n");
						else {
							printf("l%1d",i+2);
							strinp("\t",addr[i],LLEN);
						}
					}
					for (k=zerr=0,i -= 2,j=strlen(addr[i])-6;k<5;k++) {
						*(zip+k) = *(addr[i]+j++);
						if (issdigit(*(zip+k))<=0) {
							strcpy(zip,"00000 ");
							squeeze(zip,'\n');
						}
					}
					*(zip+k) = '\0';
					switch (zerr) {


						case 0:
							printf("%s",name);
							for (j=0;j<=i,j<MAXL-1;j++)
								printf("%s",addr[j]);
							for (k=0;k<MAXL-j;k++)
								printf("\n");
							strinp("Is this correct (yes) ? ",ans,8);
							switch (*ans) {

								case 'y':
								case '\n':
									strcpy(item,name);
									for (j=0;j<=i;j++) {
										strcat(item,addr[j]);
									}
									replace(item,"\n",":",0,0);
									fprintf(fp,"%s %s\n",zip,item);
									break;
								default:
									break;
							}
						break;
						default:
							printf("Could not find legal zip code in %s\n",zip);
							break;
					}
				}
		}
	}
	fclose(fp);
}
/*******************************************************/
/***   STRINP.C     Feb 18, 1982    S. KLYCE         ***/
/*******************************************************/

#define MINASCII	'\037'   /**  These two lines           **/
#define MAXASCII	'\176'   /** terminal dependent.        **/

strinp(s,svar,lim)
char *s, *svar;
int lim;
{
	int c,i;
	int error = 1;
	while (error) {
		printf("%s",s);
		error=i=0;
		while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		{
			if ((i>LLEN-2 || c < MINASCII || c > MAXASCII) && c != '\n') {
				error=1;
				printf("?");
			}
			svar[i++] = c;
		}
		if (c == '\n')
			svar[i++]=c;
		svar[i] = '\0';
	}
}

/************************************************************************
 *									*
 *		REPLACE.C     03/11/82      S. Klyce			*
 *									*
 *	This is a function that will replace occurrences of an		*
 *  "old" phrase with a "new" phrase in a "s"tring.  Replacement	*
 *  continues until all "old" are substituted (n=0) or until		*
 *  n occurrences have been replaced.  NOTE that passing a HUGE 	*
 *  n will have the same effect as passing 0.				*
 *									*
 *	Passing a 1 to "quot" will cause replace to ignore expressions	*
 *  in quotations.  This is useful for parsing program command lines.	*
 *									*
 *  USE:  replace(string,old_phrase,new_phrase,how_many,quote_ignore);	*
 *									*
 ************************************************************************/

replace(s,old,new,n,quot)
char  *old, *new;
char s[ILEN];
int n,quot;
{
	register i,j,k;
	char t[ILEN];
	int ind,cnt=0;
	while ((ind=index(s,old,quot)) >= 0 && (!n || (cnt++ < n))) {

		for (i=j=k=0;i<ind;i++,j++) *(t+i) = *(s+j);
		for (j += strlen(old);k < strlen(new); i++,k++) *(t+i) = *(new+k);
		while ((*(t+i++) = *(s+j++)) != '\0');
		for (i=0;((*(s+i) = *(t+i)) != '\0');i++);
	}
}
issdigit(c)
char c;
{
	if (c < '0' || c > '9') return(0);
	return(1);
}
pr_label()
{
	register i,j;
	int cnt = 0;
	char who[128],num[8];
	char ans[8];
	if ((fp=fopen(fpath,"r"))<=0) {
		printf("Can't find mail list.\n");
		main();
	}
	else {
		if ((fo=fopen("/tmp/labels","w"))<=0) {
			printf("Can't open output file\n");
			exit(0);
		}
		while (*ans != 'w' && *ans != '1') {
			strinp("Whole list (w) or multiple copies of 1 entry (1) ? ",ans,5);
		}
		if (*ans=='1') {
			strinp("Name of person in entry =? ",who,128);
			squeeze(who,'\n');
			strinp("Number of labels needed =? ",num,8);
		}
		putdoc();
		while (fgets(item,ILEN,fp) != NULL && done) {
			replace(item,":","\n",0,0);
			for (i=0;i<6;i++)
				*(item+i) = 'x';
			replace(item,"xxxxxx","",1,0);
			for (i=cnt=0;*(item+i)!='\0';i++)
				if (*(item+i)=='\n') cnt++;
			if (*ans=='1' && index(item,who,0)>=0) {
				for (i=0;i<atoi(num);i++) {
					fprintf(fo,"%s",item);
					for (j=0;j<=MAXL-cnt;j++)
						fprintf(fo,"\n");
				}
				done=0;
			}
			else if (*ans == 'w') {
				fprintf(fo,"%s",item);
				for (i=0;i<=MAXL-cnt;i++)
					fprintf(fo,"\n");
			}
		}
		putdoc();
		fclose(fp);
		fclose(fo);
		printf("Results stored in /tmp/labels.\n");
	}
}
helper()
{
	printf("\n");
	printf("             MLIST - PROGRAM FOR MAKING LABELS\n");
	printf("\n");
	printf("     This program maintains user files  consisting  of  name\n");
	printf("and  address information for use in printing mailing labels.\n");
	printf("Two options are available: printing  labels  of  a  complete\n");
	printf("file  and  printing multiple copies of a single name and ad-\n");
	printf("dress.  Generally, one would probably have separate files to\n");
	printf("accommodate each optional task.\n");
	printf("\n");
}
/************************************************************************
 *									*
 *		INDEX.C    06/10/82     S. Klyce			*
 *									*
 *	Return the index of a string, t, in string, s.  Routine can be	*
 *  told to ignore quoted segments of a string (qt=1).			*
 *									*
 *	USE:  ivar=index(string,pattern,ignore_quotes);			*
 *									*
 ************************************************************************/
index(s,t,qt)
char *s, *t;
int qt;
{
	register i,j,k;
	int q;
	for (q=i=0;*(s+i) != '\0';i++) {
		if (*(s+i) == '\"') q++;
		if (q==2) q=0;
		if (!q || !qt) {
			for (j=i,k=0;*(t+k) != '\0' && *(s+j) == *(t+k);j++,k++);
			if (*(t+k) == '\0') return(i);
		}
	}
	if (q) return(-2); /* Use this in calling routine to check for mismatched quotation marks */
	return(-1); /* No match */
}
squeeze(s,c)  /* remove all occurrences of c in s */
char *s;
int c;
{
	int i,j;
	for (i=j=0;*(s+i)!='\0';i++)
		if (*(s+i)!=c)
			*(s+j++) = *(s+i);
	*(s+j)='\0';
}
putdoc()
{
	register i;
	fprintf(fo,"\n");
	for (i=0;i<4;i++) fprintf(fo,"%s\n",fname);
	fprintf(fo,"\n");
}
