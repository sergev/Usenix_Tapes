/* Examine the result of  stat  and make a string describing file modes.
   Copyright (C) 1985, 1987 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


#include <sys/types.h>
#include <sys/stat.h>

/* filemodestring - set file attribute data 

*** WARNING!  FILE STRUCTURE DEPENDENT ***

   Filemodestring converts the data in the st_mode field of file status
block `s' to a 10 character attribute string, which it stores in
the block that `a' points to.
This attribute string is modelled after the string produced by the Berkeley ls.

As usual under Unix, the elements of the string are numbered
from 0.  Their meanings are:

   0	File type.  'd' for directory, 'c' for character
	special, 'b' for block special, 'm' for multiplex,
	'l' for symbolic link, 's' for socket, 'p' for fifo,
	'-' for any other file type

   1	'r' if the owner may read, '-' otherwise.

   2	'w' if the owner may write, '-' otherwise.

   3	'x' if the owner may execute, 's' if the file is
	set-user-id, '-' otherwise.
	'S' if the file is set-user-id, but the execute
	bit isn't set.  (sys v `feature' which helps to
	catch screw case.)

   4	'r' if group members may read, '-' otherwise.

   5	'w' if group members may write, '-' otherwise.

   6	'x' if group members may execute, 's' if the file is
	set-group-id, '-' otherwise.
	'S' if it is set-group-id but not executable.

   7	'r' if any user may read, '-' otherwise.

   8	'w' if any user may write, '-' otherwise.

   9	'x' if any user may execute, 't' if the file is "sticky"
	(will be retained in swap space after execution), '-'
	otherwise.

 */

#define VOID void

static char ftypelet ();
static VOID rwx (), setst ();

VOID
filemodestring (s,a)
   struct stat	*s;
   char *a;
{
   a[0] = ftypelet (s);
   /* Aren't there symbolic names for these byte-fields? */
   rwx ((s->st_mode & 0700) << 0, &(a[1]));
   rwx ((s->st_mode & 0070) << 3, &(a[4]));
   rwx ((s->st_mode & 0007) << 6, &(a[7]));
   setst (s->st_mode, a);
}

/* ftypelet - file type letter

*** WARNING!  FILE STRUCTURE DEPENDENT ***

   Ftypelet accepts a file status block and returns a character
code describing the type of the file.  'd' is returned for
directories, 'b' for block special files, 'c' for character
special files, 'm' for multiplexor files, 'l' for symbolic link,
's' for socket, 'p' for fifo, '-' for any other file type
 */

static char
ftypelet(s)
   struct stat *s;
{
  switch (s->st_mode & S_IFMT)
    {
    default:
      return '-';
    case S_IFDIR:
      return 'd';
#ifdef S_IFLNK
    case S_IFLNK:
      return 'l';
#endif
#ifdef S_IFCHR
    case S_IFCHR:
      return 'c';
#endif
#ifdef S_IFBLK
    case S_IFBLK:
      return 'b';
#endif
#ifdef S_IFMPC
/* These do not seem to exist */
    case S_IFMPC:
    case S_IFMPB:
      return 'm';
#endif
#ifdef S_IFSOCK
    case S_IFSOCK:
      return 's';
#endif
#ifdef S_IFIFO
    case S_IFIFO:
      return 'p';
#endif
#ifdef S_IFNWK /* hp-ux hack */
    case S_IFNWK:
      return 'n';
#endif
    }
}


/* rwx - look at read, write, and execute bits and set character
flags accordingly

*** WARNING!  FILE STRUCTURE DEPENDENT ***

 */

static VOID
rwx (bits, chars)
   unsigned short bits;
   char chars[];
{
  chars[0] = (bits & S_IREAD)  ? 'r' : '-';
  chars[1] = (bits & S_IWRITE) ? 'w' : '-';
  chars[2] = (bits & S_IEXEC)  ? 'x' : '-';
}


/* setst - set s & t flags in a file attributes string */
/* *** WARNING!  FILE STRUCTURE DEPENDENT *** */
static VOID
setst (bits, chars)
   unsigned short bits;
   char chars[];
{
#ifdef S_ISUID
   if (bits & S_ISUID)
     {
       if (chars[3] != 'x')
	 /* Screw case: set-uid, but not executable. */
	 chars[3] = 'S';
       else
	 chars[3] = 's';
     }
#endif
#ifdef S_ISGID
   if (bits & S_ISGID)
     {
       if (chars[6] != 'x')
	 /* Screw case: set-gid, but not executable. */
	 chars[6] = 'S';
       else
	 chars[6] = 's';
     }
#endif
#ifdef S_ISVTX
   if (bits & S_ISVTX)
      chars[9] = 't';
#endif
}
