%{
/************************************************************************
 *									*
 *			Copyright (c) 1985, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released into the		*
 *	public domain for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	lex.l
 *
 *  SCCS
 *
 *	@(#)lex.l	1.2	2/10/85
 *
 *  DESCRIPTION
 *
 *	Lex code for setdate utility.  This must be processed by the
 *	UNIX "lex" utility to produce a lexical analyzer.
 *
 *	Note that month names and other such strings are recognized
 *	case independently.
 *
 *	The values returned for month names are in the range [1-12]
 *	for the months [Jan-Dec] respectively.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	(Currently at UniSoft Systems Inc.)
 *
 */
%}

%%

(P|p)(M|m)	{
			return (PM); 
		}

(A|a)(M|m)	{
			return (AM); 
		}

[0-9]+		{
			yylval = atoi(yytext);
			return(DIGITS);
		}

((J|j)(A|a)(N|n))((U|u)(A|a)(R|r)(Y|y))? {
			yylval = 1; 
			return (MONTH);
		}

((F|f)(E|e)(B|b))((R|r)(U|u)(A|a)(R|r)(Y|y))? {
			yylval = 2; 
			return (MONTH);
		}

((M|m)(A|a)(R|r))((C|c)(H|h))? {
			yylval = 3; 
	 		return (MONTH);
		}

((A|a)(P|p)(R|r))((I|i)(L|l))? {
			yylval = 4; 
			return (MONTH);
		}

(M|m)(A|a)(Y|y) {
			yylval = 5;
			return(MONTH);
		}

((J|j)(U|u)(N|n))((E|e))? {
			yylval = 6;
			return(MONTH);
		}

((J|j)(U|u)(L|l))((Y|y))? {
			yylval = 7;
			return(MONTH);
		}

((A|a)(U|u)(G|g))((U|u)(S|s)(T|t))? {
			yylval = 8;
			return(MONTH);
		}

((S|s)(E|e)(P|p))((T|t)(E|e)(M|m)(B|b)(E|e)(R|r))? {
			yylval = 9;
			return(MONTH);
		}

((O|o)(C|c)(T|t))((O|o)(B|b)(E|e)(R|r))? {
			yylval = 10;
			return(MONTH);
		}

((N|n)(O|o)(V|v))((E|e)(M|m)(B|b)(E|e)(R|r))? {
			yylval = 11;
			return(MONTH);
		}

((D|d)(E|e)(C|c))((E|e)(M|m)(B|b)(E|e)(R|r))? {
			yylval = 12;
			return(MONTH);
		}

","		{
			return(COMMA);
		}

"/"		{
			return(SLASH);
		}

"-"		{
			return(DASH);
	      	}

":"		{
			return(COLON);
		}

[ \t]		{;}

[\n]		{
			return(-1);
		}
%%

