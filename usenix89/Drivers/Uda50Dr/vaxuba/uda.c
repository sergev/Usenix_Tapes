/*
 * UDA50/MSCP device driver
 */

/*
 * TODO
 *	write bad block forwarding code
 */

#include "ra.h"

#if NUDA > 0

/*
 * CONFIGURATION OPTIONS.  The next three defines are tunable -- tune away!
 *
 * NRSPL2 and NCMDL2 control the number of response and command
 * packets respectively.  They may be any value from 0 to 7, though
 * setting them higher than 5 is unlikely to be of any value.
 * If you get warnings about your command ring being too small,
 * try increasing the values by one.
 *
 * MAXUNIT controls the maximum unit number (number of drives per
 * controller) we are prepared to handle.
 *
 * DEFAULT_BURST must be at least 1.
 */
#define	NRSPL2	5		/* log2 number of response packets */
#define NCMDL2	5		/* log2 number of command packets */
#define	MAXUNIT	8		/* maximum allowed unit number */
#define	DEFAULT_BURST	4	/* default DMA burst size */

#include "../machine/pte.h"

#include "param.h"
#include "systm.h"
#include "buf.h"
#include "conf.h"
#include "dir.h"
#include "user.h"
#include "map.h"
#include "vm.h"
#include "dk.h"
#include "cmap.h"
#include "syslog.h"

#include "../vax/cpu.h"
#include "ubareg.h"
#include "ubavar.h"

#define	NRSP	(1 << NRSPL2)
#define	NCMD	(1 << NCMDL2)

#include "udareg.h"
#include "../vax/mscp.h"
#include "../vax/mscpvar.h"
#include "../vax/mtpr.h"

/*
 * Backwards compatibility:  Reuse the old names.  Should fix someday.
 */
#define	udaprobe	udprobe
#define	udaslave	udslave
#define	udaattach	udattach
#define	udaopen		udopen
#define	udastrategy	udstrategy
#define	udaread		udread
#define	udawrite	udwrite
#define	udareset	udreset
#define	udaintr		udintr
#define	udadump		uddump
#define	udasize		udsize

/*
 * UDA communications area and MSCP packet pools, per controller.
 */
struct	uda {
	struct	udaca uda_ca;		/* communications area */
	struct	mscp uda_rsp[NRSP];	/* response packets */
	struct	mscp uda_cmd[NCMD];	/* command packets */
} uda[NUDA];

/*
 * Software status, per controller.
 */
struct	uda_softc {
	struct	uda *sc_uda;	/* Unibus address of uda struct */
	short	sc_state;	/* UDA50 state; see below */
	short	sc_flags;	/* flags; see below */
	int	sc_micro;	/* microcode revision */
	int	sc_ivec;	/* interrupt vector address */
	struct	mscp_info sc_mi;/* MSCP info (per mscpvar.h) */
	int	sc_wticks;	/* watchdog timer ticks */
} uda_softc[NUDA];

/*
 * Controller states
 */
#define	ST_IDLE		0	/* uninitialised */
#define	ST_STEP1	1	/* in `STEP 1' */
#define	ST_STEP2	2	/* in `STEP 2' */
#define	ST_STEP3	3	/* in `STEP 3' */
#define	ST_SETCHAR	4	/* in `Set Controller Characteristics' */
#define	ST_RUN		5	/* up and running */

/*
 * Flags
 */
#define	SC_MAPPED	0x01	/* mapped in Unibus I/O space */
#define	SC_INSTART	0x02	/* inside udastart() */
#define	SC_GRIPED	0x04	/* griped about cmd ring too small */
#define	SC_INSLAVE	0x08	/* inside udaslave() */
#define	SC_DOWAKE	0x10	/* wakeup when ctlr init done */

/*
 * Device to unit number and partition:
 */
#define	UNITSHIFT	3
#define	UNITMASK	7
#define	udaunit(dev)	(minor(dev) >> UNITSHIFT)
#define	udapart(dev)	(minor(dev) & UNITMASK)

/* THIS SHOULD BE READ OFF THE PACK, PER DRIVE */
struct size {
	daddr_t nblocks;
	daddr_t blkoff;
} ra81_sizes[8] = {
#ifdef MARYLAND
#ifdef ENEEVAX
	30706,	0,		/* A=cyl    0 thru   42 + 2 sectors */
	40696,	30706,		/* B=cyl   43 thru   99 - 2 sectors */
	-1,	0,		/* C=cyl    0 thru 1247 */
	-1,	71400,		/* D=cyl  100 thru 1247 */

	15884,	0,		/* E=blk      0 thru  15883 */
	33440,	15884,		/* F=blk  15884 thru  49323 */
	82080,	49324,		/* G=blk  49324 thru 131403 */
	-1,	131404,		/* H=blk 131404 thru    end */
#else
	67832,	0,		/* A=cyl    0 thru   94 + 2 sectors */
	67828,	67832,		/* B=cyl   95 thru  189 - 2 sectors */
	-1,	0,		/* C=cyl    0 thru 1247 */
	-1,	135660,		/* D=cyl  190 thru 1247 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
#endif ENEEVAX
#else
	/* THE FOLLOWING ARE STRAIGHT FROM THE 4.3BSD uda.c */
	/* THIS KIND OF GARBAGE IS WHY THIS SHOULD BE READ FROM THE PACK */

/*
 * These are the new standard partition sizes for ra81's.
 * An RA_COMPAT system is compiled with D, E, and F corresponding
 * to the 4.2 partitions for G, H, and F respectively.
 */
#ifndef	UCBRA
	15884,	0,		/* A=sectors 0 thru 15883 */
	66880,	16422,		/* B=sectors 16422 thru 83301 */
	891072,	0,		/* C=sectors 0 thru 891071 */
#ifdef RA_COMPAT
	82080,	49324,		/* 4.2 G => D=sectors 49324 thru 131403 */
	759668,	131404,		/* 4.2 H => E=sectors 131404 thru 891071 */
	478582,	412490,		/* 4.2 F => F=sectors 412490 thru 891071 */
#else
	15884,	375564,		/* D=sectors 375564 thru 391447 */
	307200,	391986,		/* E=sectors 391986 thru 699185 */
	191352,	699720,		/* F=sectors 699720 thru 891071 */
#endif RA_COMPAT
	515508,	375564,		/* G=sectors 375564 thru 891071 */
	291346,	83538,		/* H=sectors 83538 thru 374883 */

/*
 * These partitions correspond to the sizes used by sites at Berkeley,
 * and by those sites that have received copies of the Berkeley driver
 * with deltas 6.2 or greater (11/15/83).
 */
#else UCBRA

	15884,	0,		/* A=sectors 0 thru 15883 */
	33440,	15884,		/* B=sectors 15884 thru 49323 */
	891072,	0,		/* C=sectors 0 thru 891071 */
	15884,	242606,		/* D=sectors 242606 thru 258489 */
	307200,	258490,		/* E=sectors 258490 thru 565689 */
	325382,	565690,		/* F=sectors 565690 thru 891071 */
	648466,	242606,		/* G=sectors 242606 thru 891071 */
	193282,	49324,		/* H=sectors 49324 thru 242605 */

#endif UCBRA
#endif MARYLAND
},
#if GYRE
cdc9771_sizes[8] = {		/* HACK: treat some RA81s as 9771s on gyre */
	79680,	0,		/* A = cyl    0 thru   59 */
	79680,	79680,		/* B = cyl   60 thru  119 */
	-1,	0,		/* C = cyl    0 thru 1021 */
	-1,	159360,		/* D = cyl  120 thru 1021 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
},
#endif
ra80_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	0,	0,
	0,	0,
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404,		/* H=blk 131404 thru end */
}, ra60_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end */
	-1,	49324,		/* D=blk 49324 thru end */
	0,	0,
	0,	0,
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404,		/* H=blk 131404 thru end */
};
/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */

/*
 * Drive type index decoding table.  `ut_name' is null iff the
 * type is not known.
 */
