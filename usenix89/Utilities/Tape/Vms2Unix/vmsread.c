/*
 * vmsread  [ -adfitv ]  [ name ... ]
 *
 * Read a Files-11 tape from RSX or VMS.
 * Presently we handle multifile volumes only.
 * Variable length records are read and written to the
 * file named in the HDR1 label.
 * Fixed length records are also handled correctly.
 *
 * NCT @ MUCS
 * 6-SEPT-84    Added -D flag for debugging
 * 6-SEPT-84    Fixed dodata to handle FIXLENGTH type records,
 *                              previously this mode generated empty file names.
 * 6-SEPT-84    Fix up name clashes
 * Modified by CCK
 * 12-NOV-85	Fixed problem connected with trailing ^ characters in a block.
 *			These were always removed - even when not filler !
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

#define MAXBUF          8192
#define MAXLINE         1024
#define MAXRECLEN       1024
#define VARLENGTH       1
#define FIXLENGTH       2
#define TEOF            0               /* Version 7 */

char    bufin[MAXBUF], bufout[MAXBUF], fname[17], fnamesav[17],
        ftype[4];
char    **argvsave;
char    *pname;
int     tapefd, fdout, rformat, length, procfile, reclength,
        numfnames, bufiptr, bufoptr;
int     crlf = 0;       /* if true, terminate each line with cr/lf */

int     verbose = 0;    /* verbose output */
int     procall = 1;    /* process all files on tape */
int     versionf = 0;   /* append version# to filename */
int     tableofc = 0;   /* table of contents (directory listing) only */
int     prvolid = 0;    /* print volume id as first line out */
int     select = 0;     /* remaining args are filenames */
int     allextents = 0; /* process all file types */
int     afrome = 0;     /* convert to ascii from ebcdic */
int debug = 0;          /* debug mode on */
int nctmode = 0;        /* fix up name clashes */

char    *usefile;
char    tapename[] = "/dev/rmt0";       /* default tape device */

main(argc, argv)
int     argc;
char    *argv[];
{
        char    *s;

        pname = argv[0];
        usefile = tapename;
        while (--argc > 0  &&  (*++argv)[0] == '-')
                for (s = argv[0]+1; *s != '\0'; s++)
                        switch (*s) {
                        case 'c':       /* complete filename, append version# */
                                versionf = 1;
                                break;

                        case 'e':       /* process all file extensions */
                                allextents = 1;
                                break;

                        case 'f':       /* use next argument as filename */
                                if (--argc <= 0)
                                   error("-f option requires another argument", NULL);
                                usefile = *++argv;
                                break;

                        case 'i':       /* print volume id */
                                prvolid = 1;
                                break;

                        case 't':       /* produce table of contents (directory listing) only */
                                tableofc = 1;
                                procall = select = 0;
                                break;

                        case 'v':       /* verbose option */
                                verbose = 1;
                                break;

                        case 'a':       /* convert to ascii */
                                afrome = 1;
                                break;

                        case 'D':       /* Debug on */
                                debug = 1;
                                break;

                        case 'n':       /* fix name clashes */
                                nctmode = 1;
                                break;

                        default:
                                error("illegal option %c", *s);
                        }

        if (argc > 0)  {        /* additional arguments are names of files
                                   on the tape to process */
                if (tableofc)
                        error("can't specify filename arguments with -t option", NULL);
                procall = 0;
                select = 1;     
                numfnames = argc;
                argvsave = argv;
        }

        if ((tapefd = open(usefile, 0)) == -1)
                error("can't open %8s", usefile);
        length = read(tapefd, bufin, MAXBUF);
        if (afrome) ascii(bufin,length) ; /* convert to ascii */
        if (length == TEOF)
                error("unexpected eof on tape before VOL1", NULL);
        if (length != 80  ||  strcmp("VOL1", bufin))
                error("missing VOL1", NULL);
        vol1();

        while ((length = read(tapefd, bufin, MAXBUF)) != TEOF)  {
                if (afrome) ascii(bufin,length) ;
                if (length != 80  ||  strcmp("HDR1", bufin))
                        error("missing HDR1", NULL);
                hdr1();

                if ((length = read(tapefd, bufin, MAXBUF)) == TEOF)
                        error("unexpected eof on tape after HDR1", NULL);
                if (afrome) ascii(bufin,length) ;
                if (length != 80  ||  strcmp("HDR2", bufin))
                        error("missing HDR2", NULL);
                hdr2();

                while ((length = read(tapefd, bufin, MAXBUF)) != TEOF)
                        ;  /* skip over any other HDRn labels, up to
                              and over end-of-file */

                if (procfile)  {  /* process this file */
                        bufiptr = MAXBUF + 1;
                        dodata();
                } else
                        length = read(tapefd, bufin, MAXBUF);
                        /* skip into data portion */

                if (length != TEOF)
                        while ((length = read(tapefd, bufin, MAXBUF)) != TEOF)
                                ;  /* skip over any unprocessed data
                                      records, up to and over the eof
                                      following the data records */

                if ((length = read(tapefd, bufin, MAXBUF)) == TEOF)
                        error ("unexpected eof on tape before EOF1", NULL);
                if (afrome) ascii(bufin,length) ;
                if (length != 80  ||  strcmp("EOF1", bufin))
                        error("missing EOF1", NULL);
                eof1();

                if ((length = read(tapefd, bufin, MAXBUF)) == TEOF)
                        error("unexpected eof on tape after EOF1", NULL);
                if (afrome) ascii(bufin,length) ;
                if (length != 80  ||  strcmp("EOF2", bufin))
                        error("missing EOF2", NULL);
                eof2();

                while((length = read(tapefd, bufin, MAXBUF)) != TEOF)
                        ;  /* skip over any additional EOFn labels,
                              up to and over the eof preceding the
                              next HDR1 label */
        }
        exit(0);
}

