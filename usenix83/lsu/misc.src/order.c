#
#include <stdio.h>
#include <h1500.h>
	/*  h1500.h contains macros for Hazeltine 1500 control.  Make
	a similar file for your site or scratch the referenced macros	*/
#define MAXLINE 512
#define DESC_FIELD 42
#define MAXA 10
#define MAXI 50
#define PUTEQLS for (i=0;i<79;i++) fprintf(fp,"=")
/****************************************************************
 *								*
 *		ORDER.C						*
 *								*
 *	LSU Eye Center Purchase Order form generator		*
 *								*
 *	Oct. 20, 1981		S. Klyce			*
 *								*
 *		REVISION HISTORY				*
 *								*
 *	11/05/81 (klyce)	Bug in recursive use fixed by	*
 *				initializing grand_tot,		*
 *								*
 *	02/25/82 (klyce)	Fixed printout of description   *
 *				field in order_log so long  	*
 *				descriptions aren't truncated.	*
 *								*
 ****************************************************************/
FILE *fp, *fopen();
int c;
int j;
int eflag 0;
int nosubs 0;
char vendor[MAXA][MAXLINE];
char desc[MAXI][MAXLINE];
char who[MAXLINE];
char dept[] = "Ophthalmology";  /* Change to your dept name */
int quantity[MAXI];
double price[MAXI],total_price[MAXI],grand_tot;
char *file = "/tmp/order";

