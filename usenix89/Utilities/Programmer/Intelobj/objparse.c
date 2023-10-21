/*
 * objparse.c
 * rlgvax!dennis (Dennis Bednar)
 * parse intel 286 object file on Xenix System 5 for the PC/AT.
 */
#include <stdio.h>
#include <sys/relsym86.h>

/* errors passed to error() */
char err1[] = "premature eof on object file";
char err2[] = "*** Need to add more decoding\n";

/* char to int extend.  Safeguard against high bit propogation errors */
#define	c_ext(c)	(int) ( (c) & 0xff)
#define u_int	unsigned int
#define u_char	unsigned char

#define EOF_PREM err1
#define ADD_MORE err2

#define MAXRECNAME 1052	/* max bytes in name field of record in obj file */
#define	MIDHEX	10	/* after 10 hex digits, middle of line */
#define	MAXHEX	20	/* after 20 hex digits, end of the line, output \n */

/* extern data */

/* global data */
char	*cmd;

/* extern non-int functions */

/* local non-int functions */
char	*u_errmesg();
u_int	pr_indx();
u_int	get_index();


main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i;
	FILE	*fp;

	cmd = argv[0];

	if (argc == 1)
		{
		errsync();
		fprintf(stderr, "Usage: %s objfile [... objfile]\n", cmd);
		exit(1);
		}

	for (i = 1; i < argc; ++i)
		{
		fp = fopen(argv[i], "r");
		if (fp == NULL)
			{
			errsync();
			fprintf(stderr, "%s: can't read %s: %s\n", cmd, argv[i], u_errmesg());
			exit(1);
			}
		objparse(fp, argv[i]);	/* pass the filename */
		fclose(fp);
		}
}

/* offset into file, shared by mgetc() and objparse() */
/* internally, store offsets with 0 = 1st, change adj() macro to be
 * printed with 1 = 1st!
 */
static	long	g_offset = 0L;	/* relative byte offset in obj file 0=1st*/
static	u_char	g_checksum;	/* global checksum, updated by my_getc() */
static	int	g_lineoffset= 0;	/* byte offset within this record */
/* use adj() macro to adjust printf()s to make 1=1st */
#define	adj(num)	((num))		/* 0 relative for now */

/*
 * my version of getc()
 * added value: increments global offset within the file
 *	(must be zeroed at the begin of each file by other code)
 * added value: updates running checksum.
 *	(must be zeroed before each record by other code)
 * added value: prints the byte as it is read, in a nice format
 * added value: keeps track of the byte offset within the record,
 *	(must be zeroed at the begin of each record by other code)
 */
my_getc(fp)
	register	FILE	*fp;
{
	register	int	c;
	c = getc(fp);		/* ignore premature EOF */
	if (c != EOF)
		{

		/* print the file offset only at line begin, and the char */
		if (g_lineoffset == 0)
			printf("%5d: ", g_offset);
		printf("%.2x ", c);

		/* accumulate stuff on the char just read */
		g_checksum += c;
		++g_offset;	/* inc offset within the object file */
		++g_lineoffset;	/* inc offset within the line in object file */

		/* pretty print stuff */

		/* force new line at MAXHEX numbers per line */
		if (g_lineoffset >= MAXHEX)
			{
			printf("\n");
			g_lineoffset = 0;	/* zero iff at left margin */
			}

		/* output extra blanks in middle of the line */
		if (g_lineoffset == MIDHEX)
			printf("  ");	/* to make it easier to read */
		}
	else
		;		/* dont print "<EOF>" */

	return	c;		/* return char or EOF */
}

/*
 * parse an object file - see "iAPX-286 and File Formats",
 * chapter 7, in the Xenix System 5 "C Users Guide".
 */