error(str1, str2)       /* print error message and die */
char    *str1, *str2;
{
        fprintf(stderr, "%s: ", pname);
        fprintf(stderr, str1, str2);
        fprintf(stderr, "\n");
}

strcpy(str1, str2)      /* copy str2 to str1 */
register char   *str1, *str2;
{
        while (*str1++ = *str2++)
                ;
}

strcmp(str1, str2)      /* compare two character strings.
                           return <0  if str1 < str2
                                   0  if str1 = str2
                                  >0  if str1 > str2
                           str1 must be zero terminated, str2 need not
                           be zero terminated.  If str1 is a null string
                           we return 1 as they are not equal */
register char   *str1, *str2;
{
        if (*str1 == '\0')
                return(1);

        for ( ; *str1 == *str2;  str1++, str2++)
                if (*str1 == '\0')
                        return(0);
        if (*str1 == '\0')
                return(0);
        return(*str1 - *str2);
}

namecmp(str)    /* Compare the filename pointed to by str with
                   the list of filenames specified by the -f option.
                   Return 1 if file is to be processed,
                          0  if file is not to be processed */
register char   *str;
{
        register char   **argv;
        register int    i;

        for (i = 1, argv = argvsave; i <= numfnames; i++, argv++)
                if (strcmp(*argv, str) == 0)
                        return(1);
        return(0);
}

typecmp(str)    /* Compare the filename type in str with our list
                   of file type to be ignored.  Return 0 if the
                   file is to be ignored, return 1 if the
                   file is not in our list and should not be ignored. */
register char   *str;
{
        static char *type[] = {
                "exe",          /* vms executable image */
                "lib",          /* vms object library */
                "obj",          /* rsx object file */
                "odl",          /* rsx overlay description file */
                "olb",          /* rsx object library */
                "pmd",          /* rsx post mortem dump */
                "stb",          /* rsx symbol table */
                "sys",          /* rsx bootable system image */
                "tsk",          /* rsx executable image */
                ""              /* null string terminates list */
        };
        register int    i;

        i = -1;
        while (*type[++i])
                if (strcmp(str, type[i]) == 0)
                        return(0);      /* found a match, file to be ignored */
        return(1);                      /* no match found */
}

lower(str)      /* convert any upper case alphabetics in str to lower */
register char   *str;
{
        for ( ; *str != '\0'; str++)
                if (isupper(*str))
                        *str = tolower(*str);
}

vol1()          /* process VOL1 label */
{
        bufin[10] = '\0';
        if (prvolid)
                printf("volume label: %6s\n", &bufin[4]);
}

