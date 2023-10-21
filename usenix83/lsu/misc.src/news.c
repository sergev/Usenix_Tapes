#include <stdio.h>
#define MAXLINE 128
#define FATAL 1
#define NONFATAL 0

/****************************************************************
 *								*
 *			NEWS.C					*
 *								*
 *	11/1/82				S. Klyce		*
 *								*
 *	News is a local LSU utility that reads pieces of the    *
 *  news file (kept in /etc/news).  Generally, this keeps the	*
 *  user up to date on all the local changes and additions to	*
 *  the system.  A standard header is used for the file:	*
 *								*
 *    >>>  Mmm dd, yyyy (creator) title --> headline		*
 *								*
 *    USE: ..................					*
 *								*
 *    DESCRIPTION: ..........					*
 *    								*
 *    BUGS: .................					*
 *								*
 *  Simple, but nice....					*
 *								*
 ****************************************************************/

FILE *fi, *fopen();
int p 0,q 0;
main(argc,argv)
int argc;
char **argv;
{
	char line[MAXLINE];
	if ((fi=fopen("/etc/news","r"))<=0)
		err("Can't open news file",FATAL);
	*++argv;
	if (argc==1) {
		q++;
		while(fgets(line,MAXLINE,fi)!=NULL)
			if (index(line,">>>",0)>=0)
				printf("%s",line);
	}
	else while(fgets(line,MAXLINE,fi)!=NULL) {
		if (index(line,">>>",0)>=0) {
			if (index(line,*argv,0)>=0) {
				p++;
				q++;
			}
			else p=0;
		}
		if (p) printf("%s",line);
/*
		else if(!p && q) err("",FATAL);
*/
	}
	if (!q) printf("No news item on %s\n",*argv);
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
err(mesg,type)
char *mesg;
int type;
{
	printf("%s",mesg);
	if (type==FATAL) exit(0);
	return(0);
}
