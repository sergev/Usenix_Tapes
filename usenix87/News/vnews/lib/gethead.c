#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "defs.h"
#include "arthead.h"


static char *hdname[NHDNAME + 1] = {
      "Relay-Version",
      "Posting-Version",
      "Path",
      "From",
      "Newsgroups",
      "Subject",
      "Message-ID",
      "Date",
      "Article-I.D.",
      "Posted",
      "Date-Received",
      "Expires",
      "References",
      "Control",
      "Sender",
      "Reply-To",
      "Followup-To",
      "Distribution",
      "Organization",
      "Lines",
      "Keywords",
      "Approved",
      "Summary",
      "Priority",
      0
};



FILE *
gethead(hp, fp)
      register struct arthead *hp ;
      FILE *fp ;
      {
      char line[LBUFLEN] ;
      register char *p ;
      register char **pp ;
      register int i ;
      char *spnext ;
      int spleft ;
      int spnum ;
      int unreccnt ;
      extern char FULLSYSNAME[] ;
      char *index(), *ckmalloc() ;
      char *hfgets() ;

      for (pp = &hp->h_from, i = NHDNAME + NUNREC ; --i >= 0 ; *pp++ = 0) ;
      unreccnt = 0 ;
      spleft = 0 ;
      spnum = 0 ;
      while (hfgets(line, LBUFLEN, fp) != NULL && line[0] != '\n') {
            if ((p = index(line, ':')) == NULL)
                  goto bad ;
            *p = '\0' ;
            if (index(line, ' ') != NULL)	/* this test unnecessary */
                  goto bad ;
            for (pp = hdname ; *pp ; pp++) {
                  if (**pp == line[0] && strcmp(*pp, line) == 0)
                        break ;
            }
            if (*pp) {
                  while (*++p == ' ') ;
                  if (pp == hdname + 2) {	/* Path: */
                        if (prefix(p, FULLSYSNAME)
                         && index(NETCHRS, p[i = strlen(FULLSYSNAME)]))
                              p += i + 1 ;
                  }
                  pp = &hp->h_relayversion + (pp - hdname) ;
            } else {
                  *p = ':' ;
                  p = line ;
                  if (unreccnt >= NUNREC)
                        continue ;		/* ignore if won't fit */
                  pp = &hp->h_unrec[unreccnt++] ;
            }
            nstrip(p) ;
            i = strlen(p) + 1;
            if (i > spleft) {
                  if (spnum >= 8)
                        xerror("Out of header space") ;
                  if (hp->h_space[spnum] == NULL)
                        hp->h_space[spnum] = ckmalloc(H_SPACE) ;
                  spnext = hp->h_space[spnum++] ;
                  spleft = H_SPACE ;
            }
            *pp = spnext ;
            strcpy(spnext, p) ;
            spnext += i ;
            spleft -= i ;
      }
      if (hp->h_from != NULL && hp->h_path != NULL && hp->h_subdate != NULL && hp->h_ident != NULL)
            return fp ;
bad:
      return NULL ;
}