struct	udatypes {
	char	*ut_name;	/* drive type name */
	struct	size *ut_sizes;	/* partition tables */
} udatypes[] = {
	NULL,		NULL,
	"ra80",		ra80_sizes,	/* 1 = ra80 */
	"old ra81",	ra81_sizes,	/* 2 = old ra81 microcode */
	NULL,		NULL,		/* 3 = old ra60?? */
	"ra60",		ra60_sizes,	/* 4 = ra60 */
	"ra81",		ra81_sizes,	/* 5 = ra81 */
#if GYRE
	/*
	 * This CDC partition hack depends on the fact that the
	 * Emulex SC41/MS controller is `version 6 model 6' and
	 * the current DEC devices are `version 5 model 6'.
	 */
	"cdc9771",	cdc9771_sizes,
#define	CDCTYPE	6	/* note that this is past the last real type */
#define	ISCDC(sc) ((sc)->sc_micro == 0x66)
#endif
};

#define NTYPES 6

/*
 * Definition of the driver for autoconf.
 */
int	udaprobe(), udaslave(), udaattach(), udadgo(), udaintr();
struct	uba_ctlr *udaminfo[NUDA];
struct	uba_device *udadinfo[NRA];

u_short	udastd[] = { 0772150, 0772550, 0777550, 0 };
struct	uba_driver udadriver =
 { udaprobe, udaslave, udaattach, udadgo, udastd, "ra", udadinfo, "uda",
   udaminfo };

/*
 * More driver definitions, for generic MSCP code.
 */
int	udadgram(), udactlrdone(), udaunconf(), udaonline(), udagotstatus();
int	udaioerror(), udareplace(), udabb();

struct	buf udautab[NRA];	/* per drive transfer queue */

struct	mscp_driver udamscpdriver =
 { MAXUNIT, NRA, UNITSHIFT, udautab,
   udadgram, udactlrdone, udaunconf,
   udaonline, udagotstatus, udareplace, udaioerror, udabb };

/*
 * Miscellaneous private variables.
 */
struct	buf rudabuf[NRA];	/* raw I/O buffer headers */

char	udasr_bits[] = UDASR_BITS;

struct	uba_device *udaip[NUDA][MAXUNIT];
				/* inverting pointers: ctlr & unit => Unibus
				   device pointer */

int	udaburst[NUDA] = {0};	/* burst size, per UDA50, zero => default;
				   in data space so patchable via adb */

daddr_t	ra_dsize[NRA];		/* drive sizes, from on line end packets */

struct	mscp udaslavereply;	/* get unit status response packet, set
				   for udaslave by udaunconf, via udaintr */

static struct uba_ctlr *probeum;/* this is a hack---autoconf should pass ctlr
				   info to slave routine; instead, we remember
				   the last ctlr argument to probe */

int	udawstart, udawatch();	/* watchdog timer */

/*
 * Externals
 */
int	wakeup();
int	hz;

/*
 * Poke at a supposed UDA50 to see if it is there.
 * This routine duplicates some of the code in udainit() only
 * because autoconf has not set up the right information yet.
 * We have to do everything `by hand'.
 */
udaprobe(reg, ctlr, um)
	caddr_t reg;
	int ctlr;
	struct uba_ctlr *um;
{
	register int br, cvec;
	register struct uda_softc *sc;
	register struct udadevice *udaddr;
	register struct mscp_info *mi;
	int timeout, tries;

#ifdef VAX750
	/*
	 * The UDA50 wants to share BDPs on 750s, but not on 780s or
	 * 8600s.  (730s have no BDPs anyway.)  Toward this end, we
	 * here set the `keep bdp' flag in the per-driver information
	 * if this is a 750.  (We just need to do it once, but it is
	 * easiest to do it now, for each UDA50.)
	 */
	if (cpu == VAX_750)
		udadriver.ud_keepbdp = 1;
#endif

	probeum = um;			/* remember for udaslave() */
#ifdef lint
	br = 0; cvec = br; br = cvec; udaintr(0);
#endif
	/*
	 * Set up the controller-specific generic MSCP driver info.
	 * Note that this should really be done in the (nonexistent)
	 * controller attach routine.
	 */
	sc = &uda_softc[ctlr];
	mi = &sc->sc_mi;
	mi->mi_md = &udamscpdriver;
	mi->mi_um = um;
	mi->mi_ip = udaip[ctlr];
	mi->mi_cmd.mri_size = NCMD;
	mi->mi_cmd.mri_desc = uda[ctlr].uda_ca.ca_cmddsc;
	mi->mi_cmd.mri_ring = uda[ctlr].uda_cmd;
	mi->mi_rsp.mri_size = NRSP;
	mi->mi_rsp.mri_desc = uda[ctlr].uda_ca.ca_rspdsc;
	mi->mi_rsp.mri_ring = uda[ctlr].uda_rsp;
	mi->mi_wtab.av_forw = mi->mi_wtab.av_back = &mi->mi_wtab;

	/*
	 * More controller specific variables.  Again, this should
	 * be in the controller attach routine.
	 */
	if (udaburst[ctlr] == 0)
		udaburst[ctlr] = DEFAULT_BURST;
		
	/*
	 * Get an interrupt vector.  Note that even if the controller
	 * does not respond, we keep the vector.  This is not a serious
	 * problem; but it would be easily fixed if we had a controller
	 * attach routine.  Sigh.
	 */
	sc->sc_ivec = (uba_hd[numuba].uh_lastiv -= 4);
	udaddr = (struct udadevice *) reg;

	/*
	 * Initialise the controller (partially).  The UDA50 programmer's
	 * manual states that if initialisation fails, it should be retried
	 * at least once, but after a second failure the port should be
	 * considered `down'; it also mentions that the controller should
	 * initialise within ten seconds.  Or so I hear; I have not seen
	 * this manual myself.
	 *
	 * N.B.: mfpr(TODR) will not work on uVaxen.
	 */
	tries = 0;
again:
	udaddr->udaip = 0;		/* start initialisation */
	timeout = mfpr(TODR) + 1000;	/* timeout in 10 seconds */
	while ((udaddr->udasa & UDA_STEP1) == 0)
		if (mfpr(TODR) > timeout)
			goto bad;
	udaddr->udasa = UDA_ERR | (NCMDL2 << 11) | (NRSPL2 << 8) | UDA_IE |
		(sc->sc_ivec >> 2);
	while ((udaddr->udasa & UDA_STEP2) == 0)
		if (mfpr(TODR) > timeout)
			goto bad;

	/* should have interrupted by now */
	return (sizeof (struct udadevice));
bad:
	if (++tries < 2)
		goto again;
	return (0);
}

/*
 * Find a slave.  We allow wildcard slave numbers (something autoconf
 * is not really prepared to deal with); and we need to know the
 * controller number to talk to the UDA.  For the latter, we keep
 * track of the last controller probed, since a controller probe
 * immediately precedes all slave probes for that controller.  For the
 * former, we simply put the unit number into ui->ui_slave after we
 * have found one.
 *
 * Note that by the time udaslave is called, the interrupt vector
 * for the UDA50 has been set up (so that udaunconf() will be called).
 */
