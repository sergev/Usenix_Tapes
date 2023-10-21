#include <stdio.h>


/*
 * Petals-around-the-rose.
 *
 * A Mind-set puzzle game.
 */



int ndie = 5 ;
int die[5] ;
int answer ;

char *dice[6][5] = {
	"-------",
	"|     |",
	"|  *  |",
	"|     |",
	"-------",

	"-------",
	"|*    |",
	"|     |",
	"|    *|",
	"-------",

	"-------",
	"|*    |",
	"|  *  |",
	"|    *|",
	"-------",

	"-------",
	"|*   *|",
	"|     |",
	"|*   *|",
	"-------",

	"-------",
	"|*   *|",
	"|  *  |",
	"|*   *|",
	"-------",

	"-------",
	"|* * *|",
	"|     |",
	"|* * *|",
	"-------"
	} ;



main()
{
	int c ;
	int guess ;

	printf("			PETALS-AROUND-THE-ROSE\n\n") ;
	printf("The object of this game is to discover the pattern used to\n") ;
	printf("generate the value of the roll of the dice.\n") ;
	printf("The computer will print out a picture of five dice and wait\n") ;
	printf("for the entry of a guess at the answer.  In order to get the\n") ;
	printf("answer, type in a question mark, '?'.  In order to end the\n") ;
	printf("game, type in a 'q'.\n") ;
	printf("\nHere are three clues, if you need them:\n") ;
	printf("	1) The name of the game is 'Petals-around-the-rose.\n") ;
	printf("	2) The name of the game is important.\n") ;
	printf("	3) The answer is always even.\n") ;
	printf("\nGOOD LUCK!\n\n") ;

	randomize() ;

	while (1) {
		roll() ;
		while (1) {
			if( (c=getchar()) == EOF || (char) c == 'q') goto done ;
			ungetc(c, stdin) ;
			if( c == '?' ) {
				break ;
				}
			else {
				scanf("%d",&guess) ;
				if( guess == answer ) {
					printf("Yes - ") ;
					break ;
					}
				else {
					printf("No.\n") ;
					}
				}
			skipnl() ;
			}

		skipnl() ;
		printf("The answer is %d.\n", answer) ;
		}

	done :

	exit(0) ;

	} /* main */





roll()
{
	int d,l ;

	answer = 0 ;
	for( d=0; d<ndie; d++) {
		die[d] = (urand()%6) ;
		if (die[d] == 2) answer += 2 ;
		if (die[d] == 4) answer += 4 ;
		}

	putchar('\n') ;
	for( l=0; l<5; l++) {
		for( d=0; d<ndie; d++) {
			printf("    %s",dice[die[d]][l]) ;
			}
		putchar('\n') ;
		}

	} /* roll */





skipnl()
{
	int c ;

	while( (c=getchar()) != EOF && c != '\n') {
		}

	} /* skipnl */
