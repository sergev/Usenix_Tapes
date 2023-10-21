#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * tcpdbug
 *
 * snapshot tcpcbs
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>

#include <nlist.h>
#include <netdb.h>

struct nlist nl[] = {
    {"_tcb"},
#define X_TCB	0
    {0}
};

int kmem;
struct inpcb tcb, inpcb;
struct tcpcb tcpcb;

int AFlag, FFlag, LFlag;
struct in_addr FAddr;
u_short FPort;

ReadKernelItem (kaddr, caddr, size)
caddr_t kaddr, caddr;
int size;
{
    (void) lseek (kmem, (long) kaddr, 0);
    if (read (kmem, caddr, size) != size) {
	perror ("kmem read");
	exit (1);
    }
}

OpenKmem () {
    if ((kmem = open ("/dev/kmem", 0)) < 0) {
	perror ("/dev/kmem");
	exit (1);
    }
}

GetNames () {
    nlist ("/vmunix", nl);
    if (nl[0].n_type == 0) {
	fprintf (stderr, "/vmunix: no namelist\n");
	exit (1);
    }
}

LoopThrough () {
    register struct inpcb *inp, *endp;
    register struct tcpcb *tp;

    endp = (struct inpcb *) nl[X_TCB].n_value;
    ReadKernelItem ((caddr_t) endp, (caddr_t) &tcb, sizeof tcb);
    for (inp = tcb.inp_next; inp != endp; inp = inpcb.inp_next) {
	ReadKernelItem ((caddr_t) inp, (caddr_t) &inpcb, sizeof inpcb);
	tp = (struct tcpcb *) inpcb.inp_ppcb;
	if (tp == 0)
	    continue;
	ReadKernelItem ((caddr_t) tp, (caddr_t) &tcpcb, sizeof tcpcb);
	DumpTCP (tp, &tcpcb);
    }
}

DumpTCP (ktp, tp) register struct tcpcb *ktp, *tp; {
#define flag(x) ((x) ? "true" : "false")
    struct timeval tv;
    struct timezone tz;

    if (FFlag) {		/* ignore mismatches */
	if (inpcb.inp_faddr.s_addr != FAddr.s_addr)
	    return;
	if (FPort && inpcb.inp_fport != FPort)
	    return;
    }
    else {
	if (tp -> t_state == TCPS_LISTEN && !AFlag)
	    return;		/* ignore LISTENers */
    }
    (void) gettimeofday (&tv, &tz);
    printf ("time = %d.%03d\n", tv.tv_sec, tv.tv_usec);
    printf ("\
tcpcb:\n\
\tlocal addr = %d.%d.%d.%d.%d\tforeign addr = %d.%d.%d.%d.%d\n\
\taddr = 0x%x\tstate = %s\n\
\ttimers: rexmt = %d, persist = %d, keep = %d, 2msl = %d\n\
\trxtshift = %d\tmaxseg = %d\tforce %s\n\
\tflags: ack now %s, del ack %s, don't keep %s, no opt %s\n\
\tsend seq: unack = 0x%x, next = 0x%x, urg = 0x%x,\n\
\t\twl1 = 0x%x, wl2 = 0x%x,\n\
\t\tiss= 0x%x, window = 0x%x\n\
\trecv seq: window = 0x%x, next = 0x%x, urg = 0x%x, irs = 0x%x\n\
\tadvertised window = 0x%x, highest seq sent = 0x%x\n\
\tidle = %d, rtt = %d, rttseq = 0x%x, srtt = %12.6f\n\
\toobflag %s, iobc = 0x%x\n\n",
#define xpand(x) (x).S_un.S_un_b.s_b1, (x).S_un.S_un_b.s_b2, \
		 (x).S_un.S_un_b.s_b3, (x).S_un.S_un_b.s_b4
	    xpand (inpcb.inp_laddr), ntohs (inpcb.inp_lport),
	    xpand (inpcb.inp_faddr), ntohs (inpcb.inp_fport),
#undef xpand
	    ktp, tcpstates[tp -> t_state],
	    tp -> t_timer[TCPT_REXMT], tp -> t_timer[TCPT_PERSIST],
	    tp -> t_timer[TCPT_KEEP], tp -> t_timer[TCPT_2MSL],
	    tp -> t_rxtshift, tp -> t_maxseg, flag (tp -> t_force),
	    flag (tp -> t_flags & TF_ACKNOW),
	    flag (tp -> t_flags & TF_DELACK),
	    flag (tp -> t_flags & TF_DONTKEEP),
	    flag (tp -> t_flags & TF_NOOPT),
	    tp -> snd_una, tp -> snd_nxt, tp -> snd_up, tp -> snd_wl1,
	    tp -> snd_wl2, tp -> iss, tp -> snd_wnd,
	    tp -> rcv_wnd, tp -> rcv_nxt, tp -> rcv_up, tp -> irs,
	    tp -> rcv_adv, tp -> snd_max,
	    tp -> t_idle, tp -> t_rtt, tp -> t_rtseq, tp -> t_srtt,
	    flag (tp -> t_oobflags), tp -> t_iobc & 0xff);
}

GetAddr (s) register char *s; {
    register char  *dotp;
    register struct hostent *hp;
    char   *rindex ();

    dotp = rindex (s, '.');
    if (!dotp) {
	fprintf (stderr, "%s: bad address format (should be host.port)\n", s);
	exit (1);
    }
    *dotp++ = 0;
    hp = gethostbyname (s);
    if (hp == 0) {
	fprintf (stderr, "%s: unknown host\n", s);
	exit (1);
    }
    FAddr.s_addr = *(n_long *) hp -> h_addr;
    FPort = htons (atoi (dotp));
}

Options (argc, argv)
register int argc;
register char **argv; {
    register char *s;

    argc--, argv++;
    while (--argc >= 0) {
	s = *argv++;
	if (*s++ != '-')
	    return;
	switch (*s) {
	    case 'a':
		AFlag++;
		break;
	    case 'f':
		if (*++s == 0) {
		    if (--argc >= 0)
			s = *argv++;
		    else
			return;
		}
		FFlag++;
		GetAddr (s);
		break;
	    case 'l':
		LFlag++;
		break;
	}
    }
}

main (argc, argv) char **argv; {
    Options (argc, argv);
    if (argc > 1 && strcmp (argv[1], "-a", 0) == 0)
	AFlag++;
    OpenKmem ();
    GetNames ();
    if (LFlag) {
	for (;;) {
	    LoopThrough ();
	    fflush (stdout);
	    nap (500);
	}
    }
    else
	LoopThrough ();
    exit (0);
}

nap (ms) {
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;
    (void) select (0, (int *) 0, (int *) 0, (int *) 0, &tv);
}
