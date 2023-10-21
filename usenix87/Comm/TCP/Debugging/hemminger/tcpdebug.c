/*
 * Tcp Debug - visual picture of Tcp connections
 *
 * Simple menu driven display of information about Tcp based connections.
 * This combines the information of netstat + trpt.
 */

#ifndef lint
static char _rcsid[] = "$Header: tcpic.c,v 1.1 85/04/09 15:25:00 steveh Exp $$Locker:  $";
#endif

#include <stdio.h>
#include <netdb.h>
#include <nlist.h>
#include <local/window.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#define PRUREQUESTS
#include <sys/protosw.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#define TCPTIMERS
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
#define TANAMES
#include <netinet/tcp_debug.h>


struct nlist nml[] = {
#define N_TCB		0
	{ "_tcb" },
#define N_TCP_DEBUG	1
	{ "_tcp_debug" },
#define N_TCP_DEBX	2
	{ "_tcp_debx" },
#define N_MBSTAT	3
	{ "_mbstat" },
#define N_TCPSTAT	4
	{ "_tcpstat" },
	0
};


char	Tf_flags[] = "\1ACKNOW\2DELACK\3DONTKEEP\4NOOPT";
char	Th_flags[] = "\1FIN\2SYN\3RST\4PUSH\5ACK\6URG";

int	kmem;
int	aflag = 0;
int	iflag = 0;
int	mflag = 0;
int	dflag = 0;
int 	interval = 10;

/* This determines what windows to display */
/* The 4 ints specify the position & size;
   the two strings are optional, and
   are the label and initial contents for the window
 */
struct initscreen {
	int	i_x, i_y, i_xe, i_ye;
	char	*i_lbl, *i_str;
} InitScreen[] = {
#define W_NETSTAT (wins[0])	
/*0*/	0, 0,	80, 23, 0,
			0,
#define	W_SOCK	(wins[1])
/*1*/	0, 0,	16, 4,	"Socket",
			"SendQ\nRecvQ",
#define W_BYTES	(wins[2])
/*2*/	0, 4,	16, 4,	"Data Xfer",
			"Sent\nRcvd",
#define W_RECV (wins[3])
/*3*/	0, 9,	16, 6,	"Recv Seq",
			"Wind\nNext\nUrg\nAdv",
#define	W_TIMER	(wins[4])
/*4*/	17, 0,	16, 6, "Tcp Timers",
			0,
#define	W_SEND	(wins[5])
/*5*/	17, 6,	16, 9,	"Send Seq",
			"Wind\nNext\nUrg\nMax\nUna\nSeq#\nAck#",
#define W_RETRY (wins[6])
/*6*/	35, 0,	16, 7,	"Retransmit",
			"Shift\nIdle\nRtt\nRtseq\nSrtt",
#define W_PARAM (wins[7])
/*7*/	34, 9,	20,  6,	"Parameters",
			"State\nMax seg size\nForce\nFlags",
#define W_IFNET (wins[8])
/*8*/	56, 0,	20, 7,	"",
			"In packets\nIn error\nOut packets\nOut errors\nCollisions",
#define W_MBSTAT (wins[9])
/*9*/	56, 8,	20, 6,  "Mbuf stat",
			"In use\nFree\nPages\nFree Pages",
#define W_TCPSTAT (wins[10])
/*10*/	56, 8,  20, 7,  "Tcp stat",
			"Bad Sum\nBad Off\nHdr Drop\nBad Segs\nUnack",
#define W_DEBUG	(wins[11])		
/*11*/	0, 15,	80, 8,	0, 0,
#define W_HELP  (wins[12])
/*12*/	0, 23,	80, 1,	0, 0		
};

#define	NWINS	(sizeof InitScreen/sizeof *InitScreen)

static char buf[BUFSIZ];

#define	WPR(w,y,x,fmt,n)	WAcursor (w, y, x),	\
				(void) sprintf (buf, fmt, n),	\
				Wputs (buf, w)

Win	*wins[NWINS];
int	Maxrows, Maxcols;

struct inpcb	*GetInpcb();
char		*strsave();
extern char	*strcpy(), *malloc(), *calloc(), *index(),
		*inet_ntoa();
extern long	lseek();

