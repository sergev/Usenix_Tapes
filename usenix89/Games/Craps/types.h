#include <curses.h>
#include <signal.h>
#define SCORES				/* yes, maintain a high score list */
					/* if you dont want to, comment it */
					/* out 				   */
#define BEEP	putchar((char)7)
#define ESC	(char)27
#define LIMIT	500.00
#define SLIMIT	1000.00
#define R	0
#define C	1
#define D	C+62
#define PASS	0
#define COME	1
#define DONT	2
#define DCOME	3
#define ODDS	4
#define FIELD	5
#define PLACE	6
#define LODDS	7
#define HWAY	8
#define ASEVEN	9
#define	ACRAPS	10
#define	TWO	11
#define	THREE	12
#define	EEYO	13
#define TWELVE	14
#define CHEAT	15
#define QUIT	16
#define HELP	17
#define SHELL	18
#define ROLL	19
