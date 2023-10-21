#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>
#include <strings.h>

#ifndef ROOTPRIV		/* internet port # to use if not using	*/
#define PORT 1027		/* /etc/services			*/
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define NPLAYERS 6		/* max # players */
#define SLEN 40			/* short string */
#define PROTO "tcp"		/* protocol */
#define SERVICE "blackjack"	/* official service name */
#define WIDTH 32		/* # file descriptors */
#define SOFT17 0x80		/* bit set for soft 17 */
#define CARDMASK 0x7f		/* bit mask for players */
#define C_ALLUP 0		/* all cards up (at end of hand, splitting) */
#define C_2DOWNRESTUP 1		/* two down, rest up (other player's hand) */
#define C_2UP1DOWN 2		/* two up, one down (doubling down) */
#define C_1DOWN1UP 3		/* one down, one up (dealer) */
#define C_1UP1DOWN 4		/* one up, one down (splitting aces) */


typedef struct playent  {
	int socket;		/* socket for player */
	int cards[4][11];	/* players hand */
	int quitting;		/* TRUE when player is going to leave */
	int bet[4];		/* amount player is betting */
	int occupied;		/* TRUE when a player is sitting here */
	char name[SLEN];	/* players name */
	int nhands;		/* # hands player has */
	int ncards[4];		/* # cards in each hand */
	int insured;		/* TRUE when player is insured */
	int cash;		/* amount of cash player has */
	int won, lost, push;	/* # hands won, lost, pushed */
	int ctot, nobust;	/* running total of non-bust hands */
	int dtot, dcnt;		/* running total for dealer since player started */
};

void sockread(), sockwrite();
char *sprintf();
