#define	MSGUSE	"Usage: ansiw [-cfhlmignpv] [ file ... ]"
#include	"ansi.h"
#include	<sys/types.h>
#include	<pwd.h>
#include	<time.h>
#include	<ident.h>	/* declares char myname[] */

/*
 * Name: ansiw, write ANSI standard labelled tape
 * Author: Dick Grune
 * Version: 820314
 */

#define	MAXBLK	2048
#define	MINBLK	18
#define	FILLER	'^'

extern time_t time();
extern struct tm *localtime();
extern struct passwd *getpwuid();

char iflag = FALSE, gflag = FALSE, pflag = FALSE, vflag = FALSE;
char block[MAXBLK];

struct DD	{
	long size;
	long lrecl;
	char ascii95;
	char example;
} dd;
struct DD empty_dd = {0L, 0L, TRUE, EOS};

#define	divis(m, n)	((n)!=0&&(m)%(n)==0)

main(argc, argv) int argc; char *argv[];	{

	argc--; argv++;
	if (argc > 0)	{
		if (argv[0][0] == '-')	{
			char *pp = argv[0];

			while (*++pp)
			switch (*pp)	{
			case 'c':
				unit = TP_CDEV;
				if (argc < 2)
					goto Lbad;
				nmdns = argv[1];
				argc--; argv++;
				break;
			case 'f':
				unit = TP_IMAG;
				if (argc < 2)
					goto Lbad;
				nmdns = argv[1];
				argc--; argv++;
				break;
			case 'g':
				gflag = TRUE;
				break;
			case 'h':
				nmdns = TP_DENH;
				break;
			case 'i':
				iflag = TRUE;
				break;
			case 'l':
				nmdns = TP_DENL;
				break;
			case 'm':
				unit = *++pp - '0';
				break;
			case 'n':
				unit = TP_IMAG;
				nmdns = "/dev/null";
				break;
			case 'p':
				pflag = TRUE;
				break;
			case 'v':
				vflag = TRUE;
				break;
			default:
				goto Lbad;
			}
			argc--; argv++;
		}
	}
	tf = tpopen(unit, nmdns, "w");

	check_args(argc, argv);

	lblVOL1(iflag || vflag ? ASK_SUG : ASK_NO);

	while (argc-- != 0)	{
		strcpy(filename, argv[filseqnum]);
		filseqnum++;
		blkcount = 0;

		opendd(filename);
		if (file == NULL)	{
			printf(">>> File %s ", filename);
			errors("has suddenly disappeared!!!");
		}
		lblHDR1(iflag ? ASK_SUG : ASK_NO);
		lblHDR2(iflag ? ASK_SUG : ASK_NO);
		wrt_tm();
		copy();
		wrt_tm();
		lblEOF1();
		lblEOF2();
		wrt_tm();
	}

	wrt_tm();
	tpclose(tf);
	exit(0);

Lbad:
	errors(MSGUSE);
}

check_args(c, v) char *v[];	{
	char ok = TRUE;

	while (c-- > 0)	{
		int f = open(*v, 0);

		if (f < 0)	{
			printf("Cannot open %s\n", *v);
			ok = FALSE;
		}
		else	VOID(close(f));
		v++;
	}
	if (!ok)
		errors("Stop");
}

wrt_tm() /*writes a tapemark*/	{
	tpwrite(tf, "", 0);
}

long
tab(p) long p;	{	/* the position in which a tab from p would land */
	return (p/8+1)*8;
}

/*
 * `opendd' opens the file `fn' and determines its `dd' parameters
 */

opendd(fn) char *fn;	{
	int ch;
	long lr;

	dd = empty_dd;
	if ((file = fopen(fn, "r")) == NULL)
		return;

	lr = 0L;
	while ((ch = getc(file)) != EOF)	{
		dd.size++;
		if (ch == NL)	{
			if (lr > dd.lrecl)
				dd.lrecl = lr;
			lr = 0L;
		}
		else
		if (ch == TAB)	{
			lr = tab(lr);
		}
		else	{
			lr++;
			if (!Ascii95(ch))	{
				dd.ascii95 = FALSE;
				dd.example = ch;
			}
		}
	}
	if (lr > dd.lrecl)
		dd.lrecl = lr;

	VOID(fclose(file));
	file = fopen(fn, "r");
}

/*
 * the writing of labels
 */

lblVOL1(md)	{
	struct passwd *pwd = getpwuid(getuid());

	str2fld("", Whole(VOL1buf));
	str2fld("VOL1", Labidf(VOL1buf));

	enq_fld("\
	The `volume serial number' is the six-character identification\n\
	number of the tape itself, as it should appear on the sticker on\n\
	the reel. When in doubt, use the default %s.\n",
		md, "222222", Volidf(VOL1buf));
	enq_fld("\
	The `volume accessibility symbol' is a single character,\n\
	recorded on the tape, which indicates how publicly accessible the\n\
	whole tape is. It is not well defined, but a single space is\n\
	generally taken to mean: accessible by anybody.\n",
		md, " ", Volacc(VOL1buf));
	enq_fld("\
	The `owner identification' is the name of the owner, as recorded\n\
	on the tape. On some systems it interacts with the file\n\
	accessibility symbol. When in doubt, specify a short string of\n\
	letters.\n",
		pwd == NULL ? ASK_YES : md, pwd->pw_name, Ownidf(VOL1buf));
	str2fld("1", Labvers(VOL1buf));

	tpwrite(tf, Whole(VOL1buf));
}

