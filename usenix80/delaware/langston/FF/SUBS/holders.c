#include    "../ffdef.h"
holders(cnum, hp)
struct  holder  *hp;
{
	register int i, j, mini;
	int nhold;

	nhold = 0;
	for (i = 0; i < num_players; i++) {
	    if (getplyr(i, SORRY) == -1 || plyr.p_name[0] == '\0')
		hp[i].h_shares = 0;
	    else {
		hp[i].h_shares = plyr.p_shares[cnum];
		nhold++;
	    }
	    hp[i].h_pnum = i;
	    hp[i].h_maj = hp[i].h_min = 0;
	}
	for (i = num_players; --i > 0; ) {      /* sort into inverse order */
	    mini = i;
	    for (j = 0; j < i; j++)
		if (hp[j].h_shares < hp[mini].h_shares)
		    mini = j;
	    if (mini != i) {
		j = hp[mini].h_pnum;
		hp[mini].h_pnum = hp[i].h_pnum;
		hp[i].h_pnum = j;
		j = hp[mini].h_shares;
		hp[mini].h_shares = hp[i].h_shares;
		hp[i].h_shares = j;
	    }
	}
	if (hp[0].h_shares == hp[1].h_shares) {           /* tie for major */
	    j = 2;
	    for (i = 2; i < num_players; i++) {
		if (hp[i].h_shares < hp[0].h_shares)
		    break;
		j++;
	    }
	    while (--i >= 0)
		hp[i].h_maj = hp[i].h_min = j;
	} else if (hp[1].h_shares == 0)                 /* only one holder */
	    hp[0].h_maj = hp[0].h_min = 1;
	else {
	    hp[0].h_maj = 1;
	    if (hp[1].h_shares == hp[2].h_shares) {       /* tie for minor */
		j = 2;
		for (i = 3; i < num_players; i++) {
		    if (hp[i].h_shares < hp[1].h_shares)
			break;
		    j++;
		}
		while (--i >= 1)
		    hp[i].h_min = j;
	    } else
		hp[1].h_min = 1;
	}
	return(nhold);
}
