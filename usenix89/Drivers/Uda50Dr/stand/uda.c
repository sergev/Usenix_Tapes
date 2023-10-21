/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uda.c	7.1 (Berkeley) 6/5/86
 */

/*
 * UDA50/RAxx disk device driver
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/inode.h"
#include "../h/fs.h"

#include "saio.h"
#include "savax.h"

#define	NRA	4
/*
 * Parameters for the communications area
 */
#define	NRSPL2	0
#define	NCMDL2	0
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

#include "../vaxuba/udareg.h"
#include "../vaxuba/ubareg.h"
#include "../vax/mscp.h"

u_short udastd[] = { 0772150 };

struct iob	cudbuf;

struct udadevice *udaddr = 0;

struct uda {
	struct udaca	uda_ca;
	struct mscp	uda_rsp;
	struct mscp	uda_cmd;
} uda;

struct uda *ud_ubaddr;			/* Unibus address of uda structure */

/* int ra25_off[] = { 0, 15884, 0, -1, -1, -1, 25916, -1 }; */
int ra60_off[] = { 0, 15884, 0, 49324, 131404, 49324, 242606, 49324 };
int ra80_off[] = { 0, 15884, 0, -1, 49324, 49324, 49910, 131404 };
#ifndef	UCBRA
#ifdef RA_COMPAT
int ra81_off[] = { 0, 16422, 0, 49324, 131404, 412490, 375564, 83538 };
#else
int ra81_off[] = { 0, 16422, 0, 375564, 391986, 699720, 375564, 83538 };
#endif
#else
int ra81_off[] = { 0, 15884, 0, 242606, 258490, 565690, 242606, 49324 };
#endif
int *ra_off[] = {
	0,
	ra80_off,		/* 1 = ra80 */
	ra81_off,		/* 2 = old ra81 microcode */
	0,			/* 3 = old ra60?? */
	ra60_off,		/* 4 = ra60 */
	ra81_off,		/* 5 = ra81 */
	/* WHAT TYPE IS ra25? */
};
#define	NTYPES (sizeof(ra_off) / sizeof(ra_off[0]))

static int ra_type[NRA];

raopen(io)
	register struct iob *io;
{
	register struct mscp *mp;
	static int udainit, udadriveinit[NRA];
	register int i, t;
	daddr_t off;

	if (udaddr == 0)
		udaddr = (struct udadevice *)ubamem(io->i_unit, udastd[0]);
	if (ud_ubaddr == 0) {
		/*
		 * Initialise cudbuf.i_unit so that controllers
		 * on UNIBUSes other than 0 can be used.
		 */
		cudbuf.i_unit = io->i_unit;
		cudbuf.i_ma = (caddr_t)&uda;
		cudbuf.i_cc = sizeof(uda);
		ud_ubaddr = (struct uda *)ubasetup(&cudbuf, 2);
	}
	if (udainit == 0) {
		udaddr->udaip = 0;
		while ((udaddr->udasa & UDA_STEP1) == 0)
			;
		udaddr->udasa = UDA_ERR;
		while ((udaddr->udasa & UDA_STEP2) == 0)
			;
		udaddr->udasa = (short)&ud_ubaddr->uda_ca.ca_rspdsc[0];
		while ((udaddr->udasa & UDA_STEP3) == 0)
			;
		udaddr->udasa =
			(short)(((int)&ud_ubaddr->uda_ca.ca_rspdsc[0]) >> 16);
		while ((udaddr->udasa & UDA_STEP4) == 0)
			;
		udaddr->udasa = UDA_GO;
		uda.uda_ca.ca_rspdsc[0] = (long)&ud_ubaddr->uda_rsp.mscp_cmdref;
		uda.uda_ca.ca_cmddsc[0] = (long)&ud_ubaddr->uda_cmd.mscp_cmdref;
		if (udcmd(M_OP_SETCTLRC)) {
			_stop("ra: open error, SETCTLRC");
			return;
		}
		udainit = 1;
	}
	i = io->i_unit & 7;
	if (udadriveinit[i] == 0) {
		uda.uda_cmd.mscp_unit = i;
		if (udcmd(M_OP_ONLINE)) {
			_stop("ra: open error, ONLINE");
			return;
		}
		t = ra_type[i] = uda.uda_rsp.mscp_onle.onle_drivetype;
		if (t < 0 || t >= NTYPES || ra_off[t] == 0) {
			printf("uda%d ra%d: disk type %d unknown\n",
				io->i_unit >> 3, i, t);
			_stop("ra: bad type");
		}
		udadriveinit[i] = 1;
	}
	if ((t = io->i_boff) < 0 || t > 7)
		_stop("ra: bad unit");
	off = ra_off[ra_type[i]][t];
	if (off == -1)
		_stop("ra: bad partition");
	io->i_boff = off;
}

udcmd(op)
	int op;
{
	int i;

	uda.uda_cmd.mscp_opcode = op;
	uda.uda_rsp.mscp_msglen = MSCP_MSGLEN;
	uda.uda_cmd.mscp_msglen = MSCP_MSGLEN;
	uda.uda_ca.ca_rspdsc[0] |= MSCP_OWN | MSCP_INT;
	uda.uda_ca.ca_cmddsc[0] |= MSCP_OWN | MSCP_INT;
	i = udaddr->udaip;
	for (;;) {
		if (uda.uda_ca.ca_cmdint)
			uda.uda_ca.ca_cmdint = 0;
		/* should ignore trash a la uda dump code */
		if (uda.uda_ca.ca_rspint)
			break;
	}
	uda.uda_ca.ca_rspint = 0;
	if (uda.uda_rsp.mscp_opcode != (op | M_OP_END) ||
	    (uda.uda_rsp.mscp_status & M_ST_MASK) != M_ST_SUCCESS)
		return (1);
	return (0);
}

rastrategy(io, func)
	register struct iob *io;
{
	register struct mscp *mp;
	int ubinfo;

	ubinfo = ubasetup(io, 1);
	mp = &uda.uda_cmd;
	mp->mscp_unit = io->i_unit&7;
	mp->mscp_seq.seq_lbn = io->i_bn;
	mp->mscp_seq.seq_bytecount = io->i_cc;
	mp->mscp_seq.seq_buffer = (ubinfo & 0x3ffff) |
		((ubinfo >> 4) & (0xf << 24));
	if (udcmd(func == READ ? M_OP_READ : M_OP_WRITE)) {
		printf("ra: I/O error\n");
		ubafree(io, ubinfo);
		return(-1);
	}
	ubafree(io, ubinfo);
	return(io->i_cc);
}

/*ARGSUSED*/
raioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}
