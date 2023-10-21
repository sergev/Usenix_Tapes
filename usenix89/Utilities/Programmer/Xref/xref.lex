
	/*******************************************************\
	*							*
	*	X_ref for YACC - LEX file			*
	*	~~~~~~~~~~~~~~~~~~~~~~~~~			*
	*							*
	*	Date: Sat Sep 13 19:13:12 BST 1986		*
	*	Cathy Taylor,					*
	*	c/o Department of Computing,			*
	*	University of Lancaster,			*
	*	Bailrigg, Lancaster, England.			*
	*							*
	\*******************************************************/

%{
# include	<stdio.h>
# include	"xref.line.h"
# define	TRUE 1
# define 	FALSE 0

int	recognised,depth,quoted;
char	c,oldc;
%}

	/* abbreviations */

digit		[0-9]
u_case		[A-Z]
l_case		[a-z]
id_char	[A-Za-z0-9_.]
letter		[A-Za-z]
white		[\t ]

%%

"/*"			{
				ECHO;
				recognised = FALSE;
				c = nextc();
				while (recognised == FALSE)
				{
					while (c != '*')
						c = nextc();
					c = nextc();
					if (c == '\/')
						recognised = TRUE;
				}
			}
"%{"			{
				ECHO;
				recognised = FALSE;
				c = nextc();
				while (recognised == FALSE)
				{
					while (c != '\%')
						c = nextc();
					c = nextc();
					if (c == '\}')
						recognised = TRUE;
				}
				return(PERCURL);
			}
"{"			{

/*
*	This definition handles nested {} and braces inside print statements.
*/

				ECHO;
				depth=1;
				oldc = '\0';
				quoted = FALSE;
				while (depth != 0)
				{
				    c = nextc();
				    while (c != '\}')
				    {
					if (( c == '\{' ) && (quoted == FALSE) && (oldc != '\\'))
					    depth++;
					if ((c == '\"') && (oldc != '\\'))
					   quoted = (quoted+1) % 2;
					oldc = c;
					c = nextc();
				    }
				    oldc = c;
				    if ((quoted == FALSE) && (oldc != '\\'))
					depth--;
				}
				return(ACT);
			}
{letter}{id_char}*	{
				ECHO;
				return(IDENTIFIER);
			}
"'"\\?[^']+"'"		{
				ECHO;
				return(CHARACTER);
			}
{white}+		{	
				ECHO;
			}
{digit}+		{
				ECHO;
				return(NUMBER);
			}
"%"{white}*"left"	{
				ECHO;
				return(LEFT);
			}
"%"{white}*"right"	{
				ECHO;
				return(RIGHT);
			}
"%"{white}*"nonassoc"	{
				ECHO;
				return(NONASSOC);
			}
"%"{white}*"token"	{
				ECHO;
				return(TOKEN);
			}
"%"{white}*"prec"	{
				ECHO;
				return(PREC);
			}
"%"{white}*"type"	{
				ECHO;
				return(TYPE);
			}
"%"{white}*"start"	{
				ECHO;
				return(START);
			}
"%"{white}*"union"	{
				ECHO;
				return(UNION);
			}
"%%"			{
				ECHO;
				return(PER);
			}
":"			{
				ECHO;
				return(COLON);
			}
";"			{
				ECHO;
				return(SEMICOLON);
			}
","			{
				ECHO;
				return(COMMA);
			}
"|"			{
				ECHO;
				return(OR);
			}
"<"			{
				ECHO;
				return(LESS);
			}
">"			{
				ECHO;
				return(GREATER);
			}
"\n"			{
				ECHO;
				nline(++line);
			}

%%

yywrap()
{
	/* wrap-up procedure */
	return(1);
}

nextc()
{
	char	c;
	
	c = input();
	printf("%c",c);
	if (c == '\n')
		nline(++line);
	return(c);
}
