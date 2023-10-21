/*----------------------------------------------------------------------
  PAS2C.C  Version 1.1
  Translate Pascal keywords and operators to C.
  useage:  pas2c < pascal_source  > c_source
    i.e., this is a filter program which filters out the Pascal.
  By James A Mullens <jcm@ornl-msr.arpa>	29-Jan-87

  Revisions:
    Version 1.1  17-Feb-87  Changed several keyword translations on the
    advice of James R. Van Zandt <jrv@mitre-bedford.ARPA>.  Added many
    more translations.  Added a source for function strcmpi for the
    unfortunates who don't have this case-insensitive string comparison
    in their C library.

    Dan Kegel     15 Mar 87	Made it work on Sun workstation.  Ripped out
    translations that hurt translation of a large (20,000 line) Turbo program.
----------------------------------------------------------------------*/

#include <stdio.h>	/* standard I/O */
#include <ctype.h>	/* character macros */
#include <string.h>	/* string functions */
#include "p2c.h"
#include "ktypes.h"	/* keyword type definitions */

boolean WasSemi;	/* kludge to avoid duplicating semicolons */

/* Change these translations to fit your desires, but the Pascal names must 
   be written in lower case and must be in alphabetical order.  If you include 
   a C comment in your translation output as a HINT to the programmer, write 
   it in CAPITALs, else write the comment in lower case, eh?
*/

wnode xlate[] = {
  {T_ZIP,	"and",		"&&"	},
  {T_ARRAY,	"array",	""	},	/* see parseTypeDecl */
  {T_BEGIN,	"begin",	"{"	},
  {T_ZIP,	"boolean",	"boolean"},
  {T_ZIP,	"byte",		"char"	},	/* Turbo */
  {T_CASE,	"case",		"switch"},
  {T_CONST,	"const",	"/* CONST */"},
  {T_ZIP,	"div",		"/"	},
  {T_ZIP,	"do",		")"	},
  {T_ZIP,	"downto",	";/*DOWNTO*/"},
  {T_ZIP,	"else",		"; else"},
  {T_END,	"end",		"}"	},
  {T_ZIP,	"false",	"FALSE"	},	
  {T_FILE,	"file",		""	},	/* see parseTypeDecl() */
  {T_ZIP,	"for",		"for ("	},
  {T_FORWARD,	"forward",	""	},
  {T_FUNC,	"function",	""	},	/* see parseProcedure() */
  {T_ZIP,	"if",		"if ("	},
  {T_ZIP,	"implementation", "/* private (static) section */"},
  {T_ZIP,	"input",	"stdin"	},
  {T_ZIP,	"integer",	"int"	},
  {T_ZIP,	"interface",	"/* exported symbol section */"},
  {T_ZIP,	"ioresult",	"errno"	},	/* UCSD, Turbo */
  {T_LABEL,	"label",	""	},	/* see parseLabel() */
  {T_ZIP,	"mod",		"%"	},
  {T_ZIP,	"not",		"!"	},
  {T_OF,	"of",		""	},	/* see parseTypeDecl() */
  {T_ZIP,	"or",		"||"	},
  {T_ZIP,	"output",	"stdout"},
  {T_ZIP,	"packed",	"/* PACKED */"},
  {T_PROC,	"procedure",	"void"	},	/* see parseProcedure() */
  {T_ZIP,	"program",	"main"	},
  {T_ZIP,	"read",		"scanf"	},
  {T_ZIP,	"readln",	"/*LINE*/scanf"},/* hint - read end-of-line */
  {T_ZIP,	"real",		"double"},	/* or "float" */
  {T_RECORD,	"record",	""	},	/* see parseTypeDecl() */
  {T_ZIP,	"repeat",	"do {"	},
  {T_STRINGTYPE,"string",	""	},	/* UCSD, Turbo string type */
  {T_ZIP,	"text",		"FILE *"},	/* UCSD, Turbo file type */
  {T_ZIP,	"then",		")"	},
  {T_ZIP,	"to",		";"	},
  {T_ZIP,	"true",		"TRUE"	},	
  {T_TYPE,	"type",		""	},	/* see parseType() */
  {T_ZIP,	"until",	"} until ("},
  {T_ZIP,	"uses",		"/* USES */\n#include"},
  {T_VAR,	"var",		"/* VAR */"},	/* see parseProc, parseVar() */
  {T_ZIP,	"while",	"while ("},
  {T_ZIP,	"with",		"/* WITH */"},	/*hint-set pointer to struct*/
  {T_ZIP,	"write",	"printf"},
  {T_ZIP,	"writeln",	"/*LINE*/printf"},/* hint - write newline */
  {T_ZIP,	"",		""	}	/* marks end of xlate table */
};

wnode theend = {T_ZIP,	"", ""};

wnode *hash[26];		/* quick index into the translation array */

