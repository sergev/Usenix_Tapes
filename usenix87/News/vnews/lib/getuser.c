/*
 * Get the user's name and home directory.
 */

#include "config.h"
#include <stdio.h>


getuser() {
      extern char *username, *userhome;
      extern char *getenv();

#if USGREL > 6
      username = getenv("LOGNAME") ;
#else
      username = getenv("USER") ;
#endif
      if ((userhome = getenv("HOME")) == NULL)
            userhome = getenv("LOGDIR") ;		/* PWB??? */

      /* if not in environment, get from /etc/passwd file */
      if (username == NULL || userhome == NULL)
            pgetuser() ;
}
