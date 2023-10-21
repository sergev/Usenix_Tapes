#define	MSGUSE	"Usage: ansir [-cfhlmijnpg] [ name ... ]"
#include	"ansi.h"

#define	MAXBLK	TP_MAXB
#define	MINSIZE	6	/* smaller is a noise block */
#define	CRECSEP	"\n"	/* record separator for coded files */

/*
 * Name: ansir, read ANSI standard labelled tape
 * Author: Dick Grune
 * Version: 820314
 */

char iflag = FALSE, jflag = FALSE, nflag = FALSE, pflag = FALSE, gflag = FALSE;
char ok_fln, ok_rct, ok_blk, ok_rec, ok_bfo, ok_loc, skip_it;
char **filelist = NULL;

char *labcode = NULL;
char *filecode = NULL;	/* may be `ascii', `ebcdic' or `binary' */
char *ascii = "ASCII";
char *ebcdic = "EBCDIC";
char *binary = "BINARY";
char binrecsep[MAXSTR];

int bsize = 0;
char block[MAXBLK];
#define	is_block	(bsize>0)
#define	is_tm		(bsize==0)

#define	MAXUGLY	99	/* upper limit for tallying */
char uglies[256];	/* for tallying non-UNIX-ASCII chars */
long tot_uglies;

int HDR1blc;	/* block count as found in HDR1-label */

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
			case 'j':
				iflag = jflag = TRUE;
				break;
			case 'l':
				nmdns = TP_DENL;
				break;
			case 'm':
				unit = *++pp - '0';
				break;
			case 'n':
				nflag = TRUE;
				break;
			case 'p':
				pflag = TRUE;
				break;
			default:
				goto Lbad;
			}
			argc--; argv++;
		}
	}

	if (argc > 0)
		filelist = argv;

	tf = tpopen(unit, nmdns, "rx");
	nextblk();

	lblVOL1();
	lblUSR("UVL");
	while (is_block)	{
		clearflags();
		lblHDR1();
		lblHDR2();
		lblUSR("HDR");
		lblUSR("UHL");
		read_tm();
		copy();
		lblEOF1();
		lblEOF2();
		lblUSR("EOF");
		lblUSR("UTL");
		read_tm();
	}
	if (pflag)
		printf("\nEnd of tape\n");
	tpclose(tf);
	exit(0);

Lbad:
	errors(MSGUSE);
}

clearflags()	{
	ok_fln = ok_rct = ok_blk = ok_rec = ok_bfo = ok_loc = skip_it = FALSE;
}

/*
 * Reads the VOL1-label and prints its contents
 */

lblVOL1()	{

	if (!label("VOL1", Whole(VOL1buf)))	{
		missing("VOL1");
		return;
	}

	if (pflag)	{
		prhead(Labidf(VOL1buf));
		printf("The labels are in %s\n", labcode);
		prfield("Volume serial number", Volidf(VOL1buf));
		prfield("Volume accessibility", Volacc(VOL1buf));
		prunused(Sp1(VOL1buf));
		prfield("Owner identification", Ownidf(VOL1buf));
		prunused(Sp2(VOL1buf));
		prfield("Label version", Labvers(VOL1buf));
	}
}

lblUSR(idf) char *idf;	{
	char USRnbuf[80];

	while (label(idf, Whole(USRnbuf)))
		if (pflag && !skip_it)	{
			prhead(Labidf(USRnbuf));
			prfield("Contents", Contents(USRnbuf));
		}
}

/*
 * Reads the HDR1-label, prints its contents, and sets `filename'
 */

lblHDR1()	{

	filseqnum++;
	if (!label("HDR1", Whole(HDR1buf)))	{
		missing("HDR1");
		ok_fln = FALSE;
		return;
	}

	strcpy(filename, fld2str(Fileidf(HDR1buf)));
	ok_fln = TRUE;

	if (!interesting(filename))	{
		skip_it = TRUE;
		return;
	}

	if (pflag)
		prHDR1();

	if (fld2int(Filsecnum(HDR1buf)) != filsecnum)	{
		locate();
		printf("File section number not %d\n", filsecnum);
	}
	if (fld2int(Filseqnum(HDR1buf)) != filseqnum)	{
		locate();
		printf("File sequence number not in order\n");
	}

	HDR1blc = fld2int(Blkcount(HDR1buf));
	if (chk_int("block count", HDR1blc) && HDR1blc != 0)	{
		locate();
		printf("Block count starts from %d\n", HDR1blc);
	}
}

