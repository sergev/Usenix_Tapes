/*
 * vmswrite [ -abfinrv ] name ...
 *
 * Write a Files-11 tape.
 * Variable length records (text files) are the common case.
 * If the "-b" option is specified, then all the files will be written
 * as fixed length records.  We further require that the fixed length record
 * size be evenly divisible into the block size (2048).  Typically, the
 * only use for fixed length records will be Forth blocks.
 *
 * Modified by NCT @ MUCS
 *
 *
 * 		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *		!!					!!
 *		!! UUCP :- ...!mcvax!ukc!man.cs.ux!neil !!
 *		!! JANET:- neil@uk.ac.man.cs.ux		!!
 *		!! ARPA :- neil%uk.ac.man.cs.ux@ucl.cs  !!
 *		!!					!!
 *		!!					!!
 * 		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */


#include        <stdio.h>
#include        <ctype.h>
#include        <sys/time.h>
#include        <sys/types.h>
#include        <sys/stat.h>

#define LINELENGTH      512     /* line length of input */
#define TAPEBUFFLEN     2050    /* tape buffer length */
#define TEOF            0       /* Version 7 */
#define TERR            (-1)    /* Version 7 */
#define VARLENGTH       1       /* write variable length records */
#define FIXLENGTH       2       /* write fixed length records */

char    vol1buff[82], hdr1buff[82], hdr2buff[82];
char    line[LINELENGTH], tapebuff[TAPEBUFFLEN];

char    *usefile;
char    deftape[] = "0";                /* default is /dev/rmt0 & /dev/nrmt0 */
char    tapename[12]  = "/dev/nrmt";    /* default output device */
char    tapername[12] = "/dev/rmt";     /* tape name for rewinding at end */
                                        /* suffixes are added later */

char    vollabel[]      = "BSD4.3";
char    access[]        = " ";          /* DEC file security */
char    volowner[]      = "D%B4444300300"; /* written on PDP-11, all access */
char    credate[]       = "81001";
char    expdate[]       = "91001";
char    syscode[]       = "DECFILE11";  /* written on PDP-11 */
char    recformat[]     = "D";          /* variable length records */
char    formcontrol[]   = " ";          /* lf,cr to be inserted after */

int     rformat = VARLENGTH;
int     rlength = 0;                    /* record length for fixed length records */
int     tapefd;
int     append = 0;
int     norewind = 0;
int     verbose = 0;
char    *pname;

int     filenum;                /* file # on tape */
int     blklength = 2048;       /* #bytes of data per tape buffer */
int     bufflen;                /* #bytes of data in tape buffer */
char    *buffptr;               /* pointer to next byte to store in tape buff */
int     blkcount;               /* count of data records per tape file */

main(argc, argv)
int   argc;
char  *argv[];
{
        FILE   *fp, *fopen();
        char   *s;
        int    i;

        usefile = deftape;
        pname = argv[0];
        while ((--argc > 0) && ((*++argv)[0] == '-'))
                for (s = argv[0]+1; *s != '\0'; s++)
                        switch (*s) {
                        case 'a':               /* append to tape, no vol1 */
                                append = 1;
                                break;

                        case 'b':               /* next arg is tape block length */
                                if (--argc <= 0)
                                   error("-b option requires another argument", NULL);
                                blklength = atoi(*++argv);
                                if (blklength < 0)
                                        error("invalid block length with -b option", NULL);
                                break;

                        case 'f':               /* next arg is tape name */
                                if (--argc <= 0)
                                   error("-f option requires another argument", NULL);
                                usefile = *++argv;
                                break;

                        case 'n':
                                norewind = 1;
                                break;

                        case 'r':               /* next arg is fixed length record size */
                                if (--argc <= 0)
                                   error("-r option requires another argument", NULL);
                                rlength = atoi(*++argv);
                                if (rlength < 0)
                                   error("invalid record length with -r option", NULL);
                                rformat = FIXLENGTH;
                                break;

                        case 'v':
                                verbose = 1;    /* verbose output */
                                break;

                        default:
                                error("illegal option %c", *s);
                        }

        if (rformat == FIXLENGTH) {
                if (rlength > blklength)
                        error("invalid record/block length", NULL);
                if (blklength % rlength)
                        error("block length must be a multiple of the record length", NULL);
        }

        nametape();
        if (append)
                filenum = findleot() / 3;
        else    
                filenum = 0;

        if (argc <= 0)
                error("filename argument(s) required", NULL);
        if ((tapefd = open(tapename, 1)) < 0)
                error("can't open %s", tapename);
        i = 0;
        do {
                if ((argc > 0) && (fp = fopen(argv[i], "r")) == NULL) {
                        fprintf(stderr, "%s: can't open %s\n", pname, argv[i]);
                        continue;
                }
                if ((filenum++ == 0) && (!append))
                        vol1();
                hdr1(fp, argv[i]);
                hdr2();
                tapemark();
                (rformat == VARLENGTH) ? vardata(fp) : fixdata(fp);
                tapemark();
                eof1();
                eof2();
                tapemark();
                fclose(fp);
        } while (++i < argc);
        if (!norewind)
                rewind();
}