lblHDR1(md)	{
	time_t tnow = time((time_t*)0);
	struct tm *timeptr = localtime(&tnow);

	str2fld("", Whole(HDR1buf));
	str2fld("HDR1", Labidf(HDR1buf));
	enq_fld("\
	The `file identifier' is the name of the file, as recorded on\n\
	the tape.  When in doubt, specify a six-letter mnemonic name.\n",
		md, filename, Fileidf(HDR1buf));

	fld2fld(Volidf(VOL1buf), Filesetidf(HDR1buf));
	int2fld(filsecnum, Filsecnum(HDR1buf));
	int2fld(filseqnum, Filseqnum(HDR1buf));
	enq_num("\
	The `generation number' is a counter that some operating systems\n\
	attach to a file and that is increased each time the file is\n\
	updated. Use 1.\n",
		gflag ? md : ASK_NO, 1, Gennum(HDR1buf));
	enq_num("\
	The `generation version number' tells how often an attempt to\n\
	write the file to tape has failed. Use 0.\n",
		gflag ? md : ASK_NO, 0, Genversnum(HDR1buf));
	dat2fld(timeptr->tm_year, timeptr->tm_yday+1, Creatdate(HDR1buf));
	enq_dat("\
	The `expiration date' is the date, recorded on the tape, after\n\
	which the file may be overwritten. The format is 2 digits for\n\
	the year, followed by 3 digits for the day in the year, e.g.,\n\
	82365 for the last day of 1982.  When in doubt, use today's date,\n\
	%s, to make the receiver's life easier.\n",
		md, timeptr->tm_year, timeptr->tm_yday+1, Expirdate(HDR1buf));
	enq_fld("\
	The `file accessibility symbol' is a single character, recorded\n\
	on the tape, which indicates how publicly accessible the file is.\n\
	It is not well defined, but a single space is generally taken to\n\
	mean: accessible by anybody.\n",
		md, " ", Fileacc(HDR1buf));
	int2fld(blkcount, Blkcount(HDR1buf));

	str2fld(myname /* from ident.h */, Syscode(HDR1buf));

	tpwrite(tf, Whole(HDR1buf));
}

lblHDR2(md)	{

	str2fld("", Whole(HDR2buf));
	str2fld("HDR2", Labidf(HDR2buf));

	if (!iflag)	{
		str2fld("F", Whole(rectype));
		blklength = 1920;
		reclength = 80;
	}

	if (!dd.ascii95)	{
		printf("`%s' contains non-ASCII95 characters, e.g., %s\n",
			filename, char2str(dd.example));
		printf("Perhaps the file code should have been BINARY");
		str2fld("U", Whole(rectype));
	}

	if (dd.lrecl > MAXBLK)	{
		printf("`%s' has a record length > %d\n", filename, MAXBLK);
		printf("Only U-format is possible\n");
		str2fld("U", Whole(rectype));
		recformat = format('U');
	}
	else
	inmood (!dd.ascii95 ? ASK_SUG : md)
		char *rct = enq_str("\
	The `record format' tells how lines from the disk file should be\n\
	converted to records to be packed into blocks to be recorded on\n\
	tape. Although many formats exist, only two are any good in\n\
	Information Interchange:\n\
	F (Fixed): spaces are added at the end of the line until its\n\
	    length is `record length', and `block length'/`record length'\n\
	    of these records form a block;\n\
	U (Undefined): `block length' characters are stored in a block.\n\
	Unless the disk file contains non-ASCII characters, use F.\n",
			mood, fld2str(Whole(rectype)));

		iferr (strlen(rct) != 1
			|| (recformat = format(rct[0])) == NULL
			|| recformat->cpblk == NULL)
			printf(
			"Only F- and U-formats are allowed for portability\n"
			);
		enderr;

		str2fld(rct, Whole(rectype));
	endmood;

	(*recformat->checkpar)(md);

	fld2fld(Whole(rectype), Recformat(HDR2buf));
	int2fld(blklength, Blklength(HDR2buf));
	int2fld(reclength, Reclength(HDR2buf));
	int2fld(bufoffset, Bufoffset(HDR2buf));

	tpwrite(tf, Whole(HDR2buf));
}

