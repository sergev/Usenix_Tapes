/*	rmaker.c	1.0	04/11/84	*/

/*
 * Resource compiler.
 */

/* 
 * Copyright (C) 1984, Stanford Univ. SUMEX project.  
 * May be used but not sold without permission.
 */

/*
 * history
 * 04/11/84	Croft	Created.
 * 04/16/84	Croft	Reloc info now imbedded in long addresses.
 * 06/18/84	Schilit Added DITL and ALRT.
 * 07/06/84	Croft	Added DLOG WIND MENU CNTL ICON CURS PAT.
 * 07/21/84	Croft	Added DRVR; multiple code segs (jks).
 * 07/26/84	Schuster Added resource names.
 * 09/18/84	Croft	Fixed fwrite bug for resource names.
 * 11/06/84	Croft	Added INIT and PACK.
 * 11/11/84   	Maio/Schilit no NULL in DRVR name if device driver.
 */

#include <stdio.h>
#include <res.h>
#include <mac/b.out.h>
#include <mac/quickdraw.h>
#include <toolintf.h>

#define	bh	filhdr

struct	bhdr	bh;		/* b.out header */
char 	*malloc();
char	*index();
char	*rindex();
unsigned short	htons();	/* host to "net" byte order, short. */
unsigned long	htonl();

char	seg0[sizeof(struct jumphead) + sizeof(struct jumptab)] = {
	0,0,0,0x28,  0,0,2,0,  0,0,0,8,  0,0,0,0x20,
	0,0,  0x3F,0x3C,  0,1,  0xA9,0xF0
};				/* "standard" segment 0 jump header/table */
char	seg1[sizeof(struct codehead)] = { 0,0,  0,1 };

#define	CRTMAGIC 0x602C		/* jump at beginning of crtmac.s */
#define	CRTLEN	10		/* length of relocation table in crtmac.s */

#define	NRESCOMP 50		/* max number of resources per compile */

struct	rescomp {		/* resource being compiled */
	char	rc_type[8];	/* resource type (e.g. "CODE") */
 	char	*rc_name;	/* resource name */
	int	rc_id;		/* resource id number */
	int	rc_att;		/* attributes */
	int	rc_length;	/* length in resource file */
	char	*rc_data;	/* pointer to data */
	int	rc_datalen;	/* length of data */
	FILE	*rc_file;	/* file to read data from */
	int	rc_filelen;	/* length of data in file */
	int	rc_bss;		/* number of zero pad bytes */
} rescomp[NRESCOMP], *rcp;

struct	resfile	rf;		/* compiled resource file header */
struct	resmap	rm;		/* compiled resource map header */
struct	restype	rt[NRESCOMP];	/* compiled resource type list */
struct	resid	ri[NRESCOMP];	/* compiled resource id list */

#define NAMELEN 1024
char 	rn[NAMELEN];		/* compiled resource name list */

char	debugtype[8];		/* debug type switch */
FILE	*fout;			/* file for compiler output */
FILE	*fin;			/* file for compiler commands */
char	fineof;			/* true after all commands read */
char	line[256];		/* line buffer */
char	*lp;			/* current position in line */
int	linenum;		/* line number in command file */

char	*relpnt;		/* current position in longrun area */
int	rellen;			/* length of longrun area */
int	reloff;			/* current relocation offset */

char	token[128];		/* current token being parsed */
char	data[8*1024];		/* area to build simple resource data */
char	*datap;			/* pointer to data area */

/* type format handlers */
extern	handstr(), handhexa(), handcode(), handdrvr();
extern	handdlog(), handalrt(), handditl();
extern	handwind(), handmenu(), handcntl(), handinit();

struct	typehand {		/* type string to handler table */
	char	th_type[8];	/* e.g. "CODE" */
	int	(*th_handler)(); /* format handler function */
} typehand[] = {
	"STR ",	handstr,
	"HEXA",	handhexa,
	"CODE",	handcode,
	"DRVR",	handdrvr,
	"INIT",	handinit,
	"PACK",	handinit,
	"ALRT", handalrt,
	"DITL", handditl, 
	"DLOG", handdlog,
	"WIND", handwind,
	"MENU", handmenu,
	"CNTL", handcntl,
	"ICON", handhexa,
	"CURS", handhexa,
	"PAT ", handhexa,
	0,	0
};