/*
 * Reads the HDR2-label, prints its contents and sets
 * `recformat', `blklength', `reclength' and `bufoffset'.
 */

lblHDR2()	{
	int found = label("HDR2", Whole(HDR2buf));

	if (skip_it)
		return;

	if (!found)	{
		missing("HDR2");
		ok_rct = ok_blk = ok_rec = ok_bfo = FALSE;
		return;
	}

	if (pflag)
		prHDR2();

	fld2fld(Recformat(HDR2buf), Whole(rectype));
	ok_rct = TRUE;
	blklength = fld2int(Blklength(HDR2buf));
	ok_blk = chk_int("block length", blklength);
	reclength = fld2int(Reclength(HDR2buf));
	ok_rec = chk_int("record length", reclength);
	bufoffset = fld2int(Bufoffset(HDR2buf));
	ok_bfo = chk_int("buffer offset", bufoffset);
}

int
label(idf, addr, size) char *idf, *addr;	{

	if (!is_block)
		return FALSE;
	if (labcode != NULL)
		return is_lab(idf, addr, size);
	labcode = ascii;
	if (is_lab(idf, addr, size))
		return TRUE;
	labcode = ebcdic;
	if (is_lab(idf, addr, size))
		return TRUE;
	labcode = NULL;
	return FALSE;
}

int
is_lab(idf, addr, size) char *idf, *addr;	{
	int sz = bsize < size ? bsize : size;

	strncpy(addr, block, sz);
	if (labcode == ebcdic)
		conv(addr, sz);
	str2fld("", addr+sz, size-sz);
	if (strhead(idf, addr))	{
		nextblk();
		return TRUE;
	}
	return FALSE;
}

int
strhead(s1, s2) char *s1, *s2;	{
	while (*s1)
		if (*s1++ != *s2++)
			return FALSE;
	return TRUE;
}

read_tm()	{
	if (is_tm)
		nextblk();
	else	{
		locate();
		printf("Tape mark missing\n");
	}
}

nextblk()	{

	do bsize = tpread(tf, block, MAXBLK);
	while (bsize > 0 && bsize < MINSIZE);
}

int
interesting(fn) char *fn;	{
	char **lst = filelist, *nm;

	if (lst == NULL)
		return TRUE;
	while ((nm = *lst++) != NULL)
		if (strcmp(nm, fn) == 0)
			return TRUE;
	return FALSE;
}

conv(addr, size) char *addr;	{

	while (size-- > 0)	{
		*addr = ebc2asc(*addr);
		addr++;
	}
}

char
asc2ebc(ch) char ch;	{
	char ch1 = 0;

	while (ch != ebc2asc(ch1))
		ch1++;
	return ch1;
}

/*
 * copy one file
 */

copy()	{

	init_copy();

	do copypart();
	while (change_tape());

	end_copy();
}

init_copy()	{
	int i;

	blkcount = reccount = 0;
	tot_uglies = 0L;
	for (i = 0; i < n_items(uglies); i++)
		uglies[i] = 0;

	getpars();
}

end_copy()	{

	if (pflag && !skip_it)	{
		printf("\n");
		if (file != NULL)
			printf("Record count: %d\n", reccount);
		printf("Block count: %d\n", blkcount);
		if (file != NULL)
			prf_s("File copied: %s\n", filename);
		else
			printf("File skipped\n");
	}

	if (file != NULL)	{
		VOID(fclose(file));
		if (tot_uglies != 0L)	{
			int i;

			locate();
			printf("File contained %D non-ASCII character%s:\n",
				english(tot_uglies));
			for (i = 0; i < n_items(uglies); i++)	{
				int n = char2int(uglies[i]);

				if (n != 0)	{
					if (n <= MAXUGLY)
						printf("%s: %d\n",
							char2str(i), n);
					else	printf("%s: >%d\n",
							char2str(i), MAXUGLY);
				}
			}
		}
	}
}