hdr1()          /* process HDR1 label */
{
        register int    in, out, count;
        int             genver, gennum, version, digit, sig, typeptr;
        char            temp;

        out = 0;
        /* copy up to period or 9 characters */
        for (in = 4; in <= 12; in++)
                if (bufin[in] == '.')
                        break;
                else
                        if (bufin[in] != ' ')
                                fname[out++] = bufin[in];

        fname[out++] = '.';
        if (bufin[in] == '.')
                in++;

        typeptr = 0;
        /* copy up to space or 3 character file type */
        for (count = 1;  count <= 3;  count++)
                if (bufin[in] == ' ')
                        break;
                else  {
                        fname[out++] = bufin[in];
                        ftype[typeptr++] = bufin[in++];
                }
        
        ftype[typeptr] = '\0';  /* null terminte file type */
        lower(ftype);           /* convert file type to lower case */
        fname[out] = '\0';      /* null terminate filename */
        lower(fname);           /* convert filename to lower case */

        if (versionf || (access(fname, 6) != -1))  {    /* process version# */
                temp = bufin[39];
                bufin[39] = '\0';
                gennum = atoi(&bufin[35]);
                bufin[39] = temp;
                temp = bufin[41];
                bufin[41] = '\0';
                genver = atoi(&bufin[39]);
                bufin[41] = temp;
                version = (gennum - 1)*64 + genver + 1;
                if (!nctmode) fname[out++] = ':';       /* no : if resolving clashes */
                sig = 0;
                digit = version / 4096;
                if (digit)  {
                        fname[out++] = digit + '0';
                        sig = 1;
                }
                digit = (version / 512) % 8;
                if (digit || sig)  {
                        fname[out++] = digit + '0';
                        sig = 1;
                }
                digit = (version / 64) % 8;
                if (digit || sig)  {
                        fname[out++] = digit + '0';
                        sig = 1;
                }
                digit = (version / 8) % 8;
                if (digit || sig)
                        fname[out++] = digit + '0';
                digit = version % 8;
                fname[out++] = digit + '0';
                fname[out] = '\0';      /* null terminate filename */
                if (nctmode)
                /* resolve name clashes */
                {
                        fname[out+1] = '\0';
                        do
                        {
                                count = open(fname,1);
                                if (count >= 0)
                                /* file exists */
                                {
                                        close(count);
                                        if (fname[out]>= 'A' && fname[out] < 'Z')
                                                fname[out]++;
                                        else
                                        if (fname[out] == 'Z')
                                        {
                                                printf("Unresolvable ");
                                                break;
                                        }
                                        else fname[out] = 'A';
                                }
                        } while (count != -1);
                        if (!fname[out]) printf("Name clash ");
                }
        }

        procfile = procall;
        if (select)
                /* if we are selecting files by name, compare this name
                   with the list specified by operator */
                procfile = namecmp(fname);

        else if (procall & (!allextents))
                /* if we are processing all files and we are to be selective
                   about the file types, then see if this type is to be
                   processed or ignored */
                procfile = typecmp(ftype);

        if ((verbose && procfile) || (tableofc))
                printf("%s\n", fname);

        if (procfile)
                if ((fdout = creat(fname, 0644)) == -1)
                        error("HDR1: error creating file %s", fname);
}

hdr2()          /* process HDR2 label */
{
        register int    temp;

        if (afrome) return;
        if (bufin[4] == 'F')  {
                rformat = FIXLENGTH;
                temp = bufin[15];
                bufin[15] = '\0';
                reclength = atoi(&bufin[10]);
                if (debug)      printf("Fixed length records size %d\n",reclength);
                bufin[15] = temp;
                if (reclength < 0  ||  reclength > MAXRECLEN)
                        error("HDR2: invalid record length", NULL);
        }
        else if (bufin[4] == 'D') {
                rformat = VARLENGTH;
                if (bufin[36] == ' ')
                        crlf = 1;       /* append cr/lf to every record */
                else if (bufin[36] == 'M')
                        crlf = 0;       /* record has all forms control */
                else if (bufin[36] == 'A')
                        crlf = 1;       /* record has Fortran carriage control,
                                           leave first char alone, add cr/lf */
                else
                        error("HDR2: unknown form control character %c", bufin[36]);
        } else
                error("HDR2: unknown record type %c", bufin[4]);

}

eof1()          /* process EOF1 label */
{
}

eof2()          /* process EOF2 label */
{
}

gbytea()        /* get next ascii byte from tape bufinfer */
{
        if (bufiptr >= length)  {  /* read next tape record */
                if ((length = read(tapefd, bufin, MAXBUF)) == TEOF)
                        return(-1);  /* eof on tape */
                bufiptr = 0;
        }

        return(bufin[bufiptr++] & 255);
}

gbyteb()        /* get next binary byte from bufinfer */
{
        if (bufiptr >= length)  {  /* read next tape record */
                if ((length = read(tapefd, bufin, MAXBUF)) == TEOF)
                        return(-1);  /* eof on tape */
                bufiptr = 0;
        }
        return(bufin[bufiptr++] & 255);
}

getcount()      /* get 4-byte ascii decimal/bin count field */
{
        char          cstr[5];
        register int  data, i;

        if (afrome)
		data = gbyteb();
	else
		while ((data = gbytea()) == '^') ;
	if (data == -1)
                return(-1);
        cstr[0] = data;

        for (i = 1;  i <= 3;  i++)  {
                if ((data = (afrome?(gbyteb()):gbytea())) == -1)
                        error("getcount: unexpected end of data", NULL);
                cstr[i] = data;
        }
        cstr[4] = '\0';
        return(afrome?(binnum(cstr)):atoi(cstr));
}

