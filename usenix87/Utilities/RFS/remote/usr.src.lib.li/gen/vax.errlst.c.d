These changes being installed should be just fine if you were able to make
the earlier changes to h/errno.h.  Otherwise, you must install these changes
by hand.
***************
*** 81,85
  	"Too many processes",			/* 67 - EPROCLIM */
  	"Too many users",			/* 68 - EUSERS */
  	"Disc quota exceeded",			/* 69 - EDQUOT */
  };
  int	sys_nerr = { sizeof sys_errlist/sizeof sys_errlist[0] };

--- 81,88 -----
  	"Too many processes",			/* 67 - EPROCLIM */
  	"Too many users",			/* 68 - EUSERS */
  	"Disc quota exceeded",			/* 69 - EDQUOT */
+ 	"File is on remote host",		/* 70 - EISREMOTE */
+ 	"Too many remote hosts defined",	/* 71 - ETOOMANYREMOTE */
+ 	"No remote file system",		/* 72 - ENOREMOTEFS */
  };
  int	sys_nerr = { sizeof sys_errlist/sizeof sys_errlist[0] };