main(argc, argv)
	char **argv;
{
	for (argc--,argv++ ; argc > 0 ; argc--,argv++) {
		if (argv[0][0] != '-')
			break;
		switch (argv[0][1]) {
		case 'd':
			argc--,argv++;
			if (argc < 1) abort("syntax: -d TYPE");
			strcpy(debugtype,argv[0]);
			break;
		}
	}
	if (argc != 1)
		abort("usage: rmaker commandfilename");
	if ((fin = fopen(argv[0], "r")) == NULL)
		abort("can't open commandfile");
	rmaker();
	buildrf();	/* build resource file from rescomp */
	exit(0);
}


rmaker()
{
	register i;
 	char infile[32], *ip;
	struct typehand *thp;
	int haveoutfile = 0;
	int items, id, att;
	char littype[32], type[32], format[32];

	rcp = &rescomp[0];
	while (fineof == 0) {
		if (getline() == 0)
			continue;	/* skip blank lines */
		if (haveoutfile == 0) {	/* if output file not yet open */
			if ((fout = fopen(lp, "w")) == NULL)
				abort("can't open output file %s", lp);
			haveoutfile++;
			continue;
		}
		littype[0] = type[0] = 0;
		items = sscanf(lp, "%s %s = %s", littype, type, format);
		if (items < 2 || strcmp(littype, "Type") != 0)
			abort("bad Type line");
		checktype(type);
		strcpy(rcp->rc_type, type);
		if (items == 3) {
			checktype(format);
			strcpy(type, format);
		}
		if (getline() == 0)
			abort("bad id");
		if (skipsp() == 0)
			abort("bad id");
		for (i=0 ; *lp != ',' && *lp != 0 ; lp++,i++)
			infile[i] = *lp;
		infile[i] = 0;
		if (*lp != ',')
			abort("bad id");
		lp++;
		id = att = 0;
		items = sscanf(lp, " %d(%d) ", &id, &att);
 		ip = index(infile, '|');
 		if (ip) {
 			*ip++ = 0;
 			if ((rcp->rc_name = malloc(strlen(ip) + 1)) == 0)
 				abort("name malloc");
 			strcpy(rcp->rc_name, ip);
 		} else
 			rcp->rc_name = 0;
		if (items < 1)
			abort("bad id");
		if (strlen(infile)) {
			if ((rcp->rc_file = fopen(infile, "r")) == NULL)
				abort("can't open input file %s", infile);
		} else {
			rcp->rc_file = 0;
		}
		rcp->rc_id = id;
		rcp->rc_att = att;
		/* search for type handler */
		for (thp = &typehand[0] ; ; thp++) {
			if (thp->th_handler == 0)
				abort("type %s not implemented", type);
			if (strcmp(thp->th_type, type) == 0)
				break;
		}
		datap = data;
		(*thp->th_handler)();
		if (datap != data) {
			int len = datap - data;
			if (len & 1) {
				len++;
				*datap++ = 0;
			}
			if ((rcp->rc_data = malloc(len)) == 0)
				abort("data malloc");
			bcopy(data, rcp->rc_data, len);
			rcp->rc_datalen = rcp->rc_length = len;
		}
		if (strcmp(type, debugtype) == 0)
			printrcp();
		rcp++;
		if ((rcp - &rescomp[0]) > NRESCOMP - 3)
			abort("too many resources");
	}
	if (rcp == &rescomp[0] || fout == 0)
		abort("nothing to do");
}


/*
 * Build resource file output from incore rescomp structure.
 */
