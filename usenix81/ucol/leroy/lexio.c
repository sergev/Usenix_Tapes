#include <stdio.h>

#define DEPTH	10	/* max no of included files */
#define BLEN	100	/* max no of pushed-back characters*/

char *source[DEPTH+1] = {'\0'};
char buff1[BLEN];
char *buffer[DEPTH+1] = { buff1 };
int bp[DEPTH+1] = { -1 };
int nlevel = 0;
FILE *fs[DEPTH+1] = {stdin};

int input()
{
	int c;
	extern int yydebug;
	if( bp[nlevel] >= 0 ) return( buffer[nlevel][bp[nlevel]--]);
	else if ( (c=getc(fs[nlevel])) != EOF ) {
		if(yydebug==1) fprintf(stderr,"%c",c);
		return(c);
		}
	else if (nlevel<=0) return(0);
	else {
		free(source[nlevel]);
		free(buffer[nlevel]);
		fclose( fs[nlevel--] );
		return(input());
	}
	return(-1);
}

int unput(c)
char c;
{
	if( bp[nlevel] < (BLEN-1) ) buffer[nlevel][++bp[nlevel]]=c;
	else lexerr("couldnt push any more");
	return(0);
}


int iswitch(s)
char *s;
{
	if( nlevel >= DEPTH ) lexerr("no more includes");
	else if ( (fs[++nlevel]=fopen(s,"r")) == NULL ) {
		--nlevel;
		lexerr("cant open include");
	}
	else {
		source[nlevel]=(char *)malloc((unsigned)(strlen(s)+1));
		strcpy( source[nlevel],s);
		buffer[nlevel]=(char *)malloc((unsigned)BLEN);
		bp[nlevel] = 0;
		buffer[nlevel][0]='\n';
	}
	return(0);
}

int lexerr(s)
char *s;
{
	fprintf(stderr,"leroy:lexio:%s\n",s);
	lexmark();
	exit(-1);
	return(0);
}

lexmark()
{
	fprintf(stderr,"currently reading: %s\n",
		strlen(source[nlevel])>0?source[nlevel]:"standard input");
	return(0);
}
