/*
 *
 */

#include <stdio.h>

yyerror(s)
char *s;
{
	fprintf( stderr, "%s at line %D\n", s, inline);
	lexmark();
	exit(-1);
}

main(argc, argv)
int argc;
char *argv[];
{
	extern int yydebug;
	for(argv++; argc>1; argc--, argv++ )
		if( (*argv)[0] == '-' )
			switch( (*argv)[1] ) {

			case 'n':
				narrate = 1;
				continue;

			case 'd':
				yydebug = 1;
				continue;

			default:
				continue;

			}
		else if ( freopen(*argv,"r",stdin)==NULL){
			fprintf(stderr,"leroy:can\"t open %s\n",*argv);
			exit(0);
		}


	yyparse();
	done_();
}