buildrf()
{
	register struct rescomp *rcpp;
	register struct resid *rip;
	register struct restype *rtp;
 	register char *rnp;
	struct restype *rtpp;
	int offdata, roundtomap, sizetypes, sizemap;
	register i;
	register char *cp;
	numtypes_t numtypes;
	lendata_t lendata;

	/* XXX TODO: before scanning, sort rescomp by type/id */
	rtp = &rt[0];
	rip = &ri[0];
 	rnp = rn;
	rcpp = &rescomp[0];
	offdata = 0;
	/*
	 * Scan through the resources making type and id lists.  In this
	 * 1st pass, the rt_offids field is set to the offset from the
	 * start of the id's list.  Below, the 2nd pass adds the size
	 * of the type list to this field.
	 */
	bcopy(rcpp->rc_type, rtp->rt_type, 4);	/* preset 1st type */
	for ( ; rcpp < rcp ; rcpp++) {
		if (strncmp(rcpp->rc_type, rtp->rt_type, 4) != 0) {
			rtp++;		/* we've found a new type */
			bcopy(rcpp->rc_type, rtp->rt_type, 4);
			rtp->rt_offids = (rip - &ri[0]) * sizeof *rip;
		}
		rtp->rt_numids++;
		rip->ri_id = htons(rcpp->rc_id); /* ensure final byte order */
 		if (rcpp->rc_name) {
 			rip->ri_offname = htons(rnp - rn);
 			i = strlen(rcpp->rc_name);
 			if (((rnp - rn) + i + 2) > NAMELEN)
 				abort("namemap exhausted");
			/*
			 * desk accessories start with null, device
			 * drivers start with '.'.
			 */
 			if (strncmp(rcpp->rc_type, "DRVR", 4) == 0
			    && rcpp->rc_name[0] != '.') {
 				*rnp++ = (char)(i + 1);
 				*rnp++ = 0;
			} else {
				*rnp++ = (char) i;
			}
 			bcopy(rcpp->rc_name, rnp, i);
 			rnp += i;
 		}
 		else
 			rip->ri_offname = htons(-1);
		rip->ri_att = rcpp->rc_att;
		rip->ri_offdata = htons(offdata & 0xFFFF);
		rip->ri_offdatahi = ((offdata >> 16) & 0xFF);
		rip++;
		offdata += (rcpp->rc_length + sizeof lendata);
	}
	rtp++;
	/*
	 * Write the file header and pad it out.
	 */
	rf.rf_offdata = htonl(OFFDATA);
	offdata += OFFDATA;
	roundtomap = (offdata & (ROUNDMAP-1));
	if (roundtomap)
		roundtomap = ROUNDMAP - roundtomap;  /* # of pad bytes */
	rf.rf_offmap = offdata + roundtomap;
	rf.rf_lendata = htonl(rf.rf_offmap - OFFDATA);
	rf.rf_offmap = htonl(rf.rf_offmap);
	sizetypes = ((numtypes = rtp - &rt[0]) * sizeof *rtp);
 	if ((rnp - rn) & 1)		/* to be conservative */
 		*rnp++ = 0;
	sizemap = sizeof rm + sizetypes + sizeof numtypes
 	    + ((rip - &ri[0]) * sizeof *rip) + (rnp - rn);
	rf.rf_lenmap = htonl(sizemap);
	fwrite(&rf, sizeof rf, 1, fout);
	i = OFFDATA - sizeof rf;
	do { putc(0, fout); } while (--i);
	/*
	 * correct type list.
	 */
	for (rtpp = &rt[0] ; rtpp < rtp ; rtpp++) {
		rtpp->rt_offids = htons(rtpp->rt_offids 
		    + sizetypes + sizeof numtypes);
		rtpp->rt_numids = htons(rtpp->rt_numids - 1);
	}
	/*
	 * For each resource, write data, file, and bss.
	 */
	for (rcpp = &rescomp[0] ; rcpp < rcp ; rcpp++) {
		lendata = htonl(rcpp->rc_length);
		fwrite(&lendata, sizeof lendata, 1, fout);
		if ((cp = rcpp->rc_data))
			for (i = rcpp->rc_datalen ; i > 0 ; i--)
				putc(*cp++, fout);
		if (rcpp->rc_file)
			for (i = rcpp->rc_filelen ; i > 0 ; i--)
				putc(getc(rcpp->rc_file), fout);
		for (i = rcpp->rc_bss ; i > 0 ; i--)
			putc(0, fout);
	}
	for (i = roundtomap ; i > 0 ; i--)
		putc(0, fout);
	/*
	 * Write the resource map.
	 */
	rm.rm_offtype = htons(sizeof rm);
 	rm.rm_offname = htons(sizemap - (rnp - rn));
	fwrite(&rm, sizeof rm, 1, fout);
	numtypes--;
	numtypes = htons(numtypes);
	fwrite(&numtypes,sizeof numtypes, 1, fout);
	fwrite(&rt[0], sizeof *rtp, rtp - &rt[0], fout);
	fwrite(&ri[0], sizeof *rip, rip - &ri[0], fout);
	if (rnp - rn)
		fwrite(rn, rnp - rn, 1, fout);
}


