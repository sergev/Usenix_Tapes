#include <curses.h>
#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <values.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>

#define BUFFSZ 256
#define MAXLINES 70
#define MAXFILES 16
#define MAX(x,y) ((x) > (y) ? (x) : (y))

struct fl {
   char *name ;  /* name of file or dir */
   int fd ;      /* file descriptor     */
   int isdir ;   /* TRUE if directory   */
   int lines ;   /* screen lines        */
   WINDOW *win ; /* window descriptor   */
   long tstamp ; /* timestamp for dir   */
   int bell ;    /* ring bell on output */
   int indpos ;  /* posn of bell ind    */
   int lc ;      /* last char in last buff */
   } file [MAXFILES] ;

struct fileinfo {
   char name [DIRSIZ+1] ;
   time_t mtime ;
   off_t fsize ;
   int uid ;
   } ;

int nf ;
int interval = 1 ;
int label = 1 ;

void indicate(f, c)
   struct fl *f ; int c ;
   {
   int x, y ;

   if (c == ' ')
      beep() ;
   if (label) {
      getyx(f->win, y, x) ;
      mvwaddch(f->win, 0, f->indpos+1, c) ;
      wmove(f->win, y, x) ;
      wrefresh(f->win) ;
      }
   f->bell = c ;
   }

int compare (a, b)
   /* Compare file creation times */
   struct fileinfo *a, *b ;
   {
   if (a->mtime == b->mtime)
      return strcmp(a->name, b->name) ;
   return a->mtime < b->mtime ? -1 : 1 ;
   }

int dodirect (d)
   struct fl *d ;
   /*
    * Check a directory for new files.
    * First scan requires a sort to
    * get files into time order.
    */
   {
   register int i ;
   int rflag = FALSE ;
   int n = 0 ;
   int slot ;
   time_t stamp, oldest ;
   static char fname [DIRSIZ+1] ;
   static char sname [128] ;
   static char tbuf [26] ;
   struct passwd *pwd, *getpwuid() ;
   struct direct dir ;
   struct stat fs ;

   struct fileinfo finfo [MAXLINES] ;

   if (d->tstamp == 0L) {
      for (i = 0 ; i < d->lines ; i++) /* initialize */
	 finfo[i].mtime = 0L ;

      while (read(d->fd, &dir, sizeof(struct direct)) ==
	 sizeof(struct direct))
	 if (dir.d_ino && dir.d_name[0] != '.') { /* non-empty */
	    strncpy(fname, dir.d_name, DIRSIZ) ;
	    sprintf(sname, "%s/%s", d->name, fname) ;
	    stat(sname, &fs) ;
	    slot = -1 ;
	    if (n < d->lines)
	       slot = n++ ;
	    else
	       for (oldest = MAXLONG, i = 0 ; i < d->lines ; i++)
		  if (finfo[i].mtime < oldest) {
		     oldest = finfo[i].mtime ;
		     slot = i ;
		     }
	    if (slot >= 0) {
	       strcpy(finfo[slot].name, fname) ;
	       finfo[slot].mtime = fs.st_mtime ;
	       finfo[slot].fsize = fs.st_size ;
	       finfo[slot].uid = fs.st_uid ;
	       }
	    }

      if (n > 1)
	 qsort((char *)finfo, n, sizeof(struct fileinfo), compare) ;

      for (i = 0 ; i < n ; i++) {
	 strncpy(tbuf, ctime(&finfo[i].mtime), 24) ;
	 if ((pwd = getpwuid(finfo[i].uid)) == NULL)
	    sprintf(sname, "%4d", finfo[i].uid) ;
	 else
	    strcpy(sname, pwd->pw_name) ;

	 if (i > 0)
	    waddch(d->win, '\n') ;
	 wprintw(d->win, "%s %10s %8ld  %s",
	 tbuf, sname, finfo[i].fsize, finfo[i].name) ;
	 stamp = finfo[i].mtime ;
	 }
      rflag = TRUE ;
      }

   else {                    /* look for new files */
      lseek(d->fd, 0L, 0) ;
      stamp = d->tstamp ;

      while (read(d->fd, &dir, sizeof(struct direct)) ==
	 sizeof(struct direct))
	 if (dir.d_ino && dir.d_name[0] != '.') { /* non-empty */
	    strncpy(fname, dir.d_name, DIRSIZ) ;
	    sprintf(sname, "%s/%s", d->name, fname) ;
	    stat(sname, &fs) ;
	    if (fs.st_mtime > d->tstamp) {
	       stamp = MAX(stamp, fs.st_mtime) ;
	       strncpy(tbuf, ctime(&fs.st_mtime), 24) ;
	       if ((pwd = getpwuid(fs.st_uid)) == NULL)
		  sprintf(sname, "%4d", fs.st_uid) ;
	       else
		  strcpy(sname, pwd->pw_name) ;
	       if (d->tstamp > 0L)
		  waddch(d->win, '\n') ;
	       wprintw(d->win, "%s %10s %8ld  %s",
	       tbuf, sname, fs.st_size, fname) ;
	       rflag = TRUE ;
	       }
	    }
	 }
   d->tstamp = stamp ;
   return rflag ;
   }

