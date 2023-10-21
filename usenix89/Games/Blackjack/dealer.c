#include "defs.h"

int s;						/* main socket */
int numplayer();
struct playent player[NPLAYERS];		/* players in the game */
struct sockaddr_in sockaddr;
int deck[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};
int card = 0;
int dealerhand[11];				/* dealers hand */
int needtoshuffle = 1;
int dealercard;					/* # of cards dealer has */
int lastdealt;					/* first card dealt from this hand - for shuffling during a hand */
void insurance(), proc_insurance(), tellall(), tell2all(), process();
void shuffle(), betting(), process_dealer(), deal(), show_deal(), who_won();
void opensock(), showdeal();

main()
{
	int i;
	int readfds;			/* main socket bit mask */
	struct timeval t;		/* zero timeout for selects */
	int news;			/* new fd */
	char buf[80], pname[40];
	struct sockaddr_in sockad;
	int ssize;			/* to make accept happy */
	int fderr;
	void closesock(), del_players();
	int getchair();
	int newp;			/* temp socket of new player */
	struct playent *p;
	long random();

	opensock();
	srandom(getpid());
	t.tv_sec = t.tv_usec = 0;
	for (i = 0; i < NPLAYERS; i++)
		player[i].socket = -1;
	for(;;)  {
		readfds = 1 <<  s;
/*
 * wait for players to show up
 */
		if (!numplayer())  {
			if (select(WIDTH, &readfds, (int *) 0, (int *) 0, (struct timeval *) 0) == -1)  {
				perror("select");
				exit(1);
			}
		}
		del_players();
		readfds = 1 <<  s;
		while(fderr = select(WIDTH, &readfds, (int *) 0, (int *) 0, &t))  {
			if (fderr == -1)  {
				perror("select");
				exit(1);
			}
/*
 *  add whoever's waiting
 */
			if ((news = accept(s, sockad, &ssize)) == -1)  {
				perror("accept");
				exit(1);
			}
			sockread(news, pname);
/*
 * tell new player who's playing
 */
			for (i = 0, p = player; i < NPLAYERS; i++, p++)  {
				if (p->occupied)
					(void) sprintf(buf, "Ps%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
				else
					(void) sprintf(buf, "Ps%1dEmpty", i);
				sockwrite(news, buf);
			}
			if ((newp = getchair(news)) == -1)  {
				sockwrite(news, "DSorry, game full");
				(void) close(news);
			}
			else  {
/*
 * add player and initialize things
 */
				player[newp].occupied = TRUE;
				player[newp].socket = news;
				player[newp].cash = 0;
				player[newp].bet[0] = 0;
				player[newp].won = 0;
				player[newp].lost = 0;
				player[newp].push = 0;
				player[newp].ctot = 0;
				player[newp].dcnt = 0;
				player[newp].dtot = 0;
				player[newp].nobust = 0;
				(void) strcpy(player[newp].name, pname);
				(void) sprintf(buf, "MWelcome to network blackjack, you are player #%d", newp + 1);
				sockwrite(news, buf);
			}
			readfds = 1 << s;
		}
		show_players();
		if (needtoshuffle || (card < (numplayer() * 3) + 3))
			shuffle(0);
		betting();
		show_players();
		deal();
		showdeal();
/*
 * offer insurance if dealer shows an ace
 */
		if (!(dealerhand[1] % 13))  {
			insurance();
			proc_insurance();
		}
/*
 * let people play if dealer doesn't have blackjack
 */
		if ((handval(dealerhand, 2) & CARDMASK) != 21)
			for (i = 0, p = player; i < NPLAYERS; i++, p++)
				if (p->occupied)  {
					(void) sprintf(buf, "PS%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
					tellall(buf);
					process(i);
					(void) sprintf(buf, "Ps%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
					tellall(buf);
				}
		process_dealer();
		who_won();
		tellall("RHit any key to continue");
		readall();
		tellall("c");
	}

}

/*
 * Get a char from everyone (hit space to return or something like that )
 */
readall()
{
	int i;
	char buf[5];
	struct playent *p;

	for (i = 0, p = player; i < NPLAYERS; i++, p++)
		if (p->occupied)
			sockread(p->socket, buf);
}

show_players()
/*
 * show everyone who's playing
 */
{
	int i;
	struct playent *p;
	char buf[40];

	for (i = 0, p = player; i < NPLAYERS; i++, p++)  {
		if (p->occupied)
			(void) sprintf(buf, "Ps%1d%s $%1d (%c%1d)", i, p->name, p->bet[0], p->cash >= 0 ? '+' : '-', abs(p->cash));
		else
			(void) sprintf(buf, "Ps%1dEmpty", i);
		tellall(buf);
	}
	(void) sprintf(buf, "Ps%1d", NPLAYERS);
	tellall(buf);
}
