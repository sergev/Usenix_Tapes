/* $Header: cmstape.c,v 1.1 86/07/18 10:44:17 alan Rel $
 *
 * NAME
 *    CMSTAPE - manipulate a CMS TAPE DUMP tape.
 *
 * SYNOPSIS
 *    cmstape [-tvcrxfiud] tape [-Fnn] file...
 *
 * DESCRIPTION
 *
 *    Cmstape manipulates an IBM CMS tape.  Flags are:
 *     -t (default) type tape directory of tape file.
 *     -v verbose
 *     -c create a CMS tape dump
 *     -r append to a CMS tape dump ('r' is non-mnemonic but matches tar)
 *     -x extract files from tape.  If no names given all are extracted.
 *        otherwise only those named are extracted.  Syntax for names is: fn.ft
 *     -f tape is in readtape(1) format: each file block is preceded by
 *        a two byte block length.
 *     -i image (no translation or tacking on of newlines).  For V-format files
 *        the extracted/written file includes a halfword length preceding each
 *        record.  This makes image extracts and creates a reversible process
 *        for V as well as F format files.
 *     -u don't convert filenames to lowercase and vice-versa
 *     -d debug (multiple occurences increase volume of output)
 *
 *    Reads either from a tape device or from files that have been
 *    read off a tape by readtape(l).  That is,  files that contain
 *    a 2 byte tape block size before each data block (with vax
 *    swapped byte ordering!).
 *
 *    When writing a tape dump,  file names preceded by "-Fnn" cause those
 *    files to be written in fixed instead of the default variable format.
 *    If "nn" is specified,  it will be used for the record length, otherwise
 *    80 is assumed.
 *
 * BUGS
 *    Due to the odd format of CMS tape files,  you will need twice the working
 *    space as that taken up by the final files.  Also,  even files you don't
 *    want to extract get copied to a temporary file and are then discarded
 *    since the name of the file is at the end of the data!
 *
 * FILES
 *    cms??????          temporary work file.
 *    CMSTAPE.CMSUT1     temporary work file when CMS #defined.
 *
 * SEE ALSO
 *    IBM VM/SP Data Areas and Control Block Logic, LY20-0891
 *
 *    Readtape, writetape utilities:  Jim Guyton, Rand Corp.
 *
 * AUTHOR
 *    E. Alan Crosswell
 *    Columbia University
 *    alan@columbia.edu
 *
 * #define CMS when compiling for Waterloo C under CMS.
 *
 * COMPILATION INSTRUCTIONS
 *  4.x BSD:
 *    cc -o cmstape cmstape.c ebcdic.c
 *  Waterloo C/370:
 *    cw cmstape (prm CMS 1
 *    linkc cmstape
 *
 * You may wonder why I wrote this.  It's simple:  I get a lot of CMS tapes
 * in the mail.  The IBM machine room is across campus but the vax is just
 * down the hall and they're both on the network.  Here's how I do it:
 *    1) mount the tape on the vax and use readtape to read all the files
 *       into a directory.
 *    2) login on the IBM and FTP the files from the vax in image mode:
 *       ftp thevax
 *       login me
 *       represent i
 *       get file1 file1.file
 *       get file2 file2.file
 *         etc.
 *    3) crank up cmstape on the IBM:
 *       cmstape -xvfi file1
 *       cmstape -xvfi file2
 *         etc.
 *
 * $Log:	cmstape.c,v $
 * Revision 1.1  86/07/18  10:44:17  alan
 * Initial revision
 * 
 */
 
 
/* various header files */
 
#ifdef IBM370                   /* future version of Wloo C may predefine it */
# define CMS 1
#endif IBM370
 
#include <stdio.h>
#include <ctype.h>
#ifdef CMS
# include <time.h>
# include <file.h>
# include <stdlib.h>
# include <string.h>
# include <types.h>
# include <stat.h>
#else
# include <strings.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/file.h>
# include <sys/stat.h>
#endif CMS
 
/*
 * Format of a CMS TAPE DUMP tape entry for each file:
 *
 * 1) One or more blocks like this:
 *      +--+-+-+-+-+-+-+-----/ /-+-+-+-----
 *      |02|C M S V| l | data... | l | data...
 *      +--+-+-+-+-+-+-+-----/ /-+-+-+-----
 *       <-----------4101 bytes max---------->
 *
 *    or this (for fixed records (lrecl in the FST)):
 *      +--+-+-+-+-+-----/ /-+-+-+-----
 *      |02|C M S F| data... | l | data...
 *      +--+-+-+-+-+-----/ /-+-+-+-----
 *       <-----------4101 bytes max---------->
 *
 * 2) Followed by an ending FST + filename
 *      +--+-+-+-+-+------------+
 *      |02|C M S N|  FST data  |
 *      +--+-+-+-+-+------------+
 *       <-------87 bytes------>
 *
 * There are a whole bunch of other CMSx letters as well but I don't know
 * what they mean and have never seen one on a tape I've received.  Some
 * have to do with sparse files.
 *
 * Many CMS files may be in a single physical tape file.  Notice that since
 * the FST comes after the data,  there is no way to know the record length
 * of "F" (fixed) files until after the entire file has been read.
 * "V" (variable) files have the length of each record in front of
 * the record.
 *
 * The tape fst written into the CMSN block is actually the CMS fst starting
 * a few words from the beginning,  then some filler,  then the filename.
 */
 
