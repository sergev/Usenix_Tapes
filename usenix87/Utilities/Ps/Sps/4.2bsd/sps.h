# include       <h/param.h>
# include       <h/dir.h>
# include       <h/user.h>
# include       <h/proc.h>

/*
** Maximum # of users to be considered. (Should probably be
** approximately double the # of users in /etc/passwd.)
*/
# define        MAXUSERID       100
/* Maximum # ttys to be considered ... */
# define        MAXTTYS         60
/* Maximum user name length ... */
# define        UNAMELEN        8
/* Maximum process-id not to be considered busy ... */
# define        MSPID           2
/* # of wait states defined in the `struct info' ... */
# define        NWAITSTATE      36

/* Convert clicks to kbytes ... */
# ifdef SUN
# define        KBYTES( size )  ((size) << 1)
# else
# define        KBYTES( size )  ((size) >> (10 - PGSHIFT))
# endif

/* Standard files to be examined ... */
# define        FILE_MEM        "/dev/mem"      /* System physical memory */
# define        FILE_KMEM       "/dev/kmem"     /* Kernel virtual memory */
# define        FILE_SWAP       "/dev/drum"     /* Swap/paging device */
# define        FILE_DEV        "/dev"          /* Directory of tty entries */
# define        FILE_SYMBOL     "/vmunix"       /* Symbol file for nlist() */
# define        FILE_INFO       "/etc/spsinfo"  /* Sps information file */

/* Structure to hold necessary information concerning a tty ... */
struct ttyline
{
	struct tty              *l_addr ;       /* Ptr to tty struct in kmem */
	unsigned short          l_pgrp ;        /* Tty process group */
	char                    l_name[2] ;     /* Tty character name */
	dev_t                   l_dev ;         /* Tty device # */
} ;

/* Structure holding a single hash table entry ... */
struct hashtab
{
	unsigned short          h_uid ;         /* Uid of user entry */
	char                    h_uname[ UNAMELEN ] ; /* Corresponding name */
} ;

/*
** Format of the standard information file maintained by sps.
** This structure is filled in at initialisation time and then is read back
** in whenever sps is invoked.
** Note that the pointer variables in this structure refer to
** kernel virtual addresses, not addresses within sps.
** These variable are typed as such so that pointer arithmetic
** on the kernel addresses will work correctly.
*/
struct info
{       /* Kernel values determining process, tty and upage info ... */
	struct proc             *i_proc0 ;      /* address of process table */
	int                     i_nproc ;       /* length of process table */
	struct text             *i_text0 ;      /* address of text table */
	int                     i_ntext ;       /* length of text table */
	struct inode            *i_inode0 ;     /* address of inode table */
	int                     i_ninode ;      /* length of inode table */
	struct buf              *i_swbuf0 ;     /* address of swap buffers */
	int                     i_nswbuf ;      /* # swap buffers */
	struct buf              *i_buf0 ;       /* address of i/o buffers */
	int                     i_nbuf ;        /* # i/o buffers */
	int                     i_ecmx ;        /* max physical memory address*/
	struct pte              *i_usrptmap ;   /* page table map */
	struct pte              *i_usrpt ;      /* page table map */
	struct cdevsw           *i_cdevsw ;     /* device switch to find ttys */
# ifdef BSD42
	struct quota            *i_quota0 ;     /* disc quota structures */
	int                     i_nquota ;      /* # quota structures */
	int                     i_dmmin ;       /* The start of the disc map */
	int                     i_dmmax ;       /* The end of the disc map */
	struct mbuf             *i_mbutl ;      /* Start of mbuf area */
# else
	int                     i_hz ;          /* Clock rate */
# endif
# ifdef CHAOS
	caddr_t                 i_Chconntab ;   /* Chaos connection table */
# endif
	/* Kernel addresses are associated with process wait states ... */
	caddr_t                 i_waitstate[ NWAITSTATE ] ;
	/* User names, stored in a hash table ... */
	struct hashtab          i_hnames[ MAXUSERID ] ;
	/* Tty device info ... */
	struct ttyline          i_ttyline[ MAXTTYS ] ;
} ;

/*
** The symbol structure cross-references values read from the kernel with
** their place in the info structure, and if such a value is associated with
** a process wait state or not.
*/
struct symbol
{
	char                    *s_kname ;      /* Kernel symbol name */
	char                    s_indirect ;    /* Value requires indirection */
	caddr_t                 *s_info ;       /* Corresponding info address */
	char                    *s_wait ;       /* Reason for wait, if any */
} ;

/* The `user' structure obtained from /dev/mem or /dev/swap ... */
union userstate
{
	struct user             u_us ;
	char                    u_pg[ UPAGES ][ NBPG ] ;
} ;

/* Information concerning each process filled from /dev/kmem ... */
struct process
{
	struct proc             pr_p ;          /* struct proc from /dev/kmem */
	struct process          *pr_plink ;     /* Normalised ptrs from above */
	struct process          *pr_sibling ;   /* Ptr to sibling process */
	struct process          *pr_child ;     /* Ptr to child process */
	struct process          *pr_pptr ;      /* Ptr to parent process */
# ifdef BSD42
	struct rusage           pr_rself ;      /* Read from upage for self */
	struct rusage           pr_rchild ;     /* ... and the children */
# else
	struct vtimes           pr_vself ;      /* Read from upage for self */
	struct vtimes           pr_vchild ;     /* ... and the children */
# endif
	int                     pr_files ;      /* # open files */
	struct ttyline          *pr_tty ;       /* Associated tty information */
	char                    *pr_cmd ;       /* Command args, from upage */
	int                     pr_upag:1 ;     /* Upage was obtained */
	int                     pr_csaved:1 ;   /* Cmd args saved by malloc() */
} ;

/* Structure to hold summarising information ... */
struct summary
{
	long                    sm_ntotal ;     /* Total # processes */
	long                    sm_ktotal ;     /* Total virtual memory */
	long                    sm_nbusy ;      /* # busy processes */
	long                    sm_kbusy ;      /* Busy virtual memory */
	long                    sm_nloaded ;    /* # loaded processes */
	long                    sm_kloaded ;    /* Active resident memory */
	long                    sm_nswapped ;   /* # swapped processes */
	long                    sm_kswapped ;   /* Size totally swapped out */
} ;