udaslave(ui, reg)
	register struct uba_device *ui;
	caddr_t reg;
{
	register struct uba_ctlr *um = probeum;
	register struct mscp *mp;
	register struct uda_softc *sc;
	int next = 0, type, timeout, tries, i;

#ifdef lint
	i = 0; i = i;
#endif
	/*
	 * Make sure the controller is fully initialised, by waiting
	 * for it if necessary.
	 */
	sc = &uda_softc[um->um_ctlr];
	if (sc->sc_state == ST_RUN)
		goto findunit;
	tries = 0;
again:
	if (udainit(ui->ui_ctlr))
		return (0);
	timeout = mfpr(TODR) + 1000;		/* 10 seconds */
	while (mfpr(TODR) < timeout)
		if (sc->sc_state == ST_RUN)	/* made it */
			goto findunit;
	if (++tries < 2)
		goto again;
	printf("uda%d: controller hung\n", um->um_ctlr);
	return (0);

	/*
	 * The controller is all set; go find the unit.  Grab an
	 * MSCP packet and send out a Get Unit Status command, with
	 * the `next unit' modifier if we are looking for a generic
	 * unit.  We set the `in slave' flag so that udaunconf()
	 * knows to copy the response to `udaslavereply'.
	 */
findunit:
	udaslavereply.mscp_opcode = 0;
	sc->sc_flags |= SC_INSLAVE;
	if ((mp = mscp_getcp(&sc->sc_mi, MSCP_DONTWAIT)) == NULL)
		panic("udaslave");		/* `cannot happen' */
	mp->mscp_opcode = M_OP_GETUNITST;
	if (ui->ui_slave == '?') {
		mp->mscp_unit = next;
		mp->mscp_modifier = M_GUM_NEXTUNIT;
	} else {
		mp->mscp_unit = ui->ui_slave;
		mp->mscp_modifier = 0;
	}
	*mp->mscp_seq.seq_addr |= MSCP_OWN | MSCP_INT;
	i = ((struct udadevice *) reg)->udaip;	/* initiate polling */
	mp = &udaslavereply;
	timeout = mfpr(TODR) + 1000;
	while (mfpr(TODR) < timeout)
		if (mp->mscp_opcode)
			goto gotit;
	printf("uda%d: no response to Get Unit Status request\n",
		um->um_ctlr);
	sc->sc_flags &= ~SC_INSLAVE;
	return (0);

gotit:
	sc->sc_flags &= ~SC_INSLAVE;

	/*
	 * Got a slave response.  If the unit is there, use it.
	 */
	switch (mp->mscp_status & M_ST_MASK) {

	case M_ST_SUCCESS:	/* worked */
	case M_ST_AVAILABLE:	/* found another drive */
		break;		/* use it */

	case M_ST_OFFLINE:
		/*
		 * Figure out why it is off line.  It may be because
		 * it is nonexistent, or because it is spun down, or
		 * for some other reason.
		 */
		switch (mp->mscp_status & ~M_ST_MASK) {

		case M_OFFLINE_UNKNOWN:
			/*
			 * No such drive, and there are none with
			 * higher unit numbers either, if we are
			 * using M_GUM_NEXTUNIT.
			 */
			return (0);

		case M_OFFLINE_UNMOUNTED:
			/*
			 * The drive is not spun up.  Use it anyway.
			 *
			 * N.B.: this seems to be a common occurrance
			 * after a power failure.  The first attempt
			 * to bring it on line seems to spin it up
			 * (and thus takes several minutes).  Perhaps
			 * we should note here that the on-line may
			 * take longer than usual.
			 */
			break;

		default:
			/*
			 * In service, or something else equally unusable.
			 */
			printf("uda%d: unit %d off line: ", um->um_ctlr,
				mp->mscp_unit);
			mscp_printevent(mp);
			goto try_another;
		}
		break;

	default:
		printf("uda%d: unable to get unit status: ", um->um_ctlr);
		mscp_printevent(mp);
		return (0);
	}

	/*
	 * Does this ever happen?  What (if anything) does it mean?
	 */
	if (mp->mscp_unit < next) {
		printf("uda%d: unit %d, next %d\n",
			um->um_ctlr, mp->mscp_unit, next);
		return (0);
	}

	if (mp->mscp_unit >= MAXUNIT) {
		printf("uda%d: cannot handle unit number %d (max is %d)\n",
			um->um_ctlr, mp->mscp_unit, MAXUNIT - 1);
		return (0);
	}

	/*
	 * See if we already handle this drive.
	 * (Only likely if ui->ui_slave=='?'.)
	 */
	if (udaip[um->um_ctlr][mp->mscp_unit] != NULL)
		goto try_another;

	/*
	 * Make sure we know about this kind of drive.
	 * Others say we should treat unknowns as RA81s; I am
	 * not sure this is safe.
	 */
	type = mp->mscp_guse.guse_drivetype;
	if (type >= NTYPES || udatypes[type].ut_name == 0) {
		printf("uda%d: unit %d (media ID `", um->um_ctlr,
			mp->mscp_unit);
		uda_decode_media(mp->mscp_guse.guse_mediaid);
		printf("') is of unknown type %d; ignored\n", type);
try_another:
		if (ui->ui_slave != '?')
			return (0);
		next = mp->mscp_unit + 1;
		goto findunit;
	}
#if GYRE
	if (ISCDC(sc))
		type = CDCTYPE;
#endif

	/*
	 * Voila!
	 */
	ui->ui_type = type;
	ui->ui_flags = 0;	/* not on line, nor anything else */
	ui->ui_slave = mp->mscp_unit;
	return (1);
}

/*
 * Decode and print the media ID.  It is made up of five 5-bit
 * `characters' and 7 bits of numeric information.  BITS(i)
 * selects character i's bits; CHAR returns the corresponding
 * character.
 */
uda_decode_media(id)
	register long id;
{
	int c4, c3, c2, c1, c0;
#define	BITS(i)	((id >> ((i) * 5 + 7)) & 0x1f)
#define	CHAR(c)	((c) ? (c) + '@' : ' ')

	c4 = BITS(4);
	c3 = BITS(3);
	c2 = BITS(2);
	c1 = BITS(1);
	c0 = BITS(0);
	printf("%c%c %c%c%c%d", CHAR(c4), CHAR(c3), CHAR(c2),
		CHAR(c1), CHAR(c0), id & 0x7f);
#undef	BITS
#undef	CHAR
}

/*
 * Attach a found slave.  Make sure the watchdog timer is running.
 * If this disk is being profiled, fill in the `mspw' value (used by
 * what?).  Set up the inverting pointer, and attempt to bring the
 * drive on line.
 */
udaattach(ui)
	register struct uba_device *ui;
{

	if (udawstart == 0) {
		timeout(udawatch, (caddr_t) 0, hz);
		udawstart++;
	}
	if (ui->ui_dk >= 0)
		dk_mspw[ui->ui_dk] = 1.0 / (60 * 31 * 256);	/* approx */
	udaip[ui->ui_ctlr][ui->ui_slave] = ui;
	(void) uda_bringonline(&uda_softc[ui->ui_ctlr], ui, 1);
	/* should we get its status too? */
}

/*
 * Initialise a UDA50.  Return true iff something goes wrong.
 */
udainit(ctlr)
	int ctlr;
{
	register struct uda_softc *sc;
	register struct udadevice *udaddr;
	struct uba_ctlr *um;
	int timo, ubinfo;

	sc = &uda_softc[ctlr];
	um = udaminfo[ctlr];
	if ((sc->sc_flags & SC_MAPPED) == 0) {
		/*
		 * Map the communication area and command and
		 * response packets into Unibus space.
		 */
		ubinfo = uballoc(um->um_ubanum, (caddr_t) &uda[ctlr],
			sizeof (struct uda), UBA_CANTWAIT);
		if (ubinfo == 0) {
			printf("uda%d: uballoc map failed\n", ctlr);
			return (-1);
		}
		sc->sc_uda = (struct uda *) (ubinfo & 0x3ffff);
		sc->sc_flags |= SC_MAPPED;
	}

	/*
	 * While we are thinking about it, reset the next command
	 * and response indicies.
	 */
	sc->sc_mi.mi_cmd.mri_next = 0;
	sc->sc_mi.mi_rsp.mri_next = 0;

	/*
	 * Start up the hardware initialisation sequence.
	 */
#define	STEP0MASK	(UDA_ERR | UDA_STEP4 | UDA_STEP3 | UDA_STEP2 | \
			 UDA_STEP1 | UDA_NV)

	sc->sc_state = ST_IDLE;	/* in case init fails */
	udaddr = (struct udadevice *) um->um_addr;
	udaddr->udaip = 0;
	timo = mfpr(TODR) + 1000;
	while ((udaddr->udasa & STEP0MASK) == 0) {
		if (mfpr(TODR) > timo) {
			printf("uda%d: timeout during init\n", ctlr);
			return (-1);
		}
	}
	if ((udaddr->udasa & STEP0MASK) != UDA_STEP1) {
		printf("uda%d: init failed, sa=%b\n", ctlr,
			udaddr->udasa, udasr_bits);
		return (-1);
	}

	/*
	 * Success!  Record new state, and start step 1 initialisation.
	 * The rest is done in the interrupt handler.
	 */
	sc->sc_state = ST_STEP1;
	udaddr->udasa = UDA_ERR | (NCMDL2 << 11) | (NRSPL2 << 8) | UDA_IE |
	    (sc->sc_ivec >> 2);
	return (0);
}

