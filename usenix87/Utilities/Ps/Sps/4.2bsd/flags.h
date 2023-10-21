/* Structure holding information specified in the option list ... */
union flaglist
{
	char                    *f_chp ;        /* Option specified as string */
	int                     f_uid ;         /* Numerical user id */
	int                     f_pid ;         /* Numerical process id */
	struct ttyline          *f_ttyline ;    /* Specified tty */
} ;

/* Structure holding global information specifed by arg list options ... */
struct flags
{
	int                     flg_d:1 ;       /* disc orientated output */
	int                     flg_e:1 ;       /* print environment string */
	int                     flg_f:1 ;       /* print process father # */
	int                     flg_g:1 ;       /* print process group # */
	int                     flg_i:1 ;       /* initialise sps */
	char                    *flg_j ;        /* Use this as the info file */
	char                    *flg_k ;        /* Use this as the {k}mem file*/
	int                     flg_o:1 ;       /* avoid the swap device */
	int                     flg_q:1 ;       /* show user time only */
	int                     flg_r:1 ;       /* repeat output */
	unsigned                flg_rdelay ;    /* ... with this much delay */
	char                    *flg_s ;        /* Use this as the symbol file*/
	int                     flg_v:1 ;       /* print verbose listing */
	int                     flg_w:1 ;       /* print wide output */
	int                     flg_y:1 ;       /* print tty information */
	int                     flg_A:1 ;       /* print all processes */
	int                     flg_B:1 ;       /* print busy processes */
	int                     flg_F:1 ;       /* print foreground processes */
	int                     flg_N:1 ;       /* print no processes */
	int                     flg_P:1 ;       /* print specified process #'s*/
	int                     flg_S:1 ;       /* print stopped processes */
	int                     flg_T:1 ;       /* print procs for given ttys */
	int                     flg_U:1 ;       /* print procs for given users*/
	int                     flg_W:1 ;       /* print waiting processes */
	int                     flg_Z:1 ;       /* print zombie processes */
	int                     flg_AZ:1 ;      /* One of A to Z was specified*/
	union flaglist          *flg_Plist ;    /* List of specified processes*/
	union flaglist          *flg_Tlist ;    /* List of specified ttys */
	union flaglist          *flg_Ulist ;    /* List of specified users */
} ;