/*
 * getpars sets `filename', `filecode', `recformat'
 * and as many further parameters as is necessary
 */

extern FORMAT fdummy;

getpars()	{

	if (nflag || skip_it)	{
		recformat = &fdummy;
		return;
	}

	inmood (!ok_fln ? ASK_YES : iflag ? ASK_SUG : ASK_NO)
		strcpy(filename, enq_str("\
	The `file name' is the name under which the tape file will be\n\
	stored on disk. The named file must not already exist.\n\
	Use a minus - to skip the file.\n",
			mood, filename));

		if (strcmp(filename, "-") == 0)	{
			recformat = &fdummy;
			return;
		}
		iferr (!isascstr(filename))
			prf_s("Filename %s contains non-printing chars\n",
				filename);
		enderr;
		iferr ((file = fopen(filename, "r")) != NULL)
			prf_s("File %s already exists\n", filename);
			VOID(fclose(file));
		enderr;
		iferr ((file = fopen(filename, "w")) == NULL)
			prf_s("Cannot create %s\n", filename);
		enderr;
	endmood;

	inmood (labcode == NULL && filecode == NULL ? ASK_YES :
			labcode == NULL || iflag ? ASK_SUG : ASK_NO)
		filecode = enq_str("\
	The `character code' is the code of the file on tape; it may be\n\
	ASCII, EBCDIC (usual for IBM-tapes) or BINARY (no conversion).\n\
	When in doubt, use %s.\n",
			mood,
			filecode != NULL ? filecode :
			labcode != NULL ? labcode :
			ascii);
		switch (filecode[0])	{
		case 'A':
			filecode = ascii;
			break;
		case 'B':
			filecode = binary;
			break;
		case 'E':
			filecode = ebcdic;
			break;
		default:
			printf("`%s' is not a character code\n", filecode);
			filecode = NULL;
		}
		iferr (filecode == NULL)
			printf("Specify %s, %s or %s\n", ascii, ebcdic, binary);
		enderr;
	endmood;

	if (filecode == binary)
		strcpy(binrecsep,
			enq_str("\
	The `record separator' only applies to the character code\n\
	BINARY, where it specifies what character(s) should be written\n\
	to disk for each end-of-record on the tape. You will probably\n\
	want it to be empty, but, when in doubt, try a recognizable\n\
	string like }}}} and examine the results.\n",
				ASK_SUG, binrecsep));

	inmood (!ok_rct || jflag ? ASK_SUG : ASK_NO)
		char *rct = enq_str("\
	The `record format' tells how the block on tape must be cut into\n\
	text records (lines). There are five standard ways to do so:\n\
	F: each N consecutive characters form a record, where N is the\n\
	   `record length',\n\
	D: a header in each record tells its length,\n\
	U: each block is one record,\n\
	S: a special format in which records may be longer than blocks,\n\
	V: IBM Variable format.\n\
	When in doubt, use %s and examine the results, or try them all.\n",
			mood, !ok_rct ? "U" : fld2str(Whole(rectype)));

		iferr (strlen(rct) != 1
			|| (recformat = format(rct[0])) == NULL)
			printf("`%s' is not an ANSI format\n", rct);
			printf("Please specify F, D, U, S or V\n");
		enderr;
		iferr (recformat->cpblk == NULL)
			printf("%s-format not implemented\n", rct);
		enderr;
		str2fld(rct, Whole(rectype));
	endmood;

	inmood (!ok_blk || jflag ? ASK_SUG : ASK_NO)
		blklength = enq_int("\
	The `block length' is the maximum number of significant\n\
	characters in any physical block (as demarcated by two\n\
	interrecord gaps) on the tape.  Unless you know the exact value,\n\
	use a large number like %s.\n",
			mood, !ok_blk ? MAXBLK : blklength, MAXBLK);

		iferr (blklength == 0)
			printf("Block length cannot be zero\n");
		enderr;
	endmood;

	(*recformat->checkpar)();

	bufoffset = enq_int("\
	The `buffer offset' is the position in each block on the tape at\n\
	which the real information starts. Unless you know the exact\n\
	value, use 0.\n",
		!ok_bfo || jflag ? ASK_SUG : ASK_NO,
		!ok_bfo ? 0 : bufoffset,
		blklength - 1);
}