/*
 * Open a drive.
 */
/*ARGSUSED*/
udaopen(dev, flag)
	dev_t dev;
	int flag;
{
	register int unit;
	register struct uba_device *ui;
	register struct uda_softc *sc;
	int s;

	/*
	 * Make sure this is a reasonable open request.
	 */
	unit = udaunit(dev);
	if (unit >= NRA || (ui = udadinfo[unit]) == 0 || ui->ui_alive == 0)
		return (ENXIO);

	/*
	 * Make sure the controller is running, by (re)initialising it if
	 * necessary.
	 */
	sc = &uda_softc[ui->ui_ctlr];
	s = spl5();
	if (sc->sc_state != ST_RUN) {
		if (sc->sc_state == ST_IDLE && udainit(ui->ui_ctlr)) {
			splx(s);
			return (EIO);
		}
		/*
		 * In case it does not come up, make sure we will be
		 * restarted in 10 seconds.  This corresponds to the
		 * 10 second timeouts in udaprobe() and udaslave().
		 */
		sc->sc_flags |= SC_DOWAKE;
		timeout(wakeup, (caddr_t) sc, 10 * hz);
		sleep((caddr_t) sc, PRIBIO);
		if (sc->sc_state != ST_RUN) {
			splx(s);
			printf("uda%d: controller hung\n", ui->ui_ctlr);
			return (EIO);
		}
		untimeout(wakeup, (caddr_t) sc);
	}
	if ((ui->ui_flags & UNIT_ONLINE) == 0) {
		/*
		 * Bring the drive on line so we can find out how
		 * big it is.  If it is not spun up, it will not
		 * come on line; this cannot really be considered
		 * an `error condition'.
		 */
		if (uda_bringonline(sc, ui, 0)) {
			splx(s);
			printf("ra%d: drive will not come on line\n", unit);
			return (EIO);
		}
	}
	splx(s);
	return (0);
}

/*
 * Bring a drive on line.  In case it fails to respond, we set
 * a timeout on it.  The `nosleep' parameter should be set if
 * we are to spin-wait; otherwise this must be called at spl5().
 */
uda_bringonline(sc, ui, nosleep)
	register struct uda_softc *sc;
	register struct uba_device *ui;
	int nosleep;
{
	register struct mscp *mp;
	int i;

	if (nosleep) {
		mp = mscp_getcp(&sc->sc_mi, MSCP_DONTWAIT);
		if (mp == NULL)
			return (-1);
	} else
		mp = mscp_getcp(&sc->sc_mi, MSCP_WAIT);
	mp->mscp_opcode = M_OP_ONLINE;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_cmdref = (long) &ui->ui_flags;
	*mp->mscp_seq.seq_addr |= MSCP_OWN | MSCP_INT;
	i = ((struct udadevice *) ui->ui_addr)->udaip;

	if (nosleep) {
		i = mfpr(TODR) + 1000;
		while ((ui->ui_flags & UNIT_ONLINE) == 0)
			if (mfpr(TODR) > i)
				return (-1);
	} else {
		timeout(wakeup, (caddr_t) &ui->ui_flags, 10 * hz);
		sleep((caddr_t) &ui->ui_flags, PRIBIO);
		if ((ui->ui_flags & UNIT_ONLINE) == 0)
			return (-1);
		untimeout(wakeup, (caddr_t) &ui->ui_flags);
	}
	return (0);	/* made it */
}

/*
 * Queue a transfer request, and if possible, hand it to the controller.
 *
 * This routine is broken into two so that the internal version
 * udastrat1() can be called by the (nonexistent, as yet) bad block
 * revectoring routine.
 */
udastrategy(bp)
	register struct buf *bp;
{
	register int unit;
	register struct uba_device *ui;
	register struct size *st;
	daddr_t sz, maxsz;

	/*
	 * Make sure this is a reasonable drive to use.
	 */
	if ((unit = udaunit(bp->b_dev)) >= NRA ||
	    (ui = udadinfo[unit]) == NULL || ui->ui_alive == 0) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		biodone(bp);
		return;
	}

	/*
	 * Determine the size of the transfer, and make sure it is
	 * within the boundaries of the drive.
	 */
	sz = (bp->b_bcount + 511) >> 9;
	st = &udatypes[ui->ui_type].ut_sizes[udapart(bp->b_dev)];
	if ((maxsz = st->nblocks) < 0)
		maxsz = ra_dsize[unit] - st->blkoff;
	if (bp->b_blkno < 0 || bp->b_blkno + sz > maxsz ||
	    st->blkoff >= ra_dsize[unit]) {
		/* if exactly at end of disk, return an EOF */
		if (bp->b_blkno == maxsz)
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_error = EINVAL;
			bp->b_flags |= B_ERROR;
		}
		biodone(bp);
		return;
	}
	udastrat1(bp);
}

/*
 * Work routine for udastrategy.
 */
udastrat1(bp)
	register struct buf *bp;
{
	register int unit = udaunit(bp->b_dev);
	register struct uba_ctlr *um;
	register struct buf *dp;
	struct uba_device *ui;
	int s = spl5();

	/*
	 * Append the buffer to the drive queue, and if it is not
	 * already there, the drive to the controller queue.  (However,
	 * if the drive queue is marked to be requeued, we must be
	 * awaiting an on line or get unit status command; in this
	 * case, leave it off the controller queue.)
	 */
	um = (ui = udadinfo[unit])->ui_mi;
	dp = &udautab[unit];
	APPEND(bp, dp, av_forw);
	if (dp->b_active == 0 && (ui->ui_flags & UNIT_REQUEUE) == 0) {
		APPEND(dp, &um->um_tab, b_forw);
		dp->b_active++;
	}

	/*
	 * Start activity on the controller.  Note that unlike other
	 * Unibus drivers, we must always do this, not just when the
	 * controller is not active.
	 */
	udastart(um);
	splx(s);
}

/*
 * Start up whatever transfers we can find.
 * Note that udastart() must be called at spl5().
 */
