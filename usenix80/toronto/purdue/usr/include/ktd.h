#

/*
 *      Structure definition for accounting programs
 */

#define NUSR    3               /* Number of groups of users */
#define KTMP    "/usr/adm/ktmp" /* Accounting file */

struct {
	int     timev[2];       /* Time when sampled */
	int     cnt[NMON];      /* All other info (including user cnt) */
	int     u_count[NUSR];  /* Different groups of users */
} sum;

char staff[]    "!8rqwN";       /* System terminals */
char techtyp[]  "efnpuvX";      /* Technical typing terminals */
