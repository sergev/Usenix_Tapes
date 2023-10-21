#include "defs.h"

extern struct playent player[];
extern int deck[];
extern int card;
extern int dealerhand[];
extern int dealercard;
extern int lastdealt;
void show_cards(), show_hand(), tellall();

/*
 * Sends string s1 to the current player (recipient), and s2 to
 * all the other players
 */
void tell2all(recipient, s1, s2)
int recipient;				/* player who gets s1 */
char *s1, *s2;				/* s1 usually hand with all cards showing, other guys see s2 */
{
	int i;
	struct playent *p;

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)
			if (i == recipient)
				sockwrite(p->socket, s1);
			else
				sockwrite(p->socket, s2);
}

/*
 * Process the hands of player i.  Handles hitting, doubing down, splitting,
 * standing, busting, updating screen.
 */
void process(i)
int i;				/* player to process */
{
	struct playent *p;
	int hand;		/* which hand of maximum of 4 */
	int code;		/* controls display of hand	*/
	int quit;		/* TRUE when single hand is done */
	char buf[SLEN];
	int value;		/* value of players hand */

	p = &player[i];
/*
 * Player got dealt a blackjack
 */
	if ((handval(&p->cards[0][0], 2) & CARDMASK) == 21)  {
		sockwrite(p->socket, "MBlackjack!  You win!");
		p->ncards[0] = 2;
		return;
	}
	code = C_2DOWNRESTUP;
	for (hand = 0; hand < p->nhands; hand++)  {
		p->ncards[hand] = (hand == 0) ? 2 : 1;
		quit = 0;
		while (!quit && ((value = (handval(&p->cards[hand][0], p->ncards[hand]) & CARDMASK)) < 21))  {
			if (p->ncards[hand] == 1)  {
/*
 * Player just split, so make sure he has two cards.  Make sure this hand
 * is face up, because doubling down could have set the previous hand to
 * two up one down.
 */
				p->cards[hand][p->ncards[hand]++] = deal_it(lastdealt);
				code = C_ALLUP;
				show_cards(i, hand, &p->cards[hand][0], p->ncards[hand], code);
				if ((handval(&p->cards[hand][0], 2) & CARDMASK) == 21)
					sockwrite(p->socket, "MBlackjack!  You win!");
				continue;
			}
			sockwrite(p->socket, "R(h)it ( )stand (s)plit (d)ouble down (q)uit?");
			sockread(p->socket, buf);
			switch(buf[0])  {
/*
 * Player is quitting after finishing this hand
 */
			case 'q' :
			case 'Q' :
				sockwrite(p->socket, "MQuitting...");
				p->quitting = TRUE;
				break;
/*
 * Player is hitting.
 */
			case 'h' :
			case 'H' :
				p->cards[hand][p->ncards[hand]++] = deal_it(lastdealt);
				show_cards(i, hand, &p->cards[hand][0], p->ncards[hand], code);
				break;
/*
 * Player is splitting hand
 */
			case 's' :
			case 'S' :
				if (((p->cards[hand][0] % 13) != (p->cards[hand][1] % 13)) || (p->ncards[hand] != 2))  {
					sockwrite(p->socket, "MCan't split that");
					break;
				}
/*
 * Split the hand up and adjust hand, # of cards, and bet.
 */
				p->ncards[hand] = 1;
				p->ncards[p->nhands] = 1;
				p->cards[p->nhands][0] = p->cards[hand][1];
				p->bet[p->nhands] = p->bet[hand];
				if (!(p->cards[hand][0] % 13))  {
					p->nhands++;
					p->ncards[0]++;
/*
 * Player just split aces, so deal him two more cards face down, and
 * don't let him play anymore.  Tell him if he gets blackjack now.
 */
					p->ncards[1]++;
					p->cards[0][1] = deal_it(lastdealt);
					p->cards[1][1] = deal_it(lastdealt);
					code = C_1UP1DOWN;
					show_cards(i, 0, &p->cards[0][0], p->ncards[0], code);
					show_cards(i, 1, &p->cards[1][0], p->ncards[1], code);
					return;
					
				}
/*
 * When player splits, cards are all up.  I am told that when you split,
 * the new hand becomes the LAST hand, and that you finish each hand
 * before making sure each hand has two cards.  If this were not the case
 * you would need a code for each hand.
 */
				code = C_ALLUP;
				show_cards(i, hand, &p->cards[hand][0], p->ncards[hand], code);
				show_cards(i, p->nhands, &p->cards[p->nhands][0], p->ncards[p->nhands], code);
				p->nhands++;
				break;
/*
 * Player is standing.
 */
			case ' ' :
				quit = 1;
				break;
/*
 * Player is doubling down.
 */
			case 'd' :
			case 'D' :
				code = C_2UP1DOWN;
				if (((value != 10) && (value != 11)) || (p->ncards[hand] != 2))  {
					sockwrite(p->socket, "MCan't double down on that");
					break;
				}
				quit = 1;
				p->cards[hand][p->ncards[hand]++] = deal_it(lastdealt);
				p->bet[hand] *= 2;
				show_cards(i, hand, &p->cards[hand][0], p->ncards[hand], code);
				break;
			}
		}
/*
 * Player busted, so he throws his cards down in disgust.
 */
		if ((handval(&p->cards[hand][0], p->ncards[hand]) & CARDMASK) > 21)  {
			show_cards(i, hand, &p->cards[hand][0], p->ncards[hand], C_ALLUP);
			(void) sprintf(buf, "M%s busts", p->name);
			tell2all(i, "MYou bust", buf);
		}
	}

}