udastart(um)
	register struct uba_ctlr *um;
{
	register struct uda_softc *sc = &uda_softc[um->um_ctlr];
	register struct buf *bp, *dp;
	register struct mscp *mp;
	struct uba_device *ui;
	struct udadevice *udaddr;
	int i;

#ifdef lint
	i = 0; i = i;
#endif
	/*
	 * If it is not running, try (again and again...) to initialise
	 * it.  If it is currently initialising just ignore it for now.
	 */
	if (sc->sc_state != ST_RUN) {
		if (sc->sc_state == ST_IDLE && udainit(um->um_ctlr))
			printf("uda%d: still hung\n", um->um_ctlr);
		return;
	}

	/*
	 * If um_cmd is nonzero, this controller is on the Unibus
	 * resource wait queue.  It will not help to try more requests;
	 * instead, when the Unibus unblocks and calls udadgo(), we
	 * will call udastart() again.
	 */
	if (um->um_cmd)
		return;

	sc->sc_flags |= SC_INSTART;
	udaddr = (struct udadevice *) um->um_addr;

loop:
	/*
	 * Service the drive at the head of the queue.  We take exactly
	 * one transfer from this drive, then move it to the end of the
	 * controller queue, so as to get more drive overlap.
	 */
	if ((dp = um->um_tab.b_actf) == NULL) {
		um->um_tab.b_active = 0;
		goto out;
	}

	/*
	 * Get the first request from the drive queue.  There must be
	 * one, or something is rotten.
	 */
	if ((bp = dp->b_actf) == NULL)
		panic("udastart: bp==NULL\n");

	if (udaddr->udasa & UDA_ERR) {	/* ctlr fatal error */
		udasaerror(um);
		goto out;
	}

	/*
	 * Get an MSCP packet, then figure out what to do.  If
	 * we cannot get a command packet, the command ring may
	 * be too small:  We should have at least as many command
	 * packets as credits, for best performance.
	 */
	if ((mp = mscp_getcp(&sc->sc_mi, MSCP_DONTWAIT)) == NULL) {
		if (sc->sc_mi.mi_credits > MSCP_MINCREDITS &&
		    (sc->sc_flags & SC_GRIPED) == 0) {
			log(LOG_NOTICE, "uda%d: command ring too small\n",
				um->um_ctlr);
			sc->sc_flags |= SC_GRIPED;/* complain only once */
		}
		goto out;
	}

	/*
	 * Mark the controller active, but do not move the drive off
	 * the controller queue: ubago() wants it there.
	 */
	um->um_tab.b_active = 1;

	/*
	 * Bring the drive on line if it is not already.  Get its status
	 * if we do not already have it.  Otherwise just start the transfer.
	 */
	ui = udadinfo[udaunit(bp->b_dev)];
	if ((ui->ui_flags & UNIT_ONLINE) == 0) {
		mp->mscp_opcode = M_OP_ONLINE;
		goto common;
	}
	if ((ui->ui_flags & UNIT_HAVESTATUS) == 0) {
		mp->mscp_opcode = M_OP_GETUNITST;
common:
if (ui->ui_flags & UNIT_REQUEUE) panic("udastart");
		/*
		 * Take the drive off the controller queue.  When the
		 * command finishes, make sure the drive is requeued.
		 */
		um->um_tab.b_actf = dp->b_forw;
		dp->b_active = 0;
		ui->ui_flags |= UNIT_REQUEUE;
		mp->mscp_unit = ui->ui_slave;
		*mp->mscp_seq.seq_addr |= MSCP_OWN | MSCP_INT;
		i = udaddr->udaip;
		goto loop;
	}

	mp->mscp_opcode = (bp->b_flags & B_READ) ? M_OP_READ : M_OP_WRITE;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_seq.seq_lbn = bp->b_blkno +
		udatypes[ui->ui_type].ut_sizes[udapart(bp->b_dev)].blkoff;
	mp->mscp_seq.seq_bytecount = bp->b_bcount;
	/* mscp_cmdref and mscp_buffer are filled in by mscp_go() */

	/*
	 * Drop the packet pointer into the `command' field so udadgo()
	 * can tell what to start.  If ubago returns 1, we can do another
	 * transfer.  If not, um_cmd will still point at mp, so we will
	 * know that we are waiting for resources.
	 */
	um->um_cmd = (int) mp;
	if (ubago(ui))
		goto loop;

	/*
	 * All done, or blocked in ubago().
	 */
out:
	sc->sc_flags &= ~SC_INSTART;
}

/*
 * Start a transfer.
 *
 * If we are not called from within udastart(), we must have been
 * blocked, so call udastart to do more requests (if any).  If
 * this calls us again immediately we will not recurse, because
 * that time we will be in udastart().  Clever....
 */
udadgo(um)
	register struct uba_ctlr *um;
{
	struct uda_softc *sc = &uda_softc[um->um_ctlr];
	int i;

#ifdef lint
	i = 0; i = i;
#endif
	/*
	 * Fill in the MSCP packet and move the buffer to the
	 * I/O wait queue.  Mark the controller as no longer on
	 * the resource queue, and initiate polling.
	 */
	mscp_go(um, &sc->sc_mi, (struct mscp *) um->um_cmd);
	um->um_cmd = 0;	
	i = ((struct udadevice *) um->um_addr)->udaip;

	if ((sc->sc_flags & SC_INSTART) == 0)
		udastart(um);
}

/*
 * The error bit was set in the controller status register.  Gripe,
 * reset the controller, requeue pending transfers.
 */
udasaerror(um)
	register struct uba_ctlr *um;
{

	printf("uda%d: controller error, sa=%b\n", um->um_ctlr,
		((struct udadevice *) um->um_addr)->udasa, udasr_bits);
	mscp_requeue(&uda_softc[um->um_ctlr].sc_mi);
	(void) udainit(um->um_ctlr);
}

/*
 * Interrupt routine.  Depending on the state of the controller,
 * continue initialisation, or acknowledge command and response
 * interrupts, and process responses.
 */
udaintr(ctlr)
	int ctlr;
{
	register struct uba_ctlr *um = udaminfo[ctlr];
	register struct uda_softc *sc = &uda_softc[ctlr];
	register struct udadevice *udaddr = (struct udadevice *) um->um_addr;
	register struct uda *ud;
	register struct mscp *mp;
	register int i;

	sc->sc_wticks = 0;	/* reset interrupt watchdog */

	/*
	 * Combinations during steps 1, 2, and 3: STEPnMASK
	 * corresponds to which bits should be tested;
	 * STEPnGOOD corresponds to the pattern that should
	 * appear after the interrupt from STEPn initialisation.
	 * All steps test the bits in ALLSTEPS.
	 */
#define	ALLSTEPS	(UDA_ERR | UDA_STEP4 | UDA_STEP3 | UDA_STEP2 | \
			 UDA_STEP1 | UDA_IE)

#define	STEP1MASK	(ALLSTEPS | UDA_NCNRMASK)
#define	STEP1GOOD	(UDA_STEP2 | UDA_IE | (NCMDL2 << 3) | NRSPL2)

#define	STEP2MASK	(ALLSTEPS | UDA_IVECMASK)
#define	STEP2GOOD	(UDA_STEP3 | UDA_IE | (sc->sc_ivec >> 2))

