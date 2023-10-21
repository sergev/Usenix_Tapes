#define MAXNGNAME 32

#define G_MOD 01		/* indicates moderated or fa.all group */

struct ngrec {
      char  g_name[MAXNGNAME];	/* newsgroup name */
      short g_num;		/* newsgroup number */
      short g_flags;		/* various flags */
} ;

extern int maxng;		/* max value for g_num */
extern FILE *ngfp;

#define ALL_GROUPS(ngrec)	for (nginit() ; ngread(&(ngrec)) ; )
