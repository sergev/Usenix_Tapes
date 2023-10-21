shred.l:

%{
/*
%e 2000
%p 5000
%n 1000
%k 500
%a 4000
%o 2000
*/
	/* Copyright, 1987, Marcus J Ranum */
	/* All rights reserved. This code can be distributed, modified, */
	/* or altered at will, but it or versions of it may not be sold */
	/* for profit */
	
	/* krufty hack to rip large source modules into subroutine modules */
	/* this program is only semi-intelligent and should be treated as such */

	/* April 1, 1987 Steve Swales (Lab. for Laser Energetics) */
	/* added features/options and a manual page (see this for details) */
	/* added a Makefile */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


int trigger =0;
int bracelev =0;
int incomment =0;
int infunction =0;
int ininclude =0;
int junkint =0;
char *junkp =0;
struct stat stbuf;
char jfname[512];
char jfname2[512];
char *curfile;
char *strcat();
char headerfile[512] = "_header.h";
int use_underscore = 0;
char tempcom[] = "shed_tempXXXXXX";
FILE *tempfp = 0;

%}

%%
^[a-zA-Z1-9*_ \t\n]+"("[ \t\n]*")"[ \t\n]*";" { /* declaration only*/
				printout(yytext);
			}
^[a-zA-Z1-9*_ \t\n]+"("[a-zA-Z1-9_, ]*")" { /* found a function definition ! */
				if (!bracelev && !incomment && !ininclude) {
					infunction = 1;
					(void) sprintf(jfname2, "%s", yytext);
					for (junkint = 0; junkint < strlen(jfname2); junkint++) {
						if (jfname2[junkint] == '(') {
							jfname2[junkint] = '\0';
							while (jfname2[--junkint] <= ' ')
								jfname2[junkint] = '\0';
							break;
						}
					}
					junkp = jfname2 + strlen(jfname2) - 1;
					while (junkp >= jfname2) {
						if (*junkp <= ' ' || *junkp == '*') {
							break;
						}
						--junkp;
					}
					junkp++;
					(void) sprintf(jfname, "%s%s",
					  (use_underscore) ? "_" : "", junkp);
					(void) strcat(jfname, ".c");
					if (!strncmp(jfname, curfile)) {
						fprintf(stderr, "Cant write over the file you're reading from!!\n");
						if (tempfp) {
							fclose(tempfp);
							unlink(tempcom);
							tempfp = 0;
							}
						printout(yytext);
					}
					else {
						if (tempfp) {
							fclose(tempfp);
							link(tempcom, jfname);
							unlink(tempcom);
							tempfp = 0;
						}
						fprintf(stderr, "writing %s\n", jfname);
						(void) fclose(yyout);
						if ((yyout = fopen(jfname, "a")) == NULL) {
							perror("shred");
							unlink(tempcom);
							exit(9);
						}
						fprintf(yyout, "\n#include \"%s\"\n\n", headerfile);
						fprintf(yyout, "%s", yytext);
					}
				}

				else {
					printout(yytext);
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
					fprintf(stderr,"too many \"}\" !  unmatched \"{\"\n");
					exit(1);
				}
				/* if we're not in braces, anything goes to header file */
				if(!bracelev) {
					fprintf(yyout,"\n");
					(void)fclose(yyout);
					infunction = 0;
					if((yyout = fopen(headerfile,"a")) ==NULL) {
						perror("shred");
						exit(9);
					}
				}
		}
\*\/		{
				printout(yytext);
				if(incomment)
					incomment--;
			}
\/\*		{
				incomment++;
				if(bracelev == 0) {
					if(!tempfp)
						tempfp = fopen(tempcom,"w");
				}
				printout(yytext);
			}
^[]*"#"		{
				ininclude++;
				/* flush includes */
				fprintf(yyout,"%s",yytext);
			}
\n			{
				ininclude =0;
				printout(yytext);
			}
.			{printout(yytext); } /* default action */
%%

/* beginning of MAIN */
main(argc,argv)
int argc;
char *argv[];
{
	int index =0;
	char    *s;                     /* used by command line parser */

/* 
 *      Parse command line for options  
 */
        while ( --argc > 0 && ( *++argv) [0] == '-' )
                for( s = argv[0]+1; *s != '\0'; s++)
                        switch(*s) {
						case 'u':
							use_underscore = 1;
							break;
						case 'h':
							strcpy(headerfile,*++argv);
							--argc;
							break;
						default:
							fprintf(stderr,"'-%c' is not a legal option\n",*s);
							usage();
							break;
						}

		if(argc == 0)
			usage();
		mktemp(tempcom);

	if(!stat(headerfile,&stbuf)) {
printf("%d braces\n",bracelev);
		fprintf(stderr,"will not overwrite existing %s\n",headerfile);
		exit(1);
	} else {
		if((yyout = fopen(headerfile,"w")) ==NULL) {
			perror("shred");
			exit(9);
		}
	}

	for(index = 0; index <argc; index++) {
		if((yyin = fopen(argv[index],"r")) ==NULL) {
			perror("shred");
			unlink(tempcom);
			exit(1);
		} else { 
			curfile = argv[index];
			yylex();
		}
	}
	unlink(tempcom);
}

usage()
{
	fprintf(stderr,"Usage: shred [-u] [-h <header file>] <file.c>...\n");
	fprintf(stderr,"	-u - prefix subroutine file names with underscore\n");
	fprintf(stderr,"	-h <header file> - use this name for include file.\n");
	exit(1);
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


printout(yytext)
char *yytext;
{
				fprintf(yyout,"%s",yytext);
				if(bracelev == 0 && !infunction && incomment) {
					if(tempfp)
                        fprintf(tempfp,"%s",yytext);
				}
}