/* Fill in the quick index ("hash") array 
*/
void init_hash()
{
    int ch, cmp;
    wnode *nptr = xlate;

    for (ch='a'; ch<='z'; ch++) {
	while (nptr->pname[0] && (cmp = ch - *nptr->pname) > 0) 
	    nptr++;
	hash[ch-'a'] = (cmp==0) ? nptr : &theend;
    }
}


/* compare two strings without regard to case,
   the equivalent of this function may already be in your C library 
   Used to fail on Suns because it used tolower on lowercase chars...
   Assumes second argument already lowercase.
*/
int strcmpi(s1,s2)
    register char *s1, *s2;
{ 
    register char c1;

    while ((c1= *s1++) && *s2) {	/* get char, advance ptr */
	if (isupper(c1)) c1 = tolower(c1);
	if (c1 != *s2) break;
	s2++;
    }
    return(c1 - *s2);
}


/* Pass an identifier through the translation table; return its
   keyword type.  Translated keyword left in same buffer.
*/
int
translate(word)
    register char *word;
{ 
    register wnode *xptr;
    int nomatch;
    int c;

    c = *word;
    if (isalpha(c)) {
	if (isupper(c)) c=tolower(c);
	xptr = hash[c - 'a'];
	while ( xptr->pname[0] && (nomatch = strcmpi(word,xptr->pname)) > 0 ) 
	    xptr++;
	if (!nomatch) {
	    word[0]=0;
	    if (!WasSemi && xptr->ktype == T_END)
		strcpy(word, ";");
	    strcat(word, xptr->cname);
	    return(xptr->ktype);
	}
    }
    return(T_ZIP);
}

#define Q_NOQUOTE  1
#define Q_ONEQUOTE 2
#define Q_DONE     3
#define Q_ERR      4

#define Q_C_ESCAPES  FALSE   /* Set true if your Pascal knows backslashes */

/*---- parseQuotedString -------------------------------------------------
Accepts Pascal quoted string from stdin, converts to C quoted string, and 
places in buf.
Examples:
  'hi' -> "hi"    'hi''' -> "hi'"  'hi''''' -> "hi''"
  ''   -> ""      ''''   -> "'"    ''''''   -> "''"
  ''hi' -> ERROR  '''hi' -> "'hi"  '''''hi' -> "''hi"
  'I''m'  -> "I'm"
Double quotes and backslashes are preceded with backslashes, except that
if Q_C_ESCAPES is TRUE, backslashes are left naked.
--------------------------------------------------------------------------*/
void
parseQuotedString(buf)
char *buf;
{
    register char c;
    register char *letter=buf;
    int qstate;

    *letter++ = '"';
    qstate = Q_NOQUOTE;
    while (qstate < Q_DONE) {
	switch (c=getchar()) {
	case '\'':
	    switch (qstate) {
	    case Q_NOQUOTE:  
		qstate = Q_ONEQUOTE; break;
	    case Q_ONEQUOTE: 
		*letter++ = c; qstate = Q_NOQUOTE; break;
	    }
	    break;
	case EOF:
	case '\n':
	    qstate= (qstate==Q_ONEQUOTE) ? Q_DONE : Q_ERR;
	    ungetc(c,stdin);
	    break;
	default:
	    switch (qstate) {
	    case Q_ONEQUOTE: 
		ungetc(c,stdin); qstate = Q_DONE; break;
	    case Q_NOQUOTE:
		if (c=='\\' && !Q_C_ESCAPES) *letter++ = c;
		if (c=='"') *letter++ = '\\';
		*letter++ = c; 
		break; 
	    }
	}
    }
    *letter++ = '"';
    *letter++ = '\0';
    if (qstate == Q_ERR) {
	fprintf(stderr,"Newline in string constant: %s\n",buf);
	fprintf(stdout," %c*** \\n IN STRING ***%c ",
	    '/', buf, '/');
    }
}

