/* include file for memory resident unique sorting routines.
 *
 * Written & hacked by Stephen Uitti, PUCC staff, ach@pucc-j, 1985
 * libsrtunq is copyright (C) Purdue University, 1985
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */

/* database entry */
struct srtbl {
    struct srtbl *srt_prev;		/* parent */
    struct srtbl *srt_less;		/* something < srt_str */
    struct srtbl *srt_more;		/* something > srt_str */
    char srt_str[1];			/* dynamic: 1 for null at EOS */
};

/* database tag */
typedef struct srtent {
    struct srtbl *srt_top;		/* root of the tree */
    struct srtbl *srt_next;		/* pointer for srtget */
} SRTUNQ;

/* The functions */
void	srtinit();			/* init for srtin */
void	srtdtree();			/* recursive delete of subtree */
char   *srtin();			/* insert string - return err */
void	srtgti();			/* init for srtgets */
char   *srtgets();			/* get next string */
void	srtfree();			/* free a database */