int dofile (f)
   struct fl *f ;
   /*
    * Display stuff added to the file.
    */
   {
   static char buff [BUFFSZ+1] ;
   int n ;
   int rflag = FALSE ;

   while ((n = read(f->fd, buff, BUFFSZ)) > 0) {
      if (f->lc == '\n')
	 waddch(f->win, '\n') ;
      if ((f->lc = buff[n-1]) == '\n')
	 buff[n-1] = '\0' ; /* remove final nl */
      waddstr(f->win, buff) ;
      rflag = TRUE ;
      }
   return rflag || f->lc == 0 ;
   }

void quit()
   /* clean up curses stuff */
   {
   alarm(0) ;
   signal(SIGINT,  SIG_IGN) ; /* Avoid interrupts */
   signal(SIGQUIT, SIG_IGN) ; /*      while       */
   signal(SIGTERM, SIG_IGN) ; /*   cleaning up    */
   endwin() ;
   putchar('\n') ;
   exit(0) ;
   }

long line2byte (fd, lines)
   int fd, lines ;
   /*
    * Convert an EOF relative line number
    * offset to a byte offset for lseek.
    */
   {
   int lc=0, cc=0, n, off ;
   register char *p ;
   long fsize ;
   static char buff [BUFFSZ] ;
   struct stat fs ;

   fstat(fd, &fs) ;
   fsize = fs.st_size ; /* file size in bytes */

   for (off = BUFFSZ ; ; off += BUFFSZ) {
      if (off >= fsize) {
	 lseek(fd, 0L, 0) ;
	 off = fsize ;
	 }
      else
	 lseek(fd, -(long)off, 2) ;

      n = read(fd, buff, BUFFSZ) ;
      if (lc == 0 && buff[n-1] == '\n')
	 lc = -1 ; /* don't count nl at EOF */

      for (p = buff+(n-1) ; p >= buff ; p--)
	 if (*p == '\n') {
	    if ((lc += cc / COLS + 1) >= lines)
	       return off - (p-buff) - ((lc-lines) * COLS + 1) ;
	    cc = 0 ;
	    }
	 else
	    cc++ ;

      if (off >= fsize)
	 return fsize ;
      }
   }

void wakeup ()
   {
   register int i ;
   int upd = 0 ;

   for (i = 0 ; i < nf ; i++)
      if ( (file[i].isdir ? dodirect : dofile) (&file[i]) ) {
	 if (file[i].bell != ' ')
	    indicate(&file[i], ' ') ;
	 wnoutrefresh(file[i].win) ;
	 upd = 1 ;
	 }
   
   if (upd)
      doupdate() ;
   signal(SIGALRM, wakeup) ;
   alarm(interval) ;
   }
    
int num (p)
   char *p ;
   /* test if p all digits */
   {
   for (; *p ; p++)
      if (!isdigit(*p))
	 return FALSE ;
   return TRUE ;
   }