/*
 * Get next command line.
 * Returns 0 if end of block, 1 if normal line.
 */
getline()
{
	register i;

again:
	if ((fgets(line, sizeof line, fin)) == NULL) {
		fineof++;
		return (0);
	}
	linenum++;
	if ((i = strlen(line)) <= 0)
		return (0);
	if (line[i-1] == '\n')
		line[i-1] = 0;
	lp = line;
	if (*lp == 0)
		return (0);
	if (*lp == '*')
		goto again;
	return (1);
}


/*
 * Abort with message.
 */
abort(s,a,b)
	char *s;
{
	fprintf(stderr, "rmaker: ");
	fprintf(stderr, s, a, b);
	if (linenum)
		fprintf(stderr, "; line number %d", linenum);
	fprintf(stderr, "\n");
	exit(1);
}


/*
 * Check for legal length type.  Fix "STR ".
 */
checktype(s)
	char *s;
{
	register len;

	len = strlen(s);
	if (len < 3 || len > 4)
		abort("bad type");
	if (len == 3) {
		s[3] = ' ';
		s[4] = 0;
	}
}


/*
 * Copy bytes.
 */
bcopy(a, b, n)
	register n;
	register char *a, *b;
{
	if (n <= 0)
		return;
	do { *b++ = *a++; } while (--n);
}


/*
 * Store a short into the data area.
 */
datashort(i)
{
	*(unsigned short *)datap = htons(i);
	datap += sizeof (short);
}


/*
 * Store a long into the data area.
 */
datalong(i)
{
	*(unsigned long *)datap = htonl(i);
	datap += sizeof (long);
}


/*
 * Store string into data area.  If "round" is set, round up length.
 * Returns length of data.
 */
datastring(sp,round)
	char *sp;
{
	char *cp = datap;	/* remember old value to store length */
	int len;

	for (datap++, len = 0 ; *sp ; len++)
		*datap++ = *sp++;
	*cp = len;
	len++;
	if (round && (len & 1)) {
		len++;
		*datap++ = 0;
	}
	return (len);
}


/*
 * Scan next number from line and return it.
 */
scanfn(fmt)
	char *fmt;
{
	int val;

	if (skipsp() == 0 || sscanf(lp,fmt,&val) != 1)
		abort("no number found");
	do {
		lp++;
	} while (*lp != ' ' && *lp != '/0'); /* skip to next number */
	return (val);
}


/*
 * Scan next string from "line" into "token";  return 0 if none left.
 */
scanft()
{
	char *cp;

	if (skipsp() == 0)
		return (0);
	cp = token;
	while (*lp != ' ' && *lp != 0)
		*cp++ = *lp++;
	*cp = 0;
	return (1);
}


/*
 * Skip spaces.  Return 0 if end of line.
 */
skipsp() 
{
	while (*lp == ' ' && *lp != 0) 
  		lp++;
	return(*lp);
}


/*
 * Print the data portion of the current rcp record.
 */
printrcp()
{
 	char *cp;
	int i;
	unsigned int j;

	printf("Type %s, ID %d, length %d, datalen %d\n",
		rcp->rc_type, rcp->rc_id, rcp->rc_length, rcp->rc_datalen);
	cp = rcp->rc_data;		/* pick up the data pointer */
	for (i=0 ; i < rcp->rc_datalen ; i++) {
		j = *cp++ & 0xFF;
		if ((i % 16) == 15) 
			printf("%02X\n",j);
		else 
			printf("%02X ",j);
	}
	printf("\n");
}


