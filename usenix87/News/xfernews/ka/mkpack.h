#include <stdio.h>


struct pair {
      char *name ;
      char *pval ;
}
headers[[] = {
	"NHFROM",	"From: ",
	"NHPATH",	"Path: ",
	"NHSUBJECT",	"Subject: ",
	"NHDATE",	"Date: ",
	"NHPOSTED",	"Posted: ",
	"NHEXPIRES",	"Expires: ",
	"NHARTID",	"Article-I.D.: ",
	"NHMSGID",	"Message-ID: ",
	"NHREPLYTO",	"Reply-To: ",
	"NHREFERENCES",	"References: ",
	"NHCONTROL",	"Control: ",
	"NHSENDER",	"Sender: ",
	"NHFOLLOWTO",	"Followup-To: ",
	"NHVERSION",	"Posting-Version: version ",
	"NHRELAY",	"Relay-Version: version ",
	"NHDISTRIB",	"Distribution: ",
	"NHORG",	"Organization: ",
	"NHLINES",	"Lines: ",
	"NHKEYWORDS",	"Keywords: ",
	"NHAPPROVED",	"Approved: ",
	0,		0
};

char *special[] = {
	"PKEOFMARK",
	0,
};





main() {
      













#include "y.tab.h"

%%




[A-Za-z_][A-Za-z_0-9]*	{
	yylval = savestr(yytext) ;
	return WORD ;
	}

\"([^"\n]|\\.)*\"	{
	yylval = savestr(yytext) ;
	return STRING ;
	}

[01]+	{
	yylval = savestr(yytext) ;
	return BITSTRING ;
	}

[0-9]+	{
	yylval = savestr(yytext) ;
	return NUMBER ;
	}

%%

savestr(s)
	char *s ;
	{
	char *p ;

	if ((p = malloc(strlen(s) + 1)) == NULL)
		yyerror("Out of space") ;
	strcpy(p, s) ;
	return p ;
}
