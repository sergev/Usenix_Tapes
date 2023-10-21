/*
 * Defines for the article data base file
 */

/* flags in artrec structure */
#define A_DUMMY 01		/* placeholder; article not yet arrived */
#define A_EXPIRED 02		/* article has been expired */
#define A_CANCELLED 04		/* article has been cancelled */
#define A_NOFILE 07		/* no file for this article */


typedef long DPTR ;		/* file address */
typedef int ARTNO ;		/* article number */
#define DNULL 0L


#define MAXNG	5		/* max # newsgroups per article */
#define A_MAXKW	6		/* max # keywords on an article */
#define A_SPACE 576		/* space for storing strings */


struct artgroup {
      short a_ngnum;		/* number of this newsgroup */
      long  a_artno;		/* number of article within newsgroup */
      DPTR  a_ngchain;		/* pointer to previous article in newsgroup */
};


struct artrec {
      long  a_subtime;		/* when article was posted */
      long  a_rectime;		/* when article was received */
      long  a_exptime;		/* when this article expires (0 if not specified) */
      DPTR  a_parent;		/* article this is a followup to */
      DPTR  a_children;		/* linked list of followups */
      DPTR  a_childchain;	/* link for followup chain */
      DPTR  a_idchain;		/* chain for article id hash */
      DPTR  a_titlechain;	/* title hash chain */
      short a_flags;		/* various flags */
      short a_nlines;		/* length of article */
      char  a_ngroups;		/* number of newsgroups this article posted to */
      char  a_nkwords;		/* number of keywords on article */
      struct artgroup a_group[MAXNG];	/* list of groups article posted to */
      char *a_ident;		/* message id */
      char *a_title;		/* article subject line */
      char *a_from;		/* author of article */
      char *a_file;		/* file containing article */
      char *a_kword[A_MAXKW];	/* keywords */
      char  a_space[A_SPACE];	/* space to store strings */
};


extern DPTR nglnext ;

DPTR nglfirst() ;
ARTNO ngltest() ;
DPTR lookart() ;


#define BKWD_GROUP(ngnum, artno, dp, a)	for (dp = nglfirst(ngnum) ; (artno = ngltest(ngnum, &(a))) >= 0 ; dp = nglnext)

#define ainit(a)
#define afree(a)
