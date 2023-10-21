#ifndef MASDOS_INCLUDED
#define MASDOS_INCLUDED

/*
*
* D_ debuging
* A_ attributes for files
* P_ print characteristics
* Q_ characters
* B_ BDOS interupt parameters
*
*/

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define abs(a)		(((a) < 0) ? -(a) : (a))

#ifdef DEBUGGING

#define D_IN(p) fprintf(stderr,"(%s) In\n",p)
#define D_OUT(p) fprintf(stderr,"(%s) Out\n",p)
#define D_PRT(p,q) fprintf(stderr,"(%s) %s\n",p,q)

#define D_S(p,q,r) fprintf(stderr,"(%s) %s %s\n",p,q,r)
#define D_I(p,q,r) fprintf(stderr,"(%s) %s %d\n",p,q,r)

#else

#define D_IN(p)
#define D_OUT(p)
#define D_PRT(p,q)

#define D_S(p,q,r)
#define D_I(p,q,r)

#endif

#define NNULL 1

struct FILEINFO
{
        unsigned char    fi_resv[21];                /* Reserved by DOS */
        unsigned char    fi_attrib;                  /* File Attributes */
        unsigned int     fi_time;                    /* Create/Update time */
        unsigned int     fi_date;                    /* Create/Update date */
        unsigned long    fi_fsize;                   /* File Size (bytes) */
        char             fi_name[13];                /* File Name & Extension */
};

#define A_READONLY      0x01
#define A_HIDDEN        0x02
#define A_SYSTEM        0x04
#define A_LABEL         0x08
#define A_SUBDIR        0x10
#define A_DIRTY         0x20

#define A_ALL           0xff
#define A_ALL_FILES     (A_HIDDEN&A_SYSTEM)
#define A_NORM_FILES    0x00

#define IS_READONLY(p)  ((p)->fi_attrib & A_READONLY)
#define IS_HIDDEN(p)    ((p)->fi_attrib & A_HIDDEN)
#define IS_SYSTEM(p)    ((p)->fi_attrib & A_SYSTEM)
#define IS_LABEL(p)     ((p)->fi_attrib & A_LABEL)
#define IS_SUBDIR(p)    ((p)->fi_attrib & A_SUBDIR)
#define IS_DIRTY(p)     ((p)->fi_attrib & A_DIRTY)

#define C_HOUR(p)       (((p)->fi_time >> 11) & 0x1f)
#define C_MIN(p)        (((p)->fi_time >> 5) & 0x3f)
#define C_SEC(p)        (((p)->fi_time << 1) & 0x3e)
#define C_YEAR(p)       ((((p)->fi_date >> 9) & 0x7f) + 1980)
#define C_MONTH(p)      (((p)->fi_date >> 5) & 0x0f)
#define C_DAY(p)        (((p)->fi_date) & 0x1f)

#define Q_ESC           0x1b

#define P_ALL_OFF       "\033[0m"
#define P_BOLDFACE      "\033[1m"
#define P_UNDERLINE     "\033[4m"
#define P_BLINKING      "\033[5m"
#define P_REVERSE       "\033[7m"

#define I_BDOS			0x21

#define B_PRGTRM        0x0000
#define B_KBDINP        0x0100
#define B_DSPOUT        0x0200
#define B_AUXINP        0x0300
#define B_AUXOUT        0x0400
#define B_PRNOUT        0x0500
#define B_CONIO         0x0600
#define B_CONDIN        0x0700
#define B_CONINN        0x0800
#define B_PRTSTR        0x0900
#define B_KBDBIN        0x0a00

#define B_SEL_DISK      0x0e00

#define B_SETDTA        0x1a00

#define B_GETDTA        0x2f00

#define B_IOCTL			0x4400

#define B_FINDFIRST     0x4e00
#define B_FINDNEXT      0x4f00

#ifdef LINT_ARGS

int		stoi(char **);
int		ptext( int, char **, int, int, int );
void	ssort( char *, int, int, int (*)() );
int		toscreen();

#else

extern	int		stoi();
extern	int		ptext();
extern	void	ssort();
extern	int		toscreen();

#endif	/* LINT_ARGS */

#endif /* MASDOS_INCLUDED */