#define VAX
#define nohtonl
#ifdef nohtonl	/* if not in library */
#ifdef	VAX
/*
 * "Host" to "net" byte order swappers.
 */
unsigned short htons(a)
	unsigned short a;
{
	unsigned short result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}

unsigned long htonl(a)
	unsigned long a;
{
	unsigned long result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[3] = *sp++;
	dp[2] = *sp++;
	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}

#else	/* if running on a native 68K, don't need byte swapping */

unsigned short htons(a)
	unsigned short a;
{
	return (a);
}

unsigned long htonl(a)
	unsigned long a;
{
	return (a);
}

#endif	VAX

#endif	nohtonl



/*
 *	T Y P E   H A N D L E R S
 */


/*
 * Handle string format data.
 */
handstr()
{
	if (getline() == 0)
		abort("missing string");
	datastring(lp,1);
}
	

/*
 * Handle hexadecimal format data.
 */
handhexa()
{
	char hex[4];
	int val, items, len;

	hex[2] = 0;
	while (getline() != 0) {
		for (len = strlen(lp) ; len > 0 ; ) {
			if (*lp == ' ') {
				lp++;  len--;
				continue;
			}
			strncpy(hex, lp, 2);
			items = sscanf(hex, "%x", &val);
			if (items != 1)
				abort("bad digits");
			*datap++ = val;
			lp += 2;  len -= 2;
			if ((datap - data) >= sizeof data)
				abort("too much data");
		}
	}
	len = datap - data;
	if (len & 1) {
		len++;
		*datap++ = 0;
	}
}


/*
 * Handle program (code) data.
 */
handcode()
{
	register i;
	struct reloc rl;

	/*
	 * setup CODE, id=0 (jumptable)
	 */
	if (rcp->rc_id == 0) {
		rcp[1] = rcp[0];	/* duplicate rescomp entry */
		rcp->rc_att = ATT_PURGEABLE;
		rcp->rc_datalen = rcp->rc_length = sizeof seg0;
		rcp->rc_data = seg0;
		rcp->rc_file = (FILE *)0;
		rcp->rc_bss = 0;
		rcp++;
	}
	/*
	 * setup CODE, id=1 (text/data)
	 */
	if (fread(&bh, sizeof bh, 1, rcp->rc_file) != 1
	    || bh.fmagic != FMAGIC)
		abort("bad b.out header");
	if ((rcp->rc_data = malloc(rcp->rc_datalen = bh.tsize
	    + bh.dsize + sizeof seg1)) == 0)
		abort("code malloc");
	rcp->rc_id++;		/* normally id is now 1 */
	seg1[3] = rcp->rc_id;	/* put id in jump table */
	bcopy(seg1, rcp->rc_data, sizeof seg1);
	rcp->rc_data += sizeof seg1;
	if (fread(rcp->rc_data, rcp->rc_datalen - sizeof seg1,
	    1, rcp->rc_file) != 1)
		abort("code readerror");
	rcp->rc_bss = bh.bsize;
	rcp->rc_length = rcp->rc_datalen + rcp->rc_bss;
	if (!rcp->rc_att)	/* set default attributes if none supplied */
		rcp->rc_att = ATT_PURGEABLE | ATT_LOCKED | ATT_PRELOAD;
	if ((bh.rtsize + bh.rdsize) <= 0)
		abort("b.out must have reloc info");
	fseek(rcp->rc_file, RTEXTPOS, 0);
	if (*(short *)rcp->rc_data != htons(CRTMAGIC))
		abort("no crtmac.s prefix");
	relpnt = rcp->rc_data + 2;		/* start of longrun table */
	rellen = CRTLEN;			/* length of longrun table */
	reloff = 0;
	readrel(0, bh.rtsize/sizeof rl);	/* reloc text */
	readrel(bh.tsize, bh.rdsize/sizeof rl);	/* reloc data */
	*(rcp->rc_data+reloff) = 0377;		/* signals end of reloc data */
	rcp->rc_data -= sizeof seg1;
	fclose(rcp->rc_file);
	rcp->rc_file = 0;
	fprintf(stderr, " text %d, data %d, bss %d, longruns %d\n",
		bh.tsize, bh.dsize, bh.bsize, CRTLEN - rellen);
}