objparse(infp, filenm)
	FILE	*infp;
	char	*filenm;
{
	/* fields of an object file */
	int	rectype;	/* type of record */
	int	lolen;		/* lo byte of record len */
	int	hilen;		/* hi byte of record len */
	int	reclen;		/* computed: hilen << 8 + lolen */
	int	namelen;	/* first byte of name field */
	char	name[MAXRECNAME];	/* store name here */
	int	chksum;		/* checksum byte */

	int	recnum;		/* record number being read - adj() to print */
	int	i;		/* loop index reading name[] */
	int	c;		/* char in read loop of name[] */
	char	*d;		/* dest pointer in read loop of name[] */
	long	rec_offset;	/* byte offset within file of this record */

	/* stuff to handle processing of repeated byte/name type strings that
	 * are *NOT* terminated by NULL, but terminated by the byte len.
	 * LNAMES List of Names Record.
	 */
#define ST_LEN	0	/* state to mean length byte expected */
#define ST_DAT	1	/* state to mean data byte in "len" byte string, !0 terminated */
	int	rpt_len;		/* len of substring chars following */
	int	rpt_state;		/* state flage for byte/string processing */
	char	*s;			/* fetch pointer with name[] */

	/* symbol table management for LNAMES */
#define MAXLNAMES	50
#define SYMTABSIZE	512
	int	num_lnames = 0;	/* number of LNAMES records */
	int	name_inx;	/* index of name within a LNAMES(96) record */
static	char	*name_ptr[MAXLNAMES];	/* ptrs to indexed names */
static	char	name_tab[SYMTABSIZE];	/* symbol table for holding names */
	/* name_ptr[1] points within name_tab[], each name terminated by NULL */


	/* initialize data before first record */
	g_offset = 0L;		/* beginning of object file */
	recnum = 0;		/* reading first record - adj() to print */
	g_checksum = 0;		/* checksum is okay */
	g_lineoffset = 0;	/* offset within the current record */

	/* read records from object file */
	while ((rectype = my_getc(infp)) != EOF)
		{
		rec_offset = g_offset - 1L;	/* byte offset of this record */
		lolen = my_getc(infp);
		if (lolen == EOF)
			error(EOF_PREM);
		hilen = my_getc(infp);
		if (hilen == EOF)
			error(EOF_PREM);

		reclen = (hilen << 8) | (lolen);

		/* read the name/number (reclen - 1 chars),
		 * but NOT the checksum into the name[] array.
		 */
		for (i = 0, d=name; i < reclen-1; ++i)
			{
			if (i >= MAXRECNAME)
				error("Record Name too long");
			c = my_getc(infp);
			if (c == EOF)
				error(EOF_PREM);
			*d++ = c;
			}


		/* overstores place where checksum goes in name[] */
		if (reclen > 0 )
			name[reclen-1] = '\0';	/* terminate if string */
		else
			error("Rec Len Unexpected Zero Length");

		{
		u_char	exp_sum;
		exp_sum = 256 - g_checksum; /* two's complement expected sum */
		chksum = my_getc(infp);	/* read the checksum byte */
		/* low byte of global checksum should now be zero */
		if (g_checksum & 0xff)	/* overkill - works for int g_checksum too */
			{
			printf("\n*** Bad Checksum, Expected Checksum Was %.2x ***\n", exp_sum);
			error("Bad checksum in obj file");	/* nothing fancy */
			}
		}


		/* don't output two newlines! */
		if (g_lineoffset != 0)	/* cursor not at left margin */
			printf("\n");	/* terminate the hex chars */

		/* clear counters before next record */
		g_checksum = 0;	/* clear it out for next record */
		g_lineoffset = 0;	/* clear for next record */

		printf("Record # %d begins at offset %ld",
		 adj(recnum), (long) adj(rec_offset));
		printf(" ends at offset %ld\n", adj(g_offset)-1L);

		/* decode record type */
		switch (rectype) {

		/* T-module Header Record, pg 7-18, RecType=80H */
		case MTHEADR:
			namelen = c_ext(name[0]);
			/* check if the namelen is okay */
			if (reclen != namelen+2)
				error("Name Length of MTHEADR is bad");
			printf("THEADR (%xH) T-Module Header: T-module name = '%s'\n",
				rectype, name+1);
			break;

		/* List of Names Record, pg 7-18 - RecType=96H */
		case MLNAMES:
			if (++num_lnames >= 2)
				error("Cannot handle more than one LNAMES record");
			if (c_ext(name[0]) == 0)
				{
				printf("Warning, undocumented 7.4.10 T-module Name record, proceeding anyway!\n");
				reclen -= 2;	/* dont count name[0], checksum */
				s = name + 1;	/* start of repeated names */
				}
			else
				{
				reclen -= 1;	/* minus checksum byte */
				s = name;
				}

			/* header message for this line */
			printf("LNAMES (%xH) T-module name:", rectype);

			/* process repeated names in record */
			/* consists of repeated "byte len, len chars" */
			/* this logic both displays the field names, plus
			 * copies the data into the symbol table, such that
			 * name_ptr[i] is a pointer to a null-terminated
			 * string for the ith name, where i=1 is the
			 * first name as defined on page 7-18 of the
			 * reference.
			 */
			/* i steps thru the record */
			/* name_inx steps thru each name */
			/* d steps thru the symbol table where we store name */
			for (i = 0, rpt_state = ST_LEN, name_inx=1, d=name_tab;
				i < reclen; ++i, ++s)
				{
				switch(rpt_state) {
				case ST_LEN:
					rpt_len = c_ext(*s);	/* get byte len */
					printf(" (L=%d)I=%d:", rpt_len, name_inx);
					name_ptr[name_inx++] = d; /* store name */
					if (name_inx >= MAXLNAMES)
						error("Too many names in LNAMES record");
					if (rpt_len == 0)	/* zero len name */
						*d++ = '\0';	/* terminate the name */
						/* &&  don't change state */
					else
						rpt_state = ST_DAT;	/* start of data next */
					break;
				case ST_DAT:
					c = *s;		/* get char from buffer */
					*d++= c;	/* copy into symbol tbl */
					/* check if last byte of symb tbl used */
					if (d >= &name_tab[SYMTABSIZE])
						error("Symbol Table Overflow");
					putchar(c);	/* display it */
					if (--rpt_len <= 0)	/* reached EOS */
						{
						rpt_state = ST_LEN; /* expect len of next name */
/* out (bumped elsewhere)			++name_inx;	/* next name */
						*d++ = '\0';	/* terminate name */
						}
					break;
				} /* case */
				} /* for - done w repeated strings */

			/* terminate line */
			putchar('\n');

			if (rpt_state != ST_LEN)
				error("Len byte expected in repeated Namelist");
			break;




		/* Segment Definition Record, pg 7-19, RecType=98H */
		case MSEGDEF:
			{
			register	char	*s;	/* char pointer within name[] */
			int	segattr;
			static	int	num_segdef = 0;	/* segdef recorde # */

			/* segattr exploded, so called "ACBP" */
			int	af;	/* 3-bit Alignment field */
			int	cf;	/* 3-bit Combination field */
			int	bf;	/* 1-bit Big 64K seg field */
			int	pf;	/* 1-bit Page Resident field */
			/* no var for frame number or offset */

			unsigned int	seglen;	/* segment length */
			int	seglo;	/* hi byte of segment len */
			int	seghi;	/* lo byte of segment len */


			int	segxname;	/* index of Segment Name */
			int	segxclas;	/* index of Class Name */
			int	segxovly;	/* index of Overlay Name */

			/* Alignment Names indexed by 3-bit field */
			static	char *align_name[] = {
			"Absolute(0)",		/* 0 */
			"Byte(1)",		/* 1 */
			"Word(2)",		/* 2 */
			"Para(3)",		/* 3 */
			"Page(4)",		/* 4 */
			"???(5)",		/* 5 */
			"???(6)",		/* 6 */
			"???(7)",		/* 7 */
			};

			/* from page 7-20 of iAPX-286 and File Formats */
			static char *comb_name[] = {
			"Not combinable(0)",	/* 0 */
			"undefined(1)",		/* 1 */
			"Public(2)",		/* 2 */
			"Undefined(3)",		/* 3 */
			"Public(4)",		/* 4 */
			"Stack(5)",		/* 5 */
			"Common(6)",		/* 6 */
			"Public(7)",		/* 7 */
			};

			++num_segdef;	/* count another SEGDEF record */

			/* start at the beginning */
			s = name;
			printf("SEGDEF (%xH) Segment Definition Record: #%d",
			rectype, num_segdef);

			/* get the attribute bytes */
			segattr = *s++;
			af = (segattr >> 5) & 07;
			cf = (segattr >> 2) & 07;
			bf = (segattr >> 1) & 01;
			pf = (segattr & 01);
			printf(" Align=%s Combination=%s BigSeg=%d PageRes=%d",
			align_name[af], comb_name[cf], bf, pf);


			if (af == 0)		/* 2 byte optional follows */
				{
				printf(" Frame=%d Offset=%d",
				 c_ext(*s), c_ext(*(s+1)) );
				++s;	/* skip opt frame number */
				++s;	/* skip opt offset */
				}

			/* get segment length */
			seglo = c_ext(*s++);
			seghi = c_ext(*s++);
			seglen = (seghi << 8) + seghi;
			printf(" SegLen=%u", seglen);	/* print exactly the number */

			segxname = c_ext(*s++);	/* Segment Name Index */
			segxclas = c_ext(*s++);	/* Class Name Index */
			segxovly = c_ext(*s++);	/* Overlay Name Index */

			printf(" SegName=%s(I=%d)", name_ptr[segxname], segxname);
			printf(" ClassName=%s(I=%d)", name_ptr[segxclas], segxclas);
			printf(" OverlayName=%s(I=%d)", name_ptr[segxovly], segxovly);

			printf("\n");	/* whew! */
			}
			break;

		/* Group Definition Record, 9AH, pg 7-22 */
		case MGRPDEF:
			{
			char	*s;
			u_int	grpinx;
			u_int	seginx;

			printf("GRPDEF (%xH) Group Definition Record:", rectype);
			s = name;
			grpinx = get_index(&s);
			printf(" GroupName=%s(I=%d)", name_ptr[grpinx], grpinx);

			/* loop thru the Group Component Descriptors, a
			 * series of pairs of usu 2 bytes: 0xff, followed by
			 * Segment Index byte/word. (record # of SEGDEF)
			 * minus 2 bytes, Group Name Index and the Checksum.
			 */
			for (i = 0; i < reclen-2; ++i,++i)
				{
				if ((c = c_ext(*s++)) != 0xff)
					{
					printf("Got 0x%x", c);
					error("SI not 0xff!");
					}
				seginx = get_index(&s);	/* 1 or 2 byte index */
				printf(" SegmentIndex=SEGDEF#%d", seginx);
				}
			printf("\n");
			}
			break;

		/* page 7-23 (8EH) */
		case MTYPDEF:
			printf("TYPDEF (%xH) Type Definition Record\n", rectype);
			printf(ADD_MORE);
			break;

		/* page 7-26 (90H) */
		case MPUBDEF:
			{
			char	*s;
			int	grp_inx,
				seg_inx;
/*				typ_inx;	*/

			printf("PUBDEF (%xH) Public Names Definition Record: ", rectype);

			s = name;	/* begin of "public base" */

			/* print the index values, as a side effect,
			 * also get the values.
 			 */
			grp_inx = pr_indx("Group Index", &s);
			seg_inx = pr_indx("Segment Index", &s);

			if (grp_inx == 0 && seg_inx == 0)
				pr_16lh("Frame Number", &s);

			/* print repeated 3-tuple of (public name,
			 * public offset, type index)
			 */
			for (	; s < &name[reclen-1];	)	/* before chksum */
				{
				pr_name(&s);	/* print the public symbol name */

				pr_16lh("Public Offset", &s);	/* print 16-bit value */

				(void) pr_indx( "Type Index", &s);

				/* we should NOT be AFTER the checksum byte */
				if (s >= &name[reclen])
					error("Pubdef record too long or bad format");
				}

			/* we should be on the chksum byte */
			if (s != &name[reclen-1])
				error("Pubdef record not terminated properly");
			else
				printf("\n");	/* terminate the record */
			}
			break;

		/* page 7-28 (8CH) */
		case MEXTDEF:
			printf("EXTDEF (%xH) External Names Definition Record\n", rectype);
			printf(ADD_MORE);
			break;


		/* page 7-29 (94H) */
		case MLINNUM:
			printf("LINNUM (%xH) Line Numbers Record\n", rectype);
			printf(ADD_MORE);
			break;


		/* page 7-30 (A0H) */
		case MLEDATA:
			printf("LEDATA (%xH) Logical Enumerated Data Record\n", rectype);
			printf(ADD_MORE);
			break;

		/* page 7-31 (A2H) */
		case MLIDATA:
			printf("LIDATA (%xH) Logical Iterated Data Record\n", rectype);
			printf(ADD_MORE);
			break;

		/* page 7-33 (9CH) */
		case MFIXUPP:
			printf("FIXUPP (%xH) Fixup or Thread Record\n", rectype);
			printf(ADD_MORE);
			break;

		/* page 7-38 (8AH) */
		case MMODEND:
			printf("MODEND (%xH) Module End Record\n", rectype);
			printf(ADD_MORE);
			break;

		/* Comment Record, pg 7-39, RecType=88H */
		case MCOMENT:
			printf("COMENT (%xH) Comment Record: Type=0x%x Class=0x%x '%s'\n",
			rectype, c_ext(name[0]), c_ext(name[1]), name+2);
			break;

		default:
			errsync();
			fprintf(stderr, "%s: unknown record number 0x%x type 0x%x in file %s, offset %ld\n",
			 cmd, adj(recnum), (int)rectype&0xff, filenm, (long) adj(rec_offset));
			exit(1);
		} /* case */
		++recnum;		/* reading next record in obj file */
		printf("\n");		/* separator between records */
/* debug */	fflush(stdout);
		}

	if(ferror(infp))
		error("IO Error Reading from Object File");
	else
		printf("EOF on object file '%s'\n", filenm);
}

