#
/*
**      Header file for them beasts
** (c) P. Langston 1979
*/

#define H_SCCS      "@(#)beasts.h	1.2  last mod 3/8/80 -- (c) psl 1979"
#define N_QUEST     256                 /* max num questions possible */
#define NQI         ((N_QUEST+15)/16)   /* space for bit map */
#define N_BEAST     1024                /* max num of beasts */
#define NBI         ((N_BEAST+15)/16)   /* space for bit map */

#define YES         1
#define NO          0

#define BEASTFILE   "/sys/games/.../beastfile"
#define QUESTFILE   "/sys/games/.../questfile"
#define LOCKNODE    "/sys/games/.../questfile"
#define LOCKFILE    "/sys/games/.../beastlock"


struct  bstr {
	char    b_name[32];             /* name of beast */
	int     b_ans[NQI];             /* bit map of question answers */
};

struct  qstr {
	char    q_text[128];            /* text of question, w/o `?' */
	int     q_uid;                  /* who supplied question */
	long    q_date;
};