/*
 * Main program
 *  Parse arguments, choose connection and display it.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	register struct inpcb *inp;

	while(--argc && **++argv == '-') {
		switch(argv[0][1]) {
		case 'a': ++aflag;		break;
		case 'd': ++dflag;		break;
		case 'i': ++iflag;		break;
		case 'm': ++mflag;		break;
		default:
			fprintf(stderr, "tcpic: unknown arg %s\n", argv[1]);
			fprintf(stderr,"Usage: tcpic  [flags] [ interval ]\n");
			fprintf(stderr,"flags: %s\n%s\n%s\n%s\n",
				"\t-a\tall (include) servers",
				"\t-d\tshow debug trace (trpt)",
				"\t-i\tshow interfaces",
				"\t-m\tshow mbuf stats");
			exit(1);
		}
	}

	if(argc)
		interval = atoi(*++argv);		


	nlist("/vmunix", nml);
	
	if(nml[0].n_value == 0) {
		fprintf(stderr,"tcpic: can't read kernel symbol table\n");
		exit(1);
	}

	if( (kmem = open("/dev/kmem", 0)) < 0) {
		perror("/dev/kmem");
		exit(1);
	}

	SetupWindows();
	Wfront(W_NETSTAT);
	Wsetmode(W_HELP, WBOLD);
	Wnewline(W_HELP, 0);
	Wlabel( W_NETSTAT,
		"# Local Address        Foreign Address   State",
		0, 0);

	if(!iflag)
		Whide(W_IFNET);
	if(!mflag)
		Whide(W_MBSTAT);
	else
		Whide(W_TCPSTAT);
	BuildPortTable();

	sethostent(1);

	while( (inp = GetInpcb()) != NULL) {
		Whide(W_NETSTAT);
		Display(inp);
		Wunhide(W_NETSTAT);
	}

	Wexit(0);
}

/*
 * Set up the initial window display.
 */
SetupWindows()
{
	register struct initscreen *ip;
	register char *str;
	register Win *w;
	register int n;

	if(Winit(0,0))  {
		fprintf(stderr, "This terminal doesn't support windows\n");
		exit(1);
	}
	Wscreensize(&Maxrows, &Maxcols);

	/* initialize windows */
	for(n = 0, ip = InitScreen; n < NWINS; n++, ip++) {
	 	/*  adjust size of some windows to fit screen */
		
		if(n == &W_NETSTAT - wins) {
			ip->i_ye = Maxrows-1;	/* netstat takes full screen */
			ip->i_xe = Maxcols;
		} else if(n == &W_HELP - wins) {
			ip->i_y = Maxrows-1;	/* help at bottom */
			ip->i_xe = Maxcols;
		} else if(n == &W_DEBUG - wins) {/* debug takes what is left on screen */
			ip->i_ye = Maxrows - ip->i_y - 2;
			ip->i_xe = Maxcols;
		}

		w = Wopen(0, ip->i_x, ip->i_y, ip->i_xe, ip->i_ye, 0, 0);
		if (w == 0) {
			Wcleanup();
			fprintf(stderr, "can't fit window '%s' at %d,%d of %d,%d\n",
				ip->i_lbl, ip->i_x, ip->i_y, ip->i_xe, ip->i_ye);
			fprintf(stderr, "Screen is too small\n");
			exit(1);
		}
		wins[ip - InitScreen] = w;
		Woncursor(w, 0);
		Wnewline(w, 1);
		Wwrap(w, 0);
		if(ip->i_lbl) {
			Wframe(w);
			Wlabel(w, ip->i_lbl, 0, 1);
		}
		if(str = ip ->i_str) {
			if(*str == '$') {
				Wsetmode(w, WINVERSE);
				str++;
			}
			Wputs(str, w);
			Wsetmode(w,0);
		}
	}
}

/*
 * Read the port names out of /etc/services and store in memory
 *  for faster access.
 */
char *PortTable[IPPORT_RESERVED];

BuildPortTable() {
	register struct servent *sp;
	u_short port;
	
	setservent();
	while( (sp = getservent()) != NULL)  {
		if(strcmp(sp->s_proto, "tcp") != 0)
			continue;
		port = ntohs((u_short) sp->s_port);
		if(port < IPPORT_RESERVED)
			PortTable[port] = strsave(sp->s_name);
	}
	endservent();
}

/*
 * Lookup a port name
 */