vol1()
{
        char *ptr;

        ptr = vol1buff;
        sprintf(ptr, "VOL1%-6.6s%1s%26s%-13.13s %28s1",
                vollabel, access, " ", volowner, " ");

        if (write(tapefd, vol1buff, 80) != 80)
                error("error writing VOL1", NULL);
}

hdr1(fp, filename)
FILE  *fp;
char  *filename;
{
        char            *ptr;
        int             filesectnum, gennum, genver;
        long            cretime;
        struct stat     statbuff;
        struct tm       *tmptr, *localtime();

        if (fstat(fileno(fp), &(statbuff)) < 0)
                error("error from fstat", NULL); /* get info on the file */
        cretime = statbuff.st_ctime;            /* get creation time */
        tmptr = localtime(&cretime);
        sprintf(credate, "%02d%03d", tmptr->tm_year, (tmptr->tm_yday)+1);
        strcpy(expdate, credate);               /* set exp date = cre date */

        filesectnum = 1;        /* we only write single volume tapes */
        blkcount    = 0;        /* start counter of data blocks in this file */
        gennum      = 1;        /* generation # */
        genver      = 0;        /* generation version */
                                /* above two lines give version number of 1 */
        rmprefix(filename);     /* remove any directory prefixes */
        upper(filename);        /* convert filename to upper case */
        if (verbose)
                fprintf(stderr, "%s\n", filename);

        ptr = hdr1buff;
        sprintf(ptr, "HDR1%-17.17s%-6.6s%04d%04d%04d%02d%6s%6s%1s%06d%-13.13s%7s",
                filename, vollabel, filesectnum, filenum,
                gennum, genver, credate, expdate,
                access, blkcount, syscode, " ");

        if (write(tapefd, hdr1buff, 80) != 80)
                error("error writing HDR1", NULL);
}

hdr2()          /* write the header 2 record */
{
        char *ptr;
        int  reclength, buffoffset;

        if (rformat == VARLENGTH) {
                reclength = 512;
                recformat[0] = 'D';
                formcontrol[0] = ' ';
        } else {
                reclength = rlength;
                recformat[0] = 'F';
                formcontrol[0] = 'M';
        }
        buffoffset = 0;
        ptr = hdr2buff;
        sprintf(ptr, "HDR2%s%05d%05d%21s%1s%13s%02d%28s",
                recformat, blklength, reclength, " ",
                formcontrol, " ", buffoffset, " ");

        if (write(tapefd, hdr2buff, 80) != 80)
                error("error writing HDR2", NULL);
}

eof1()
{
        strncpy(hdr1buff, "EOF1", 4);
        sprintf(hdr1buff+54, "%06d%-13.13s%7s", blkcount,syscode," ");

        if (write(tapefd, hdr1buff, 80) != 80)
                error("error writing EOF1", NULL);
}

eof2()
{
        strncpy(hdr2buff, "EOF2", 4);

        if (write(tapefd, hdr2buff, 80) != 80)
                error("error writing EOF2", NULL);

}

vardata(fp)     /* write an entire file of variable length records to tape */
FILE *fp;
{
        char     *fgets();
        register int  length;

        buffptr = &tapebuff[0];
        bufflen = 0;
        while (fgets(line, LINELENGTH, fp) != NULL) {
                length = strlen(line) + 3;
                                /* -1 for newline at end */
                                /* +4 for our four count bytes at front */
                if ((bufflen + length) > blklength)
                        writebuff(blklength);

                sprintf(buffptr, "%04d", length);
                buffptr += 4;
                strncpy(buffptr, line, length-4);
                buffptr += (length - 4);
                bufflen += length;
        }
        writebuff(blklength);
}