error(msg)
	char	*msg;
{
	errsync();
	fprintf(stderr, "Error Found: '%s' Exitting Early\n", msg);
	exit(1);
}

/*
 * return UNIX error message associated with errno
 * more flexible than perror(3)
 */

char *
u_errmesg()
{
	extern	int	errno;
	extern	int	sys_nerr;
	extern	char	*sys_errlist[];
static	char	buffer[50];

	if (errno < 0 || errno >= sys_nerr)
		{
		sprintf( buffer, "errno %d undefined (%d=max)", errno, sys_nerr);
		return(buffer);
		}

	return( sys_errlist[errno] );
}

/*
 * TRICK CODE:
 * call this before all fprintf(stder, ... )
 * so that stdout and stderr appears in right order when both stdout & stderr
 * are redirected to a file, as in
 *	cmd args >file 2>&1.
 * so that output is the same as when you run cmd from the shell.
 *
 * This code assumes that when you redirect to a file, that printf()'s
 * are not fflushed() until stdout buffer fills, but that fprintf(stderr)
 * are fflushed immediately!  This is the way UNIX stdio package works.
 *
 * The main purpose of this code is to prevent stderr messages
 * from being interspersed in the middle of a printf line.
 */
errsync()
{
	fflush(stdout);		/* so if cmd >file, you get errs in right order */
}
/*
 * decode index field, see section 7.9, page 7-8.
 * reads index numbers 0 to 32K-1 encoded as follows:
 * one byte: encodes numbers 0 to 127 (ie high bit 0x80 is 0)
 * two bytes: encodes numbers 128 to 32K-1 (ie high bit 0x80 is 1)
 * with high order seven bytes in first byte, and low order
 * eight bits in 2nd byte.
 *
 * Input: '*pi' points to 1st byte (and possibly only byte) of
 * index field in record.
 *
 * Output: new *pi pointing to first byte AFTER (1or2 byte) index field,
 * and decoded integer.
 */
