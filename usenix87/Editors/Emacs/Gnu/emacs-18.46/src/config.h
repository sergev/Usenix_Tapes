/* GNU Emacs site configuration template file.
   Copyright (C) 1986 Free Software Foundation, Inc.

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



/* Include here a s- file that describes the system type you are using.
   See the file ../etc/MACHINES for a list of systems and
   the names of the s- files to use for them.
   See s-template.h for documentation on writing s- files.  */
#include "s-usg5-2-2.h"
 
/* Include here a m- file that describes the machine and system you use.
   See the file ../etc/MACHINES for a list of machines and
   the names of the m- files to use for them.
   See m-template.h for info on what m- files should define.
   */
#include "m-arete.h"

/* Load in the conversion definitions if this system
   needs them and the source file being compiled has not
   said to inhibit this.  */

#ifdef SHORTNAMES
#ifndef NO_SHORTNAMES
#include "../shortnames/remap.h"
#endif /* not NO_SHORTNAMES */
#endif /* SHORTNAMES */

/* define HAVE_X_WINDOWS if you want to use the X window system */

/* #define HAVE_X_WINDOWS */

/* define HAVE_X_MENU if you want to use the X window menu system.
   This appears to work on some machines that support X
   and not on others.  */

/* #define HAVE_X_MENU */

/* subprocesses should be defined if you want to
 have code for asynchronous subprocesses
 (as used in M-x compile and M-x shell).
 These do not work for some USG systems yet;
 for the ones where they work, the s-*.h file defines this flag.  */

#ifndef VMS
#ifndef USG
#define subprocesses
#endif
#endif

/* Define USER_FULL_NAME to return a string
 that is the user's full name.
 It can assume that the variable `pw'
 points to the password file entry for this user.

 At some sites, the pw_gecos field contains
 the user's full name.  If neither this nor any other
 field contains the right thing, use pw_name,
 giving the user's login name, since that is better than nothing.  */

#define USER_FULL_NAME pw->pw_gecos

/* Define AMPERSAND_FULL_NAME if you use the convention
  that & in the full name stands for the login id.  */

/* #define AMPERSAND_FULL_NAME */

/* Maximum screen width we handle. */

#define MScreenWidth 300

/* Maximum screen length we handle. */

#define MScreenLength 300

/* # bytes of pure Lisp code to leave space for.
  s-vms.h and m-sun2.h can override this default.  */

#ifndef PURESIZE
#ifdef HAVE_X_WINDOWS
#define PURESIZE 120000
#else
#define PURESIZE 117000
#endif
#endif

/* Define HIGHPRI as a negative number
 if you want Emacs to run at a higher than normal priority.
 For this to take effect, you must install it as setuid root. */

/* #define HIGHPRI */