void
getTok()
{
    register char *letter = cTok.str;
    register char *sEnd = letter + MAXTOKLEN-3;
    register int c;

    c = getchar();
    if (isalnum(c)) {
	while (c != EOF && isalnum(c)) {
	    *letter++ = c;
	    c = getchar();
	}
	ungetc(c,stdin);
	*letter++ = 0;
	cTok.kind = translate(cTok.str);
    } else {
	switch(c) {
	case '\n':	/* newline */
	case 0x20:	/* space */
	case 0x9:	/* tab */
	    do		/* Gather a string of blank space into one token */
		*letter++ = c;
	    while ((c=getchar()) != EOF && isspace(c));
	    ungetc(c, stdin);
	    *letter++ = '\0';
	    cTok.kind = T_SPACE;
	    break;
	case '\'': 				/* Quoted String */
	    parseQuotedString(cTok.str);
	    cTok.kind = T_STRING;
	    break;
	case '{' : 				/* Curly Comment */
	    *letter++='/'; 
	    *letter++='*';
	    while ((c=getchar()) != EOF && c!='}' && letter!=sEnd)
		*letter++ = c;
	    if (letter == sEnd) {
		printf("/***ERROR: Comment too long (sorry) ***/");
		while ((c=getchar()) != EOF && c!='}')
		    ;
	    }
	    strcpy(letter, "*/");
	    cTok.kind = T_COMMENT;
	    break;
	case '(' : 
	    if ((c=getchar())!='*') {		/* Parenthesis */
		ungetc(c,stdin);
		strcpy(letter, "(");
		cTok.kind = T_LPAREN;
	    } else {
		register int lastc = '\0';	/* (* Comment *) */
		*letter++='/'; 
		*letter++='*';
		while ((c=getchar())!=EOF && !(c==')' && lastc == '*') && 
		    letter != sEnd) {
		    lastc = c;
		    *letter++ = c;
		}
		if (letter == sEnd) {
		    printf("/***ERROR: Comment too long (sorry) ***/");
		    while ((c=getchar())!=EOF && !(c==')' && lastc == '*'))
			lastc = c;
		}
		strcpy(letter, "/");		/* * already there! */
		cTok.kind = T_COMMENT;
	    }
	    break;
	case ')' :
	    strcpy(letter, ")");
	    cTok.kind = T_RPAREN;
	    break;
	case ':' : 
	    if ((c=getchar())=='=') {		/* Assignment */
		strcpy(letter, "=");
		cTok.kind = T_ASSIGN;
	    } else {				/* Colon */
		ungetc(c,stdin);
		strcpy(letter, ":");
		cTok.kind = T_COLON;
	    }
	    break;
	case '=':
	    strcpy(letter, "==");		/* Might be equality test...*/
	    cTok.kind = T_EQUALS;		/* depends on parse state */
	    break;
	case '<' : 
	    switch (c=getchar()) {
	    case '>':  
		strcpy(letter, "!=");
		break;
	    case '=':  
		strcpy(letter, "<=");
		break;
	    default :  
		ungetc(c,stdin);
		strcpy(letter,"<");
	    }
	    cTok.kind = T_COMPARE;
	    break;
	case '>' : 
	    if ((c=getchar()) == '=')
		strcpy(letter, ">=");
	    else {
		ungetc(c,stdin);
		strcpy(letter, ">");
	    }
	    cTok.kind = T_COMPARE;
	    break;
	case '^' :
	    if ((c=getchar()) == '.') {	/* perhaps we should skip blanks? */
		strcpy(letter, "->");
		cTok.kind = T_STRUCTMEMBER;
	    } else {
		ungetc(c,stdin);
		strcpy(letter, "[0]");	/* '*' would have to go in front */
		cTok.kind = T_DEREF;
	    }
	    break;
	case '$' :			/* Turbo Pascal extension */ 
	    strcpy(letter, "0x");
	    cTok.kind = T_ZIP;
	    break;
	case ';' : 			/* Semicolon- translation depends on */
	    strcpy(letter, ";");	/* parse state... */
	    cTok.kind = T_SEMI;
	    break;
	case '.':
	    if ((c=getchar()) == '.') {
		cTok.kind = T_RANGE;
		letter[0]=0;
	    } else {
		ungetc(c,stdin);
		strcpy(letter, ".");
		cTok.kind = T_ZIP;
	    }
	    break;
	case '[':
	    *letter++ = c; *letter = '\0';
	    cTok.kind = T_LBRACKET;
	    break;
	case ']':
	    *letter++ = c; *letter = '\0';
	    cTok.kind = T_RBRACKET;
	    break;
	case ',':
	    *letter++ = c; *letter = '\0';
	    cTok.kind = T_COMMA;
	    break;
	case EOF:			/* end of file */
	    cTok.kind = T_EOF;
	    break;
	default: 
	    *letter++ = c;		/* Pass unknown chars thru as tokens */
	    *letter = '\0';
	    cTok.kind = T_ZIP;
	}
    }
}

main(argc, argv)
int argc;
char **argv;
{
    int debug;
    
    debug = (argc > 1);
    init_hash();
    WasSemi = FALSE;

    getTok(); 
    do {
	switch(cTok.kind) {
	case T_VAR:
	    parseVar();
	    break;
	case T_PROC:
	case T_FUNC:
	    parseProcedure();
	    break;
	case T_LABEL:
	    parseLabel();
	    break;
	case T_TYPE:
	    parseType();
	    break;
	default:
	    if (debug)
		printf("'%s' %d\n", cTok.str, cTok.kind);
	    else {	/* fancy stuff to avoid duplicating semicolons */
		if (cTok.kind != T_SEMI || !WasSemi)
		    fputs(cTok.str, stdout);
		if (cTok.kind != T_SPACE && cTok.kind != T_COMMENT)
		    WasSemi = (cTok.kind == T_SEMI);
	    }
	    getTok();
	}
    } while (cTok.kind != T_EOF);
}

