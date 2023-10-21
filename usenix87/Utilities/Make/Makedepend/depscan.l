%{
/* -- Scanner for depend
 *
 * $File: depscan.l  1.7  1985-03-29  14:10:37 $
 *
 *  Copyright (c) 1983  Kim Walden
 *      ENEA DATA, Sweden and    
 *      SYSLAB, Department of Computer Science,
 *      University of Stockholm, Sweden.
 *
 *	This software may be used and copied freely,
 *	provided that proper credit is given to the originator
 *	by including the above text in each copy.
 *
 *	Descr:	This scanner is part of an implementation of the
 *		algorithm described in K. Walden, "Automatic Generation
 *		of Make Dependencies",
 *      	Softw. Practice & Exper., vol. 14, no. 6, June 1984 
 *
 *		When yylex is first called, it is supposed to be in
 *		state <rules>, and 'yystring' is supposed to point
 *		to a string containing declared include rules.
 *		The input is taken from this string through a
 *		modified getc() routine.
 *
 *		The scanner recognizes rules of the form:
 *
 *		  .o: .c .p .f
 *		  .c= .y .l .q
 *
 *		where '\n' or ';' may be used as record separators.
 *
 *		The rules string is scanned until it is exhausted,
 *		(Lex considers '\0' to be EOF,) and then yywrap()
 *		will switch to state <includes>.
 *
 *		The scanner now recognizes include statements of the form:
 *
 *		  file1 < file2
 *
 *		and getc() starts reading from the Standard Input.
 *
 *
 * $Log:	depscan.l,v $
 * 
 * Revision 1.7  1985-03-29  14:10:37  kim
 * New source format
 * 
 * Revision 1.6  1985-03-08  01:18:45  kim
 * Added support for parallell transfer rules.
 * Added -1 option to makedep and depend.
 * 
 * Revision 1.5  1985-02-14  01:44:06  kim
 * Converted to RCS format
 *
 * Revision 1.4  1984-08-21  19:50:58  kim
 * Change SYSLAB report reference to SP & E
 * 
 * Revision 1.3  1983-10-31  19:50:54  kim
 * Include Copyright lines
 * 
 * Revision 1.2  1983-07-20  19:50:50  kim
 * Closer to algorithm in SYSLAB Report No. 19 (June 1983)
 * 
 * Revision 1.1  1983-02-22  19:50:47  kim
 * Initial revision
 */

# undef getc
# undef YYLMAX
# define YYLMAX 10000

%}

%START	rules includes

DOT	[.]
W	[ \t]

SUF	[!%+\-0-9?@A-Z\^_a-z~]
PATH	[!%+\-./0-9?@A-Z\^_a-z~]

	int nlfound;

%%

{W}+			{	return(SPACE);
			}
<rules>{DOT}{SUF}+	{	return(SUFF);
			}
<rules>{W}*[\n;]{W}*	{	return(RULSEP);
			}
<includes>^.+$		{	strcpy(line, yytext);
				lineno = nlfound;
				REJECT;
			}
<includes>{W}*\n{W}*	{	nlfound++;
				return(INCSEP);
			}
<includes>{PATH}+	{	return(PATH);
			}
.			{	return(yytext[0]);
			}
%%

int
yywrap()
{
	if (yystring)
	{
		yystring = NULL;
		BEGIN includes;
		nlfound = 0;
		lineno = 0;
		unput('\n');
		return(0);
	}
	else
		return(1);
}

int
getc(fp)
	FILE	*	fp;
{
	if (yystring)
		return(*yystring ? *yystring++ : 0);
	else
		return(fgetc(fp));
}