main (argc, argv)
   int argc ; char *argv[] ;
   {
   register int i, c ;
   char *p ;
   int curline, tlines=0, defs ;
   int bad = FALSE ;
   int dummy ;
   struct stat fs ;
   WINDOW *w ;
   extern int optind ;
   extern char *optarg ;

   while ((i = getopt(argc, argv, "li:")) != EOF)
      switch(i) {
	 case 'l':
	    label = 0 ;
	    break ;
	 case 'i':
	    interval = atoi(optarg) ;
	    break ;
	 case '?': /* invalid flag */
	    bad = TRUE ;
	    break ;
	    }

   if (bad || optind == argc) {
      fprintf(stderr, "\nUsage: %s [-lt] [-i n] file [lines] ...\n", argv[0]) ;
      fprintf(stderr, "\nflags:\tl - Omit window labels\n") ;
      fprintf(stderr, "\ti - Set sleep interval to n secs (default 1)\n") ;
      exit(1) ;
      }

   for (nf = 0, i = optind ; i < argc ; i++) {
      p = argv[i] ;
      if (num(p) && nf > 0)
	 file[nf-1].lines = atoi(p) ;  /* screen lines */
      else {
	 if (nf >= MAXFILES) {
	    fprintf(stderr, "Too many files (max %d)\n", MAXFILES) ;
	    exit (1) ;
	    }
	 if ((file[nf].fd = open(file[nf].name = p, O_RDONLY)) < 0) {
	    fprintf(stderr, "Couldn't open %s\n", p) ;
	    bad = TRUE ;
	    }
	 else {                       /* dir or regular file ? */
	    fstat(file[nf].fd, &fs) ;
	    if (fs.st_mode & S_IFDIR)
	       file[nf].isdir = TRUE ;
	    else if (!fs.st_mode & S_IFREG) {
	       fprintf(stderr, "Cannot seek on %s\n", p) ;
	       bad = TRUE ;
	       }
	    }
	 nf++ ;
	 }
      }
   if (bad)
      exit(1) ;

   if (getenv("TERM") == NULL) {
      fprintf(stderr, "set TERM and try again\n") ;
      exit(1) ;
      }

   /* Check allocations of screen lines and */
   /* compute leftover to be allocated to   */
   /* unspecified allocations.              */

   for (i = 0 ; i < nf ; i++) {
      tlines += file[i].lines ;
      if (file[i].lines == 0)
	 defs++ ;
      }

   initscr() ;

   for (i = 0 ; i < nf ; i++)  /* allocate screen space */
      if (file[i].lines == 0) {
	 file[i].lines = MAX((LINES-tlines) / defs, label+1) ;
	 tlines += file[i].lines ;
	 defs-- ;
	 }

   if (tlines > LINES) {
      fprintf(stderr, "Need more than %d lines (%d)\n", LINES, tlines) ;
      quit() ;
      }

   /* Open and initialize windows */

   signal(SIGINT , quit) ; /* set up             */
   signal(SIGQUIT, quit) ; /*   signals for      */
   signal(SIGTERM, quit) ; /*     graceful death */

   for (curline = 0, i = 0 ; i < nf ; i++) {
      w = file[i].win = newwin(file[i].lines, 0, curline, 0) ;
      curline += file[i].lines ;
      wsetscrreg(w, label, file[i].lines-label) ;
      if (label) {
	 wmove(w, 0, 0) ;
	 if (nf > 1)
	    wprintw(w, "%d ", i+1) ;
	 wstandout(w) ;
	 waddstr(w, file[i].name) ;
	 if (file[i].isdir && 
	    *(file[i].name + (strlen(file[i].name)-1)) != '/')
	    waddch(file[i].win, '/') ;
	 getyx(w, dummy, file[i].indpos) ;
	 waddch(file[i].win, '\n') ;
	 wstandend(w) ;
	 file[i].lines-- ;
	 }
      file[i].bell = ' ' ;
      idlok(w, TRUE) ;
      scrollok(w, TRUE) ;
      leaveok(w, TRUE) ;
      }

   /* Seek to end of files */

   for (i = 0 ; i < nf ; i++)
      if (!file[i].isdir)
	 lseek(file[i].fd, -line2byte(file[i].fd, file[i].lines), 2) ;

   /* Set up modes for input */
   
   crmode() ;
   nonl() ;
   cbreak() ;
   noecho() ;
   clear() ;

   wakeup() ;

   for (;;) {
      while ((c = getchar()) == -1 && errno == EINTR)
         ;
      switch (c) {

	 case '\f':  /* refresh */
	    alarm(0) ;
	    clearok(curscr, TRUE) ;
	    wrefresh(curscr) ;
	    alarm(interval) ;
	    break ;

	 case '0':
	    for (i = 0 ; i < nf ; i++)
	       indicate(&file[i], '*') ;
	    break ;

	 case '1':
	 case '2':
	 case '3':
	 case '4':
	 case '5':
	 case '6':
	 case '7':
	 case '8':
	 case '9':
	    c -= '0' ;
	    if (c > nf)
	       flash() ;
	    else
	       indicate(&file[c-1], '*') ;
	    break ;

	 case 'q' :   /* quit */
	 case '\4':
	    quit() ;
	 }
      }
   /*NOTREACHED*/
   }