#define	STEP3MASK	ALLSTEPS
#define	STEP3GOOD	UDA_STEP4

	switch (sc->sc_state) {

	case ST_IDLE:
		/*
		 * Ignore unsolicited interrupts.
		 */
		log(LOG_WARNING, "uda%d: stray intr\n", ctlr);
		return;

	case ST_STEP1:
		/*
		 * Begin step two initialisation.
		 */
		if ((udaddr->udasa & STEP1MASK) != STEP1GOOD) {
			i = 1;
initfailed:
			printf("uda%d: init step %d failed, sa=%b\n",
				ctlr, i, udaddr->udasa, udasr_bits);
			sc->sc_state = ST_IDLE;
			if (sc->sc_flags & SC_DOWAKE) {
				sc->sc_flags &= ~SC_DOWAKE;
				wakeup((caddr_t) sc);
			}
			return;
		}
		udaddr->udasa = (int) &sc->sc_uda->uda_ca.ca_rspdsc[0] |
			(cpu == VAX_780 || cpu == VAX_8600 ? UDA_PI : 0);
		sc->sc_state = ST_STEP2;
		return;

	case ST_STEP2:
		/*
		 * Begin step 3 initialisation.
		 */
		if ((udaddr->udasa & STEP2MASK) != STEP2GOOD) {
			i = 2;
			goto initfailed;
		}
		udaddr->udasa = ((int) &sc->sc_uda->uda_ca.ca_rspdsc[0]) >> 16;
		sc->sc_state = ST_STEP3;
		return;

	case ST_STEP3:
		/*
		 * Set controller characteristics (finish initialisation).
		 */
		if ((udaddr->udasa & STEP3MASK) != STEP3GOOD) {
			i = 3;
			goto initfailed;
		}
		i = udaddr->udasa & 0xff;
		if (i != sc->sc_micro) {
			sc->sc_micro = i;
			printf("uda%d: version %d model %d\n",
				ctlr, i & 0xf, i >> 4);
		}

		/*
		 * Present the burst size, then remove it.  Why this
		 * should be done this way, I have no idea.
		 *
		 * Note that this assumes udaburst[ctlr] > 0.
		 */
		udaddr->udasa = UDA_GO | (udaburst[ctlr] - 1) << 2;
		udaddr->udasa = UDA_GO;
		printf("uda%d: DMA burst size set to %d\n",
			ctlr, udaburst[ctlr]);

		udainitds(ctlr);	/* initialise data structures */

		/*
		 * Before we can get a command packet, we need some
		 * credits.  Fake some up to keep mscp_getcp() happy,
		 * get a packet, and cancel all credits (the right
		 * number should come back in the response to the
		 * SCC packet).
		 */
		sc->sc_mi.mi_credits = MSCP_MINCREDITS + 1;
		mp = mscp_getcp(&sc->sc_mi, MSCP_DONTWAIT);
		if (mp == NULL)	/* `cannot happen' */
			panic("udaintr");
		sc->sc_mi.mi_credits = 0;
		mp->mscp_opcode = M_OP_SETCTLRC;
		/*
		 * WHICH OF THE FOLLOWING ARE REQUIRED??
		 * IT SURE WOULD BE NICE IF DEC SOLD DOCUMENTATION
		 * FOR THEIR OWN CONTROLLERS.
		 */
		mp->mscp_unit = 0;
		mp->mscp_flags = 0;
		mp->mscp_modifier = 0;
		mp->mscp_seq.seq_bytecount = 0;
		mp->mscp_seq.seq_buffer = 0;
		mp->mscp_sccc.sccc_errlgfl = 0;
		mp->mscp_sccc.sccc_copyspd = 0;
		mp->mscp_sccc.sccc_ctlrflags = M_CF_ATTN | M_CF_MISC |
			M_CF_THIS;
		*mp->mscp_sccc.sccc_addr |= MSCP_OWN | MSCP_INT;
		i = udaddr->udaip;
		sc->sc_state = ST_SETCHAR;
		return;

	case ST_SETCHAR:
	case ST_RUN:
		/*
		 * Handle Set Ctlr Characteristics responses and operational
		 * responses (via mscp_dorsp).
		 */
		break;

	default:
		printf("uda%d: driver bug, state %d\n", ctlr, sc->sc_state);
		panic("udastate");
	}

	if (udaddr->udasa & UDA_ERR) {	/* ctlr fatal error */
		udasaerror(um);
		return;
	}

	ud = &uda[ctlr];

	/*
	 * Handle buffer purge requests.
	 * I have never seen these to work usefully, thus the log().
	 */
	if (ud->uda_ca.ca_bdp) {
		log(LOG_DEBUG, "uda%d: purge bdp %d\n",
			ctlr, ud->uda_ca.ca_bdp);
		UBAPURGE(um->um_hd->uh_uba, ud->uda_ca.ca_bdp);
		ud->uda_ca.ca_bdp = 0;
		udaddr->udasa = 0;	/* signal purge complete */
	}

	/*
	 * Check for response and command ring transitions.
	 */
	if (ud->uda_ca.ca_rspint) {
		ud->uda_ca.ca_rspint = 0;
		mscp_dorsp(&sc->sc_mi);
	}
	if (ud->uda_ca.ca_cmdint) {
		ud->uda_ca.ca_cmdint = 0;
		MSCP_DOCMD(&sc->sc_mi);
	}
	udastart(um);
}

/*
 * Initialise the various data structures that control the UDA50.
 */
udainitds(ctlr)
	int ctlr;
{
	register struct uda *ud = &uda[ctlr];
	register struct uda *uud = uda_softc[ctlr].sc_uda;
	register struct mscp *mp;
	register int i;

	for (i = 0, mp = ud->uda_rsp; i < NRSP; i++, mp++) {
		ud->uda_ca.ca_rspdsc[i] = MSCP_OWN | MSCP_INT |
			(long) &uud->uda_rsp[i].mscp_cmdref;
		mp->mscp_seq.seq_addr = &ud->uda_ca.ca_rspdsc[i];
		mp->mscp_msglen = MSCP_MSGLEN;
	}
	for (i = 0, mp = ud->uda_cmd; i < NCMD; i++, mp++) {
		ud->uda_ca.ca_cmddsc[i] = MSCP_INT |
			(long) &uud->uda_cmd[i].mscp_cmdref;
		mp->mscp_seq.seq_addr = &ud->uda_ca.ca_cmddsc[i];
		mp->mscp_msglen = MSCP_MSGLEN;
	}
}

/*
 * Handle an error datagram.  All we do now is decode it.
 */
udadgram(um, mp)
	struct uba_ctlr *um;
	struct mscp *mp;
{

	mscp_decodeerror(um, mp);
}

/*
 * The Set Controller Characteristics command finished.
 * Record the new state of the controller.
 */
udactlrdone(um, mp)
	register struct uba_ctlr *um;
	struct mscp *mp;
{
	register struct uda_softc *sc = &uda_softc[um->um_ctlr];

	if ((mp->mscp_status & M_ST_MASK) == M_ST_SUCCESS)
		sc->sc_state = ST_RUN;
	else {
		printf("uda%d: SETCTLRC failed: ",
			um->um_ctlr, mp->mscp_status);
		mscp_printevent(mp);
		sc->sc_state = ST_IDLE;
	}
	um->um_tab.b_active = 0;
	if (sc->sc_flags & SC_DOWAKE) {
		sc->sc_flags &= ~SC_DOWAKE;
		wakeup((caddr_t) sc);
	}
}

/*
 * Received a response from an as-yet unconfigured drive.  Configure it
 * in, if possible.
 */
udaunconf(um, mp)
	struct uba_ctlr *um;
	register struct mscp *mp;
{

	/*
	 * If it is a slave response, copy it to udaslavereply for
	 * udaslave() to look at.
	 */
	if (mp->mscp_opcode == (M_OP_GETUNITST | M_OP_END) &&
	    (uda_softc[um->um_ctlr].sc_flags & SC_INSLAVE) != 0) {
		udaslavereply = *mp;
		return (MSCP_DONE);
	}

	/*
	 * Otherwise, it had better be an available attention response.
	 */
	if (mp->mscp_opcode != M_OP_AVAILATTN)
		return (MSCP_FAILED);

	/* do what autoconf does */
	return (MSCP_FAILED);	/* not yet, arwhite, not yet */
}

/*
 * A drive came on line.  Check its type and size.  Return DONE if
 * we think the drive is truly on line.  In any case, awaken anyone
 * sleeping on the drive on-line-ness.
 */
udaonline(ui, mp)
	register struct uba_device *ui;
	struct mscp *mp;
{
	register int type;

	wakeup((caddr_t) &ui->ui_flags);
	if ((mp->mscp_status & M_ST_MASK) != M_ST_SUCCESS) {
		printf("uda%d: attempt to bring ra%d on line failed: ",
			ui->ui_ctlr, ui->ui_unit);
		mscp_printevent(mp);
		return (MSCP_FAILED);
	}

	type = mp->mscp_onle.onle_drivetype;
	if (type >= NTYPES || udatypes[type].ut_name == 0) {
		printf("uda%d: ra%d: unknown type %d\n",
			ui->ui_ctlr, ui->ui_unit, type);
		return (MSCP_FAILED);
	}
#if GYRE			/* special partition hack */
	if (ISCDC(&uda_softc[ui->ui_ctlr]))
		type = CDCTYPE;
#endif
	/*
	 * Note any change of types.  Not sure if we should do
	 * something special about them, or if so, what....
	 */
	if (type != ui->ui_type) {
		printf("ra%d: changed types! was %s\n",
			ui->ui_unit, udatypes[ui->ui_type].ut_name);
		ui->ui_type = type;
	}
	ra_dsize[ui->ui_unit] = (daddr_t) mp->mscp_onle.onle_unitsize;
	printf("ra%d: %s, size = %d sectors\n",
		ui->ui_unit, udatypes[type].ut_name, ra_dsize[ui->ui_unit]);
	return (MSCP_DONE);
}

/*
 * We got some (configured) unit's status.  Return DONE if it succeeded.
 */
udagotstatus(ui, mp)
	register struct uba_device *ui;
	register struct mscp *mp;
{

	if ((mp->mscp_status & M_ST_MASK) != M_ST_SUCCESS) {
		printf("uda%d: attempt to get status for ra%d failed: ",
			ui->ui_ctlr, ui->ui_unit);
		mscp_printevent(mp);
		return (MSCP_FAILED);
	}
	/* need to record later for bad block forwarding - for now, print */
	printf("\
ra%d: unit %d, nspt %d, group %d, ntpc %d, rctsize %d, nrpt %d, nrct %d\n",
		ui->ui_unit, mp->mscp_unit,
		mp->mscp_guse.guse_nspt, mp->mscp_guse.guse_group,
		mp->mscp_guse.guse_ntpc, mp->mscp_guse.guse_rctsize,
		mp->mscp_guse.guse_nrpt, mp->mscp_guse.guse_nrct);
	return (MSCP_DONE);
}

