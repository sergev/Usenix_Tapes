#include <stdio.h>
#include "defs.h"
#ifndef ROOTID
#include <pwd.h>
#endif


/*
 * Return 1 if the user is the superuser or the netnews administrator.
 */
isadmin() {
      int uid ;
#ifndef ROOTID
      struct passwd *pw ;
#endif

      if ((uid = getuid()) == 0)		/* superuser */
            return 1 ;
#ifdef ROOTID
      return uid == ROOTID ;
#else
      return (pw = getpwnam(ADMIN)) != NULL && pw->pw_uid == uid ;
#endif
}


/* The following code is better for backward compatability, but
   too complex for my liking:

      (*
       * No hardwired administrator ID, so we have to do this the
       * hard way.  First we find the person to be notified.  If
       * that fails, we try the user HOME.
       *)

#ifdef NOTIFY
      char notify[64] ;
      char fname[64] ;
      FILE *nfd ;

      sprintf(fname, "%s/notify", LIB);
      nfd = fopen(fname, "r");
      if (nfd == NULL)
            strcpy(notify, NOTIFY) ;
      else if (fscanf(nfd, "%s", TELLME) == EOF)
            goto nonotify;
      if ((pw = getpwnam(notify)) == NULL)
            goto nonotify;
      return pw->pw_uid == uid;      
#endif

nonotify:
#ifdef HOME
      return (pw = getpwnam(HOME)) != NULL && pw->pw_uid == uid;
#else
      return 0;
#endif
#endif ROOTID
}
*/