char *inetport(port)
	u_short port;
{
	u_short p = ntohs(port);
	static char portbuf[10];

	if(p == 0) 
		return "*";
	else if(p >= IPPORT_RESERVED || PortTable[p] == NULL) {
		(void) sprintf(portbuf, "%d", p);
		return portbuf;
	} else
		return PortTable[p];
}

/*
 * Host address to name translation
 *   keep already looked up entries around to speed things up.
 */

struct hosts {
	struct hosts *ho_next;
	char	*ho_name;
	u_long	ho_addr;
} *headhost = NULL;

char *inethost(in)
	struct in_addr in;
{
	register struct hosts *ho;
	register struct hostent *hp;

	if (inet_lnaof(in) == INADDR_ANY) 
		return "*";

	/* look in save list */
	for(ho = headhost; ho != NULL; ho = ho->ho_next) 
		if(ho->ho_addr == in.s_addr)
			return ho->ho_name;
	hp = gethostbyaddr(&in, sizeof (struct in_addr), AF_INET);
	if (hp == NULL)
		return inet_ntoa(in);

	/* add to save list */
	ho = (struct hosts *) malloc(sizeof(struct hosts));
	ho->ho_addr = in.s_addr;
	ho->ho_name = strsave(hp->h_name);
	ho->ho_next = headhost;
	headhost = ho;

	return hp->h_name;
}
/*
 * Pretty print an Internet address (net address + port).
 */
char *
inetprint(in, port)
	struct in_addr in;
	u_short port;
{
	static char line[80];

	(void) sprintf(line, "%s.%s", inethost(in), inetport(port));
	return line;
}

/*
 * Print a value a la the %b format of the kernel's printf
 */
char *
flagprint(v, bits)
	register char *bits;
	u_short v;
{
	register char *cp;
	register int i, any = 0;
	static char obuf[128];

	(void) sprintf(obuf,"%x", v);
	cp = obuf + strlen(obuf);
	if (v) {
		*cp++ = '<';
		while (i = *bits++) {
		if (v & (1 << (i-1))) {
				if (any)
					*cp++ = ',';
				any = 1;
				for (; *bits > 32; bits++)
					*cp++ = *bits;
			} else
				for (; *bits > 32; bits++)
					;
		}
		*cp++ = '>';
		*cp = '\0';
	}
	return obuf;
}

/*
 * Print out tcp state
 */
char *
inetstate(off)
	long off;
{
	static char line[40];
	struct tcpcb tcpcb;

	(void) lseek(kmem, off, 0);
	(void) read(kmem, (char *) &tcpcb, sizeof (tcpcb));

	if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
		(void) sprintf(line, "%d", tcpcb.t_state);
	else
		(void) strcpy(line, tcpstates[tcpcb.t_state]);
	return line;
}

ReadChar() {
	char c;

	if(read(0, &c, 1) != 1)
		Wexit(0);
	c &= 0x7f;
	ioctl(0, FIONREAD, (char *) &InputPending);

	return c;
}