fixdata(fp)             /* write a file of fixed length records to tape */
FILE  *fp;
{
        int     fd, nperblock, recnum, nbytes;

        recnum = 0;
        fd = fileno(fp);        /* avoid the stdio library for binary data */
        nperblock = blklength / rlength;
        buffptr = &(tapebuff[0]);
        bufflen = 0;

        while (1) {
                nbytes = read(fd, buffptr, rlength);
                if (nbytes == 0)
                        break;          /* end of file */
                if (nbytes < 0)
                        error("fixdata: read error", NULL);
                if (nbytes != rlength)
                        error("fixdata: didn't read enough", NULL);
                buffptr += rlength;
                bufflen += rlength;
                if ((++recnum % nperblock) == 0)
                        writebuff(blklength);
        }
        if (bufflen > 0)
                writebuff(bufflen);
                /* Final record of fixed length data may be smaller than
                   the block length. */
}
        
writebuff(reclen)
int  reclen;    /* record length to write */
{
        register int  i, fillchar;

        if (bufflen <= 0)
                return;

        fillchar = (rformat == VARLENGTH) ? '^' : '\0';
        for (i = bufflen; i < reclen; i++)
                *buffptr++ = fillchar;

        if (write(tapefd, tapebuff, reclen) != reclen)
                error("error writing data record", NULL);

        buffptr = &tapebuff[0];
        bufflen = 0;
        blkcount++;
}

tapemark()
{
        if (close(tapefd) < 0)
                error("error closing tape", NULL);
        if ((tapefd = open(tapename, 1)) < 0)
                error("error opening tape after close", NULL);
}

rewind()                /* rewind the tape */
{
        if (close(tapefd) < 0)
                error("error closing tape", NULL);
        if ((tapefd = open(tapername, 0)) < 0)
                error("error opening %s", NULL);
        close(tapefd);
}

error(str1, str2)
char *str1, *str2;
{
        fprintf(stderr, "%s: ", pname);
        fprintf(stderr, str1, str2);
        fprintf(stderr, "\n");
        exit(1);
}

upper(str)
char *str;
{
        for ( ; *str != '\0'; str++)
                if (islower(*str))
                        *str = toupper(*str);
}

rmprefix(filename)
char    *filename;
{
        register char  *ptr;

        if ((ptr = rindex(filename, '/')) == 0)
                return;

        strcpy(filename, ++ptr);
}

findleot()              /* find the logical end-of-tape */
{
        register int  end, nfile, nread;
        int           numfiles;

        /* first rewind the tape, just in case */
        if ((tapefd = open(tapername, 0)) < 0)
                error("can't open %s", tapername);
        if (close(tapefd) < 0)
                error("error closing %s", tapername);
        if ((tapefd = open(tapername, 0)) < 0)
                error("can't open %s", tapername);

        /* now read up to double eof, counting files */
        end   = 0;
        nfile = 1;
        while (end != 2) {              /* stop on double eof */
                if ((nread = read(tapefd, tapebuff, TAPEBUFFLEN)) == TEOF) {
                        nfile++;
                        end++;
                } else if (nread == TERR)
                        fprintf(stderr, "read error; file: %4d\n", nfile);
                else
                        end = 0;
        }

        if (close(tapefd) < 0)
                error("error closing %s", tapername);
        if ((tapefd = open(tapename, 0)) < 0)
                error("can't open %s", tapename);

        /* now read up to, but not over, the double eof */
        numfiles = nfile - 1;           /* number of filemarks to cross */
        nfile = 1;
        end   = 0;
        for (;;) {
                if ((nread = read(tapefd, tapebuff, TAPEBUFFLEN)) == TEOF) {
                        if (++nfile == numfiles)
                                break;
                        end++;
                } else if (nread == TERR)
                        fprintf(stderr, "read error; file: %4d\n", nfile);
                else
                        end = 0;
        }
        close(tapefd);
        return(numfiles);
}

nametape()      /* fix the tape device names by appending the numeric
                   characters to the end */
{
        register char  *ptr, c;

        ptr = usefile;
        while (c = *ptr++)
                if (isdigit(c))
                        goto found;
        error("illegal tape device name", NULL);

found:  --ptr;
        strcat(tapename, ptr);
        strcat(tapername, ptr);
}