/*
 * A transfer failed.  We get a chance to fix or restart it.
 * Need to write the bad block forwaring code first....
 */
/*ARGSUSED*/
udaioerror(ui, mp, bp)
	register struct uba_device *ui;
	register struct mscp *mp;
	struct buf *bp;
{

	if (mp->mscp_flags & M_EF_BBLKR) {
		/*
		 * A bad block report.  Eventually we will
		 * restart this transfer, but for now, just
		 * log it and give up.
		 */
		log(LOG_ERR, "ra%d: bad block report: %d%s\n",
			ui->ui_unit, mp->mscp_seq.seq_lbn,
			mp->mscp_flags & M_EF_BBLKU ? " + others" : "");
	} else {
		/*
		 * What the heck IS a `serious exception' anyway?
		 * IT SURE WOULD BE NICE IF DEC SOLD DOCUMENTATION
		 * FOR THEIR OWN CONTROLLERS.
		 */
		if (mp->mscp_flags & M_EF_SEREX)
			log(LOG_ERR, "ra%d: serious exception reported\n",
				ui->ui_unit);
	}
	return (MSCP_FAILED);
}

/*
 * A replace operation finished.
 */
/*ARGSUSED*/
udareplace(ui, mp)
	struct uba_device *ui;
	struct mscp *mp;
{

	panic("udareplace");
}

/*
 * A bad block related operation finished.
 */
/*ARGSUSED*/
udabb(ui, mp, bp)
	struct uba_device *ui;
	struct mscp *mp;
	struct buf *bp;
{

	panic("udabb");
}


/*
 * Raw read request.
 */
udaread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = udaunit(dev);

	if (unit >= NRA)
		return (ENXIO);
	return (physio(udastrategy, &rudabuf[unit], dev, B_READ,
		minphys, uio));
}

/*
 * Raw write request.
 */
udawrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = udaunit(dev);

	if (unit >= NRA)
		return (ENXIO);
	return (physio(udastrategy, &rudabuf[unit], dev, B_WRITE,
		minphys, uio));
}

#ifdef notyet
/*
 * I/O controls.  Not yet!
 */
udaioctl(dev, cmd, flag, data)
	dev_t dev;
	int cmd, flag;
	caddr_t data;
{
	int error = 0;
	register int unit = udaunit(dev);

	if (unit >= NRA || uddinfo[unit] == NULL)
		return (ENXIO);

	switch (cmd) {

	case UDAIOCREPLACE:
		/*
		 * Initiate bad block replacement for the given LBN.
		 * (Should we allow modifiers?)
		 */
		error = EOPNOTSUPP;
		break;

	case UDAIOCGMICRO:
		/*
		 * Return the microcode revision for the UDA50 running
		 * this drive.
		 */
		*(int *) data = uda_softc[uddinfo[unit]->ui_ctlr].sc_micro;
		break;

	case UDAIOCGSIZE:
		/*
		 * Return the size (in 512 byte blocks) of this
		 * disk drive.
		 */
		*(daddr_t *) data = ra_dsize[unit];
		break;

	default:
		error = EINVAL;
		break;
	}
	return (error);
}
#endif

/*
 * A Unibus reset has occurred on UBA uban.  Reinitialise the controller(s)
 * on that Unibus, and requeue outstanding I/O.
 */
udareset(uban)
	int uban;
{
	register struct uba_ctlr *um;
	register struct uba_device *ui;
	register struct uda_softc *sc;
	register int unit, ctlr;
	struct buf *dp;

	for (ctlr = 0, sc = uda_softc; ctlr < NUDA; ctlr++, sc++) {
		if ((um = udaminfo[ctlr]) == NULL || um->um_ubanum != uban ||
		    um->um_alive == 0)
			continue;
		printf(" uda%d", ctlr);

		/*
		 * Our BDP (if any) is gone; our command (if any) is
		 * flushed; the device is no longer mapped; and the
		 * UDA50 is not yet initialised.
		 */
		if (um->um_bdp) {
			printf("<%d>", (um->um_bdp >> 28) & 15);
			um->um_bdp = 0;
		}
		um->um_ubinfo = 0;
		um->um_cmd = 0;
		sc->sc_flags &= ~SC_MAPPED;
		sc->sc_state = ST_IDLE;

		/* reset queues and requeue pending transfers */
		mscp_requeue(&sc->sc_mi);

		/*
		 * If it fails to initialise we will notice later and
		 * try again (and again...).  Do not call udastart()
		 * here; it will be done after the controller finishes
		 * initialisation.
		 */
		if (udainit(ctlr))
			printf(" (hung)");
	}
}

/*
 * Watchdog timer:  If the controller is active, and no interrupts
 * have occurred for 30 seconds, assume it has gone away.
 */
udawatch()
{
	register int i, unit;
	register struct uba_ctlr *um;
	register struct uda_softc *sc;

	timeout(udawatch, (caddr_t) 0, hz);	/* every second */
	for (i = 0, sc = uda_softc; i < NUDA; i++, sc++) {
		if ((um = udaminfo[i]) == 0 || um->um_alive == 0)
			continue;
		if (um->um_tab.b_active ||
		    sc->sc_mi.mi_wtab.av_forw != &sc->sc_mi.mi_wtab ||
		    sc->sc_state != ST_IDLE && sc->sc_state != ST_RUN)
			goto active;
		for (unit = 0; unit < NRA; unit++)
			if (udautab[unit].b_active &&
			    udadinfo[unit]->ui_mi == um)
				goto active;
		sc->sc_wticks = 0;
		continue;
active:
		if (++sc->sc_wticks >= 30) {
			sc->sc_wticks = 0;
			printf("uda%d: lost interrupt\n", i);
			ubareset(um->um_ubanum);
		}
	}
}

/*
 * Do a panic dump.  We set up the controller for one command packet
 * and one response packet, for which we use `struct uda1'.
 */
struct	uda1 {
	struct	uda1ca uda1_ca;	/* communications area */
	struct	mscp uda1_rsp;	/* response packet */
	struct	mscp uda1_cmd;	/* command packet */
} uda1;

#define	DBSIZE	32		/* dump 16K at a time */

