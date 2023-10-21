/* #define's which may require local modification */
/* 
 * The macros TP_DEN[LNH] define the names of the low-density,
 * normal-density and high-density tape devices, as double-strings.
 * The first string indicates the read name, the second the write name;
 * a ? is replaced by the unit number.
 */
#define	TP_DENL	"/dev/rmt0\0/dev/nrmt0"
#define	TP_DENN	TP_DENL
#define	TP_DENH	"/dev/rmt8\0/dev/nrmt8"

#define	TP_MAXB	32766	/* largest size parameter to `read' & `write' */
#define	_TP_MOPEN 2	/* maximum number of simultaneously open tape-files */