dodata()        /* process the data */
{
        register int  data, count, i;

        if (afrome) { doedata() ; return; }
        bufoptr = 0;
        if (rformat == VARLENGTH)  {
                while ((count = getcount()) != -1) {
                        /* process one line at a time */
                        count -= 4;
                        if (count < 0  ||  count >= MAXLINE)
                                error("dodata: invalid count", NULL);
                        for (i = 1;  i <= count;  i++)  {
                                if ((data = gbytea()) == -1)
                                        error("dodata: unexpected end of data", NULL);
                                else  {
                                        bufout[bufoptr++] = data;
                                        if (bufoptr >= MAXBUF)  {
                                           if (write(fdout, bufout, bufoptr) != bufoptr)
                                                error("dodata: error writing data", NULL);
                                           bufoptr = 0;
                                        }
                                }
                        }
                        if (crlf)
                                bufout[bufoptr++] = '\n'; /* terminate line */
                        if (bufoptr >= MAXBUF)  {
                           if (write(fdout, bufout, bufoptr) != bufoptr)
                              error("dodata: error writing data", NULL);
                           bufoptr = 0;
                        }
                }
        }
        else if (rformat == FIXLENGTH)  {
        /* FIXLENGTH records didn't work previously because someone forgot
         * the brackets after the while statement - this meant that one
         * generated all the correct file names but no data
         * NCT fixed this.
         */
                while ((length = read(tapefd, bufin, MAXBUF)) != TEOF)
                {
                        if (afrome) ascii(bufin,length) ;
                        if (write(fdout, bufin, length) == -1)
                                error("dodata: error writing data", NULL);
                }
        }
        else error("dodata: invalid rformat value", NULL);

        if (bufoptr)  /* any data left to flush out */
                if (write(fdout, bufout, bufoptr) != bufoptr)
                        error("dodata: error writing data", NULL);
        if (close(fdout) == -1)
                error("dodata: error closing disc file", NULL);
}
char    etoa[] = {
        0000,0001,0002,0003,0234,0011,0206,0177,
        0227,0215,0216,0013,0014,0015,0016,0017,
        0020,0021,0022,0023,0235,0205,0010,0207,
        0030,0031,0222,0217,0034,0035,0036,0037,
        0200,0201,0202,0203,0204,0012,0027,0033,
        0210,0211,0212,0213,0214,0005,0006,0007,
        0220,0221,0026,0223,0224,0225,0226,0004,
        0230,0231,0232,0233,0024,0025,0236,0032,
        0040,0240,0241,0242,0243,0244,0245,0246,
        0247,0250,0133,0056,0074,0050,0053,0041,
        0046,0251,0252,0253,0254,0255,0256,0257,
        0260,0261,0135,0044,0052,0051,0073,0136,
        0055,0057,0262,0263,0264,0265,0266,0267,
        0270,0271,0174,0054,0045,0137,0076,0077,
        0272,0273,0274,0275,0276,0277,0300,0301,
        0302,0140,0072,0043,0100,0047,0075,0042,
        0303,0141,0142,0143,0144,0145,0146,0147,
        0150,0151,0304,0305,0306,0307,0310,0311,
        0312,0152,0153,0154,0155,0156,0157,0160,
        0161,0162,0313,0314,0315,0316,0317,0320,
        0321,0176,0163,0164,0165,0166,0167,0170,
        0171,0172,0322,0323,0324,0325,0326,0327,
        0330,0331,0332,0333,0334,0335,0336,0337,
        0340,0341,0342,0343,0344,0345,0346,0347,
        0173,0101,0102,0103,0104,0105,0106,0107,
        0110,0111,0350,0351,0352,0353,0354,0355,
        0175,0112,0113,0114,0115,0116,0117,0120,
        0121,0122,0356,0357,0360,0361,0362,0363,
        0134,0237,0123,0124,0125,0126,0127,0130,
        0131,0132,0364,0365,0366,0367,0370,0371,
        0060,0061,0062,0063,0064,0065,0066,0067,
        0070,0071,0372,0373,0374,0375,0376,0377,
};
ascii(buff,length)
char *buff;
int length;
{
        register int i, c;
        for (i=0; i<length; i++) {
                c = buff[i];
                c &= 0xff;
                buff[i] = etoa[c] & 0377;
        }
}

doedata ()
{ int blocklength,reclength ;

                while ((blocklength = getcount()) != -1) {
                blocklength-=4 ;
                while (blocklength) {
                        blocklength-=(reclength=getcount());
                        if (reclength == -1) error("unexpected end of file", NULL);
                        ascii(&bufin[bufiptr],reclength-=4);
                        if ((write(fdout,&bufin[bufiptr],reclength) != reclength) || (write(fdout,"\n",1) != 1))
                                error("doedata: error writing data",NULL);
                bufiptr += reclength;
                }}
        if (close(fdout) == -1)
                error("doedata: error closing file",NULL);
}

binnum(c)
char *c;
{ register int j,k;
        j = c[0];
        j = (j & 0xff) << 8;
        k = c[1]; k &= 0xff ;
        return k+j;
}
