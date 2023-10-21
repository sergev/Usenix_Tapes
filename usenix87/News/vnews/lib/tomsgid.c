#include "config.h"
#include "defs.h"

/*
 * Convert an article id to a message id if it's not already a message id.
 */

tomsgid(id)
      char *id ;
      {
      char lbuf[NAMELEN];
      register char *p ;
      char *index() ;

      if (id[0] != '<') {
            strcpy(lbuf, id);
            p = index(lbuf, '.');
            if (p == 0) {
                  return;
            }
            *p++ = '\0';
            /*
             * It may seem strange that we hardwire ".UUCP" in
             * here instead of MYDOMAIN.  However, we are trying
             * to guess what the domain was on the posting system,
             * not the local system.  Since we don't really know
             * what the posting system does, we just go with the
             * majority - almost everyone will be a .UUCP if they
             * didn't fill in their Message-ID.
             */
            sprintf(id, "<%s@%s%s>", p, lbuf, ".UUCP");
      }
}
