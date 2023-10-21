#include "artfile.h"
#include "stroff.h"

/* length of section of artrec that can be written directly to artfile */
#define A_WRTLEN (int)(stroff(a_nkwords, artrec) + 1)

/* offsets */
#define PARENTOFF	(stroff(a_parent, artrec) + 1)
#define CHILDRENOFF	(stroff(a_children, artrec) + 1)
#define CHILDCHOFF	(stroff(a_childchain, artrec) + 1)
#define GROUPOFF	(stroff(a_group[0], artrec) + 1)


#define A_PREFIX 0210		/* this byte precedes each article record */
#define AF_MAGIC 0431		/* magic number to indicate artfile */
#define AF_VERSION 1		/* version number */


struct afheader {
      short af_magic;		/* magic number (0431) */
      short af_version;		/* version number */
      DPTR  af_idtab;		/* offset of message id hash table */
      DPTR  af_nglist;		/* pointer to newsgroup chain table */
      DPTR  af_titletab;	/* title hash table */
      DPTR  af_records;		/* start of article records */
      DPTR  af_free;		/* add next record here */
      short af_idtlen;		/* # elements in message id hash table */
      short af_maxng;		/* # elements in newsgroup chain table */
      short af_ttlen;		/* # elements in title hash table */
};


extern struct afheader afhd ;
extern int affd ;

#if BUFSIZ == 1024
#define BSIZE 1024
#else
#define BSIZE 512
#endif


DPTR readptr() ;

#define ngchain(ngnum)	(afhd.af_nglist + ngnum * (int)sizeof(DPTR))
#define hashid(msgid)	(afhd.af_idtab + hash(msgid, afhd.af_idtlen) * (int)sizeof(DPTR))

#define equal(s1, s2)	(strcmp(s1, s2) == 0)