udadump(dev)
	dev_t dev;
{
	struct udadevice *udaddr;
	struct uda1 *ud_ubaddr;
	char *start;
	int num, blk, unit, maxsz, blkoff, reg;
	register struct uba_regs *uba;
	register struct uba_device *ui;
	register struct uda1 *ud;
	register struct pte *io;
	register int i;

	/*
	 * Make sure the device is a reasonable place on which to dump.
	 */
	unit = udaunit(dev);
	if (unit >= NRA)
		return (ENXIO);
#define	phys(cast, addr)	((cast) ((int) addr & 0x7fffffff))
	ui = phys(struct uba_device *, udadinfo[unit]);
	if (ui == NULL || ui->ui_alive == 0)
		return (ENXIO);

	/*
	 * Find and initialise the UBA; get the physical address of the
	 * device registers, and of communications area and command and
	 * response packet.
	 */
	uba = phys(struct uba_hd *, ui->ui_hd)->uh_physuba;
	ubainit(uba);
	udaddr = (struct udadevice *) ui->ui_physaddr;
	ud = phys(struct uda1 *, &uda1);

	/*
	 * Map the ca+packets into Unibus I/O space so the UDA50 can get
	 * at them.  Use the registers at the end of the Unibus map (since
	 * we will use the registers at the beginning to map the memory
	 * we are dumping).
	 */
	num = btoc(sizeof (struct uda1)) + 1;
	reg = NUBMREG - num;
	io = &uba->uba_map[reg];
	for (i = 0; i < num; i++)
		*(int *)io++ = UBAMR_MRV | (btop(ud) + i);
	ud_ubaddr = (struct uda1 *) (((int) ud & PGOFSET) | (reg << 9));

	/*
	 * Initialise the controller, with one command and one response
	 * packet.
	 */
	udaddr->udaip = 0;
	if (udadumpwait(udaddr, UDA_STEP1))
		return (EFAULT);
	udaddr->udasa = UDA_ERR;
	if (udadumpwait(udaddr, UDA_STEP2))
		return (EFAULT);
	udaddr->udasa = (int) &ud_ubaddr->uda1_ca.ca_rspdsc;
	if (udadumpwait(udaddr, UDA_STEP3))
		return (EFAULT);
	udaddr->udasa = ((int) &ud_ubaddr->uda1_ca.ca_rspdsc) >> 16;
	if (udadumpwait(udaddr, UDA_STEP4))
		return (EFAULT);
	uda_softc[ui->ui_ctlr].sc_micro = udaddr->udasa & 0xff;
	udaddr->udasa = UDA_GO;

	/*
	 * Set up the command and response descriptor, then set the
	 * controller characteristics and bring the drive on line.
	 * Note that all uninitialised locations in uda1_cmd are zero.
	 */
	ud->uda1_ca.ca_rspdsc = (long) &ud_ubaddr->uda1_rsp.mscp_cmdref;
	ud->uda1_ca.ca_cmddsc = (long) &ud_ubaddr->uda1_cmd.mscp_cmdref;
	/* ud->uda1_cmd.mscp_sccc.sccc_ctlrflags = 0; */
	/* ud->uda1_cmd.mscp_sccc.sccc_version = 0; */
	if (udadumpcmd(M_OP_SETCTLRC, ud, ui))
		return (EFAULT);
	ud->uda1_cmd.mscp_unit = ui->ui_slave;
	if (udadumpcmd(M_OP_ONLINE, ud, ui))
		return (EFAULT);

	/*
	 * Pick up the drive type from the on line end packet;
	 * convert that to a dump area size and a disk offset.
	 */
	i = ud->uda1_rsp.mscp_onle.onle_drivetype;
	if (i >= NTYPES || udatypes[i].ut_name == 0) {
		printf("disk type %d unknown\ndump ");
		return (EINVAL);
	}
#if GYRE
	if (ISCDC(&uda_softc[ui->ui_ctlr]))
		i = CDCTYPE;
#endif
	printf("on %s ", udatypes[i].ut_name);
	maxsz = udatypes[i].ut_sizes[udapart(dev)].nblocks;
	blkoff = udatypes[i].ut_sizes[udapart(dev)].blkoff;

	/*
	 * Dump all of physical memory, or as much as will fit in the
	 * space provided.
	 */
	start = 0;
	num = maxfree;
	if (dumplo < 0)
		return (EINVAL);
	if (dumplo + num >= maxsz)
		num = maxsz - dumplo;
	blkoff += dumplo;

	/*
	 * Write out memory, DBSIZE pages at a time.
	 * N.B.: this code depends on the fact that the sector
	 * size == the page size.
	 */
	while (num > 0) {
		blk = num > DBSIZE ? DBSIZE : num;
		io = uba->uba_map;
		/*
		 * Map in the pages to write, leaving an invalid entry
		 * at the end to guard against wild Unibus transfers.
		 * Then do the write.
		 */
		for (i = 0; i < blk; i++)
			*(int *) io++ = UBAMR_MRV | (btop(start) + i);
		*(int *) io = 0;
		ud->uda1_cmd.mscp_unit = ui->ui_slave;
		ud->uda1_cmd.mscp_seq.seq_lbn = btop(start) + blkoff;
		ud->uda1_cmd.mscp_seq.seq_bytecount = blk << PGSHIFT;
		if (udadumpcmd(M_OP_WRITE, ud, ui))
			return (EIO);
		start += blk << PGSHIFT;
		num -= blk;
	}
	return (0);		/* made it! */
}

/*
 * Wait for some of the bits in `bits' to come on.  If the error bit
 * comes on, or ten seconds pass without response, return true (error).
 */
udadumpwait(udaddr, bits)
	register struct udadevice *udaddr;
	register int bits;
{
	register int timo = mfpr(TODR) + 1000;

	while ((udaddr->udasa & bits) == 0) {
		if (udaddr->udasa & UDA_ERR) {
			printf("udasa=%b\ndump ", udaddr->udasa, udasr_bits);
			return (1);
		}
		if (mfpr(TODR) >= timo) {
			printf("timeout\ndump ");
			return (1);
		}
	}
	return (0);
}

/*
 * Feed a command to the UDA50, wait for its response, and return
 * true iff something went wrong.
 */
udadumpcmd(op, ud, ui)
	int op;
	register struct uda1 *ud;
	struct uba_device *ui;
{
	register struct udadevice *udaddr;
	register int n;
#define mp (&ud->uda1_rsp)

	udaddr = (struct udadevice *) ui->ui_physaddr;
	ud->uda1_cmd.mscp_opcode = op;
	ud->uda1_cmd.mscp_msglen = MSCP_MSGLEN;
	ud->uda1_rsp.mscp_msglen = MSCP_MSGLEN;
	ud->uda1_ca.ca_rspdsc |= MSCP_OWN | MSCP_INT;
	ud->uda1_ca.ca_cmddsc |= MSCP_OWN | MSCP_INT;
	if (udaddr->udasa & UDA_ERR) {
		printf("udasa=%b\ndump ", udaddr->udasa, udasr_bits);
		return (1);
	}
	n = udaddr->udaip;
	n = mfpr(TODR) + 1000;
	for (;;) {
		if (mfpr(TODR) > n) {
			printf("timeout\ndump ");
			return (1);
		}
		if (ud->uda1_ca.ca_cmdint)
			ud->uda1_ca.ca_cmdint = 0;
		if (ud->uda1_ca.ca_rspint == 0)
			continue;
		ud->uda1_ca.ca_rspint = 0;
		if (mp->mscp_opcode == (op | M_OP_END))
			break;
		printf("\n");
		switch (MSCP_MSGTYPE(mp->mscp_msgtc)) {

		case MSCPT_SEQ:
			printf("sequential");
			break;

		case MSCPT_DATAGRAM:
			mscp_decodeerror(ui->ui_mi, mp);
			printf("datagram");
			break;

		case MSCPT_CREDITS:
			printf("credits");
			break;

		case MSCPT_MAINTENANCE:
			printf("maintenance");
			break;

		default:
			printf("unknown (type 0x%x)",
				MSCP_MSGTYPE(mp->mscp_msgtc));
			break;
		}
		printf(" ignored\ndump ");
		ud->uda1_ca.ca_rspdsc |= MSCP_OWN | MSCP_INT;
	}
	if ((mp->mscp_status & M_ST_MASK) != M_ST_SUCCESS) {
		printf("error: op 0x%x => 0x%x status 0x%x\ndump ", op,
			mp->mscp_opcode, mp->mscp_status);
		return (1);
	}
	return (0);
#undef mp
}

/*
 * Return the size of a partition, if known, or -1 if not.
 */
udasize(dev)
	dev_t dev;
{
	register int unit = udaunit(dev);
	register struct uba_device *ui;
	register struct size *st;

	if (unit >= NRA || (ui = udadinfo[unit]) == NULL || ui->ui_alive == 0)
		return (-1);
	st = &udatypes[ui->ui_type].ut_sizes[udapart(dev)];
	if (st->nblocks == -1) {
		int s = spl5();

		/*
		 * We need to have the drive on line to find the size
		 * of this particular partition.
		 * IS IT OKAY TO GO TO SLEEP IN THIS ROUTINE?
		 * (If not, better not page on one of these...)
		 */
		if ((ui->ui_flags & UNIT_ONLINE) == 0) {
			if (uda_bringonline(&uda_softc[unit], ui, 0)) {
				splx(s);
				return (-1);
			}
		}
		splx(s);
		if (st->blkoff > ra_dsize[unit])
			return (-1);
		return (ra_dsize[unit] - st->blkoff);
	}
	return (st->nblocks);
}
#endif
