/*
 * TREK73: endgame.c
 *
 * prints end-of-game messages and warnings
 *
 * leftovers, final, warn
 *
 */

#include <sys/file.h>
#include "defines.h"
#include "structs.h"

extern char captain[];
extern char title[];
extern char science[];
extern char com[];
extern char helmsman[];
extern char foerace[];
extern char foename[];
extern char foestype[];
extern char empire[];
extern int reengaged;

leftovers()
{
	extern	struct list head;
	extern	struct list *tail;
	register struct list *lp;

	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		if (lp->type != I_SHIP)
			return 1;
	}
	return 0;
}


final(mesg)
int mesg;
{
	extern	struct ship *shiplist[];
	extern	int shipnum;
	register int i;
	register int j;
	struct	ship *sp;
	struct	ship *ep;
	char buf[80];
	extern	char *plural();


	sp = shiplist[0];
	/* If we're getting that message again, ignore it. */
	if ((mesg == 2) && (reengaged))
		return;
	switch (mesg) {
	case 0:
		starfleet();
		printf("We have recieved confirmation that the USS %s,\n",
			sp->name);
		printf("   captained by %s, was destroyed by %s%s\n",captain, shipnum==1 ?"a ":"", foerace);
		printf("   %s%s.  May future Federation officers\n", foestype, plural(shipnum));
		printf("   perform better in their duties.\n\n");
		break;
	case 1:
		starfleet();
		printf("We commend Captain %s and his crew on their\n",captain);
		printf("   fine performance against the %ss.  May he\n",foerace);
		printf("   be an inspiration to future starship captains.\n");
		break;
	case 2:
		/*
		 * Give him a chance to re-engage if he wants to.  If he does,
		 * he has to get within a range of 3500 before he can again
		 * try to dis-engage
		 */
		if (!reengaged) {
			printf("%s:  %s, we are in a position to either disengage from the\n", science, title);
			printf("   %ss, or re-engage them in combat.\n", foerace);
			printf("   Do you wish to re-engage?\n");
			printf("%s: [y or n] ", captain);
			gets(buf);
			if ((*buf == NULL) || (*buf == 'y') || (*buf == 'Y')) {
				reengaged = 1;
				return;
			}
		}
		starfleet();
		printf("Captain %s of the starship %s has\n",captain,sp->name);
		printf("   out-maneuvered %s aggressors.  We commend\n",foerace);
		printf("   his tactical ability.\n");
		break;
	case 3:
		starfleet();
		printf("Captain %s of the starship %s has\n",captain,sp->name);
		printf("   surrendered his vessel to the %ss.  May\n",foerace);
		printf("   Captain Donsell be remembered.\n");
		break;
	case 4:
		starfleet();
		printf("We have recieved word from the %s that the\n",sp->name);
		printf("   %ss have surrendered.\n",foerace);
		break;
	case 5:
		starfleet();
		printf("One of our vessels has encountered the wreckage of\n");
		printf("   the %s and %d other %s vessel%s.\n", sp->name,
			shipnum, foerace, plural(shipnum));
		break;
	default:
		printf("how did we get here?\n");
		break;
	}
	printf("\n\n");
	j = 0;
	for (i=0; i<=shipnum; i++) {
		ep = shiplist[i];
		if (ep->status & S_DEAD)
			continue;
		if (!j)
			printf("Survivors Reported:\n");
		j++;
	}
	if (j) {
		for (i=0; i<=shipnum; i++) {
			ep = shiplist[i];
			if ((ep->status & S_DEAD) || (ep->crew == 0)){
				printf("   %s -- destroyed.\n",ep->name);
				}
			else
				printf("   %s -- %d\n", ep->name, ep->crew);
		}
	} else
		printf("*** No survivors reported ***\n");
	exit (1);
}



warn(mesg)
int mesg;
{
	extern	struct ship *shiplist[];
	static	int beenhere[5] = {0, 0, 0, 0, 0};
	struct	ship *sp;

	if ((reengaged) && (mesg == 2)) {
		return 0;
	}
	if (beenhere[mesg])
		return 0;
	sp = shiplist[0];
	switch (mesg) {
	case 0:
		printf("Message to the Federation:  This is Commander\n");
		printf("   %s of the %s %s.  We have defeated\n", foename, foerace, empire);
		printf("   the %s and are departing the quadrant.\n", sp->name);
		break;
	case 1:
		printf("%s: All %s vessels have been either\n", science, foerace);
		printf("   destroyed or crippled.  We still, however, have\n");
		printf("   antimatter devices to avoid.\n");
		break;
	case 2:
		printf("%s: The %ss are falling behind and seem to\n", helmsman, foerace);
		printf("   be breaking off their attack.\n");
		break;
	case 3:
		printf("%s: I'm informing Starfleet Command of our \n", com);
		printf("   disposition.\n");
		break;
	case 4:
		printf("%s: Although the %ss have surrendered,\n",science, foerace);
		printf("   there are still antimatter devices floating\n");
		printf("   around us.\n");
		break;
	default:
		printf("how did we get here?\n");
		break;
	}
	beenhere[mesg]++;
	return 0;
}

starfleet()
{
	printf("\n\nStarfleet Command: \n");
	sleep(3);
	printf("\n");
}
