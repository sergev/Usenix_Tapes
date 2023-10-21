/* The default search path for Lisp function "load".
   This sets load-path.  */
#define PATH_LOADSEARCH "/usr/spool/oldnews/Gnu/emacs-18.46/lisp"

/* the extra search path for programs to invoke.
 This is appended to whatever the PATH environment variable says
 to set the Lisp variable exec-path and the first file namein it
  sets the Lisp variable exec-directory.  */
#define PATH_EXEC "/usr/spool/oldnews/Gnu/emacs-18.46/etc"

/* the name of the directory that contains lock files
 with which we record what files are being modified in Emacs.
 This directory should be writable by everyone.
 THE STRING MUST END WITH A SLASH!!!  */
#define PATH_LOCK "/usr/local/lib/gemacs/lock/"

/* the name of the file !!!SuperLock!!! in the directory
 specified by PATH_LOCK.  Yes, this is redundant.  */
#define PATH_SUPERLOCK "/usr/local/lib/gemacs/lock/!!!SuperLock!!!"

