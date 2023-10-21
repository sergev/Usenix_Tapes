shred.l:
%e 2000
%p 5000
%n 1000
%k 500
%a 4000
%o 2000


%{
	/* Copyright, 1987, Marcus J Ranum */
	/* All rights reserved. This code can be distributed, modified, */
	/* or altered at will, but it or versions of it may not be sold */
	/* for profit */
	
	/* krufty hack to rip large source modules into subroutine modules */
	/* this program is only semi-intelligent and should be treated as such */

#include <sys/types.h>
#include <sys/stat.h>


int trigger =0;
int bracelev =0;
int incomment =0;
int ininclude =0;
int junkint =0;
struct stat stbuf;
char jfname[400];
char *strcat();

%}

%%
[a-zA-Z1-9 ]+"("[a-zA-Z1-9, ]*")" {
				/* found a function decl ! */
				if(!bracelev && !incomment &&!ininclude) {
					(void)sprintf(jfname,"_%s",yytext);
					for(junkint = 0 ; junkint <strlen(jfname); junkint++) {
						if(jfname[junkint] == ' ' || jfname[junkint] == '(')
							jfname[junkint]='\0';
					}
					(void)strcat(jfname,".c");
					fprintf(stderr,"writing %s\n",jfname);
					(void)fclose(yyout);
					if((yyout = fopen(jfname,"a")) ==NULL) {
						perror("shred");
						exit(9);
					}
					fprintf(yyout,"%s",yytext);
				} else {
					fprintf(yyout,"%s",yytext);
				}
			}
"{"			{
				if(!incomment)
					++bracelev;
				fprintf(yyout,"%s",yytext);
			}
"}"			{
				if(!incomment)
					--bracelev;
				fprintf(yyout,"%s",yytext);
				if(bracelev <0) {
					fprintf(yyout,"too many \"}\" !  unmatched \"{\"\n");
					exit(0);
				}
				/* if we're not in braces, anything goes to _header.h */
				if(!bracelev) {
					fprintf(yyout,"\n");
					(void)fclose(yyout);
					if((yyout = fopen("_header.h","a")) ==NULL) {
						perror("shred");
						exit(9);
					}
				}
		}
\*\/		{
				/*
				 foobar
				*/
				incomment--;
				fprintf(yyout,"%s",yytext);
			}
\/\*		{
				incomment++;
				fprintf(yyout,"%s",yytext);
			}
^[]*"#"		{
				ininclude++;
				/* flush includes */
				fprintf(yyout,"%s",yytext);
			}
\n			{
				ininclude =0;
				fprintf(yyout,"%s",yytext);
			}
.			fprintf(yyout,"%s",yytext);
%%

/* beginning of MAIN */
main(argc,argv)
int argc;
char *argv[];
{
	int index =0;
	
	if(!stat("_header.h",&stbuf)) {
printf("%d braces\n",bracelev);
		fprintf(stderr,"will not overwrite existing %s\n","_header.h");
		exit(1);
	} else {
		if((yyout = fopen("_header.h","w")) ==NULL) {
			perror("shred");
			exit(9);
		}
	}

	for(index = 1; index <argc; index++) {
		if((yyin = fopen(argv[index],"r")) ==NULL) {
			perror("shred");
			exit(1);
		} else { 
			yylex();
		}
	}
}

yywrap()
{
	/*yywrap returns a 1. It can be used to detect end of
		file, and whether to provide more input to
		yyinput.  I get rid of it like this.
		If you have some desire to frob the input,
		here is one place to do it. -mjr */
	return(1);
}