checkF(md)	{
	int lr;

	inmood (md)
		getBlklength(mood);
		iferr (blklength < reclength)
			printf("Block length < phys. record length (=%D)\n",
				dd.lrecl);
		enderr;
	endmood;

	lr = dd.lrecl;
	while (!divis(blklength, lr))
		lr++;
	if (lr < 80 && divis(blklength, 80))
		lr = 80;
	reclength = lr;

	inmood (md)
		reclength = enq_int("\
	The `record length' is the number of characters into which each\n\
	line (record) is stretched before it is written to tape. It must\n\
	divide the block length. Unless the receiver has been very\n\
	specific, use %s.\n",
			mood, reclength, blklength);
		iferr (reclength == 0 || !divis(blklength, reclength))
			printf(
		"The block length is not a multiple of the record length\n"
			);
		enderr;
		iferr (reclength < dd.lrecl)
			printf("Record length < phys. record length (=%D)\n",
				dd.lrecl);
		enderr;
	endmood;
}

checkU(md)	{
	getBlklength(md);
	reclength = blklength;
}

getBlklength(md)	{
	inmood (md)
		blklength =
			enq_int("\
	The `block length' is the number of characters in each physical\n\
	block written to tape. Unless the receiver has specified\n\
	something else, use %s.\n",
				mood,
				rectype[0] == 'F' && dd.lrecl > 1920 ? MAXBLK :
					!iflag ? 1920 :
					blklength,
				MAXBLK);
		iferr (blklength < MINBLK)
			printf("The minimum block length is %d\n", MINBLK);
		enderr;
	endmood;
}


lblEOF1()	{
	str2fld("EOF1", Labidf(HDR1buf));
	int2fld(blkcount, Blkcount(HDR1buf));
	tpwrite(tf, Whole(HDR1buf));
}

lblEOF2()	{
	str2fld("EOF2", Labidf(HDR2buf));
	tpwrite(tf, Whole(HDR2buf));
}

/*
 * the copying of the file
 */

copy()	{
	int size;

	blkcount = reccount = 0;

	while ((size = (*recformat->cpblk)()) > 0)	{
		while (size < MINBLK)
			block[size++] = FILLER;
		tpwrite(tf, block, size);
		++blkcount;
	}
	VOID(fclose(file));
	if (pflag)	{
		printf("File name: %s\n", filename);
		printf("Record format: %s\n", rectype);
		printf("Block length: %d; number of blocks: %d\n",
			blklength, blkcount);
		printf("Record length: %d; number of records: %d\n\n",
			reclength, reccount);
	}
}

int
cpFblk()	{
	int ch;
	int count = 0;

	while (count < blklength && (ch = getc(file)) != EOF)	{
		int rpos = 0;
		reccount++;
		while (ch != NL && ch != EOF)	{
			if (ch == TAB)	{
				int nrpos = (int)tab((long)rpos);
				while (rpos < nrpos)	{
					block[count++] = SP; rpos++;
				}
			}
			else	{
				block[count++] = ch; rpos++;
			}
			ch = getc(file);
		}
		while (rpos < reclength)	{
			block[count++] = SP; rpos++;
		}
	}
	return count;
}

int
cpUblk()	{
	int ch;
	int count = 0;

	while (count < blklength && (ch = getc(file)) != EOF)	{
		block[count++] = ch;
	}
	if (count > 0)
		reccount++;
	return count;
}

FORMAT formats[] =	{
	{'F', checkF, cpFblk},
	{'U', checkU, cpUblk},
	{'D', NULL, NULL},
	{'S', NULL, NULL},
	{EOS, NULL, NULL}
};

/*
 * the setting of fields
 */

int2fld(n, addr, size) char *addr;	{

	addr += size;
	while (size-- > 0)	{
		*--addr = n % 10 + '0';
		n = n / 10;
	}
}

dat2fld(y, d, date, size) char *date;	{

	if (size != 6)
		abort();
	str2fld("", Sp(date));
	int2fld(y, Year(date));
	int2fld(d, Yday(date));
}


enq_fld(expl, md, def, addr, size) char *expl, *def, *addr;	{
	char *ans;

	inmood (md)
		ans = enq_str(expl, mood, def);
		iferr (strlen(ans) == 0)
			printf("No empty answer allowed\n");
		enderr;
		iferr (strlen(ans) > size)
			printf("The %s is too long\n", expl2str(expl));
			printf("The maximum length is %d character%s\n",
				english(size));
		enderr;
		iferr (!isascstr(ans))
			printf("The %s `%s' contains non-printing chars\n",
				expl2str(expl), ans);
		enderr;
	endmood;
	str2fld(ans, addr, size);
}

enq_num(expl, md, n, addr, size) char *expl, *addr;	{
	int2fld(enq_int(expl, md, n, tento(size)-1), addr, size);
}

int
tento(n)	{
	int res = 1;

	while (n--)
		res *= 10;
	return res;
}

enq_dat(expl, md, y, d, date, size) char *expl; char *date;	{

	dat2fld(y, d, date, size);
	inmood (md)
		char *ans = enq_str(expl, mood, fld2str(date, size));
		char *adt = (*ans == SP ? ans : ans - 1);

		y = fld2int(Year(adt));
		d = fld2int(Yday(adt));
		iferr (y < 0 || d <= 0 || d > (divis(y, 4) ? 366 : 365))
			printf("`%s' is not an acceptable date\n", ans);
		enderr;
	endmood;
	dat2fld(y, d, date, size);
}

locate()	{
	return;
}