/*
 * Read relocation data and run length encode it.
 */
readrel(off, nrel)
{
	struct reloc rl;
	register char *cp;
	int run, newoff;

	for ( ; nrel > 0 ; nrel--) {
		if (fread(&rl, sizeof rl, 1, rcp->rc_file) != 1)
			abort("error reading reloc");
		if (rl.rsize != RLONG || (rl.rpos & 1)
		    || rl.rsymbol || rl.rsegment == REXT)
			abort("impossible relocation");
		newoff = (rl.rpos + off);
		run = (newoff - reloff) >> 1;
		if (reloff == 0 || run >= 0377) {
			*(long *)relpnt = htonl(newoff);
			relpnt += sizeof (long);
			if (--rellen <= 0)
				abort("too many longruns");
		} else {
			*(rcp->rc_data+reloff) = run;
		}
		reloff = newoff;
	}
}


/*
 * Handle device driver (or desk accessory.).
 */

#define	DRVROFF	50	/* offset of longruns in DRVR resource */

handdrvr()
{
	relocate(DRVROFF);
}


/*
 * Handle PACK or INIT resources.
 */
handinit()
{
	relocate(0);
}


/*
 * Relocate a resource.
 */
relocate(off)
{
	register i;
	struct reloc rl;

	if (fread(&bh, sizeof bh, 1, rcp->rc_file) != 1
	    || bh.fmagic != FMAGIC)
		abort("bad b.out header");
	if ((rcp->rc_data = malloc(rcp->rc_datalen = bh.tsize
	    + bh.dsize)) == 0)
		abort("text malloc");
	if (fread(rcp->rc_data, rcp->rc_datalen, 1, rcp->rc_file) != 1)
		abort("text readerror");
	rcp->rc_bss = bh.bsize;
	rcp->rc_length = rcp->rc_datalen + rcp->rc_bss;
	if ((bh.rtsize + bh.rdsize) <= 0)
		abort("b.out must have reloc info");
	fseek(rcp->rc_file, RTEXTPOS, 0);
	if (*(short *)(rcp->rc_data+off) != htons(CRTMAGIC))
		abort("no crtxxx.s prefix");
	relpnt = rcp->rc_data + off + 2;	/* start of longrun table */
	rellen = CRTLEN;			/* length of longrun table */
	reloff = 0;
	readrel(0, bh.rtsize/sizeof rl);	/* reloc text */
	readrel(bh.tsize, bh.rdsize/sizeof rl);	/* reloc data */
	*(rcp->rc_data+reloff) = 0377;		/* signals end of reloc data */
	fclose(rcp->rc_file);
	rcp->rc_file = 0;
	fprintf(stderr, " %s text %d, data %d, bss %d, longruns %d\n",
		rcp->rc_type, bh.tsize, bh.dsize, bh.bsize, CRTLEN - rellen);
}


/*
 * Handle dialog template (DLOG).
 */

/*
 * This structure is defined in toolintf.h, but to avoid byte swap
 * and alignment problems, we fill it "by hand".
 *
 *	typedef	struct	{
 *		Rect	boundsRect;
 *		short	procID;
 *		char	visible;
 *		char	filler1;
 *		char	goAwayFlag;
 *		char	filler2;
 *		long	refCon;
 *		short	itemsID;
 *		Str255	title;
 *	} DialogTemplate;
 */

handdlog()
{
	int vis,go,pid,ref;
	register i;

	if (getline() == 0) 
		abort("no dlog rectangle");
	for (i=0 ; i<4 ; i++) 		/* parse 4 ints - rectangle */
		datashort(scanfn("%d"));
	if (getline() == 0) 
		abort("no dlog vis/proc");
	scanft();
	vis = (token[0] == 'V' ? 1 : 0);
	pid = scanfn("%d");
	scanft();
	go = (token[0] == 'G' ? 1 : 0);
	ref = scanfn("%d");
	datashort(pid);
	*datap++ = vis;  *datap++ = 0;
	*datap++ = go;   *datap++ = 0;
	datalong(ref);
	if (getline() == 0)
		abort("missing Item list ID");
	datashort(scanfn("%d"));
	if (getline() != 0)
		datastring(lp,1);
	else
		datashort(0);
}


