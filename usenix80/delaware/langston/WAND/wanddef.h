#include    <stdio.h>
/*
**      WANDDEF.H -- Non-deterministic fantasy story tool header
**          Global definitions
** Copyright (c) 1978 by Peter S. Langston - New  York,  N.Y.
*/

#define H_SCCS  "@(#)wanddef.h	1.5  last mod 4/9/80 -- (c) psl 1978"

/*      CAVEAT: Only those defines marked as "(MOD)" may be changed,
**          (also check `wandglb.c' for other modifiable parameters).
*/

		      /* numbers in [] are #bytes data space used for each */
#define MAXLOCS     512                         /* [2] max locations (MOD) */
#define MAXACTS     64    /* [10+2*MAXACTWDS+6*MAXFIELDS] acts/state (MOD) */
#define MAXFIELDS   8/* [6*(MAXACTS+MAXPREACTS+MAXPOSTACTS)] flds/act (MOD) */
#define PATHLENGTH  64               /* [5] max path length to files (MOD) */

#define MAXACTWDS   5    /* [2*(MAXACTS+MAXPREACTS+MAXPOSTACTS)] words/act */
						   /* also words/utterance */
#define MAXVARS     128         /* [2] number of variables, must be == 128 */
#define BUFSIZE     1024                /* [2+6stack] size of line buffers */
#define MAXINPNUMS  2                                 /* numbers/utterance */

#define FIELDELIM   ' '     /* delimit fields (MOD) */
#define LINEDELIM   '\n'    /* delimit lines (MOD) */
#define ESCHAR      '\\'    /* escape <bs>, <lf>, <cr>, <sp> and <ht> (MOD) */
#define VARCHAR     '%'     /* indicate variable substitution (MOD) */
#define DOTCHAR     '.'     /* make "dotted pairs" as in s=3.2 (MOD) */
#define ATCHAR      '@'     /*  "at loc pairs" as in o+card@5 (MOD) */
#define COMCHAR     ':'     /* indicate comment line (must be first char) */

#define BASESTATE   -1      /* stuff common to all states of a loc */
#define FLD1_VAR    0200    /* used in field types */
#define FLD2_VAR    0100    /* used in field types */
#define TYPEONLY    0077    /* used to strip previos two from type */
#define NO_WORD     0       /* indicates no word specified (in acts) */

        /* return codes from carry_out() and check_act() */
#define COM_UNREC   0       /* command not recognized */
#define COM_RECOG   1       /* command recognized */
#define COM_DONE    2       /* command successful */
#define COM_DESC    4       /* command needs new prloc() */
#define COM_NDOBJ   8       /* command needed an object to match */
#define COM_COMPLETE 16     /* command needs no further attention */

        /* quit codes (all locs < 0 imply quitting) */
#define QUIT_SCORE  -1
#define QUIT_QUIET  -2

	/* special variables, must agree with `spvars' in wandglb.c */
#define CUR_LOC     100     /* current location */
#define PREV_LOC    101     /* previous location */
#define INP_W1      102     /* hash of first recognized word in inp comm */
#define INP_W2      103     /* hash of second recog word from inp comm */
#define INP_W3      104     /* hash of third recog word from inp comm */
#define INP_W4      105     /* hash of fourth recog word from inp comm */
#define INP_W5      106     /* hash of fifth recog word from inp comm */
#define INP_WC      107     /* number of words in input comm */
#define NUM_CARRY   108     /* # of things being carried */
#define MAX_CARRY   109     /* # of thing poss. to carry at once */
#define NOW_YEAR    110     /* year of decade (0:99) */
#define NOW_MONTH   111     /* month of year (1:12) */
#define NOW_DOM     112     /* day of month (1:31) */
#define NOW_DOW     113     /* day of week (0:6) */
#define NOW_HOUR    114     /* hour of day (0:23) */
#define NOW_MIN     115     /* minute of hour (0:59) */
#define NOW_SEC     116     /* second of minute (0:59) */
#define NOW_ET      117     /* elapsed time in Wander (seconds) */
#define BREVITY     118     /* brevity of place descriptions */
#define LOC_VIEW    119     /* location description override */
#define OBJ_VIEW    120     /* object description override */
#define INP_N1      121     /* numeric value of first number from inp comm */
#define INP_N2      122     /* numeric value of first number from inp comm */
#define NUM_MOVES   123     /* number of "moves" */
#define NUM_PLACES  124     /* number of "places" visited */


        /* field types */