#ifdef CMS
typedef unsigned char u_char;
#endif CMS
typedef struct {
    u_char f_wp[2];             /* write pointer - IBM byte ordering */
    u_char f_rp[2];             /* read pointer */
    u_char f_mode[2];           /* mode */
    u_char f_itcnt[2];          /* item count */
    u_char f_fcl[2];            /* first chain link */
    u_char f_recfm;             /* record format: F or V */
    u_char f_flag;              /* flag */
    u_char f_lrecl[4];          /* record length */
    u_char f_blkcnt[2];         /* data block count */
    u_char f_date[6];           /* date FYFYMMDDHHNN */
    u_char f_fop[4];            /* file origin pointer */
/* beginning of alternate counts (added with EDF file system) */
    u_char f_ablkcnt[4];        /* alternate block count */
    u_char f_areccnt[4];        /* alternate record count */
    u_char f_nlvl;              /* number of ptr block levels */
    u_char f_ptrs;              /* fst pointer size */
    u_char f_adate[6];          /* alternate date YYMMDDHHNNSS */
    u_char f_filler[20];        /* fill out to 64 bytes */
/* filename tacked on after fst */
    u_char f_fn[8];
    u_char f_ft[8];
    u_char f_fm[2];
} tapefst;                      /* overall length 82 bytes */
 
/* what's at the beginning of each tape block */
typedef struct {
    u_char cms[4];              /* 02 CMS */
    u_char fmt;                 /* F | V | N | ... */
} tapehead;
 
#define F 0xc6                  /* fixed */
#define V 0xe5                  /* variable */
#define N 0xd5                  /* end of file */
 
 
/* macros for ebcdic to local character set and byte ordering */
#ifdef CMS
# define ETOL(c) (c)
# define LTOE(c) (c)
#else
extern char etoa[], atoe[];
# define ETOL(c) (etoa[(c)&0xff])
# define LTOE(c) (atoe[(c)&0xff])
#endif CMS
#define IBMTOINT(x) (((x[0]&0xff)<<24) + ((x[1]&0xff)<<16) \
                     + ((x[2]&0xff)<<8) + (x[3]&0xff))
#define IBMTOSHORT(x) (((x[0]&0xff)<<8) + (x[1]&0xff))
#define INTTOIBM(a,b) a[0]=(b>>24)&0xff, a[1]=(b>>16)&0xff, \
                     a[2]=(b>>8)&0xff, a[3]=(b&0xff)
#define SHORTTOIBM(a,b) a[0]=(b>>8)&0xff, a[1]=(b&0xff)
 
/* hi and lo nibbles of a byte (packed decimal) */
#define HI(x) ((x)>>4)
#define LO(x) ((x)&0x0f)
#define PACK(x) (((((x)/10)&0xf) << 4) | (((x)%10)&0xf))
 
/* Globals */
 
 
#define MAXBUF 1<<16            /* max size file record */
#define DEFTAPE "/dev/nrmt8"    /* default tape name */
#define TAPEDATA 4096           /* size of data part of tape block */
 
static int Fflag = 0;           /* indicates readtape(l) file */
static int Tflag = 0;           /* just give a directory */
static int Vflag = 0;           /* verbose */
static int Xflag = 0;           /* read files off tape */
static int Cflag = 0;           /* write files onto tape */
static int Rflag = 0;           /* append files onto tape */
static int Iflag = 0;           /* image mode (instead of text) */
static int Uflag = 0;           /* uppercase converted file names */
static int Debug = 0;           /* debug */
static char *Tapename = NULL;   /* tape file name */
static char **Fname = NULL;     /* ptr to array of file names */
static int Fcount = 0;          /* number in above array */
static char *Cmdname;           /* name invoked as */
static char *Bufp = NULL;       /* buffer pointer */
static int Lrecl = 0;           /* record length */
#ifdef CMS
static struct stat St;          /* used for CMS tape dumps */
static char Tempname[] = "cmstape.cmsut1 (bin lrecl 65535";
#else
static char Template[] = "cmsXXXXXX";
static char Tempname[sizeof(Template)];
#endif CMS
static FILE *Tempf = NULL;      /* shared by dodata and doeof */
struct nametab {                /* ebcdic name table */
    char name[16];
} *Nametab = NULL;
 
static char CMSh[] = {0x02, 0xc3, 0xd4, 0xe2 };
                    /*  02    C     M     S  */
 
