#

/*
 *      mx definitions
 */


#define LHOST   2               /* local host number */
#define NHOST   5               /* number of hosts */
#define TASKREQ 01000           /* mitask program interrupt request */
#define PS      0177776         /* processor status address */
#define PIR     0177772         /* program interrupt request addr */
#define PDSIZE  512             /* packet data size */
#define PSTART  (sizeof (struct linebuf)-sizeof (struct head)-sizeof (struct pack))
				/* displacement in linebuf to start xm/rc */
#define PSIZE   (sizeof (struct head)+sizeof (struct pack))       /* bytes to xm/rc */


struct  { int integ;};
typedef unsigned pbn;           /* physical memory block number */
int     *ka5;                   /* access to extended buffer pool */


/*
 * host to host packet
 */
struct pack {
	char    p_shost;        /* src host */
	char    p_ssoc;         /* src socket */
	char    p_dhost;        /* dst host */
	char    p_dsoc;         /* dst socket */
	char    p_hostc;        /* host control code */
	char    p_impc;         /* imp control code */
	int     p_count;        /* count of bytes in p_data */
	char    p_data[PDSIZE];
};

/* host control codes */

#define ph_con  1               /* connect */
#define ph_dis  2               /* disconnect */
#define ph_rst  3               /* reset */
#define ph_rrp  4               /* reset reply */
#define ph_next 5               /* ready for next */
#define ph_sig  6               /* signal (in p_data[0]) */
#define ph_eof  7               /* end of file */

/* imp control codes */

#define pi_dead 1               /* dead host */


/*
 * line header, prefixed to pack
 */
struct head {
	char    h_soh;          /* start of header or flag */
	char    h_seq;          /* sequence number */
	int     h_count;        /* count of bytes including header */
	int     h_check;        /* checksum */
};

/* h_seq fields for dh link */

#define h_snum  1               /* binary sequence number */
#define h_sa0   2               /* ack 0 */
#define h_sa1   4               /* ack 1 */


/*
 * line buffer in memory containing pack and head
 */
struct linebuf {
	pbn     l_next;         /* next buffer in queue */
	char    l_flag;
	char    l_arg;          /* arg to l_task */
	int     (*l_task)();    /* line task (such as crc generation */
	struct  head    h;
	struct  pack    p;
} b;

/* l_flag values */

#define l_call  0200            /* call l_task */
#define l_local 0100            /* local origin (used to stop looping of local
				   packets when line loop test turned on*/
