#define D_NEWSVERBS
#define D_NWSSTR
#define D_FILES
#include	"empdef.h"

nreport(anum, verb, vnum)
short	anum, verb, vnum;
{
	register	i;
	long	now, lseek();

	time(&now);
X22:	
	i = read(newsf, &nws, sizeof(nws));
	if( i != sizeof(nws) ) goto X122;
	if( now - nws.nws_when <= 302400. ) goto X22; /*3.5 days=302400 sec*/
X122:	
	nws.nws_ano = anum;
	if( verb != N_WON_SECT ) goto X154;
	if( vnum != 0 ) goto X154;
	verb = N_TOOK_UNOCC;
X154:	
	nws.nws_vrb = verb;
	nws.nws_vno = vnum;
	nws.nws_ntm = ntime;
	time(&nws.nws_when);
	lseek(newsf, (long)(-i), 1);
	write(newsf, &nws, sizeof(nws));
	if( i != 0 ) return;
	lseek(newsf, 0L, 0);
	return;
}
