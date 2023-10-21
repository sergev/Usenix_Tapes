# include       "sps.h"

/* Read Only variables, global to the code of sps ... */

/* Null ttyline device ... */
struct ttyline                  Notty = { 0, 0, "  ", 0 } ;

/*
** The symbol table. For each address read from the kernel during
** initialisation, this table shows the following:
**      i.   the name of that symbol within the kernel ;
**      ii.  whether an extra indirection is needed through the kernel,
**           i.e. whether the value of that symbol should be obtained
**           rather than its address.
**      iii. where the obtained value/address is placed in the Info structure ;
**      iv.  whether the obtained value is associated with a reason for
**           a process wait state.
*/
/* The order of entries in this table is unimportant. */

extern struct info              Info ;

struct symbol   Symbollist[] =
{       
	/* Kernel addresses required in order to access process,
	   tty and upage information. All these addresses should be
	   located in the symbol file during initialisation. */
	{ "_proc",      1,  (caddr_t*)&Info.i_proc0,    (char*)0        },
	{ "_nproc",     1,  (caddr_t*)&Info.i_nproc,    (char*)0        },
	{ "_text",      1,  (caddr_t*)&Info.i_text0,    (char*)0        },
	{ "_ntext",     1,  (caddr_t*)&Info.i_ntext,    (char*)0        },
	{ "_inode",     1,  (caddr_t*)&Info.i_inode0,   (char*)0        },
	{ "_ninode",    1,  (caddr_t*)&Info.i_ninode,   (char*)0        },
	{ "_swbuf",     1,  (caddr_t*)&Info.i_swbuf0,   (char*)0        },
	{ "_nswbuf",    1,  (caddr_t*)&Info.i_nswbuf,   (char*)0        },
	{ "_buf",       1,  (caddr_t*)&Info.i_buf0,     (char*)0        },
	{ "_nbuf",      1,  (caddr_t*)&Info.i_nbuf,     (char*)0        },
	{ "_ecmx",      1,  (caddr_t*)&Info.i_ecmx,     (char*)0        },
	{ "_Usrptmap",  0,  (caddr_t*)&Info.i_usrptmap, (char*)0        },
	{ "_usrpt",     0,  (caddr_t*)&Info.i_usrpt,    (char*)0        },
	{ "_cdevsw",    0,  (caddr_t*)&Info.i_cdevsw,   (char*)0        },
# ifdef BSD42
	{ "_quota",     1,  (caddr_t*)&Info.i_quota0,   (char*)0        },
	{ "_nquota",    1,  (caddr_t*)&Info.i_nquota,   (char*)0        },
	{ "_dmmin",     1,  (caddr_t*)&Info.i_dmmin,    (char*)0        },
	{ "_dmmax",     1,  (caddr_t*)&Info.i_dmmax,    (char*)0        },
	{ "_mbutl",     0,  (caddr_t*)&Info.i_mbutl,    (char*)0        },
# else
	{ "_hz",        1,  (caddr_t*)&Info.i_hz,       (char*)0        },
# endif
# ifdef CHAOS
	{ "_Chconntab", 0,  &Info.i_Chconntab,          (char*)0        },
# endif
	/* Kernel addresses associated with process wait states.
	   It is not important if some of these addresses are unresolved
	   at initialisation. */
	{ "_fltab",     0,  &Info.i_waitstate[0],       "floppy"        },
	{ "_tu",        0,  &Info.i_waitstate[1],       "tu58"          },
	{ "_bfreelist", 0,  &Info.i_waitstate[2],       "buffer"        },
	{ "_lp_softc",  0,  &Info.i_waitstate[3],       "printr"        },
	{ "_lbolt",     0,  &Info.i_waitstate[4],       "lbolt"         },
	{ "_runin",     0,  &Info.i_waitstate[5],       "runin"         },
	{ "_runout",    0,  &Info.i_waitstate[6],       "runout"        },
	{ "_ipc",       0,  &Info.i_waitstate[7],       "ptrace"        },
	{ "_u",         0,  &Info.i_waitstate[8],       "pause"         },
	{ "_freemem",   0,  &Info.i_waitstate[9],       "freemm"        },
	{ "_kernelmap", 0,  &Info.i_waitstate[10],      "kermap"        },
	{ "_cwaiting",  0,  &Info.i_waitstate[11],      "cwait"         },
	{ "_rhpbuf",    0,  &Info.i_waitstate[12],      "rhpbuf"        },
	{ "_rhtbuf",    0,  &Info.i_waitstate[13],      "rhtbuf"        },
	{ "_ridcbuf",   0,  &Info.i_waitstate[14],      "ridcbf"        },
	{ "_rikbuf",    0,  &Info.i_waitstate[15],      "rikbuf"        },
	{ "_rmtbuf",    0,  &Info.i_waitstate[16],      "rmtbuf"        },
	{ "_rrkbuf",    0,  &Info.i_waitstate[17],      "rrkbuf"        },
	{ "_rrlbuf",    0,  &Info.i_waitstate[18],      "rrlbuf"        },
	{ "_rrxbuf",    0,  &Info.i_waitstate[19],      "rrxbuf"        },
	{ "_rswbuf",    0,  &Info.i_waitstate[20],      "rswbuf"        },
	{ "_rtmbuf",    0,  &Info.i_waitstate[21],      "rtmbuf"        },
	{ "_rtsbuf",    0,  &Info.i_waitstate[22],      "rtsbuf"        },
	{ "_rudbuf",    0,  &Info.i_waitstate[23],      "rudbuf"        },
	{ "_rupbuf",    0,  &Info.i_waitstate[24],      "rupbuf"        },
	{ "_rutbuf",    0,  &Info.i_waitstate[25],      "rutbuf"        },
	{ "_rvabuf",    0,  &Info.i_waitstate[26],      "rvabuf"        },
	{ "_rvpbuf",    0,  &Info.i_waitstate[27],      "rvpbuf"        },
	{ "_chtbuf",    0,  &Info.i_waitstate[28],      "chtbuf"        },
	{ "_cmtbuf",    0,  &Info.i_waitstate[29],      "cmtbuf"        },
	{ "_ctmbuf",    0,  &Info.i_waitstate[30],      "ctmbuf"        },
	{ "_ctsbuf",    0,  &Info.i_waitstate[31],      "ctsbuf"        },
	{ "_cutbuf",    0,  &Info.i_waitstate[32],      "cutbuf"        },
# ifdef BSD42
	{ "_selwait",   0,  &Info.i_waitstate[33],      "select"        },
# endif
# ifdef CHAOS
	{ "_Chrfclist", 0,  &Info.i_waitstate[34],      "chrfc"         },
# endif
# ifdef SUN
	{ "_async_bufhead", 0,  &Info.i_waitstate[35],  "async"		},
# endif
	{ (char*)0,     0,  (caddr_t*)0,                (char*)0        }
} ;
