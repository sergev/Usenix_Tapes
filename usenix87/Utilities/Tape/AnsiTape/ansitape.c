/*
 * ANSITAPE.C 
 *
 * This program creates, reads, writes, and takes directory listings of
 * ANSI-formatted tapes. 
 *
 * This program was developed at 
 *
 * HQDA Army Artificial Intelligence Center
 * Pentagon Attn: DACS-DMA
 * Washington, DC  20310-0200 
 *
 * Phone: (202) 694-6900 
 *
 ********************************************
 *
 * THIS PROGRAM IS IN THE PUBLIC DOMAIN 
 *
 ********************************************
 *
 * Modification History:
 *
 * 28 January 1987	David S. Hayes Cleaned up for Usenet,
	and changed switch formats.
  
 * 6 August 1986	David S. Hayes Added -st I/O switch. 
  
 * 28 July 1986		David S. Hayes Changed to stream-style tape
	access, added IBM labels and EBCDIC code conversion. 
  
 * 18 July 1986		David S. Hayes Changed to correct problem with
	files spanning more than one block. 
  
 * 17 June 1986		David S. Hayes Original version. 
 */

#define SYSID "UNIXTAPEV2.0"	/* version 2.0 patch level 0 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "ansitape.h"

#define DEFAULT_TAPE "/dev/rmt8"
#define IGN ' '			/* empty constant for hdr space fill */

/*
 * The largest file that we will try to determine recordsize for.
 * Anything larger than this, we just use recordsize = blocksize. 
 */
#define READMAX 100000


#define LIST		1
#define EXTRACT		2
#define WRITE		3
#define CREATE		4


#define makelower(c)	( isupper(c) ? tolower(c) : c)
#define makeupper(c)	( islower(c) ? toupper(c) : c)
#define max(a,b)	( a >= b ? a : b )
#define min(a,b)	( a < b ? a : b )

#define SAME 0


typedef unsigned char FLAG;	/* boolean values only */
#define TRUE		'\001'
#define FALSE		'\000'


extern int      errno;

char           *upstring();
char           *downstring();
char           *tail();
char           *malloc();
char           *alloca();
FLAG            match();
FLAG            eot();



char           *labels[] = {"", "HDR", "EOV", "EOF", ""};
#define TOP_OF_FILE	1
#define END_OF_TAPE	2
#define END_OF_FILE	3
#define END_OF_SET	4
#define UNKNOWN_LABEL	-1


struct ansi_vol1 vol1;
struct ansi_hdr1 hdr1;
struct ansi_hdr2 hdr2;
struct ansi_hdr3 hdr3;
struct ansi_hdr4 hdr4;

struct tape_control_block {
    /* Stuff pertaining to the tape set. */
    FLAG            ebcdic;
    FLAG            ibm_format;
    int             reel_switch;
    char            set_name[6];
    char            tapesys[13];
    char            owner[14];
    char            vol_access;
    int             ansi_level;
    int             reel_count;

    /* Stuff pertaining to an individual volume. */
    char            vol_name[6];
    FLAG            mounted;
    int             file_count;

    /* Stuff for an individual file. */
    char            tape_filename1[17];
    char            tape_filename2[63];
    char            unix_filename[80];
    int             blk_size;
    int             rec_size;
    char            rec_format;
    char            car_control;
    int             generation;
    int             genversion;
    char            file_access;
    long            created;
    long            expires;
    int             density;
    int             blk_count;
    int             blk_offset;

    /* Stuff to be able to read/write on the tape. */
    int             fd;
    char           *buffer;
    int             char_count;
    int             next;
    FLAG            eof;
    FLAG            open;
    int             rw_mode;
}               tcb;


int             func;		/* What we're supposed to do */
char           *tapename = DEFAULT_TAPE;
FLAG            verbose = FALSE,/* be wordy */
                query = FALSE,	/* ask before doing */
                no_hdr3 = FALSE,/* don't write HDR3 or HDR4 */
                ibm_format = FALSE,	/* use IBM standard label
					 * format */
                ebcdic = FALSE,	/* tape should be EBCDIC */
                flag_bs = FALSE,/* blocksize was given */
                flag_rs = FALSE,/* recordsize was given */
                flag_stdio = FALSE;	/* do file i/o using stdin/out */

char            main_set_name[6] = "UNIX";	/* file set id */
char            main_vol_name[6] = "UNIX";	/* currently-mounted
						 * volume */
char           *username,
               *progname;
int             main_blk_size = 2048,
                main_rec_size;
