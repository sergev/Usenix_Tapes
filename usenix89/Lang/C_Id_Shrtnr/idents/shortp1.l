%%
[A-Za-z_]+[a-zA-Z_0-9]?+	printit();
[\"]				quote();
%%
FILE *idents,*fopen();
#include "identp1.h"
main()
{
	idents=fopen("ident","w");
	if(idents==NULL) {
		fprintf(stderr,"panic! Cannot open header file!\n");
		exit(1);
	}
	yylex();
}
printit()
{
	if(yyleng>IDENTLEN)
		fprintf(idents,"%s\n",yytext);
}
quote()
{
	int c;

	while(1){
		c=input();
		switch(c) {
			case '"':
				return(0);
			case '\\':
				c=input();
		}
	}
}
yywrap()
{
	return(1);
}