main(argc,argv)
int argc;
char **argv;
{
	int i;
	argv++;
begin:
	grand_tot=j=0;
	c=1;
	preamble();
	if (eflag) {
		eflag=0;
		goto begin;
	}
	while (c=getdata() != 0);
	DIM;
	printf("\tPreferred vendor =?      ");
	BRIT;
	for (i = 0;i < MAXA ; i++)
	{
		getline(vendor[i],MAXLINE);
		if (eflag)
		{
			printf("\t\t\t\t");
			eflag=i=0;
		}
		else if (strcmp(vendor[i],"\n\0") == 0) break;
		else printf("\t\t\t\t");
	}
	BRIT;
	if (argc > 1 && **argv == '-' && ((*argv)[1] == 'n' || (*argv)[2] == 'n')) nosubs=1;
	putform();
	if (argc > 1 && **argv == '-' && (*argv)[1] == 'l')
	{
		printf("Processing order ...\b");
		system("cat /tmp/order | lpr -inf");
		unlink(file);
		goto begin;	/* A goto - horrors ! */
	}
	else
	{
		printf("\nYour order is saved in /tmp/order\nA record has been stored in order_log\n");
	}
}
getdata()
{
	int i;
	DIM;
	printf("\nItem%3d:\n\tHow many =? ",1+j++);
	BRIT;
	scanf("%d",&quantity[j]);
	c=getchar();
	if (quantity[j])
	{
		DIM;
		printf("\tFULL description =? ");
		BRIT;
		getline(desc[j],MAXLINE);
		if (!eflag)
		{
			DIM;
			printf("\tEstimated unit price =? ");
			BRIT;
			scanf("%f",&price[j]);
			total_price[j] = price[j] * quantity[j];
			grand_tot += total_price[j];
		}
	}
	if (eflag) {
		eflag=j=0;
		getdata();
	}
	else return(quantity[j]);
}
putform()
{
	auto int i;
	int desc_lines;
	int k;
	int tbuf[2];
	char *t;
	time(tbuf);
	t = ctime(tbuf);
	k=0;
	fp = fopen(file,"w");
	fprintf(fp,"\n\t\t\tREQUEST FOR PURCHASE\n\n");
	fprintf(fp,"Department of %s\t\t%s\nCare of %s\n",dept,t,who);
	PUTEQLS;
	fprintf(fp,"\n|Item\tQuantity\t\tDescription\t\tEstimated  Total Est. |\n");
	fprintf(fp,"| No.\t\t\t\t\t\t\tUnit Price    Cost    |\n");
	PUTEQLS;
	fprintf(fp,"\n");
	for (i=1;i<j;i++) {
		desc_lines=0;
		squeeze(desc[i],'\n');
		if (strlen(desc[i]) > DESC_FIELD)
		{
			desc_lines = descpr(desc[i],fp,i);
			k += desc_lines;
		}
		else fprintf(fp,"|%3d |%3d | %-44s|\$%9.02f|\$%9.02f|\n",i,quantity[i],desc[i],price[i],total_price[i]);
	}
	if (nosubs)
	{
		k += 2;
		fprintf(fp,"|    |    |                                             |          |          |\n");
		fprintf(fp,"|    |    |    NO SUBSTITUTIONS PLEASE!                 |          |          |\n");
	}
	for (i=0;i<40-j-k;i++)
		fprintf(fp,"|    |    |                                             |          |          |\n");
	fprintf(fp,"|    |    |                      Total price            |          |\$%9.02f|\n",grand_tot);
	PUTEQLS;
	fprintf(fp,"\nVendor:\n");
	for (i=0;strcmp(vendor[i],"\n\0")!=0;i++) fprintf(fp,"\t%s",vendor[i]);
	PUTEQLS;
	fprintf(fp,"\nPurpose for materials requested:\n\tMedical research.\n");
	fclose(fp);
	fp = fopen("order_log","a");
	chmod("order_log",0644);
	fprintf(fp,">>>%s ",t);
	fprintf(fp,"%-15s",who);
	for (i=1;i<j;i++)
		fprintf(fp,"%3d\n%s\n%8.02f\n",quantity[i],desc[i],price[i]);
	fprintf(fp,"0\n");
	for (i=0;strcmp(vendor[i],"\n\0")!=0;i++) fprintf(fp,"%s",vendor[i]);
	fprintf(fp,"\n");
	fclose(fp);
}
descpr(s,fildes,index)
int fildes;
int index;
char s[];
{
	register i,j,k;
	int l;
	int len;
	char fs[MAXLINE];
	len=strlen(s);
	j=l=0;
	while(len>DESC_FIELD)
	{
		k=0;
		i=DESC_FIELD+j;
		while(s[--i]!=' ');
		for (;j<i;fs[k++]=s[j++]);
		len -= k;
		fs[k]='\0';
		while (s[++j]==' ');
		if (l && k) fprintf(fildes,"|    |    | %-44s|          |          |\n",fs);
		else if (k) fprintf(fp,"|%3d |%3d | %-44s|\$%9.02f|\$%9.02f|\n",index,quantity[index],fs,price[index],total_price[index]);
		l++;
	}
	if (s[j]==' ') while (s[++j]==' ');
	for (i=j,k=0;s[j]!='\0';fs[k++]=s[j++]);
	fs[k]='\0';
	fprintf(fildes,"|    |    | %-44s|          |          |\n",fs);
	return(l);
}
preamble()
{
	static int i;
	CLS;
	printf("\n\t\tLSU EYE CENTER PURCHASE ORDER PROGRAM\n\n");
	DIM;
	printf("For whom is order being made ");
	if (!i++) printf("(enter help if you need instructions) ");
	printf("=? ");
	BRIT;
	getline(who,MAXLINE);
	if (who[0] == '0') exit();
	else if (who[0] == 'h')
	{
		CLS;
		system("cat /usr/src/docs/order.1 | pg");
		preamble();
	}
	else return(i);
}
squeeze(s,c)
char s[];
int c;
{
	int i,j;
	for (i=j=0;s[i]!='\0';i++)
		if (s[i]!=c)
			s[j++]=s[i];
	s[j]='\0';
}
typ_help()
{
	int c;
	CLS;
	printf("	Sorry to interupt your session, but you have typed an innappropriate\n");
	printf("character.  Remember, typing errors can not be corrected by using the back\n");
	printf("space.  These are corrected with the shift delete key.  Also, be careful\n");
	printf("NOT to hit the CTRL key when typing capitals.  Typing CNTRL e rather than\n");
	printf("SHIFT e (or E) will always give an error message.  This checking scheme is\n");
	printf("necessary because the CNTRL characters have special meaning and often mess\n");
	printf("up the printing of labels.\n");
	printf("\n\n\n");
	printf("				bye, bye\n");
	c=getchar();
}

getline(s,lim)
char s[];
int lim;
{
	int c,i;
	char dummy[];
	static int firsttime = 0;
	i=0;
	while (--lim > 0 && (c=getchar()) != EOF && c != '\n' && c > '\037' && c < '\176')
		s[i++] = c;
	if ( c  < '\040' || c > '\175')
		if (c != '\012' && c != '\015')
		{
			eflag = 1;
			BELL;
			printf("ERROR\n");
			if (!firsttime++) typ_help();
			getline(dummy,20);
			c='\0';
			i=0;
		}
	if (c == '\n') s[i++]='\n';
	s[i] = '\0';
	return(i);
}