char            main_rec_format = REC_VARIABLE;	/* fixed or variable */
char            main_car_control = CC_IMPLIED;	/* carriage control */

char          **main_file_list;
int             main_file_count;


/*
 * Here's the code. 
 */

main(argc, argv)
    int             argc;
    char           *argv[];
{
    int             i;
    char           *getlogin();

    setbuf(stderr, NULL);	/* unbuffer error output */

    username = getlogin();
    progname = argv[0];
    if (username) {
	downstring(username, main_vol_name, 6);
	downstring(username, main_set_name, 6);
    };

    if (argc >= 2) {
	/* Now 1-character option names. */
	for (i = 0; argv[1][i]; i++) {
	    if (argv[1][i] == '-')
		continue;	/* minus is optional on first keylist */
	    switch (argv[1][i]) {
	      case 't':
		func = LIST;
		break;

	      case 'x':
		func = EXTRACT;
		break;

	      case 'c':
		func = CREATE;
		break;

	      case 'r':
		if (func != CREATE)
		    func = WRITE;
		break;

	      case 'v':
		verbose = TRUE;
		break;

	      case 'q':
		query = TRUE;
		break;

		/* Do file I/O to stdio instead. */
	      case 'f':
		flag_stdio = TRUE;
		break;

		/*
		 * Suppress HDR3 headers for systems that can't handle
		 * them. 
		 */
	      case '3':
		no_hdr3 = TRUE;
		break;

		/* Tape is to be written in ASCII. */
	      case 'a':
		ebcdic = FALSE;
		break;

#ifdef EBCDIC
	      case 'e':
		ebcdic = TRUE;
		break;

	      case 'i':
		ibm_format = TRUE;
		ebcdic = TRUE;
		break;
#endif

	      default:
		fprintf(stderr, "unknown key %c - ignored\n", argv[1][i]);
	    };
	};
    };

    for (i = 2; i < argc; i++) {
	/* Do 2-character options */

	if (strncmp(argv[i], "cc=", 3) == SAME) {
	    switch (argv[i][3]) {
	      case 'f':
		main_car_control = CC_FORTRAN;
		break;
	      case 'i':
		main_car_control = CC_IMPLIED;
		break;
	      case 'e':
		main_car_control = CC_EMBEDDED;
		break;
	      default:
		fprintf(stderr, "unknown carriage control option %s\n", argv[i]);
	    };
	    continue;
	};

	if (strncmp(argv[i], "rs=", 3) == SAME) {
	    if (strcmp(argv[i], "rs=r") == SAME) {
		main_rec_size = 0;
		main_rec_format = REC_VARIABLE;
	    } else {
		flag_rs = TRUE;
		sscanf(argv[i], "rs=%d", &main_rec_size);
		main_rec_format = REC_FIXED;
	    };
	    continue;
	};

	if (strncmp(argv[i], "bs=", 3) == SAME) {
	    flag_bs = TRUE;
	    sscanf(argv[i], "bs=%d", &main_blk_size);
	    continue;
	};

	if (strncmp(argv[i], "vo=", 3) == SAME) {
	    downstring(&argv[i][3], main_set_name, 6);
	    downstring(&argv[i][3], main_vol_name, 6);
	    continue;
	};

	if (strncmp(argv[i], "rf=", 3) == SAME) {
	    switch (argv[i][3]) {
	      case 'f':
		main_rec_format = REC_FIXED;
		break;
	      case 'v':
		main_rec_format = REC_VARIABLE;
		break;
	      default:
		fprintf(stderr, "unknown record format %c - ignored\n",
			argv[i][3]);
	    };
	    continue;
	};

	if (strncmp(argv[i], "mt=", 3) == SAME) {
	    tapename = &argv[i][3];
	    continue;
	};

	/* That's not an option, assume it's a file name. */
	break;
    };

    /* Figure out where and how many file names we have. */
    main_file_count = argc - i;
    main_file_list = argc ? &argv[i] : (char **) 0;

    if (main_rec_size > main_blk_size) {
	if (flag_rs) {
	    main_blk_size = main_rec_size;
	    fprintf(stderr, "blocksize adjusted up to recordsize (%d)\n",
		    main_rec_size);
	} else {
	    main_rec_size = main_blk_size;
	    fprintf(stderr, "recordsize adjusted down to blocksize (%d)\n",
		    main_rec_size);
	};
    };

    switch (func) {
      case WRITE:
	tcb.fd = open(tapename, O_RDWR, 0666);
	break;

      case CREATE:
	tcb.fd = open(tapename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	break;

      default:
	tcb.fd = open(tapename, O_RDONLY, 0666);
    }

    if (errno) {
	perror("can't open tape device");
	exit(-1);
    };

    switch (func) {
      case LIST:
	list_volume_set();
	break;

      case CREATE:
      case WRITE:
	write_volume_set();
	break;

      case EXTRACT:
	read_volume_set();
	break;

      default:
	fprintf(stderr, "specify one of -t, -x, -c, -r\n");
	exit(-1);
    };
    exit(0);
}


/* List contents of a full volume set. */

list_volume_set()
{
    mount_tape();
    for (; ansi_open(FREAD) == TOP_OF_FILE;) {
	ansi_close();
	list_file();
    };
}


/* List one file, possibly with maximized verbosity. */

list_file()
{
    printf("%-22s", tcb.unix_filename);

    if (verbose) {
	printf(" %5d blocks", tcb.blk_count);
	printf(", rec fmt %s %d bytes",
	       tcb.rec_format == REC_FIXED ? "fixed" : "var max",
	       tcb.rec_size);
	switch (tcb.car_control) {
	  case CC_IMPLIED:
	    printf(", cc implied");
	    break;
	  case CC_EMBEDDED:
	    printf(", cc embedded");
	    break;
	  case CC_FORTRAN:
	    printf(", cc fortran");
	    break;
	  default:
	    printf(", cc code '%c'", tcb.car_control);
	};
    };
    putchar('\n');
}



/*
 * Write files to a volume set.  Uses the ansi_write buffered output
 * function, which automatically chains to new tapes if it has to. 
 */

write_volume_set()
{
    int             j;

    /*
     * First, make ready to write the tape.  If this is create, then we
     * must init the tape.  If not, mount the tape, and advance to the
     * logical end-of-volume-set. 
     */

    if (func == CREATE) {
	tcb.reel_count = 0;
	strncpy(tcb.vol_name, main_vol_name, 6);
	strncpy(tcb.set_name, main_set_name, 6);
	strncpy(tcb.owner, username, 14);
	init_tape();
    } else {
	mount_tape();

	do {
	    j = ansi_open(FREAD);
	    ansi_close();
	} while (j != END_OF_SET);

	tape(MTBSF, 1);		/* back up past last tape mark */
    };

    for (j = 0; j < main_file_count; j++)
	write_file(main_file_list[j]);
    tape(MTWEOF, 2);
}

/*
 * Write from a stream to a tape.  Lots of special stuff, like record
 * and block sizes, are imported implicitly from static variables that
 * were initialized by command-line options. 
 */

write_file(fn)
    char           *fn;
{
    int             stat;
    FILE           *fp;
    struct stat     sb;		/* check on our input file */
    int             rlen;	/* length of a single record */
    FLAG            disk_file;	/* our input is from a rewind-able
				 * source */

    if (query) {
	fprintf(stderr, "r %s ? ", fn);
	if (!yes_or_no())
	    return END_OF_FILE;
    };

    if (flag_stdio)
	fp = stdin;
    else
	fp = fopen(fn, "r");
    if (fp == NULL) {
	fprintf(stderr, "can't read %s", fn);
	perror(" - ignored");
	return END_OF_FILE;
    };

    strncpy(tcb.unix_filename, tail(fn), 80);
    if (verbose)
	fprintf(stderr, "r %-17s", tcb.unix_filename);

    tcb.ibm_format = ibm_format;
    tcb.ebcdic = ebcdic;
    tcb.rec_format = main_rec_format;
    tcb.car_control = main_car_control;
    tcb.generation = 1;
    tcb.genversion = 0;
    tcb.file_access = NOPROTECT;
    tcb.blk_offset = 0;
    tcb.blk_size = main_blk_size;
    tcb.rec_size = flag_rs ? main_rec_size : main_blk_size;

    errno = 0;
    fstat(fileno(fp), &sb);
    disk_file = (errno == 0) && (sb.st_mode & S_IFMT) == S_IFREG;
    tcb.created = sb.st_ctime;

    if (!flag_rs && disk_file && main_rec_format == REC_VARIABLE) {
	if (sb.st_size < READMAX || main_rec_size == 0) {
	    tcb.rec_size = 0;
	    rlen = 4;		/* line length field */
	    while (!feof(fp)) {
		if (getc(fp) == '\n') {
		    tcb.rec_size = max(tcb.rec_size, rlen);
		    rlen = 4;	/* line length field */
		} else
		    rlen++;
	    };
	    rewind(fp);
	};
    };

    if (tcb.rec_size > tcb.blk_size) {
	fprintf(stderr, "%s recordsize (%d) is too big for blocksize - ignored\n", fn, tcb.rec_size);
	return END_OF_FILE;
    };

    ansi_open(FWRITE);
    do {
	stat = write_record(fp);
    } while (stat != END_OF_FILE);

    ansi_close();
    if (!flag_stdio)
	fclose(fp);
    if (verbose)
	fprintf(stderr, "  %8d blocks\n", tcb.blk_count);
    return END_OF_FILE;
}


/* Write a block of data */

write_record(fp)
    FILE           *fp;
{
    char           *linebuf;
    int             rl;

    linebuf = alloca(tcb.blk_size);	/* goes away on function exit */

    if (tcb.rec_format == REC_FIXED) {
	rl = fread(linebuf, sizeof(char), tcb.rec_size, fp);
	if (rl == 0)
	    return END_OF_FILE;
    } else {
	/* format is variable-length records */
	if (fgets(linebuf, tcb.rec_size, fp) == NULL)
	    return END_OF_FILE;
	rl = strlen(linebuf);
    };

    ansi_write(linebuf, rl);
    return 0;
}



/*
 * Read from a tape.  The var main_file_list is a list of files that we
 * should extract.  We apply filename wildcarding using ? and * to the
 * names.  (These must be quoted to prevent shell interpretation.)  If
 * no filenames are supplied, then we extract everything. 
 */

read_volume_set()
{
    int             i;
    FLAG            extract;

    for (; ansi_open(FREAD) != END_OF_SET;) {
	extract = main_file_count == 0;
	for (i = 0; i < main_file_count && !extract; i++) {
	    extract = (match(tcb.unix_filename, main_file_list[i]));
	};

	if (extract) {
	    if (query) {
		fprintf(stderr, "x  %s ? ", tcb.unix_filename);
		if (!yes_or_no())
		    continue;
	    };
	    if (verbose)
		fprintf(stderr, "x  %-17s", tcb.unix_filename);
	    read_file();
	};

	ansi_close();
	scan_labels();
	if (extract && verbose)
	    fprintf(stderr, "  %8d blocks\n", tcb.blk_count);
    };
}


read_file()
{
    int             max_reclen,
                    reclen;
    char           *record;
    FILE           *fp;

    record = malloc(max_reclen = tcb.rec_size + 20);
    /* a little extra room */

    if (!flag_stdio)
	fp = fopen(tcb.unix_filename, "w");
    else
	fp = stdout;

    while ((reclen = ansi_read(record, max_reclen)) != NULL) {
	fwrite(record, sizeof(char), reclen, fp);
    };
    if (!flag_stdio)
	fclose(fp);
}



/***************************************************
 *                                                 *
 *   A N S I   S U P P O R T   R O U T I N E S     *
 *                                                 *
 ***************************************************/


/* ANSI_OPEN.  Open up a tape file, and prepare to read it. */

ansi_open(mode)
    int             mode;
{
    int             err;

    err = 0;
    if (!tcb.mounted) {
	if (mode == FREAD)
	    mount_tape();
	else
	    init_tape();
    };

    if (mode == FREAD) {
	/* We are going to read a tape. */
	err = read_labels();
	if (err != TOP_OF_FILE)
	    return err;
	scan_labels();
    } else {
	/* We are writing a tape. */
	tcb.reel_switch = FALSE;
	tcb.file_count++;
	tcb.blk_count = 0;

	make_labels(TOP_OF_FILE);
	write_labels();
	err = TOP_OF_FILE;
    };

    if (err != UNKNOWN_LABEL) {
	tcb.open = TRUE;
	tcb.eof = FALSE;
	tcb.rw_mode = mode;
	tcb.blk_count = 0;
	tcb.next = 0;
	tcb.char_count = 0;
    };

    return err;
}


/*
 * ANSI_READ.  Read in a record from an buffered, open tape file. We
 * assume that nothing perverted has happened, like the recordsize
 * being longer than the buffersize. We do automatic reel switching if
 * needed. Return is number of charcters read, or END_OF_TAPE. 
 */

ansi_read(bufp, bufl)
    char            bufp[];
int             bufl;
{
    char            rl[5];
    int             read_min,
                    read_count;

    if (tcb.eof || !tcb.open)
	return NULL;

    read_min = (tcb.rec_format == REC_VARIABLE) ? 4
	: tcb.rec_size;

    if (tcb.next + read_min > tcb.char_count) {
	while (tcb.next < tcb.char_count)
	    ansi_getc();
    };

    if (tcb.rec_format == REC_VARIABLE) {
	do {
	    rl[0] = ansi_getc();
	} while (!isdigit(rl[0]) && !tcb.eof);
	rl[1] = ansi_getc();
	rl[2] = ansi_getc();
	rl[3] = ansi_getc();
	rl[4] = '\0';
	read_min = atoi(rl) - 4;
    };

    if (tcb.eof)
	return NULL;

    read_count = 0;
    for (; bufl && read_count < read_min && !tcb.eof;) {
	*(bufp++) = ansi_getc();
	bufl--, read_count++;
    };

    if (bufl && tcb.car_control == CC_IMPLIED) {
	*(bufp++) = '\n';
	bufl--, read_count++;
    };
    if (bufl)
	*bufp = '\0';

    return read_count;
}


/* Read a single character from the tape buffer. */

ansi_getc()
{
    int             err;

try:
    if (tcb.eof)
	return END_OF_FILE;

    if (tcb.buffer && tcb.open) {
	if (tcb.char_count <= tcb.next) {
	    tcb.next = 0;
	    tcb.char_count = read_tape(tcb.buffer, tcb.blk_size);
	    if (tcb.char_count == 0)
		goto do_eof;
	};
	return tcb.buffer[tcb.next++];
    };

    if (!tcb.buffer) {
	tcb.buffer = malloc(tcb.blk_size);
	tcb.next = 0;
	goto try;
    };

do_eof:
    tcb.eof = TRUE;
    tcb.open = FALSE;
    err = read_labels();
    if (err == END_OF_TAPE) {
	next_volume();
	tcb.eof = FALSE;
	tcb.open = TRUE;
    };
    goto try;
}


/*
 * ANSI_WRITE.  Write a record, given a pointer and a length. We do all
 * the formatting necessary.  Flush the tape buffer if we run off the
 * end, and start a new one. 
 */

ansi_write(rec, len)
    char            rec[];
int             len;
{
    int             overhead = 0;
    char            scratch[10];

    if (!tcb.buffer) {
	tcb.buffer = malloc(tcb.blk_size);
	tcb.next = 0;
	tcb.char_count = tcb.blk_size;
	fill(tcb.buffer, tcb.blk_size, '^');
    };

    if (tcb.car_control == CC_IMPLIED && len > 0 && iscntrl(rec[len - 1]))
	rec[--len] = '\0';

    /* Check to see if we will run off the end of the block. */
    overhead = (tcb.rec_format == REC_VARIABLE) ? 4 : 0;
    if (len + overhead + tcb.next > tcb.blk_size) {

	/* Write the old tape block. */
	write_tape(tcb.buffer, tcb.blk_size);
	tcb.blk_count++;

	/* Check for physical end of tape. */
	if (eot()) {
	    make_labels(END_OF_TAPE);
	    write_labels();
	    next_volume();
	};

	/* Initialize a new tape block. */
	fill(tcb.buffer, tcb.blk_size, '^');
	tcb.next = tcb.blk_offset;
    };

    if (tcb.rec_format == REC_VARIABLE) {
	sprintf(scratch, "%04d", len + 4);
	strncpy(&tcb.buffer[tcb.next], scratch, 4);
	tcb.next += 4;
    };

    bcopy(rec, &tcb.buffer[tcb.next], len);
    tcb.next += len;

    return 0;
}


/*
 * ANSI_CLOSE.  Close out a file.  We may not have read all the data in
 * the file yet.  If that's so, we skip over the intervening data.
 * Remember that we may have to switch reels. 
 */

ansi_close()
{
    int             err;

    if (!tcb.open)
	return 0;

    if (tcb.rw_mode == FWRITE) {
	if (tcb.next > tcb.blk_offset) {
	    write_tape(tcb.buffer, tcb.next);
	    tcb.next = tcb.blk_offset;
	    tcb.blk_count++;
	};
	tape(MTWEOF, 1);
	make_labels(END_OF_FILE);
	write_labels();
	goto closed;
    };

    for (;;) {
	if (!tcb.eof)
	    skip_past_marks(1);
	err = read_labels();
	if (err == END_OF_FILE)
	    break;
	next_volume();
	tcb.eof = 0;
    };
    scan_labels();

closed:
    tcb.eof = tcb.open = FALSE;
    if (tcb.buffer)
	free(tcb.buffer);
    tcb.buffer = 0;
    tcb.char_count = tcb.next = 0;
    return 0;
}



/*
 * Support routines. 
 */


next_volume()
{
    tape(MTOFFL, 1);
    fprintf(stderr, "Mount continuation tape and push return.  ");
    yes_or_no();
    if (func == LIST || func == EXTRACT) {
	mount_tape();
	read_labels();		/* skip the file continuation HDRn
				 * records */
    } else {
	init_tape();
	ansi_open(tcb.rw_mode);
    };
}


/*
 * Write a volume header record.  We only write VOL1, since VAX/VMS
 * does not use the VOL2 record. 
 */

init_tape()
{
    char            scratch[90];

    tcb.ebcdic = ebcdic;
    tcb.ibm_format = ibm_format;
    tcb.ansi_level = 3;
    strncpy(tcb.tapesys, SYSID, 13);

    fill(scratch, 90, ' ');
    sprintf(scratch, "VOL1%-6.6s%c",
	    tcb.vol_name,	/* volume name */
	    NOPROTECT		/* access protection */
	);

    if (tcb.ibm_format)
	sprintf(&scratch[41], "%-10.10s%29c",
		tcb.owner,	/* owner of volume set */
		IGN		/* ignored 3 and ansi_level */
	    );
    else
	sprintf(&scratch[37], "%-14.14s%28c%1d",
		tcb.owner,	/* owner of volume set */
		IGN,		/* ignored 3 */
		tcb.ansi_level	/* ansi label standard level */
	    );

    fillnull(scratch, ' ', sizeof(vol1));
    upstring(scratch, (char *) &vol1, sizeof(vol1));
    write_tape((char *) &vol1, sizeof(vol1));

    tcb.reel_count++;
    tcb.file_count = 0;
    tcb.mounted = TRUE;

    if (verbose)
	fprintf(stderr, "volume %-6s initialized\n", tcb.vol_name);
}


/* Read a volume header.  Save the volume name for later. */

mount_tape()
{
    tcb.ebcdic = ebcdic;
    tcb.ibm_format = ibm_format;

    read_tape((char *) &vol1, sizeof(vol1));
    downstring((char *) &vol1, (char *) &vol1, sizeof(vol1));
    if (strncmp(vol1.header, "vol1", 4) != 0) {
	fprintf(stderr, "no VOL1 header record\n");
	exit(-1);
    };

    tcb.mounted = TRUE;
    sscanf((char *) &vol1, "vol1%6c%c",
	   tcb.vol_name,
	   &tcb.vol_access
	);

    if (tcb.ibm_format)
	sscanf(&vol1.owner[4], "%10c", tcb.owner);
    else
	sscanf(vol1.owner, "%14c%*28c%1d",
	       tcb.owner,
	       &tcb.ansi_level
	    );

    if (verbose)
	fprintf(stderr, "volume %-6s mounted\n", tcb.vol_name);
}


/*
 * Extract the information from a set of headers fetched by
 * read_labels(). 
 */

scan_labels()
{
    sscanf((char *) &hdr4, "%*3c4%63c", tcb.tape_filename2);

    sscanf((char *) &hdr1, "%*3c1%17c%6c%4d%4d%4d%2d",
	   tcb.tape_filename1,
	   tcb.set_name,
	   &tcb.reel_count,
	   &tcb.file_count,
	   &tcb.generation,
	   &tcb.genversion);
    sscanf(hdr1.created, " %5d %5d%c%6d%17c",
	   &tcb.created,
	   &tcb.expires,
	   &tcb.file_access,
	   &tcb.blk_count,
	   tcb.tapesys);

    sscanf((char *) &hdr2, "%*3c2%c%5d%5d%1d%1d%*19c%c%*15c%2d",
	   &tcb.rec_format,
	   &tcb.blk_size,
	   &tcb.rec_size,
	   &tcb.density,
	   &tcb.reel_switch,
	   &tcb.car_control,
	   &tcb.blk_offset);

    if (tcb.ibm_format && hdr2.recfmt == IBM_VARIABLE)
	tcb.rec_format = REC_VARIABLE;

    bzero(tcb.unix_filename, 80);
    sprintf(tcb.unix_filename, "%.17s%.63s",
	    tcb.tape_filename1,
	    tcb.tape_filename2);

    sscanf(tcb.unix_filename, "%80s", tcb.unix_filename);

    return 0;
}


/*
 * Write the header for a file whose particulars are in our global
 * static variables. 
 */

make_labels(type)
    int             type;
{
    char            scratch[90];
    int             namelen;

    /*
     * Split up unix_filename into two parts. First is rightmost 17
     * characters of name, second is other characters, up to 63. 
     */

    namelen = strlen(tcb.unix_filename);
    namelen = min(17, namelen);
    strncpy(tcb.tape_filename1, tcb.unix_filename, 17);
    strncpy(tcb.tape_filename2, &tcb.unix_filename[namelen], 63);

    sprintf(scratch, "%3s1%-17.17s%-6.6s%04d%04d%04d%02d",
	    labels[type],
	    tcb.tape_filename1,
	    tcb.set_name,
	    tcb.reel_count,
	    tcb.file_count,
	    tcb.generation,
	    tcb.genversion);
    sprintf(&scratch[41], " %05d %05d%1c%06d%-13.13s%7c",
	    ansi_date(tcb.created),
	    ansi_date(tcb.expires),
	    tcb.file_access,
	    tcb.blk_count,
	    tcb.tapesys,
	    IGN);
    upstring(scratch, (char *) &hdr1, sizeof(hdr1));

    sprintf(scratch, "%3s2%c%05d%05d%1d%1d%-8.8s/%-8.8s  %1c B%11c%02d%28c",
	    labels[type],
	    tcb.rec_format,
	    tcb.blk_size,
	    tcb.rec_size,
	    tcb.density,
	    tcb.reel_switch,
	    username, progname,	/* job name & job step */
	    tcb.car_control,
	    IGN,
	    tcb.blk_offset,
	    IGN);
    upstring(scratch, (char *) &hdr2, sizeof(hdr2));

    /* IBM uses different code for variable-length records. */
    if (tcb.ibm_format && tcb.rec_format == REC_VARIABLE)
	hdr2.recfmt = IBM_VARIABLE;

    if (!tcb.ibm_format && !no_hdr3) {
	sprintf(scratch, "%3s3%76c",
		labels[type],
		IGN);
	upstring(scratch, (char *) &hdr3, sizeof(hdr3));
	sprintf(scratch, "%3s4%-63.63s00%11c",
		labels[type],
		tcb.tape_filename2,
		IGN);
	upstring(scratch, (char *) &hdr4, sizeof(hdr4));
    };
}


/*
 * Write out tape headers that have already been made by make_labels(). 
 */

write_labels()
{
    upstring((char *) &hdr1, (char *) &hdr1, sizeof(hdr1));
    upstring((char *) &hdr2, (char *) &hdr2, sizeof(hdr2));
    upstring((char *) &hdr3, (char *) &hdr3, sizeof(hdr3));
    upstring((char *) &hdr4, (char *) &hdr4, sizeof(hdr4));

    write_tape((char *) &hdr1, sizeof(hdr1));
    write_tape((char *) &hdr2, sizeof(hdr2));

    if (!tcb.ibm_format && tcb.ansi_level >= 3 && !no_hdr3) {
	write_tape((char *) &hdr3, sizeof(hdr3));
	write_tape((char *) &hdr4, sizeof(hdr4));
    };

    tape(MTWEOF, 1);
    return 0;
}


/* Read a tape header.  Set it up for scan_labels(). */

read_labels()
{
    int             l;
    int             label_type;
    struct {
	char            header[3];
	char            seq;
	char            data[76];
    }               inhdr;

    fill((char *) &hdr1, sizeof(hdr1), ' ');
    fill((char *) &hdr2, sizeof(hdr2), ' ');
    fill((char *) &hdr3, sizeof(hdr3), ' ');
    fill((char *) &hdr4, sizeof(hdr4), ' ');

    l = read_tape((char *) &inhdr, sizeof(inhdr));
    if (l == 0)
	return END_OF_SET;

    for (label_type = 1; label_type <= 4; label_type++)
	if (strncmp(inhdr.header, labels[label_type], 3) == 0)
	    break;

    if (label_type > 4)
	label_type = UNKNOWN_LABEL;

    for (; l; l = read_tape((char *) &inhdr, sizeof(inhdr))) {
	if (label_type == UNKNOWN_LABEL || strncmp(inhdr.header, labels[label_type], 3) != 0)
	    continue;
	switch (inhdr.seq) {
	  case '1':
	    downstring((char *) &inhdr, (char *) &hdr1, sizeof(hdr1));
	    break;
	  case '2':
	    downstring((char *) &inhdr, (char *) &hdr2, sizeof(hdr2));
	    break;
	  case '3':
	    downstring((char *) &inhdr, (char *) &hdr3, sizeof(hdr3));
	    break;
	  case '4':
	    downstring((char *) &inhdr, (char *) &hdr4, sizeof(hdr4));
	};
    };

    return label_type;
}


/*
 * Return an integer with an ANSI date for the time of creation
 * (actually, last modification) of a file. 
 */

ansi_date(mtime)
    long            mtime;
{
    struct tm      *tm,
                   *gmtime();

    if (mtime == 0)
	mtime = time((long *) 0);
    tm = gmtime(&mtime);
    return 1000 * tm->tm_year + tm->tm_yday;
}

/*
 * Write a block of data to the tape drive.  If the tape should be
 * operated as an EBCDIC device, do the conversion. 
 */

write_tape(buffer, buflen)
    char           *buffer;
    int             buflen;
{
    int             err;

#ifdef EBCDIC
    if (tcb.ebcdic)
	to_ebcdic(buffer, buffer, buflen);
#endif

    err = write(tcb.fd, buffer, buflen);
    return err;
}


/*
 * Read a block from the tape drive.  If the tape should be operated as
 * an EBCDIC device, do the conversion. 
 */

read_tape(buffer, buflen)
    char           *buffer;
    int             buflen;
{
    int             inlen;

    inlen = read(tcb.fd, buffer, buflen);
#ifdef EBCDIC
    if (tcb.ebcdic)
	to_ascii(buffer, buffer, inlen);
#endif

    return inlen;
}


/*
 * Skip past some number of files on a tape drive. This isn't really
 * needed on anything but streaming drives. 
 */

skip_past_marks(count)
    int             count;
{
    int             i;
    static char     ignore[20];

    if (count > 1) {
	tape(MTFSF, count);
	return;
    };

    for (i = 0; i < 6; i++) {
	if (read(tcb.fd, ignore, 20) == 0)
	    return;
    };

    tape(MTFSF, 1);
}


/* Do a special operation on a tape drive. */

tape(op, count)
    int             op,
                    count;
{
    struct mtop     mt;

    mt.mt_op = op;
    mt.mt_count = count;
    ioctl(tcb.fd, MTIOCTOP, &mt);
}


/* Find the last pathname component of a pathname. */

char           *
tail(path)
    char            path[];
{
    int             ix = strlen(path);

    ix = strlen(path) - 1;
    for (; ix >= 0; --ix) {
	if (path[ix] == '/')
	    return &path[++ix];
    };

    return path;
}


/*
 * Match a string (s) against a wildcard key.  Returns 1 if they match,
 * or 0 if they don't. 
 */

FLAG
match(s, key)
    char           *s,
                   *key;
{
    if (*s == '\0') {
	switch (*key) {
	  case '\0':
	    return TRUE;

	  case '*':
	    return match(s, ++key);

	  default:
	    return FALSE;
	};
    };

    if (*key == '?')
	return match(++s, ++key);

    if (*key == '*')
	return match(s, (key + 1)) || match((s + 1), key);

    if (*s == *key)
	return match(++s, ++key);

    return FALSE;
}


/*
 * Determine if we are off the physical end of the tape.  For now, just
 * say we haven't got there yet.  There is no device-independent way to
 * know when we hit EOT, so if we do this for real, we have a program
 * that requires a pre-programmed knowledge of the tape controller in
 * use. 
 */

FLAG
eot()
{
    return FALSE;
}


/*
 * Ask the user something, and return ok if we get anything starting
 * with the letter 'y'. 
 */

yes_or_no()
{
    char            buf[256];
    static FILE    *user = NULL;

    if (user == NULL) {
	if (flag_stdio)
	    user = fopen("/dev/tty", "r");
	else
	    user = stdin;
    };

    fgets(buf, 256, user);
    return makelower(buf[0]) == 'y';
}


/* Convert a string to upper-case, for at most n characters */

char           *
upstring(from, to, len)
    char           *from,
                   *to;
    int             len;
{
    char           *to1 = to;

    for (; len--;) {
	*to = makeupper(*from);
	if (*from == '\0')
	    break;
	to++, from++;
    };

    return to1;
}

char           *
downstring(from, to, len)
    char           *from,
                   *to;
    int             len;
{
    char           *to1 = to;

    for (; len--;) {
	*to = makelower(*from);
	if (*from == '\0')
	    break;
	to++, from++;
    };

    return to1;
}


/* Fill an area with some character. */

fill(area, size, c)
    char           *area;
    int             size;
    char            c;
{
    for (; size--;)
	*(area++) = c;
}


/* Fill in nulls left by sprintf. */

fillnull(area, fill, len)
    char           *area,
                    fill;
    int             len;
{
    for (; len--; area++)
	if (*area == '\0')
	    *area = fill;
}