/*
 * Play for the dealer.  Hit soft 17.
 */
void process_dealer()
{
	char buf[40];
	int value;

	(void) sprintf(buf, "PS%1d", NPLAYERS);
	tellall(buf);
	show_hand(NPLAYERS, 0, dealerhand, dealercard, C_ALLUP, buf);
	tellall(buf);
	while ((((value = handval(dealerhand, dealercard)) & CARDMASK) < 17) || (value & SOFT17))  {
		dealerhand[dealercard++] = deal_it(lastdealt);
		show_hand(NPLAYERS, 0, dealerhand, dealercard, C_ALLUP, buf);
		tellall(buf);
	}
	(void) sprintf(buf, "Ps%1d", NPLAYERS);
	tellall(buf);
}

/*
 * Figure out who won and pay them off.
 */
void who_won()
{
	int i, j;
	struct playent *p;
	char buf[40], playerbuf[40], otherbuf[40];
	int dealertot, playertot;

	dealertot = handval(dealerhand, dealercard) & CARDMASK;
	for (i = 0, p = player; i < NPLAYERS; i++, p++)  {
		if (!p->occupied)
			continue;
		for (j = 0; j < p->nhands; j++)  {
			if (dealertot <= 21)  {
				p->dtot += dealertot;
				p->dcnt++;
			}
			show_hand(i, j, &p->cards[j][0], p->ncards[j], C_ALLUP, buf);
			tellall(buf);
			playertot = handval(&p->cards[j][0], p->ncards[j]) & CARDMASK;
			if (playertot < 21)  {
				p->ctot += playertot;
				p->nobust++;
			}
/*
 * You lose
 */
			if (((dealertot > playertot) && (dealertot <= 21)) || (playertot > 21))  {
				(void) sprintf(otherbuf, "M%s loses hand #%d (%1d)", p->name, j + 1, -p->bet[j]);
				(void) sprintf(playerbuf, "MYou lose hand #%d (%1d)", j + 1, -p->bet[j]);
				tell2all(i, playerbuf, otherbuf);
				p->cash -= p->bet[j];
				p->lost++;
			}
/*
 * You win.  Make sure you don't lose when you have blackjack and dealer
 * has 21.
 */
			else if ((playertot > dealertot) || (dealertot > 21) || (((playertot == 21) && (p->ncards[j] == 2)) &&  !((dealertot == 21) && (dealercard == 2))))  {
/*
 * Blackjack pays 3 to 2.  *= doesn't work.
 */
				if ((playertot == 21) && (p->ncards[j] == 2))
					p->bet[j] = p->bet[j] * 1.5;
				(void) sprintf(otherbuf, "M%s wins hand #%d (+%1d)", p->name, j + 1, p->bet[j]);
				(void) sprintf(playerbuf, "MYou win hand #%d (+%1d)", j + 1, p->bet[j]);
				tell2all(i, playerbuf, otherbuf);
				p->cash += p->bet[j];
				p->won++;
			}
/*
 * You push
 */
			else   {
				(void) sprintf(otherbuf, "M%s pushes hand #%d", p->name, j + 1);
				(void) sprintf(playerbuf, "MYou push hand #%d", j + 1);
				tell2all(i, playerbuf, otherbuf);
				p->push++;
			}
		}
	}
}

/*
 * Handle insurance betting
 */
void insurance()
{
	int i;
	struct playent *p;
	char buf[40], ans[40];

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)  {
			(void) sprintf(buf, "PS%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
			tellall(buf);
			sockwrite(p->socket, "RInsurance?");
			sockread(p->socket, ans);
			if ((*ans == 'y') || (*ans =='Y'))
				p->insured = TRUE;
			else
				p->insured = FALSE;
			(void) sprintf(buf, "Ps%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
			tellall(buf);
		}
}

/*
 * Process insurance bets.  Insurance pays 2 to 1.
 */
void proc_insurance()
{
	int i;
	struct playent *p;
	char playerbuf[40], otherbuf[40];
	int dealertot;

	dealertot = handval(dealerhand, 2) & CARDMASK;
	for(i = 0, p = player; i < NPLAYERS; i++, p++)  {
		if (!p->occupied || !p->insured)
			continue;
		if (dealertot == 21)  {
			(void) sprintf(otherbuf, "M%s wins insurance bet (+%1d)", p->name, p->bet[0]);
			(void) sprintf(playerbuf, "MYou win insurance bet (+%1d)", p->bet[0]);
			p->cash += p->bet[0];
			tell2all(i, playerbuf, otherbuf);
		}
		else  {
			(void) sprintf(otherbuf, "M%s loses insurance bet (%1d)", p->name, (int) -(p->bet[0] * 0.5));
			(void) sprintf(playerbuf, "MYou lose insurance bet (%1d)", (int) -(p->bet[0] * 0.5));
			p->cash -= p->bet[0] * 0.5;
			tell2all(i, playerbuf, otherbuf);
		}
	}
}


/*
 * Handle betting.
 */
void betting()
{
	int i;
	struct playent *p;
	char buf[40];

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)  {
			(void) sprintf(buf, "PS%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
			tellall(buf);
			sockwrite(p->socket, "rBet? ");
			sockread(p->socket, buf);
/*
 * A null line repeats the last bet.
 */
			if (strlen(buf))
				(void) sscanf(buf, "%d", &p->bet[0]);
			(void) sprintf(buf, "Ps%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
			tellall(buf);
		}
}
