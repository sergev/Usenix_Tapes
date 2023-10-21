/*
 * Lock and unlock the article data base.
 */

#include <stdio.h>
#include <errno.h>
#include "af.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"
#include "libextern.h"


static int islocked ;


/*
 * Lock the data base.  The data base is only locked to avoid multiple
 * writers, so the data base may be read and written simultaneously.
 */

aflock() {
      char lockfile[FPATHLEN] ;
      int ntries ;
      struct stat st ;
      int fd ;
      char pid[10] ;
      long time() ;

      sprintf(lockfile, "%s/artfile.lck", LIB) ;
      ntries = 0 ;
      while (islocked++, (fd = creat(lockfile, 0444)) < 0) {
            islocked-- ;
            if (++ntries == 1 || ntries == 31) {
                  if (stat(lockfile, &st) >= 0 && st.st_mtime < time((long *)0) - 60) {
                        unlink(lockfile) ;
                        continue ;
                  }
            } else if (ntries > 60) {
                  xerror("Can't lock data base") ;
            }
            sleep(2) ;
      }
      sprintf(pid, "%d\n", getpid()) ;
      write(fd, pid, strlen(pid)) ;
      close(fd) ;
#ifdef notdef /* forget about stdio due to BSD bug */
      fseek(affp, 0L, 0) ;
      fread((char *)&afhd, sizeof(afhd), 1, affp) ;
#else
      lseek(affd, 0L, 0) ;
      read(affd, (char *)&afhd, sizeof afhd) ;
#endif
}


afunlock() {
      char lockfile[FPATHLEN] ;

      if (islocked) {
            sprintf(lockfile, "%s/artfile.lck", LIB) ;
            unlink(lockfile) ;
            islocked-- ;
      }
}