copypart()	{
	while (is_block)	{
		int used;

		++blkcount;
		if (bsize > blklength)
			bsize = blklength;
		if (filecode == ebcdic)
			conv(block, bsize);
		used = bufoffset;
		used += (*recformat->cpblk) (&block[used], bsize-used);
		if (!filler(&block[used], bsize-used))	{
			locate();
			printf("At block %d, after record %d:",
				blkcount, reccount);
			printf(" %d char%s garbage ignored\n",
					english(bsize-used));
		}
		nextblk();
	}
	read_tm();
}

int
change_tape()	{
	return FALSE;	/* not yet implemented */
}

checkF()	{

	inmood (!ok_rec || jflag ? ASK_SUG : ASK_NO)
		reclength = enq_int("\
	The `record length' is a number N such that a <newline> is\n\
	assumed after each N characters read from tape. When in doubt,\n\
	use %s and examine the results for a better value.\n",
			mood,
			!ok_rec || blklength < reclength ?
				blklength : reclength,
			blklength);

		iferr (reclength == 0)
			printf("Record length cannot be zero\n");
		enderr;
	endmood;
}


int
cpFblk(buf, size) char *buf;	{
	int i = 0;

	while (i <= size - reclength)	{
		char *rec = &buf[i];
		int sz = reclength;

		reccount++;
		if (filecode != binary)	{
			char pad = rec[sz-1];

			if (!Ascii95(pad) || pad == SP)	/* liberal */
				while (sz > 0 && rec[sz-1] == pad)
					sz--;
		}
		put_rec(rec, sz);
		put_eor();
		i += reclength;
	}
	return i;
}

int
cpUblk(buf, size) char *buf;	{

	reccount++;
	put_rec(buf, size);
	put_eor();
	return size;
}

#define	DVPREF	4	/* size of D or V length prefix */

int
cpDVblk(buf, size) char *buf;	{
	int i = rectype[0] == 'V' ? DVPREF : 0;

	while (size - i >= DVPREF)	{
		char *rec = &buf[i];
		int sz = DVlength(rec);

		if (sz < DVPREF || size - i - sz < 0)
			break;

		reccount++;
		put_rec(rec + DVPREF, sz - DVPREF);
		put_eor();
		i += sz;
	}
	return i;
}

int
DVlength(buf) char *buf;	{
	int res = 0;

	if (rectype[0] == 'V')	{
		int i;
		for (i = 0; i <= 1; i++)	{
			char ch = buf[i];
			res = res*256 +
				char2int(filecode == ebcdic ? asc2ebc(ch) : ch);
		}
	}
	else
		res = fld2int(buf, DVPREF);
	return res;
}

#define	SPREF	5	/* size of S length prefix */

int
cpSblk(buf, size) char *buf;	{
	int i = 0;

	while (size - i >= SPREF)	{
		char *rec = &buf[i];
		char ind = rec[0];
		int sz = fld2int(rec+1, SPREF-1);

		if (sz < SPREF || size - i - sz < 0)
			break;

		if (ind == '0' || ind == '1')
			reccount++;
		put_rec(rec + SPREF, sz - SPREF);
		if (ind == '0' || ind == '3')
			put_eor();
		i += sz;
	}
	return i;
}

skip()	{
	return;
}

int
skpblk(buf, size) char *buf;	{
	VOID(buf);
	return size;
}

int
filler(addr, size) char *addr;	{
	char ch = *addr;

	while (size--)
		if (ch != *addr++)
			return FALSE;
	return TRUE;
}

