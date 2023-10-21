#include "defs.h"

extern struct playent player[];
extern int deck[];
extern int card;
extern int dealerhand[];
extern int dealercard;
extern int needtoshuffle;
extern int lastdealt;

void tell2all();

/*
 * Mark player as unoccupied and close file descriptor.
 */
void del_players()
{
	int i, total;
	struct playent *p;
	char buf[80];
	float davg, pavg;;

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->quitting)  {
			davg = (p->dcnt) ? ((float) p->dtot) / p->dcnt : 0;
			total = p->lost + p->won + p->push;
			pavg = (p->nobust) ? ((float) p->ctot) / p->nobust : 0;
			sockwrite(p->socket, "MThank you for playing");
			(void) sprintf(buf, "MYou played %d hands -  won %d lost %d pushed %d - %s $%d avg %3.1f davg %3.1f\n", total, p->won, p->lost, p->push, (p->cash >= 0) ? "up" : "down", abs(p->cash), pavg, davg);
			sockwrite(p->socket, buf);
			sockwrite(p->socket, "Q");
			p->quitting = FALSE;
			p->occupied = FALSE;
			(void) close(p->socket);
			p->socket = -1;
		}
}

/*
 * Tell player which seats are empty and let him choose one.
 */
int getchair(s)
int s;				/* socket to talk to new player on */
{
	int i, len;
	struct playent *p;
	char buf[80], tmp[10];

	(void) strcpy(buf, "RThe following seats are empty:");
	len = strlen(buf);
	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (!p->occupied)  {
			(void) sprintf(tmp, " %d", i + 1);
			(void) strcat(buf, tmp);
		}
	if (len == strlen(buf))
		return(-1);
	(void) strcat(buf, ".  Choose one: ");
	do  {
		sockwrite(s, buf);
		sockread(s, tmp);
		(void) sscanf(tmp, "%d", &i);
	}  while ((--i < 0) || (i > (NPLAYERS - 1)) || (player[i].occupied));
	return(i);
}

/*
 * Send everyone the string buf
 */
void tellall(buf)
char *buf;
{
	int i;
	struct playent *p;

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)
			sockwrite(p->socket, buf);
}

/*
 * Return # of players in game.
 */
int numplayer()
{
	int i, nump = 0;
	struct playent *p;

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)
			nump++;
	return(nump);
}

/*
 * Shuffle from the top of the deck (card 51) to the card topcard
 * (the first card of the last deal, or 0).  This way you can
 * shuffle just a portion of the deck if you run out of cards
 * during a hand (just like in Tahoe).  topcard is zero if the
 * whole deck is to be shuffled (i.e. you know you don't have
 * enough cards).
 */
void shuffle(topcard)
int topcard;				/* first card of last deal */
{
	int pos, i, swap, save;
	long random();

	save = topcard;
	for (i = 51; i >= topcard; i--)  {
		pos = ((((float) (random() & 0x7FFFFFFF)) / 0x7FFFFFFF) * (51 - topcard)) + topcard;
		swap = deck[pos];
		deck[pos] = deck[i];
		deck[i] = swap;
	}
	tellall("Mshuffling deck");
	card = 51;
	needtoshuffle = FALSE;
/*
 * If we are only shuffling part of the deck, make sure we shuffle
 * it the next time through.
 */
	if (save)
		needtoshuffle = TRUE;
}

/*
 * Deal everyone two cards.
 */
void deal()
{
	struct playent *p;
	int i, j;

	lastdealt = card + 1;
	for (i = 0; i < 2; i++)  {
		for (j = 0, p = player; j < NPLAYERS; j++, p++)
			if (p->occupied)  {
				p->cards[0][i] = deck[card--];
				p->insured = FALSE;
				p->nhands = 1;
				p->ncards[0] = 2;
			}
		dealerhand[i] = deck[card--];
	}
	dealercard = 2;
}

/*
 * Append card i (i.e. 2J KH 10S) to end of string buf.
 */