u_int
get_index(pi)
	register	char	**pi;	/* pointer indirect */
{
	unsigned	int	index;
	/* Note: unsigned char *p avoids propogating high order bit of
	 * chars to ints on assignments flagged by *** below!!
	 */
	unsigned	char		*p;		/* direct pointer */

	p = *pi;	/* get ptr to 0-127 byte or 0x80|1-32K */
	index = *p++;	/* **c_ext() - fetch array[0], and step to array[1] */
	*pi = p;	/* return new pointer to array[1] - assumes 0-127 */
	if (index >= 128)	/* two bytes encoding */
		{
		index = (index & 127) << 8;
		index += *p++;	/* *** no c_ext() required */
		*pi = p;	/* return ptr to array[2] */
		}
	return index;		/* and return with the number */
}

/*
 * print "msg=<number> " where number is encoded as follows:
 * 16-bit lo then hi byte data pointed to by **p, and then
 * return new pointer 2-bytes beyond start.
 */
pr_16lh(msg, pi)
	char	*msg;	/* like "value", prints as "value=<number>" */
	char	**pi;	/* pointer indirect */
{
	unsigned	char	*p;	/* direct pointer */
	unsigned	int	i;	/* the 16-bit number */
	unsigned	int	h, l;	/* hi, low chars */

	p = *pi;		/* fetch ptr to data-lo (1st byte) */
	l = (*p++);		/* fetch data lo */
	h = (*p++);		/* fetch data hi */
	i = (h << 8) | (l);	/* put them together */

	*pi = p;		/* return new pointer 2 bytes AFTER */
	printf("%s=%u ", msg, i); /* unsigned print */
}

/*
 * print "Name=<name> " where name is encoded in format
 * one bytelen followed by bytelen chars.
 * return ptr after the last char of the name.
 */
pr_name(pi)
	char	**pi;	/* indirect pointer */
{
	unsigned	char	*p;	/* direct pointer */
	unsigned	int	len;

	p = *pi;

	len = *p++;

	printf("Name=");

	/* loop to print each char of the name */
	while (len--)
		putchar(*p++);

	putchar(' ');

	*pi = p;	/* return new ptr AFTER the name */
}

/*
 * print "msg=<number> ", where number is an index field,
 * returns with number (in case you want it), and bumps
 * *pi AFTER the index field.
 */
u_int
pr_indx(msg, pi)
	char	*msg;
	char	**pi;
{
	unsigned	int	index;

	index = get_index(pi);
	printf("%s=%u ", msg, index);
	return	index;
}