#define F_VOID      0
#define FT_OBJ      1
#define FT_NOBJ     2
#define FT_TOOL     3
#define FT_NTOOL    4
#define FT_STATE    5
#define FT_NSTATE   6
#define FT_EVAR     7
#define FT_NVAR     8
#define FT_GVAR     9
#define FT_LVAR     10
#define FT_ODDS     11
#define FT_EBIN     12
#define FT_NBIN     13
#define FT_GBIN     14
#define FT_LBIN     15

#define FR_GOBJ     20
#define FR_LOBJ     21
#define FR_GTOOL    22
#define FR_LTOOL    23
#define FR_SSTATE   24
#define FR_ISTATE   25
#define FR_DSTATE   26
#define FR_SVAR     27
#define FR_IVAR     28
#define FR_DVAR     29
#define FR_MVAR     30
#define FR_QVAR     31
#define FR_CSUB     32
#define FR_WORLD    33
#define FR_SBIN     34
#define FR_IBIN     35
#define FR_DBIN     36

struct  paramstr {
	int     p_pathlength;
	int     p_maxlocs;
	int     p_maxwrds;
	int     p_maxvars;
	int     p_maxindex;
	int     p_maxpre;
	int     p_maxpost;
	int     p_stbuf;
	int     p_stbp;
} param;

struct  placestr {
	int     p_loc;
	char    p_state;
	long    p_sdesc;             /* text address for short description */
	long    p_ldesc;              /* text address for long description */
	struct  actstr {
		int     a_wrd[MAXACTWDS];                 /* command words */
		int     a_rloc;                         /* result location */
		char    a_rcont;       /* >0 ==> continue to other actions */
		struct  fieldstr {
			char    f_type;
			int     f_fld1;
			int     f_fld2;
		} a_field[MAXFIELDS];   /* tests & results */
		int     a_msgfp;          /* file pointer for message text */
		long    a_msgaddr;          /* result message text address */
	} p_acts[MAXACTS];
};

	/* codes for w_flg */
#define W_SING      1   /* object is singular */
#define W_PLUR      2   /* object is plural */
#define W_NOART     4   /* object contains own article */
#define W_ASIS      8   /* object contains whole phrase */
#define W_DONLY     16  /* this form only in descript, not for carrying */

struct  wrdstr {
	char    *w_word;                        /* pointer to text of word */
	char    w_syn;                                   /* synonym offset */
	char    w_flg;             /* plural? need article? descript only? */
	int     w_loc;            /* where it is; 0=>nowhere, -1=>carrying */
};

struct  indexstr {
	int  i_loc;
        char i_state;
        long i_addr;
};

extern  struct  indexstr    index[];
extern  struct  placestr    place;
extern  struct  actstr  pre_acts[], post_acts[];
extern  struct  wrdstr wrds[], spvars[];
extern  char    *thereis[], *aansome[];
extern  char    fldels[], vardel[], wrdels[];
extern  char    listunused[];
extern  char    locfile[], miscfile[], tmonfil[], monfile[];
extern  char    curfile[], newfile[], *stdpath, *defmfile;
extern  char    mfbuf[], wfbuf[];
extern  int     maxwrds, maxactwds, pathlength, maxinpwd, maxlocs, maxindex;
extern  int     maxacts, maxpreacts, maxpostacts, maxfields, maxvars;
extern  int     ldescfreq;
extern  char    fieldelim, linedelim;
extern  char    eschar, varchar, dotchar, atchar, comchar;
extern  int     monitor, monloc, monstate;
extern  int     max_carry;
extern  char    inwrd[][32];
extern  char    locseen[], locstate[];
extern  int     var[];