void cardval(i, buf)
int i;					/* card */
char *buf;				/* buffer to append to */
{
	int suit, cval;
	static char *allsuit[] = {"C ", "D ", "H ", "S "};
	static char *allval[] =
		{"A", "2", "3", "4", "5", "6", "7", "8", "9",
		"10", "J", "Q", "K"};

	suit = i / 13;
	cval = i % 13;
	(void) strcat(buf, allval[cval]);
	(void) strcat(buf, allsuit[suit]);
}

/*
 * Convert players hand to a string based upon code.
 */
void show_hand(player, hand, cards, ncards, code, buf)
int player;				/* player # */
int hand;				/* hand # */
int *cards;				/* pointer to cards of hand */
int ncards;				/* # cards of this hand */
int code;				/* controls display of hand */
char *buf;				/* buf to write on */
{
	char playerid[2], handid[2];
	int i;

	(void) strcpy(buf, "C");
	playerid[0] = player + '0';
	playerid[1] = '\0';
	(void) strcat(buf, playerid);
	handid[0] = hand + '0';
	handid[1] = '\0';
	(void) strcat(buf, handid);

#ifdef BARNEYS
	code = C_ALLUP;
#endif

	switch(code)  {
	case C_ALLUP:				/* all cards up */
		for (i = 0; i < ncards; i++)
			cardval(cards[i], buf);
		break;
	case C_2DOWNRESTUP:			/* 2 down, rest up */
		(void) strcat(buf, "?? ?? ");
		for (i = 2; i < ncards; i++)
			cardval(cards[i], buf);
		break;
	case C_2UP1DOWN:			/* 2 up, 1 down */
		cardval(cards[0], buf);
		cardval(cards[1], buf);
		(void) strcat(buf, "?? ");
		break;
	case C_1DOWN1UP:			/* 1 down, 1 up */
		(void) strcat(buf, "?? ");
		cardval(cards[1], buf);
		break;
	case C_1UP1DOWN:			/* 1 up, 1 down */
		cardval(cards[0], buf);
		(void) strcat(buf, "?? ");
		break;
	}
}

/*
 * Return the value of a hand.  Set hi bit if it's soft 17.
 */
int handval(cards, ncards)
int *cards, ncards;			/* array of cards and the # of them */
{
	int soft = 0, val = 0, i;
	static int cardvalue[] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};

	for (i = 0; i < ncards; i ++)  {
		val += cardvalue[cards[i] % 13];
		if (!(cards[i] % 13))
			soft++;
	}
	if (soft && (val > 21))
		while (soft--)  {
			val -= 10;
			if (val <= 21)
				break;
		}
	if (soft && (val == 17))
		return(SOFT17 && val);
	else
		return(val);
}

/*
 * Show the deal to all the players.
 */
void showdeal()
{
	int i;
	char playerbuf[40], otherbuf[40];
	char dbuf[40];
	struct playent *p;

	show_hand(NPLAYERS, 0, dealerhand, 2, C_1DOWN1UP, dbuf);
	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)  {
/*
 * Show player his hand.
 */
			show_hand(i, 0, (int *) p->cards, 2, C_ALLUP, playerbuf);
/*
 * Show everyone else two ??s
 */
			show_hand(i, 0, (int *) p->cards, 2, C_2DOWNRESTUP, otherbuf);
			tell2all(i, playerbuf, otherbuf);
			sockwrite(p->socket, dbuf);
		}
}

/*
 * Show all players a given hand.  Player gets to see his own cards,
 * everyone else's view is based on code.  If BARNEYS is defined, all
 * cards are dealt face up.
 */
void show_cards(playernum, hand, cards, ncards, code)
int playernum;				/* player number */
int hand;				/* hand number */
int *cards;				/* array of cards */
int ncards;				/* number of cards */
int code;				/* controls display of hand */
{
	char playerbuf[40], otherbuf[40];

	show_hand(playernum, hand, cards, ncards, code, otherbuf);
	show_hand(playernum, hand, cards, ncards, C_ALLUP, playerbuf);
	tell2all(playernum, playerbuf, otherbuf);
}

/*
 * Hand out cards.  This routine is used so that we can shuffle
 * if we run out of cards.
 */
int deal_it(lastdealt)
int lastdealt;				/* the first card of the last deal */
{
	if (card < 0)
		shuffle(lastdealt);
	return(deck[card--]);
}
