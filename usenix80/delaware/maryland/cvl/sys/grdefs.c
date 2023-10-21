#

#define	WID	0000000
#define	LSM	0010000
#define	WGD	0020000
#define	WAC	0022000
#define	LWM	0024000
#define	LIGHT	0000200
#define	ADDITV	0000100
#define	ZEROW	0000040
#define	VECTOR	0000020
#define	DHGHT	0000010
#define	DWDTH	0000004
#define	CURPOS	0000002
#define	VCURSOR	0000001
#define	LUM	0026000
#define	EC	0000001
#define	EBA	0000002
#define	ECA	0000003
#define	LC	0000004
#define	LBA	0000010
#define	LCA	0000014
#define	SHOME	0000020
#define	SDOWN	0000040
#define	SUP	0000060
#define	ERS	0030000
#define	ERL	0032000
#define	SLU	0034000
#define	SINHBT	0000100
#define	EGW	0036000
#define	GWRITE	0002000
#define	LER	0040000
#define	LEA	0044000
#define	LEB	0050000
#define	LEC	0054000
#define	LLR	0060000
#define	LLA	0064000
#define	LLB	0070000
#define	LLC	0074000
#define	LDC	0100000
#define	IMAGECH	0000003
#define	OVERLAY	0000004
#define	NOP	0110000
#define	SPD	0120000
#define	LPA	0130000
#define	LPR	0140000
#define	LPD	0150000
#define	RPD	0160000
#define	NON	0170000
#define	DIGITZ	0120002
#define	CNTUOUS	0004000
#define	CURSOR	0120004
#define CWHITE  0144000
#define	ABSMOV	0154000
#define	VCNTRL	0120020
#define	LOOKUP	0120040
#define	MEMORY	0120400
#define	UNPACK	0121000
#define	SWTBYT	0001000
#define	ODDBYT	0000400
#define	IBYTES	0141000
#define	GBYTES	0145000
#define	ABYTES	0147000
#define	ITESTS	0124000

#define	ALL9	0000777
#define	ALL12	0007777

struct  grin {
int     nfil;   /* NON - logical file number */
int     frow;   /* LLA starting row */
int     fcol;   /* LEA starting column */
int     nrow;   /* LLC - length row */
int     ncol;   /* LEC - length column */
int     irow;   /* LLB-2 row increment */
int     icol;   /* LEB+2 column increment */
int     crow;   /* LLR current row */
int     ccol;   /* LER current column */
int     chan;   /* LDC|IMAGECH channels
int     wrmd;   /* LWM|DHGHT|DWDTH|ZEROW|VCURSOR write mode */
int     updm;   /* LUM|EBA update mode */
int     scha;   /* LSM|ALL12 subchannels */
int     peri;   /* SPD|MEMORY peripherals */
int     uloc;   /* SLU|SHOME|SINHBT */
int     rdbk;   /* RPD */
};