FORMAT formats[] =	{
	{'F', checkF, cpFblk},
	{'U', skip, cpUblk},
	{'D', skip, cpDVblk},
	{'V', skip, cpDVblk},	/* to cater for you know whom */
	{'S', skip, cpSblk},
	{EOS, NULL, NULL}
};
FORMAT fdummy = {EOS, skip, skpblk};

put_rec(rec, size) char *rec;	{
	int i;

	for (i = 0; i < size; i++)	{
		int ch = char2int(rec[i]);

		if (filecode == binary || Ascii95(ch))
			putc(ch, file);
		else	{
			fprintf(file, "%s", char2str(ch));
			if (uglies[ch] <= MAXUGLY)
				uglies[ch]++;
			tot_uglies++;
		}
	}
}

put_eor()	{
	char *sep = filecode == binary ? binrecsep : CRECSEP;
	char ch;

	while ((ch = *sep++) != EOS)
		putc(ch, file);
}

/*
 * Reads the EOF1-label and checks block count
 */

lblEOF1()	{
	int bcount;
	int found = label("EOF1", Whole(HDR1buf));

	if (skip_it)
		return;

	if (!found)	{
		missing("EOF1");
		return;
	}

	bcount = fld2int(Blkcount(HDR1buf));

	if (pflag)
		prHDR1();

	if (HDR1blc >= 0 && bcount != blkcount + HDR1blc)	{
		locate();
		printf(
		"File holds %d block%s whereas labels report %d block%s\n",
			english(blkcount), english(bcount - HDR1blc));
	}
}

/*
 * Read EOF2-label
 */

lblEOF2()	{
	int found = label("EOF2", Whole(HDR2buf));

	if (skip_it)
		return;

	if (!found)	{
		missing("EOF2");
		return;
	}

	if (pflag)
		prHDR2();
}

/*
 * Print routines
 */

prHDR1()	{
	prhead(Labidf(HDR1buf));
	prfield("File identifier", Fileidf(HDR1buf));
	prfield("Set identifier", Filesetidf(HDR1buf));
	prfield("File section number", Filsecnum(HDR1buf));
	prfield("File sequence number", Filseqnum(HDR1buf));
	if (gflag)	{
		prfield("Generation number", Gennum(HDR1buf));
		prfield("Generation version number", Genversnum(HDR1buf));
	}
	prfield("Creation date", Creatdate(HDR1buf));
	prfield("Expiration date", Expirdate(HDR1buf));
	prfield("File accessibility", Fileacc(HDR1buf));
	prfield("Block count", Blkcount(HDR1buf));
	prfield("System code", Syscode(HDR1buf));
	prunused(Sp3(HDR1buf));
}

prHDR2()	{
	prhead(Labidf(HDR2buf));
	prfield("Record format", Recformat(HDR2buf));
	prfield("Block length", Blklength(HDR2buf));
	prfield("Record length", Reclength(HDR2buf));
	prfield("System software", Syssoftw(HDR2buf));
	prfield("Buffer offset", Bufoffset(HDR2buf));
	prunused(Sp4(HDR2buf));
}

prhead(addr, size) char *addr;	{

	printf("\nInformation from the %s-label\n", fld2str(addr, size));
}

prfield(nm, addr, size) char *nm, *addr;	{
	char *str = fld2str(addr, size);

	printf("%s: ", nm);
	prf_s("%s\n", *str == EOS ? sps2txt(size) : str);
}

prunused(addr, size) char *addr;	{
	char *str = fld2str(addr, size);

	if (strlen(str) > 0)
		prf_s("Unused field: %s\n", str);
}

missing(idf) char *idf;	{

	locate();
	printf("%s-label missing\n", idf);
}

int
chk_int(nm, val) char *nm;	{

		if (val < 0)	{
			locate();
			printf("Garbage in %s field\n", nm);
			return FALSE;
		}
		else
			return TRUE;
}

locate()	{

	if (ok_loc || pflag)
		return;
	if (ok_fln)
		prf_s("\n*** At file `%s':\n", filename);
	else
		printf("\n*** At file number %d:\n", filseqnum);
	ok_loc = TRUE;
}