/*
 * Handle alert template.
 */

/*
 * This structure is defined in toolintf.h, but to avoid byte swap
 * and alignment problems, we fill it "by hand".
 *
 *	typedef	struct {
 *		Rect	boundsRect;
 *		short	itemsID;
 *		short	stages;
 *	} AlertTemplate;
 */

handalrt() 
{
	int i;

	getline();
	for (i=0 ; i<4 ; i++) 		/* parse 4 ints - rectangle */
		datashort(scanfn("%d"));
	getline();
	datashort(scanfn("%d"));
	getline();
	datashort(scanfn("%x"));
}


/* 
 * Handle Dialog and Alert Item Lists (Type DITL)
 *
 */

/*
 * Structure of an item list.
 *
 *	struct {
 *		long	zero;		placeholder for handle 
 *		Rect	itemrect;
 *		char	itemtype;
 *		char	itemlength;
 *		...  			followed by variable length data
 *	}
 */

struct ditlkeys {
	char *ditl_name;
	int ditl_value;
} ditlkeys[] = {
	"CtrlItem",ctrlItem,		/*  used in conjunction with: */
	"BtnCtrl",btnCtrl,		/*   a button control */
	"ChkCtrl",chkCtrl,		/*   checkbox  */
	"RadCtrl",radCtrl,		/*   etc... */
	"ResCtrl",resCtrl,
 	"RadioItem",radCtrl+ctrlItem,
 	"ChkItem",chkCtrl+ctrlItem,
	"StatText",statText,
	"EditText",editText,
	"IconItem",iconItem,
	"PicItem",picItem,
	"UserItem",userItem,
	"ItemDisable",itemDisable,
	"BtnItem",btnCtrl+ctrlItem,	/* abbreviation */
	"Enabled",0,
	"Disabled",itemDisable,		/* synonym */
	"Disable",itemDisable,		/* synonym */
	"ItemDisabled",itemDisable,	/* synonym */
	0,0
};


handditl()
{
	char *lenp;
	int i,len;
	int val,types;
	register struct ditlkeys *dk;

	/* first line is item count, drop in count-1 */
	if (getline() == 0 || (val = scanfn("%d")) < 1)
		abort("bad DITL item count");
	datashort(val-1);

	/* for each item */
	for ( ; val > 0 ; val--) {
		datalong(0);
 		if (getline() == 0)		/* line with item types */
			abort("Missing DITL item type");
		types = 0;
		while (scanft()) {
			for (dk = &ditlkeys[0] ; dk->ditl_name ; dk++)
				if (strcmp(dk->ditl_name,token) == 0)
					goto found;
			abort("bad DITL item type %s",token);
		found:
			types += dk->ditl_value;
		}
 		if (getline() == 0)
			abort("Missing DITL rectangle");		
		for ( i=0 ; i < 4 ; i++)
			datashort(scanfn("%d"));
		*datap++ = types;
 		if ((getline() == 0) && (types&(~itemDisable) != editText))
			abort("Missing DITL data");
		skipsp();

		lenp = datap++;		/* remember spot for length */
		types &= ~itemDisable;		/* don't care about this bit */

		switch (types) {	

		case iconItem:		/* 2 byte resource ID */
		case picItem:
		case ctrlItem+resCtrl: 
			datashort(scanfn("%d"));
			*lenp = sizeof (short);
			break;	
		
		case userItem:
			*lenp = 0;		/* is nothing */
			break;

		case ctrlItem+btnCtrl:
		case ctrlItem+chkCtrl:
		case ctrlItem+radCtrl:
		case statText:	
		case editText:
			len = strlen(lp);
			strcpy(datap,lp);
			datap += len;
			if (len & 1) {
				len++;
				*datap++ = 0;
			}
			*lenp = len;
			break;
		} 
		if (getline() != 0)
			abort("Expected blank line in DITL");
	}
}