static char *Month[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec" };
 
/* shared by dumprec and dumpeof */
static char Tapebuf[TAPEDATA+sizeof(tapehead)];
static char *Tbp;               /* ptr into tapebuf */
static int Tbrem;               /* remainder length */
static int Recs;                /* records dumped */
static int Blocks;              /* blocks dumped */
static int Maxrecl;             /* max lrecl */
 
 
/*
 * the main program.  All the work is done in readtape() and writetape()
 * (not to be confused with tape utilities of the same name).
 */
main(argc, argv)
{
    options(argc, argv);        /* parse options */
    initialize();               /* intialize junk */
    if (Cflag)
      writetape();              /* output */
    else
      readtape();               /* input */
    exit(0);
}
 
static
options(argc, argv)             /* parse options */
int argc;
char **argv;
{
    Cmdname = argv[0];
    for(--argc, ++argv; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
        char *cp = argv[0];
        while (*++cp) switch(*cp) {
          case 'f':
            Fflag++;
            continue;
          case 't':
            Tflag++;
            continue;
          case 'v':
            Vflag++;
            Tflag++;
            continue;
          case 'x':
            Xflag++;
            continue;
          case 'c':
            Cflag++;
            continue;
          case 'i':
            Iflag++;
            continue;
          case 'd':
            Debug++;
            continue;
          case 'u':
            Uflag++;
            continue;
          case 'r':
            Cflag++;
            Rflag++;
            continue;
          default:
            usage();
        }
    }
    Tapename = (argc--)? *argv++ : DEFTAPE;
    if (argc > 0) {
        Fname = argv;
        Fcount = argc;
    }
    if (Xflag+Cflag == 0)
      Tflag = 1;
    else if (Xflag+Cflag > 1) {
        fprintf(stderr,"%s: Can't extract and create at the same time!\n",
                Cmdname);
        exit(1);
    }
    if (Debug)
      printoptions();
}
 
/*
 * get a buffer for I/O and build table of file names
 */
static
initialize()
{
    char *malloc(), *unix2cms();
 
#ifdef CMS
    setiomsg(1);
#endif CMS
    if ((Bufp = malloc(MAXBUF+2)) == NULL) /* get a buffer */
      croak("malloc");
    if (Fcount && (Cflag || Xflag)) { /* user gave list of names */
        int i;
 
        if ((Nametab = (struct nametab *)malloc(Fcount * sizeof(Nametab[0])))
            == NULL)
          croak("malloc");
        if (Debug > 1)
          fprintf(stderr,"Name table:\n");
        for (i = 0; i < Fcount; i++) {
            bcopy(unix2cms(Fname[i]),Nametab[i].name,sizeof(Nametab[0].name));
            if (Debug > 1) {
                int j;
                for (j = 0; j < sizeof(Nametab[0].name); j++)
                  fputc(ETOL(Nametab[i].name[j]), stderr);
                fputc('\n', stderr);
            }
        }
    }
}
 
static
printoptions()                  /* print out options */
{
    int i;
 
    fprintf(stderr,"%s: switches: ", Cmdname);
    if (Fflag) fputc('f',stderr);
    if (Tflag) fputc('t',stderr);
    if (Xflag) fputc('x',stderr);
    if (Cflag) fputc('c',stderr);
    if (Iflag) fputc('i',stderr);
    if (Uflag) fputc('u',stderr);
    if (Vflag) fputc('v',stderr);
    if (Rflag) fputc('r',stderr);
    if (Debug) fputc('d',stderr);
    fprintf(stderr,"\n\ttape '%s'\n", Tapename);
    if (Fcount) {
        fprintf(stderr,"\t%d files:", Fcount);
        for (i = 0; i < Fcount; i++)
          fprintf(stderr, " %s", Fname[i]);
        fprintf(stderr,".\n");
    }
}
 
static
usage()
{
    fprintf(stderr,"%s: Usage: %s [-ftvxcdiu] tape name...\n", Cmdname,
            Cmdname);
    exit(1);
}
 
static
readtape()                      /* read a cms format tape */
{
    int tapefd;
    int len;
    tapehead *t = (tapehead *)Bufp;
    char *dp = Bufp + sizeof(*t);
#undef NAME
#ifdef CMS
# define NAME wlooname
    char wlooname[100];
    strcpy(wlooname,Tapename);
    strcat(wlooname,"(bin");
#else
# define NAME Tapename
#endif CMS
    if ((tapefd = open(NAME, O_RDONLY)) < 0)
      croak(Tapename);
 
    while ((len = readblock(tapefd)) > 0) {
        if (Debug > 1)
          fprintf(stderr,"readblock return len=%d\n", len);
        if (len < sizeof(tapehead) || bcmp(t->cms,CMSh,sizeof(CMSh)))
          die("%s: not a CMS tape\n", Tapename);
        len -= sizeof(*t);      /* subtract length of header */
        switch (t->fmt) {
          case N:               /* EOF */
            doeof(dp,len);
            break;
          case F:               /* F data */
          case V:               /* V data */
            if (Xflag)
              dodata(dp,len);   /* copy the data and fix it up later */
            break;
          default:
            die("%s: CMS format %c not supported.\n",
                    Tapename, ETOL(t->fmt));
        }
    }
 
}
 
/*
 * copy the data verbatim to the temp file.
 */
dodata(b,l)
char *b;
int l;
{
    if (Debug > 1) {
        fprintf(stderr,"data block, len=%d\n", l);
        if (Debug > 2)
          dump(b-sizeof(tapehead),l+sizeof(tapehead));
    }
    if (Tempf == NULL) {        /* first time thru */
#ifndef CMS
        strcpy(Tempname,Template);
        mktemp(Tempname);
#endif
        Tempf = fopen(Tempname,"w+");
        if (Tempf == NULL)
          croak(Tempname);
    }
    if (fwrite(b, l, 1, Tempf) == NULL)
      croak(Tempname);
}
 
char *cms2unix();
 
/*
 * process the EOF block.  Depending on options selected:
 *   - copy the temp file using the recfm and lrecl with possible translations.
 *   - type out the fst info in a manner similar to ls.
 */
static
doeof(b,l)
tapefst *b;
int l;
{
    if (Debug) {
        fprintf(stderr,"EOF block, len=%d\n", l);
        prtfst(b);
    }
    if (l < sizeof(tapefst))
      die("%s: file label too short.\n", Tapename);
    Lrecl = IBMTOINT(b->f_lrecl);
    if (in_nametab(b)) {
        if (Tflag)
          prinfo(b);
        if (Xflag) {            /* actually reading the data */
            if (Tempf == NULL) {
                fprintf(stderr,"%s: Empty file.  Skipping...\n",Tapename);
                return;
            }
#ifndef CMS
            if (Iflag) {        /* when not CMS, this is simply a rename */
                fclose(Tempf);
                Tempf = NULL;
                if (rename(Tempname, cms2unix(b)) < 0)
                  perror(Tempname);
                return;
            }
#endif CMS
            /* call V_ or F_close for cooked files always and image files
               when under CMS.  Image files under unix were handled above. */
            if (b->f_recfm == V)
              V_close(b);
            else
              F_close(b);
        }
    } /* end if in Nametab[] */
    if (Xflag && Tempf != NULL) { /* clean up when not in Nametab[] */
        fclose(Tempf);
        Tempf = NULL;
        if (!Debug)
          unlink(Tempname);
        else
          fprintf(stderr,"%s: not unlinked (debug)\n", Tempname);
    }
}
 
static
prinfo(b)
tapefst *b;
{
    int lrecl, reccnt, year, mon, day, hour, min;
    int i;
    struct tm *tp;
    long tic;
 
    if (Vflag) {
        lrecl = IBMTOINT(b->f_lrecl);
        reccnt = IBMTOINT(b->f_areccnt);
        year = HI(b->f_adate[0]) * 10 + LO(b->f_adate[0]);
        mon =  HI(b->f_adate[1]) * 10 + LO(b->f_adate[1]);
        day =  HI(b->f_adate[2]) * 10 + LO(b->f_adate[2]);
        hour = HI(b->f_adate[3]) * 10 + LO(b->f_adate[3]);
        min =  HI(b->f_adate[4]) * 10 + LO(b->f_adate[4]);
#ifdef CMS
        tp = localtime();       /* waterloo goofed! */
#else
        time(&tic);
        tp = localtime(&tic);
#endif CMS
        printf("%c/%-4d %8d %s %2d ",ETOL(b->f_recfm),
               lrecl, reccnt, Month[mon-1], day);
        if (tp->tm_year != year)
          printf(" %4d", year);
        else
          printf("%2d:%02d ",hour,min);
    }
    for (i = 0; i < 8; i++)
      putchar(ETOL(b->f_fn[i]));
    putchar(' ');
    for (i = 0; i < 8; i++)
      putchar(ETOL(b->f_ft[i]));
    putchar(' ');
    putchar(ETOL(b->f_fm[0]));
    putchar(ETOL(b->f_fm[1]));
    putchar('\n');
}
 
 
/* V format close.  Rewind the temp file and write the final file */
static
V_close(fst)
tapefst *fst;
{
    int newfd = FVcreat('v',fst);
    int len;
 
    rewind(Tempf);
    Recs = 0;
    while (fread(Bufp, 2, 1, Tempf)) { /* get length header */
        len = IBMTOSHORT(Bufp);
        if (fread(Bufp, len, 1, Tempf) == 0) /* get data */
          croak(Tempname);
        writerec(newfd, Bufp, len); /* write out record */
    }
    close(newfd);
}
 
/* F format close.  Close the temp file, f and rewrite the final
   F format file by calling writerec. */
static
F_close(fst)
tapefst *fst;
{
    int newfd = FVcreat('f',fst);
 
    rewind(Tempf);
    Recs = 0;
    while (fread(Bufp, Lrecl, 1, Tempf)) { /* get record */
        writerec(newfd, Bufp, Lrecl); /* and write it */
    }
    close(newfd);
}
 
static int
FVcreat(recfm,fst)              /* creat final output file */
char recfm;
tapefst *fst;
{
    int newfd;
    char *newname = cms2unix(fst);
#undef NAME
#ifdef CMS
# define NAME wlooname
    char wlooname[100];
 
    sprintf(wlooname,"%s (%s recfm %c lrecl %d", newname,
            (Iflag)?"raw bin":"", recfm, Lrecl);
#else
# define NAME newname
#endif CMS
 
    if ((newfd = creat(NAME,0644)) < 0)
      croak(NAME);
    return newfd;
}
 
/*
 * write record out. Check flags and do ebcdic->ascii translation
 * and adding newline, etc.  When the file is opened "raw" under CMS,
 * each write() call is guaranteed to write a CMS record.
 */
static
writerec(fd,b,l)                /* write a record out to file fd */
int fd;
char *b;
int l;
{
    register int i;
 
    ++Recs;
    if (!Iflag) {               /* text copy */
        if (l == 1 && *b == LTOE(' ')) /* null line */
          l = 0;
        for (i = 0; i < l; i++) /* translate it */
          b[i] = ETOL(b[i]);
        b[l++] = '\n';          /* and add newline */
    }
    if (Debug > 1) {
       fprintf(stderr,"writerec: record %5d length %d:", Recs, l);
       dump(b,l);
    }
    if (write(fd,b,l) != l)
      croak("writerec");
}
 
/*
 * convert CMS to Unix name:  "AAA     BBB     A1" becomes "aaa.bbb"
 * if Uflag, don't alter case.
 */
static char *
cms2unix(fst)
tapefst *fst;
{
    static char name[20];
    char *np = name, c;
    int i;
 
    for (i = 0; i < 8 && (c = ETOL(fst->f_fn[i])) != ' '; i++)
      *np++ = (!Uflag && isupper(c)) ? tolower(c) : c;
    *np++ = '.';
    for (i = 0; i < 8 && (c = ETOL(fst->f_ft[i])) != ' '; i++)
      *np++ = (!Uflag && isupper(c)) ? tolower(c) : c;
    *np = '\0';
    if (Debug > 1)
      fprintf(stderr,"cms2unix: %s\n", name);
    return name;
}
 
/*
 * unix2cms: Convert unix name of /1/2/3/aaa.bbb to 16 character ebcdic string
 * of "AAA     BBB     " for comparisons with fn ft.  If Uflag, don't
 * alter case.
 */
static char *
unix2cms(f)                     /* convert unix name string to CMS */
char *f;
{
    static char name[17];
    char *p,*np = name;
    int i;
 
    f = (p = rindex(f,'/')) ? p+1 : f; /* skip leading path components */
    /* filename */
    for (i = 0; i < 8 && *f != '.' && *f != '\0'; i++, f++, np++)
      *np = LTOE((!Uflag && islower(*f)) ? toupper(*f) : *f);
    for (; i < 8; i++, np++)
      *np = LTOE(' ');          /* pad out to 8 */
 
    /* filetype */
    if (*f == '\0')
      f = "file";               /* fill it in with "FILE" */
    else
      f++;
    for (i = 0; i < 8 && *f != '.' && *f != '\0'; i++, f++, np++)
      *np = LTOE((!Uflag && islower(*f)) ? toupper(*f) : *f);
    for (; i < 8; i++, np++) /*  */
      *np = LTOE(' ');          /* pad out to 8 */
    name[16] = '\0';
    return name;
}
 
/*
 * readblock - read a tape block into the Bufp buffer.
 * If Fflag,  then the tape is actually a disk file created by readtape(l)
 * which has a 16 bit block length prior to each data block.
 * If reading straight from a tape device,  the read sys call will
 * hopefully return the physical block length.  In either case,
 * set *bpp to point to the beginning of the data and return the data
 * length.
 */
static
readblock(fd)                   /* read a tape block */
int fd;
{
    int nread;
 
    if (Fflag)
      return read_with_len(fd);
    else {
        nread = read(fd, Bufp, MAXBUF); /* try for the max */
        if (Debug > 2)
          fprintf(stderr,"physical block size %d\n", nread);
        if (nread < 0)
          croak(Tapename);
        return nread;
    }
}
 
static
read_with_len(fd)               /* read a readtape(l) format block */
int fd;
{
    char bytes[2];
    int blen;
    int nread;
    int amt;
    char *bp;
 
    /* read the 2 byte readtape(l) block length prefix */
    nread = read(fd, bytes, 2);
    if (nread == 0)
      return 0;                 /* EOF */
    if (nread < 0)
      croak(Tapename);
    if (nread != 2)
      die("%s: unexpected EOF reading block length (%d)\n",
          Tapename, nread);
    blen = ((unsigned)bytes[1] << 8) + (unsigned)bytes[0];
    if (Debug > 2)
      fprintf(stderr,"readtape block length [%d,%d] %d\n",
              bytes[0], bytes[1], blen);
    if (blen == 0)
      return 0;         /* empty block == EOF */
    if (blen > MAXBUF) {
        fprintf(stderr,"%s: I/O buffer too small\n", Cmdname);
        exit(1);
    }
 
    nread = read(fd, Bufp, blen);
    if (Debug > 2)
      fprintf(stderr,"%d of %d read.\n", nread, blen);
    if (nread < 0)
      croak(Tapename);
    if (nread == 0)
      die("%s: unexpected EOF.\n", Tapename);
    if (nread != blen)
      die("%s: wrong length.  Wanted %d but got %d\n",
          Tapename, blen, nread);
    return blen;
}
 
static
dump(b,l)                       /* print out a block */
char *b;
int l;
{
    int i;
 
    for (i = 0; l-- > 0; i++) {
        if (i%32 == 0)
          fprintf(stderr,"\n%5d:", i);
        if (i%4 == 0)
          fputc(' ',stderr);
        fprintf(stderr,"%02x", *b++);
    }
    fputc('\n',stderr);
}
 
static
prtfst(f)                       /* print out fst info */
register tapefst *f;
{
    int i;
 
    fprintf(stderr,"TAPEFST:\n");
    fprintf(stderr," wp: %02x%02x\n", f->f_wp[0], f->f_wp[1]);
    fprintf(stderr," rp: %02x%02x\n", f->f_rp[0], f->f_rp[1]);
    fprintf(stderr," mode: %c%c\n", ETOL(f->f_mode[0]), ETOL(f->f_mode[1]));
    fprintf(stderr," itcnt: %d\n", IBMTOSHORT(f->f_itcnt));
    fprintf(stderr," fcl: %02x%02x\n", f->f_fcl[0], f->f_fcl[1]);
    fprintf(stderr," recfm: %c\n", ETOL(f->f_recfm));
    fprintf(stderr," flag: %02x\n", f->f_flag);
    fprintf(stderr," lrecl: %d\n", IBMTOINT(f->f_lrecl));
    fprintf(stderr," blkcnt: %d\n", IBMTOSHORT(f->f_blkcnt));
    fprintf(stderr," date: %02x%02x%02x%02x%02x%02x\n", f->f_date[0],
            f->f_date[1], f->f_date[2], f->f_date[3], f->f_date[4],
            f->f_date[5]);
    fprintf(stderr," fop: %02x%02x%02x%02x\n", f->f_fop[0], f->f_fop[1],
            f->f_fop[2], f->f_fop[3]);
    fprintf(stderr," ablkcnt: %d\n", IBMTOINT(f->f_ablkcnt));
    fprintf(stderr," areccnt: %d\n", IBMTOINT(f->f_areccnt));
    fprintf(stderr," nlvl: %d\n", f->f_nlvl);
    fprintf(stderr," ptrs: %d\n", f->f_ptrs);
    fprintf(stderr," adate: %02x%02x%02x%02x%02x%02x\n", f->f_adate[0],
            f->f_adate[1], f->f_adate[2], f->f_adate[3], f->f_adate[4],
            f->f_adate[5]);
    fprintf(stderr," filler: "); dump(f->f_filler,sizeof(f->f_filler));
    fprintf(stderr," fn,ft,fm: ");
    for (i = 0; i < 18; i++)
      fputc(ETOL(f->f_fn[i]), stderr);
    fputc('\n',stderr);
}
 
#ifdef CMS
/* another Waterloo C defficiency */
perror(s)
char *s;
{
    fprintf(stderr,"%s: error.\n", s);
}
#endif CMS
 
/*
 * if Nametab is empty or given name matches an entry in it, return true.
 */
static int
in_nametab(fst)
tapefst *fst;
{
    register int i;
 
    if (Nametab == NULL)
      return 1;
    for (i = 0; i < Fcount; i++)
      if (bcmp(fst->f_fn, Nametab[i].name, sizeof(Nametab[0].name)) == 0)
        return 1;
    return 0;
}
 
 
/*
 * write files onto the tape.
 * - if Fname[i] is "-Fnn" then filename is in Fname[i+1] and it should be
 *   written recfm F lrecl nn.  Nn if ommitted is 80.
 * - if cooked (not Iflag) then records are delimitted by newlines and
 *   LTOE translation should be done.  Short recfm F records are padded
 *   with blanks.  Long ones are truncated (with a message).
 * - if raw (Iflag) then there are no record delimitters.  Just copy the
 *   data as is to tape blocks.  For recfm V,  the data is assumed to contain
 *   record lengths.  The max record length must be calculated for the fst.
 * - for CMS, recfm/lrecl are obtained with a stat(), so -F is not necessary.
 */
 
static
writetape()                     /* write a cms format tape */
{
    int i, tapefd, mode;
#undef NAME
#ifdef CMS
    char wlooname[50];
    strcpy(wlooname,Tapename);
    strcat(wlooname," (bin lrecl 65535");
# define NAME wlooname
#else
# define NAME Tapename
#endif !CMS
 
    mode = O_WRONLY|O_CREAT;
    mode |= (Rflag)? O_APPEND : O_TRUNC;
    if ((tapefd = open(NAME,mode,0644)) < 0)
      croak(Tapename);
    for (i = 0; i < Fcount; i++) {
        int lrecl = 0;
        int recfm = V;
 
        if (Fname[i][0] == '-' && Fname[i][1] == 'F') { /* check for -Fnn */
            recfm = F;
            lrecl = 80;
            sscanf(&Fname[i][2],"%d", &lrecl);
            ++i;
        }
        if (Debug)
          fprintf(stderr,"Writing %s recfm %c lrecl %d\n", Fname[i],
                  ETOL(recfm), lrecl);
        /* check accessability and get stat info for CMSN fst use */
        if (access(Fname[i], R_OK) < 0) {
            fprintf(stderr,"%s: can't access.  Skipping...\n", Fname[i]);
            continue;
        }
        tapedump(tapefd,i,lrecl,recfm);
    }
    close(tapefd);
}
 
/*
 * tapedump:  Dump file Fname[i] as Nametab[i] onto tapefd using given
 *  recfm and lrecl.  Check Iflag to see if we want a raw or cooked dump.
 */
static
tapedump(tapefd,i,lrecl,recfm)
int tapefd,i,lrecl;
char recfm;
{
    FILE *wf;
    int nread;
    int wfd;
#undef NAME
#ifdef CMS
# define NAME wlooname
    char wlooname[50];
    char *cmsexpn();
    fprintf(stderr,"cmsexpn: '%s'\n",cmsexpn(Fname[i]));
    if (stat(cmsexpn(Fname[i]), &St) < 0) {
        perror(Fname[i]);
        fprintf(stderr,"Skipping...\n");
        return;
    }
    recfm = St.st_recfm;
    lrecl = St.st_lrecl;
    sprintf(wlooname, "%s (recfm %c lrecl %d %s", Fname[i],
            St.st_recfm, St.st_lrecl, (Iflag)?"raw bin":"text");
    if (Debug)
      fprintf(stderr,"Fname is %s\n", wlooname);
#else
# define NAME Fname[i]
#endif !CMS
 
    Blocks = -1;                /* init tape block count */
    Recs = 0;                   /* and record count */
 
    if (Iflag) {                /* image dump */
        if ((wfd = open(NAME, O_RDONLY)) < 0) {
            perror(Fname[i]);
            fprintf(stderr,"Skipping...\n");
            return;
        }
#ifndef CMS
        if (recfm == F) {       /* recfm F */
            int bytes = 0;
 
            if (Debug)
              fprintf(stderr,"image dump recfm F\n");
            Maxrecl = lrecl;
            while ((nread = read(wfd,Bufp,TAPEDATA)) > 0) {
                bytes += nread;
                dumprec(tapefd,recfm,Bufp,nread);
            }
            Recs = bytes/Maxrecl;
        } else {                /* recfm V */
            int len;
 
            if (Debug)
              fprintf(stderr,"image dump recfm V\n");
            while (read(wfd,Bufp,2) == 2) { /* get lrecl */
                len = IBMTOSHORT(Bufp);
                if (Debug > 1)
                  fprintf(stderr,"read len [%d,%d] = %d\n", Bufp[0], Bufp[1],
                          len);
                Maxrecl = (Maxrecl < len) ? len : Maxrecl;
                if ((nread = read(wfd,Bufp+2,len)) != len) {
                    if (Debug)
                      fprintf(stderr,"len read = %d of %d\n", nread, len);
                    die("%s: not a V-format image.\n",Fname[i]);
                }
                dumprec(tapefd,recfm,Bufp,len+2);
            }
        }
#else
        /* for CMS image dumps,  both recfm F and V are done the same:
           Do a read with file opened raw which will return actual record
           length. For V, add that length to the dump record */
        while ((nread = read(wfd,Bufp+2,MAXBUF)) > 0) {
            if (recfm == F)
              dumprec(tapefd,recfm,Bufp+2,nread);
            else {
                SHORTTOIBM(Bufp,nread);     /* length prefix */
                dumprec(tapefd,recfm,Bufp,nread+2);
            }
            Maxrecl = (Maxrecl < nread) ? nread : Maxrecl;
        }
 
#endif CMS
        close(wfd);
    } else {                    /* cooked dump */
        if ((wf = fopen(NAME,"r")) == NULL) {
            perror(Fname[i]);
            fprintf(stderr,"Skipping...\n");
            return;
        }
        while (fgets(Bufp+2,MAXBUF,wf) != NULL) {
            if ((nread = strlen(Bufp+2) - 1) <= 0) { /* min CMS rec is 1 */
                nread = 1;
                Bufp[2] = ' ';
            }
            if (recfm == F) {   /* pad out to lrecl */
                pad(Bufp+2,nread,lrecl);
                nread = lrecl;
            }
            Maxrecl = (Maxrecl < nread) ? nread : Maxrecl;
            xlate(Bufp+2,nread); /* xlate to ebcdic */
            if (recfm == V) {   /* insert record length header */
                SHORTTOIBM(Bufp,nread);
                dumprec(tapefd,recfm,Bufp,nread+2);
            }
            else
                dumprec(tapefd,recfm,Bufp+2,nread);
        }
        fclose(wf);
    }
    dumpeof(tapefd,i,recfm);
}
 
 
static
pad(b,l1,l2)
char *b;
register int l1,l2;
{
    if (l1 >= l2)
      return;
    for (; l1 < l2; l1++)
      b[l1] = ' ';
}
 
static
xlate(b,l)
register char *b;
register int l;
{
#ifndef CMS
    while (l-- > 0) {
        *b = LTOE(*b);
        b++;
    }
#endif !CMS
}
 
/*
 * write record into tapebuf[].  If this fills it,  flush it to fd
 * and begin filling next tapebuf[].
 */
static
dumprec(fd,recfm,b,l)
int fd;
char recfm;
char *b;
int l;
{
    if (Blocks == -1) {         /* first time thru, initialize */
        Tbrem = TAPEDATA;       /* amount of room left in tapebuf */
        Tbp = &Tapebuf[sizeof(tapehead)];
        bcopy(CMSh,Tapebuf,sizeof(tapehead));
        ((tapehead *)Tapebuf)->fmt = recfm;
        Blocks = 0;
    }
    ++Recs;
    if (Debug > 1) {
      fprintf(stderr,"dumprec: rec %5d len = %d blockno = %d rem = %d:",
              Recs, l, Blocks, Tbrem);
      dump(b,l);
    }
    while (l > 0) {
        if (Tbrem >= l) {       /* room for the full record */
            bcopy(b,Tbp,l);
            Tbrem -= l;
            Tbp += l;
            l = 0;
        } else {                /* have to spill over */
            bcopy(b,Tbp,Tbrem); /* copy what will fit */
            dumpbuf(fd,Tapebuf,sizeof(Tapebuf));
            l -= Tbrem;         /* subtract length we could fit */
            b += Tbrem;         /* increment data pointer this much */
            Tbp = &Tapebuf[sizeof(tapehead)];
            Tbrem = TAPEDATA;
            Blocks++;
        }
    }
}
 
static
dumpeof(fd,i,recfm)
int fd,i;
char recfm;
{
    struct stat stbuf;
    struct tm *tm;
    tapehead *th = (tapehead *)Tapebuf;
    tapefst *t = (tapefst *)&Tapebuf[sizeof(tapehead)];
 
    if (Tbrem < TAPEDATA)       /* flush pending data */
      dumpbuf(fd,Tapebuf,TAPEDATA-Tbrem+sizeof(tapehead));
    if (Debug)
      fprintf(stderr,"dumpeof: blocks %d, recs %d, maxrecl %d\n",
              Blocks, Recs, Maxrecl);
#ifndef CMS
    if (stat(Fname[i],&stbuf) < 0)
      croak("dumpeof");
    /* fill in tapefst -- both old and new ("alternate") fields */
    bzero(t,sizeof(*t));
    t->f_fm[0] = t->f_mode[0] = LTOE('A');
    t->f_fm[1] = t->f_mode[1] = LTOE('1');
    SHORTTOIBM(t->f_itcnt,Recs);
    INTTOIBM(t->f_areccnt,Recs);
    t->f_recfm = th->fmt;
    th->fmt = N;                /* change to an eof header */
    INTTOIBM(t->f_lrecl,Maxrecl);
    SHORTTOIBM(t->f_blkcnt,Blocks);
    INTTOIBM(t->f_ablkcnt,Blocks);
    tm = localtime(&stbuf.st_mtime);
/* old date format is odd:  year is ebcdic,  everything else is packed */
    t->f_date[0] = 0xf0 | (tm->tm_year/10)&0x0f; /* ebcdic year tens */
    t->f_date[1] = 0xf0 | (tm->tm_year%10)&0x0f; /* ebcdic year ones */
    t->f_adate[0] = PACK(tm->tm_year);
    t->f_adate[1] = t->f_date[2] = PACK(tm->tm_mon+1);
    t->f_adate[2] = t->f_date[3] = PACK(tm->tm_mday);
    t->f_adate[3] = t->f_date[4] = PACK(tm->tm_hour);
    t->f_adate[4] = t->f_date[5] = PACK(tm->tm_min);
    t->f_adate[5] = PACK(tm->tm_sec);
    bcopy(Nametab[i].name,t->f_fn,sizeof(Nametab[0].name));
#else
    /* stat already done into St */
    bzero(t,sizeof(*t));
    t->f_fm[0] = t->f_mode[0] = St.st_fmode[0];
    t->f_fm[1] = t->f_mode[1] = St.st_fmode[1];
    SHORTTOIBM(t->f_itcnt,St.st_size);
    INTTOIBM(t->f_areccnt,St.st_size);
    t->f_recfm = th->fmt;
    th->fmt = N;                /* change to an eof header */
    INTTOIBM(t->f_lrecl,Maxrecl);
    SHORTTOIBM(t->f_blkcnt,Blocks);
    INTTOIBM(t->f_ablkcnt,Blocks);
/* old date format is odd:  year is ebcdic,  everything else is packed */
    bcopy(St.st_mtime,t->f_adate,sizeof(t->f_adate));
    bcopy(&t->f_adate[1],&t->f_date[2], sizeof(t->f_date)-2);
    t->f_date[0] = HI(t->f_adate[0]) | 0xf0;
    t->f_date[1] = LO(t->f_adate[0]) | 0xf0;
    bcopy(St.st_fname,t->f_fn,sizeof(St.st_fname)+sizeof(St.st_ftype));
#endif CMS
    dumpbuf(fd,Tapebuf,sizeof(tapehead)+sizeof(tapefst));
    if (Tflag)
      prinfo(t);
}
 
static
dumpbuf(fd,b,l)
int fd;
char *b;
int l;
{
    if (Debug > 1)
      fprintf(stderr,"dumping block %d, len %d.\n", Blocks, l);
    if (Fflag) {                /* write readtape(l) count */
        u_char clen[2];
 
        clen[1] = l >> 8;       /* vax byte order */
        clen[0] = l & 0xff;
        if (write(fd,clen,2) < 0)
          croak("dumpbuf");
    }
    if (write(fd,b,l) < 0)      /* flush it */
      croak("dumpbuf");
    if (Debug > 2) {
        fprintf(stderr,"data: ");
        dump(b,l);
    }
}
 
#ifdef CMS
static char *
cmsexpn(f)                      /* fudge file name for wloo C stat(), etc. */
char *f;
{
    static char e[21];
    char *ep = e;
    int i;
 
    /* filename */
    for (i = 0; i < 8 && isalnum(*f); i++,f++,ep++)
      *ep = islower(*f)? toupper(*f) : *f;
    *ep++ = ' ';
    while (isalnum(*f))         /* skip more than 8 char filename */
      f++;
    if (*f == '\0' || *f == '(') { /* filetype missing */
        strcat(ep,"FILE");
        return e;
    }
    /* filetype */
    for (f++, i = 0; i < 8 && isalnum(*f); i++,f++,ep++)
      *ep = islower(*f)? toupper(*f) : *f;
    *ep++ = ' ';
    while (isalnum(*f))         /* skip more than 8 char filetype */
      f++;
    if (*f == '\0' || *f == '(') { /* filemode missing */
        ep[-1] = '\0';             /* clobber the trailing blank */
        return e;
    }
    /* filemode */
    for (f++, i = 0; i < 2 && isalnum(*f); i++,f++,ep++)
      *ep = islower(*f)? toupper(*f) : *f;
    *ep = '\0';
    return e;
}
#endif CMS
 
 
/*
 * various ways to die
 */
static
die(s,a1,a2,a3,a4,a5)
char *s;
{
    fprintf(stderr,s,a1,a2,a3,a4,a5);
    exit(1);
}
 
static
croak(s)
char *s;
{
    perror(s);
    exit(1);
}
 
