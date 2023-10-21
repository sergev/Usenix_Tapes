
/*
 * This file contains the definitions of the tape control records. This
 * is to be used when developing a program to work with ANSI standard
 * labelled tapes. 
 */

/*
 * Actually, these should be upper case.  The ansitape program doesn't
 * convert things to upper case until the headers are made -- the tape
 * control block has everything in lower case.  For the sake of easy
 * comparisons, we define them here as lower case. 
 */

#define NOPROTECT ' '		/* space if no protection */
#define REC_FIXED 'f'		/* Records are all fixed length */
#define REC_VARIABLE 'd'	/* Records are preceeded by a 4-digit
				 * zero-filled count of their length,
				 * including the 4 digits themselves */
#define IBM_VARIABLE 'v'	/* IBM's version of ANSI D format */
#define CC_FORTRAN 'a'		/* First char of rec is f-77 carriage
				 * control */
#define CC_EMBEDDED 'm'		/* Carriage control is part of the data
				 * contained in each record */
#define CC_IMPLIED ' '		/* Each record goes on a separate line
				 * when printed */

/*
 * VOLUME 1 Describes the owner of and access to the volume. 
 */

struct ansi_vol1 {
    char            header[4];	/* VOL1 */
    char            label[6];	/* volume name, A-Z0-9 */
    char            access;	/* access code */
    char            __ignored1[20];	/* reserved, space fill */
    char            __ignored2[6];	/* reserved, space fill */
    char            owner[14];	/* user name */
    char            __ignored3[28];	/* reserved, space fill */
    char            ansi_level;	/* ansi standard level, always 3 */
};

/*
 * VOLUME 2 optional, ignored by this software. 
 */

/*
 * HEADER 1 Begins basic description of a tape file. 
 */

struct ansi_hdr1 {
    char            header[4];	/* HDR1 */
    char            name[17];	/* rightmost 17 characters of filename */
    char            fileset[6];	/* should be same as volume id of 1st
				 * tape */
    char            volume_num[4];	/* number of volume in set */
    char            file_num[4];/* number of file on tape */
    char            gen[4];	/* generation number */
    char            genver[2];	/* generation version number */
    char            created[6];	/* date of creation, "bYYDDD" */
    char            expires[6];	/* expiration date "bYYDDD" */
    char            access;	/* protection, space = unprotected */
    char            blockcount[6];	/* zero in header */
    char            tapesys[13];/* name of software creating tape */
    char            __ignored[7];	/* reserved, space fill */
};

/*
 * HEADER 2 Record format, record size, block size. 
 */

struct ansi_hdr2 {
    char            header[4];	/* HDR2 */
    char            recfmt;	/* record format */
    char            blocklen[5];/* file tape block length */
    char            reclen[5];	/* file logical record length */
    char            density;	/* density/recording mode. Should be 2
				 * for 800 bpi, 3 for 1600 bpi. */
    char            vol_switch;	/* 0, or 1 if this file was continued */
    char            job[17];	/* job name (8), /, job step (8) of
				 * creator */
    char            recording[2];	/* parity/conversion, space
					 * fill */
    char            carriage_control;
    char            alignment;	/* reserved, space fill */
    char            blocked_records;	/* 'B' if records are blocked */
    char            __ignored2[11];
    char            block_offset[2];	/* ingore 1st n char of each
					 * block */
    char            __ignored3[28];
};

/*
 * HEADER 3 This contains a bunch of rms-dependent data.  We will
 * ignore it for now, and hope that it doesnt screw up VMS too badly. 
 */

struct ansi_hdr3 {
    char            header[4];	/* HDR3 */
    char            os_reserved[76];	/* reserved to OS, space fill */
};

/*
 * HEADER 4 This is the rest of the filename if its longer than 17
 * characters.  We should have no trouble fitting UNIX filenames into
 * this. 
 */

struct ansi_hdr4 {
    char            header[4];	/* HDR4 */
    char            name2[63];	/* leftmost char, if name is more than
				 * 17 characters long. */
    char            unknown[2];	/* fill with 00 */
    char            __ignored[11];
};


/*
 * FORMAT NOTES 
 *
 * 1.  The tape always starts with a VOL1 record.  Other volume records
 * are optional.  We ignore them, and do not generate them on output. 
 *
 * 2.  Each file starts with a set of HDRn records.  The headers are
 * followed by a tape mark. 
 *
 * 3.  The data is written to the tape in fixed-length blocks.  Valid ANSI
 * block lengths are 18 to 2048 bytes, although VMS will support longer
 * blocks. The block does not have to be full.  If the block is not
 * completely filled, the remainder of the block is padded with
 * circumflex (^) characters.  The file is followed by a tape mark. 
 *
 * 4.  Trailing records are written.  Either EOF or EOV records may be
 * used. The record formats are the same as the HDR records, expect: 
 *
 * a. the "HDR" is replaced by "EOF" or "EOV". 
 *
 * b. the blockcount field in EOF1/EOV1 contains the actual number of
 * physical tape blocks written in the preceding file section.  In the
 * HDR1 record, the blockcount is 000000. 
 *
 * 5.  A tape file may extend across a volume boundary.  If it does, EOV
 * labels are used, indicating that another tape must be read.  If the
 * file ends here, an EOF label is used. 
 *
 * 6.  After the EOF records, a tape mark is written.  This separates the
 * trailers from the headers for the next file.  At the end of the
 * tape, two tape marks are written. 
 */