struct inpcb *
GetInpcb() {
	register struct inpcb *next, *prev;
	register int n;
	char c, *cp;
	struct inpcb inpcb;
	long off = nml[N_TCB].n_value;
	static int npcbs = 0;
	static struct inpcb **inpcbs;
	static char keys[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if(npcbs == 0) {
		npcbs = Maxrows - 2;
		inpcbs = (struct inpcb **)
			 calloc(npcbs, sizeof (struct inpcb *));
	}

reread:
	WAcursor(W_NETSTAT, 0, 0);
	Wclear(W_NETSTAT, 2);

	(void) lseek(kmem, off, 0);
	(void) read(kmem, (char *) &inpcb, sizeof (struct inpcb));

	prev = (struct inpcb *) off;

	n = 0;
	/* follow around circular list */
	while(inpcb.inp_next != (struct inpcb *)off && n < npcbs) {
		next = inpcb.inp_next;

		(void) lseek(kmem, (long) next, 0);
		(void) read(kmem, (char *) &inpcb, sizeof(struct inpcb));
		if(inpcb.inp_prev != prev) {
			Wputs("??? (lost sync)\n", W_NETSTAT);
			break;
		}

		if(!aflag && inet_lnaof(inpcb.inp_laddr) == INADDR_ANY) {
			prev = next;
			continue;
		}

		WAcursor(W_NETSTAT, n, 0);
		Wsetmode(W_NETSTAT, WBOLD);
		Wputc( keys[n], W_NETSTAT);
		Wsetmode(W_NETSTAT, 0);
		(void) sprintf(buf, " %-18.18s", 
			inetprint(inpcb.inp_laddr,inpcb.inp_lport));
		Wputs(buf, W_NETSTAT);
		(void) sprintf(buf, " %-18.18s %s", 
			inetprint(inpcb.inp_faddr,inpcb.inp_fport),
			inetstate((long)inpcb.inp_ppcb));
		Wputs(buf, W_NETSTAT);

		prev = next;
		if(inpcbs[n] == 0) 
			inpcbs[n] = (struct inpcb *) malloc(sizeof(inpcb));
		*inpcbs[n] = inpcb;
		++n;
	}

	Wclearline(W_HELP, 2);
	Wputs("<letter> => show conn, <SPACE> => reread, <ESC> => quit, '^L' => redraw\r",
		 W_HELP);
	Wrefresh(0);

	for(;;) {
		switch(c = ReadChar()) {
		case CTRL(l):
			++ScreenGarbaged;
			/* fall into */
		case ' ':
			goto reread;
		case CTRL([):
			return NULL;
		default:
			cp = index(keys, c);
		}
		if(cp == NULL || cp >= &keys[n])
			Ding();
		else
			break;
	}

	return inpcbs[cp - keys];
}

struct socket sockstr;
struct rtentry rtentry;
struct ifnet iflast;
struct tcpstat tcplast;
struct mbuf mtcpcb;		/* mbuf containing tcpcb */
char	ifname[10];

Display(inp)
	register struct inpcb *inp;
{
	register struct mbuf *mptr;
	register int i;

	Wclearline(W_HELP,2);
	Wputs("<ESC> => quit back to menu, '^L' => redraw\r", W_HELP);

	
	for(i = 0; i < TCPT_NTIMERS; ++i) {
		WPR(W_TIMER, i, 0, "%s", tcptimers[i]);
	}

	if(iflag) {
		(void) lseek(kmem, (long) inp->inp_route.ro_rt, 0);
		(void) read(kmem, (char *) &rtentry, sizeof(rtentry));

		(void) lseek(kmem, (long) rtentry.rt_ifp, 0);
		(void) read(kmem, (char *) &iflast, sizeof(struct ifnet));
	
		(void) lseek(kmem, (long) iflast.if_name, 0);
		(void) read(kmem, ifname, sizeof(ifname));
	
		(void) sprintf(buf, "If %s%d", ifname, iflast.if_unit);
		Wlabel(W_IFNET, buf, 0, 1);
	}

	if(!mflag) {
		(void) lseek(kmem, nml[N_TCPSTAT].n_value, 0);
		(void) read(kmem, (caddr_t) &tcplast, sizeof tcplast);
	}

	Whide(W_DEBUG);

	mptr = dtom(inp->inp_ppcb);

	for(;;) {
		(void) lseek(kmem, (long) mptr, 0);
		(void) read(kmem, (char *) &mtcpcb, sizeof(mtcpcb));

		if(mtcpcb.m_type != MT_PCB ) {
			Wclearline(W_HELP, 2);
		 	Wputs("Connection closed\r", W_HELP);
		 	Wrefresh(0);
		 	sleep(1);
			break;
		}

		(void) lseek(kmem, (long) inp->inp_socket, 0);
		(void) read(kmem, (char *) &sockstr, sizeof(sockstr));

		DisplayTcp(mtod(&mtcpcb,struct tcpcb *));
		if(iflag)
			DisplayIf();
		if(mflag)
			DisplayMbuf();
		else
			DisplayStat();

		Wrefresh (0);
		if(dflag && sockstr.so_options & SO_DEBUG) {
			Wunhide(W_DEBUG);
			DoDebug(inp->inp_ppcb);
		}

		/* Refresh and redisplay */
		while (InputPending) {
			switch( ReadChar() ) {
			case CTRL(l):
				ScreenGarbaged++;
				break;
			case 'q':
			case 'Q':
			case CTRL([):
				return;
			}
		}
	
		if(interval)
			sleep(interval);

	}
}

DisplayStat()
{
	struct tcpstat tcpcur;

	(void) lseek(kmem, nml[N_TCPSTAT].n_value, 0);
	(void) read(kmem, (char *) &tcpcur, sizeof tcpcur);

	WPR(W_TCPSTAT, 0, 10, "%8d", tcpcur.tcps_badsum-tcplast.tcps_badsum);
	WPR(W_TCPSTAT, 1, 10, "%8d", tcpcur.tcps_badoff-tcplast.tcps_badoff);
	WPR(W_TCPSTAT, 2, 10, "%8d", tcpcur.tcps_hdrops-tcplast.tcps_hdrops);
	WPR(W_TCPSTAT, 3, 10, "%8d", tcpcur.tcps_badsegs-tcplast.tcps_badsegs);
	WPR(W_TCPSTAT, 4, 10, "%8d", tcpcur.tcps_unack-tcplast.tcps_unack);
	tcplast = tcpcur;
}

DisplayMbuf()
{
	struct mbstat mbstat;

	if(nml[N_MBSTAT].n_type == 0)
		return;

	(void) lseek(kmem, nml[N_MBSTAT].n_value, 0);
	(void) read(kmem, (char *) &mbstat, sizeof mbstat);
	
	WPR(W_MBSTAT, 0, 10, "%8d", mbstat.m_mbufs - mbstat.m_mbfree);
	WPR(W_MBSTAT, 1, 10, "%8d", mbstat.m_mbfree);
	WPR(W_MBSTAT, 2, 10, "%8d", mbstat.m_clusters - mbstat.m_clfree);
	WPR(W_MBSTAT, 3, 10, "%8d", mbstat.m_clfree);
}

DisplayIf()
{
	struct ifnet ifcur;

	(void) lseek(kmem, (long) rtentry.rt_ifp, 0);
	(void) read(kmem, (char *) &ifcur, sizeof(struct ifnet));
	
	WPR(W_IFNET, 0, 11, "%8d", ifcur.if_ipackets - iflast.if_ipackets);
	WPR(W_IFNET, 1, 11, "%8d", ifcur.if_ierrors - iflast.if_ierrors);
	WPR(W_IFNET, 2, 11, "%8d", ifcur.if_opackets - iflast.if_opackets);
	WPR(W_IFNET, 3, 11, "%8d", ifcur.if_ierrors - iflast.if_ierrors);
	WPR(W_IFNET, 4, 11, "%8d", ifcur.if_collisions - iflast.if_collisions);
	iflast = ifcur;
}

DisplayTcp(tcp)
	register struct tcpcb *tcp;
{
	register int i;

	for(i = 0; i < TCPT_NTIMERS; i++) {
		WPR(W_TIMER, i, 8, "%6d", tcp->t_timer[i]);
	}

	WPR(W_RETRY, 0, 6, "%8d", tcp->t_rxtshift);
	WPR(W_RETRY, 1, 6, "%8d", tcp->t_idle);
	WPR(W_RETRY, 2, 6, "%8d", tcp->t_rtt);
	WPR(W_RETRY, 3, 6, "%8x", tcp->t_rtseq);
#ifdef NOFLOAT
	WPR(W_RETRY, 4, 6, "%8.2f", (double) tcp->t_srtt/((double) TCPT_SCALE));
#else
	WPR(W_RETRY, 4, 6, "%8.2f", tcp->t_srtt);
#endif

	WAcursor(W_PARAM, 0, 6);
	if (tcp->t_state < 0 || tcp->t_state >= TCP_NSTATES)
		(void) sprintf(buf, "%12d" , tcp->t_state);
	else 
		(void) sprintf(buf, "%12s", tcpstates[tcp->t_state]);
	Wputs(buf, W_PARAM);
	WPR(W_PARAM, 1, 13, "%5d", tcp->t_maxseg);
	WPR(W_PARAM, 2, 14, "%4d", tcp->t_force);
	WPR(W_PARAM, 3, 6, "%12s", flagprint((u_short)tcp->t_flags, Tf_flags));

	WPR(W_SEND, 0, 6, "%8d", tcp->snd_wnd);
	WPR(W_SEND, 1, 6, "%8x", tcp->snd_nxt);
	WPR(W_SEND, 2, 6, "%8x", tcp->snd_up);
	WPR(W_SEND, 3, 6, "%8x", tcp->snd_max);
	WPR(W_SEND, 4, 6, "%8x", tcp->snd_una);
	WPR(W_SEND, 5, 6, "%8x", tcp->snd_wl1);
	WPR(W_SEND, 6, 6, "%8x", tcp->snd_wl2);

	WPR(W_RECV, 0, 6, "%8d", tcp->rcv_wnd);
	WPR(W_RECV, 1, 6, "%8x", tcp->rcv_nxt);
	WPR(W_RECV, 2, 6, "%8x", tcp->rcv_up);
	WPR(W_RECV, 3, 6, "%8x", tcp->rcv_adv);

	WPR(W_BYTES, 0, 5, "%10d", (u_long)tcp->snd_nxt - (u_long)tcp->iss);
	WPR(W_BYTES, 1, 5, "%10d", (u_long)tcp->rcv_nxt - (u_long)tcp->irs);

	WPR(W_SOCK, 0, 5, "%10d", sockstr.so_snd.sb_cc);
	WPR(W_SOCK, 1, 5, "%10d", sockstr.so_rcv.sb_cc);

}

char *strsave(str)
	register char *str;
{
	register int len = strlen(str);
	register char *new;
	extern char *malloc();

	new = malloc(++len);
	(void) strcpy(new, str);
	return new;
}

/*
 * Tcp debug routines
 */

DoDebug(tcp)
	caddr_t tcp;
{
	static int last_debx = -1;
	register int debx;
	
	if(nml[N_TCP_DEBUG].n_type == 0)
		return;
	(void) lseek(kmem, nml[N_TCP_DEBX].n_value, 0);
	(void) read(kmem, (char *) &tcp_debx, sizeof(tcp_debx));

	(void) lseek(kmem, nml[N_TCP_DEBUG].n_value, 0);
	(void) read(kmem, (char *) tcp_debug, sizeof(tcp_debug));

	Wunhide(W_DEBUG);
	if(last_debx == -1) {
		last_debx = tcp_debx;
		return;
	}

	for(debx = last_debx; debx != tcp_debx; debx = ++debx % TCP_NDEBUG) {
		if(tcp != tcp_debug[debx].td_tcb)
			continue;
		tcp_trace(&tcp_debug[debx]);
		Wrefresh(0);
		if(InputPending)
			break;
	}
	last_debx = debx;
}

/*
 * Print out a Tcp debug record
 */
tcp_trace(td)
	register struct tcp_debug *td;
{
	register struct tcpiphdr *ti = &td->td_ti;
	tcp_seq seq, ack;
	short act = td->td_act;
	short req = td->td_req;
	int len, flags, win, timer;

	(void) sprintf(buf,"%03d %s:%s ", (ntohl(td->td_time)/10) % 1000,
			tcpstates[td->td_ostate], tanames[act]);
	Wputs(buf, W_DEBUG);

	switch (act) {
	case TA_INPUT:
	case TA_OUTPUT:
	case TA_DROP:
		(void) sprintf(buf,"(src=%s,%d, dst=%s,%d)\n\t",
			inet_ntoa(ti->ti_src), ntohs(ti->ti_sport),
			inet_ntoa(ti->ti_dst), ntohs(ti->ti_dport));
		Wputs(buf, W_DEBUG);

		seq = ti->ti_seq;
		ack = ti->ti_ack;
		len = ti->ti_len;
		win = ti->ti_win;
		if (act == TA_OUTPUT) {
			seq = ntohl(seq);
			ack = ntohl(ack);
			len = ntohs(len);
			win = ntohs(win);
		}
		if (act == TA_OUTPUT)
			len -= sizeof (struct tcphdr);
		if (len)
			(void) sprintf(buf,"[%x..%x)@%x", seq, seq+len, ack);
		else
			(void) sprintf(buf,"%x@%x", seq, ack);

		Wputs(buf, W_DEBUG);
		if (win) {
			(void) sprintf(buf,"(win=%x)", win);
			Wputs(buf, W_DEBUG);
		}

		if (flags = ti->ti_flags) {
			Wputs(" flags=",W_DEBUG);
			Wputs(flagprint((u_short)flags, Th_flags), W_DEBUG);
		}
		break;

	case TA_USER:
		timer = req >> 8;
		req &= 0xff;
		(void) sprintf(buf,"%s", prurequests[req]);
		Wputs(buf, W_DEBUG);
		if (req == PRU_SLOWTIMO || req == PRU_FASTTIMO) {
			(void) sprintf(buf,"<%s>", tcptimers[timer]);
			Wputs(buf, W_DEBUG);
		}
		break;
	}
	(void) sprintf(buf," -> %s\n", tcpstates[td->td_cb.t_state]);
	Wputs(buf,W_DEBUG);
}