/*
 * Handle window template (WIND).
 */

/*
 *	typedef	struct	{
 *		Rect	boundsRect;
 *		short	procID;
 *		char	visible;
 *		char	filler1;
 *		char	goAwayFlag;
 *		char	filler2;
 *		long	refCon;
 *		Str255	title;
 *	} WindowTemplate;
 */

handwind()
{
	int vis,go,pid,ref;
	char title[128];
	register i;

	getline();
	skipsp();
	strcpy(title,lp);
	getline();
	for (i=0 ; i<4 ; i++) 		/* parse 4 ints - rectangle */
		datashort(scanfn("%d"));
	getline();
	scanft();
	vis = (token[0] == 'V' ? 1 : 0);
	scanft();
	go = (token[0] == 'G' ? 1 : 0);
	getline();
	pid = scanfn("%d");
	getline();
	ref = scanfn("%d");
	datashort(pid);
	*datap++ = vis;  *datap++ = 0;
	*datap++ = go;   *datap++ = 0;
	datalong(ref);
	datastring(title,1);
}


/*
 * Handle control template (CNTL).
 */

/*
 *	typedef	struct	{
 *		Rect	boundsRect;
 *		short	value;
 *		char	visible;
 *		char	filler1;
 *		short	max;
 *		short	min;
 *		short	procID;
 *		long	refCon;
 *		Str255	title;
 *	} ControlTemplate;
 */

handcntl()
{
	int vis,min,max,pid,ref;
	char title[128];
	register i;

	getline();
	skipsp();
	strcpy(title,lp);
	getline();
	for (i=0 ; i<4 ; i++) 		/* parse 4 ints - rectangle */
		datashort(scanfn("%d"));
	getline();
	scanft();
	vis = (token[0] == 'V' ? 1 : 0);
	getline();
	pid = scanfn("%d");
	getline();
	ref = scanfn("%d");
	getline();
	datashort(scanfn("%d"));
	*datap++ = vis;  *datap++ = 0;
	min = scanfn("%d");
	max = scanfn("%d");
	datashort(max);
	datashort(min);
	datashort(pid);
	datalong(ref);
	datastring(title,1);
}


/*
 * Handle menu template (MENU).
 */

/*
 *	typedef	struct	{
 *		short	menuID;
 *		long	fill1,fill2;	placeholder
 *		long	enableFlags;
 *		Str255	title;
 *	  for each menu item:
 *		Str255	text;
 *		char	icon#;
 *		char	keyboardequiv;
 *		char	mark;
 *		char	textstyle;
 *	  finally:
 *		char	zero;		end of items.
 *	}
 */

handmenu()
{
	int iconid, styleid, cmdid, markid;
	int *flagsp, flags, item;
	register char *cp,*dp,*sp;
	char itext[128];
	static char styles[] = "BIUOS";

	datashort(rcp->rc_id);
	datalong(0);
	datalong(0);
	flagsp = (long *)datap;	/* remember where the flags were */
	flags = -1;		/* enable all items */
	datalong(-1);	/* placeholder */
	getline();
	scanft();
	datastring(token,0);
	for (item = 1 ; getline() && item < 32 ; item++) {
		skipsp();
		iconid = styleid = cmdid = markid = 0;
		for (cp = lp, dp = itext ; *cp ; cp++) {
			switch (*cp) {
			default:
				*dp++ = *cp;
				break;

			case '(':
				flags &= ~(1<<item);
				break;

			case '^':
				cp++;
				iconid = *cp;
				break;
			
			case '/':
				cp++;
				cmdid = *cp;
				break;

			case '!':
				cp++;
				markid = *cp;
				break;

			case '<':
				cp++;
				sp = index(styles, *cp);
				if (sp == 0)
					abort("BIUOS expected after <");
				styleid |= (1 << (sp-styles));
				break;
			}
		}
		*dp++ = 0;
		datastring(itext,0);
		*datap++ = iconid;
		*datap++ = cmdid;
		*datap++ = markid;
		*datap++ = styleid;
	}
	/* end of items */
	*datap++ = 0;
	*flagsp = htonl(flags);
}
