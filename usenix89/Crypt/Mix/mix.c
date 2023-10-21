/* mix.c */
/**********************************************************************
*    File Name     : mix.c
*    Object        : mix - also link to unmix, edmix, catmix, keymix
*    Berkeley cc   : cc -s -O % -o $BIN/mix
*    System V cc   : cc -s -O -DREALUNIX % -o $BIN/mix
*    Author        : Istvan Mohos, March 1987
*    Rev A Apr 14  : Do not mix last byte of file
***********************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#ifdef REALUNIX
#include <termio.h>

struct termio tbuf, tbufsave;
echo_off()
{
    ioctl (0, TCGETA, &tbuf);
    tbufsave = tbuf;
    tbuf.c_lflag &= ~ECHO;
    ioctl (0, TCSETAF, &tbuf);
}

echo_ret()
{
    ioctl (0, TCSETAF, &tbufsave);
}

#else
#include <sys/ioctl.h>

struct sgttyb tbuf, tbufsave;
echo_off()
{
    ioctl (0, TIOCGETP, &tbuf);
    tbufsave = tbuf;
    tbuf.sg_flags &= ~ECHO;
    ioctl (0, TIOCSETP, &tbuf);
}

echo_ret()
{
    ioctl (0, TIOCSETP, &tbufsave);
}

#endif

#define SMALLIM 256    /* minimum file size */
#define KMAX 128       /* maximum key size */

#define NONE    -1     /* data buffer references */
#define PLAIN    0
#define CIPHER   1
#define SMALLBUF 2
#define SCREEN   1

#define MIX    334     /* argv[0] hash values */
#define EDMIX  535
#define UNMIX  561
#define CATMIX 646
#define KEYMIX 663

struct keychar {       /* links to form chain of key bytes */
    int val;
    struct keychar *next;
};

int fstat();
struct stat sbuf;

int fd = -1, lockfd;          /* subject file, lock file */
char *plain, *cipher;         /* malloc pointers */
char *head, *tail;            /* main buffer limits */
char *gets(), *getenv();
struct keychar key[KMAX];     /* links */
int klen, flen;               /* key length, file length */
int loop;                     /* negative offset: size of main buffer */

int eflag = 0;                /* edmix command flag */
int unphase;                  /* unmix phase of edmix command */
char *estring = "vi";         /* edmix default command */
static char cmd[SMALLIM];     /* edmix system command buffer */

int argindx = 1;              /* pointer to current file */
char *fstring;                /* current file name */
static char mstring[KMAX];    /* minimum security key buffer */
char *kstring;                /* pointer to user key string */
char smallbuf[SMALLIM];       /* file buffer for files less than SMALLIM */
char lockstr[SMALLIM];        /* lock file: /tmp/mixlock$inode */
int locked = 0;               /* lock flag */
int context = 0;              /* argv[0] hash value */
int cleanup();                /* remove lock file, exit */

extern char **environ;
extern int errno;

main(argc,argv)
int argc;
char *argv[];
{

    register char *here;
    int regi;
    int donttry;              /* if unphase of edmix fails */
    int argneed = 2;
    char kbuf[SMALLIM];       /* interactive key acquisition buffers */
    char verify[SMALLIM];

    if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
        signal(SIGHUP, cleanup);
    if (signal(SIGINT, SIG_IGN) == SIG_DFL)
        signal(SIGINT, cleanup);
    if (signal(SIGQUIT, SIG_IGN) == SIG_DFL)
        signal(SIGQUIT, cleanup);
    if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
        signal(SIGTERM, cleanup);

    here = argv[0] + strlen(argv[0]);
    while(--here > argv[0]) {
        if (*here == '/') {
           ++here;
           break;
        }
    }
    if (*here == '/')  /* first byte of argv[0] */
       ++here;

    while(*here)
        context += *here++;

    switch (context) {
        case KEYMIX:
        fillm();
        fprintf(stderr, "%s\n", mstring), cleanup(0);

        case EDMIX:
            if (argc > argindx) {
                for (regi = 1; regi < 5;) {
                    if (strcmp(argv[regi], "-k") == 0) {
                        argneed += 2;
                        argindx += 2;
                        if (++regi < argc)
                            kstring = argv[regi];
                    }
                    else if (strcmp(argv[regi], "-e") == 0) {
                        eflag = 1;
                        argneed += 2;
                        argindx += 2;
                        if (++regi < argc)
                            estring = argv[regi];
                    }
                    if (++regi >= argc)
                        break;
                }
            }
            break;

        default:
            if (argc > argindx) {
                for (regi = 1; regi < 3;) {
                    if (strcmp(argv[regi], "-k") == 0) {
                        argneed += 2;
                        argindx += 2;
                        if (++regi < argc)
                            kstring = argv[regi];
                    }
                    if (++regi >= argc)
                        break;
                }
            }
            break;
    }

    if (argneed > argc) {
        switch (context) {
            default:
                fprintf(stderr, "Usage: %s [ -k key ] file ...\n",
                    argv[0]), cleanup(0);

            case EDMIX:
                fprintf(stderr,
                    "Usage: %s [ -k key ] [ -e editor ] file ...\n",
                    argv[0]), cleanup(0);
        }
    }

    if (kstring == NULL) {
        kstring = getenv("MIXKEY");
        if (kstring != NULL)
            if (strlen(kstring) == 0)
                kstring = NULL;
            /* to prevent a NULL environment variable automatically
               triggering the euid conversion */
    }

    if (kstring == NULL) {
        kstring = kbuf;
        fprintf(stderr, "Key: "), fflush(stderr);
        echo_off();
        gets(kstring);
        fprintf(stderr, "\n");
        fprintf(stderr, "Again: "), fflush(stderr);
        gets(verify);
        fprintf(stderr, "\n");
        echo_ret();
        if (strcmp(kstring, verify) != 0)
            fprintf(stderr, "%c%c",7, 7), cleanup(1);
    }

    /* verify key length validity */
    if ((klen = strlen(kstring)) == 0) {
        kstring = mstring;
        fillm();
        klen = strlen(kstring);
    }
    else if (klen > KMAX)
        *(kstring + KMAX) = '\0', klen = KMAX;

    /* link up selectors into circular chain */
    for (regi = klen; --regi; key[regi].next = key + regi -1);
    key[0].next = key + klen -1;

    while (argindx < argc) {
        fstring = argv[argindx++];
        donttry = 0;

        switch (context) {
            case MIX:
                switch (readfile(PLAIN)) {
                    case 0:
                        mix();
                        writefile(CIPHER, 0);
                        break;

                    case 1:
                        break;

                    default:
                        writefile(NONE, 0);
                        break;
                }
                break;

            case UNMIX:
                switch (readfile(CIPHER)) {
                    case 0:
                        unmix();
                        writefile(PLAIN, 0);
                        break;

                    case 1:
                        break;

                    default:
                        writefile(NONE, 0);
                        break;
                }
                break;

            case EDMIX:
                unphase = 1;
                switch(readfile(CIPHER)) {
                    case 0:
                        unmix();
                        writefile(PLAIN, 0);
                        break;

                    case 1:
                        donttry = 1;
                        break;

                    default:
                        writefile(NONE, 0);
                        break;
                }

                unphase = 0;
                if (!donttry) {
                    mksys();
                    system(cmd);
    
                    switch(readfile(PLAIN)) {
                        case 0:
                            mix();
                            writefile(CIPHER, 0);
                            break;

                        default:
                            writefile(NONE, 0);
                            break;
                    }
                }
                break;

            case CATMIX:
            default:
                switch(readfile(CIPHER)) {
                    case 0:
                        unmix();
                        writefile(PLAIN, SCREEN);
                        break;

                    case 1:
                        break;

                    default:
                        writefile(SMALLBUF, SCREEN);
                        break;
                }
                break;
        }
    }
    cleanup(0);
}

/* hash buffer to manufacture minimum security key,
   filled byte-by-byte to silence the "strings" command */
static260(buf)
char *buf;
{
    register char *h = buf;

    *h++ = 106; *h++ = 97;  *h++ = 101; *h++ = 111; *h++ = 108;
    *h++ = 102; *h++ = 107; *h++ = 108; *h++ = 116; *h++ = 106;
    *h++ = 109; *h++ = 118; *h++ = 114; *h++ = 107; *h++ = 111;
    *h++ = 111; *h++ = 118; *h++ = 107; *h++ = 109; *h++ = 117;
    *h++ = 112; *h++ = 104; *h++ = 109; *h++ = 108; *h++ = 114;
    *h++ = 103; *h++ = 105; *h++ = 112; *h++ = 106; *h++ = 97;
    *h++ = 99;  *h++ = 98;  *h++ = 103; *h++ = 117; *h++ = 117;
    *h++ = 97;  *h++ = 116; *h++ = 106; *h++ = 107; *h++ = 104;
    *h++ = 110; *h++ = 122; *h++ = 121; *h++ = 100; *h++ = 119;
    *h++ = 111; *h++ = 112; *h++ = 109; *h++ = 112; *h++ = 99;
    *h++ = 97;  *h++ = 101; *h++ = 118; *h++ = 107; *h++ = 106;
    *h++ = 101; *h++ = 104; *h++ = 114; *h++ = 113; *h++ = 116;
    *h++ = 107; *h++ = 120; *h++ = 119; *h++ = 113; *h++ = 114;
    *h++ = 97;  *h++ = 119; *h++ = 121; *h++ = 110; *h++ = 122;
    *h++ = 120; *h++ = 114; *h++ = 115; *h++ = 98;  *h++ = 120;
    *h++ = 97;  *h++ = 112; *h++ = 98;  *h++ = 121; *h++ = 115;
    *h++ = 116; *h++ = 99;  *h++ = 121; *h++ = 98;  *h++ = 113;
    *h++ = 100; *h++ = 122; *h++ = 116; *h++ = 117; *h++ = 100;
    *h++ = 122; *h++ = 99;  *h++ = 114; *h++ = 101; *h++ = 100;
    *h++ = 117; *h++ = 118; *h++ = 101; *h++ = 97;  *h++ = 100;
    *h++ = 115; *h++ = 105; *h++ = 101; *h++ = 118; *h++ = 119;
    *h++ = 102; *h++ = 98;  *h++ = 101; *h++ = 116; *h++ = 106;
    *h++ = 102; *h++ = 119; *h++ = 120; *h++ = 103; *h++ = 99;
    *h++ = 102; *h++ = 118; *h++ = 107; *h++ = 103; *h++ = 120;
    *h++ = 121; *h++ = 105; *h++ = 100; *h++ = 103; *h++ = 119;
    *h++ = 108; *h++ = 106; *h++ = 121; *h++ = 122; *h++ = 106;
    *h++ = 101; *h++ = 104; *h++ = 120; *h++ = 110; *h++ = 108;
    *h++ = 122; *h++ = 97;  *h++ = 107; *h++ = 102; *h++ = 105;
    *h++ = 122; *h++ = 113; *h++ = 109; *h++ = 98;  *h++ = 98;
    *h++ = 108; *h++ = 103; *h++ = 107; *h++ = 97;  *h++ = 99;
    *h++ = 112; *h++ = 99;  *h++ = 101; *h++ = 109; *h++ = 104;
    *h++ = 108; *h++ = 98;  *h++ = 102; *h++ = 115; *h++ = 100;
    *h++ = 103; *h++ = 110; *h++ = 105; *h++ = 109; *h++ = 99;
    *h++ = 104; *h++ = 116; *h++ = 102; *h++ = 105; *h++ = 111;
    *h++ = 108; *h++ = 110; *h++ = 100; *h++ = 106; *h++ = 117;
    *h++ = 103; *h++ = 108; *h++ = 112; *h++ = 109; *h++ = 111;
    *h++ = 102; *h++ = 109; *h++ = 120; *h++ = 104; *h++ = 110;
    *h++ = 113; *h++ = 110; *h++ = 112; *h++ = 104; *h++ = 111;
    *h++ = 121; *h++ = 105; *h++ = 112; *h++ = 115; *h++ = 111;
    *h++ = 113; *h++ = 105; *h++ = 113; *h++ = 122; *h++ = 106;
    *h++ = 114; *h++ = 116; *h++ = 113; *h++ = 114; *h++ = 107;
    *h++ = 115; *h++ = 119; *h++ = 114; *h++ = 97;  *h++ = 120;
    *h++ = 115; *h++ = 115; *h++ = 108; *h++ = 118; *h++ = 121;
    *h++ = 116; *h++ = 98;  *h++ = 122; *h++ = 117; *h++ = 117;
    *h++ = 109; *h++ = 119; *h++ = 98;  *h++ = 118; *h++ = 99;
    *h++ = 99;  *h++ = 119; *h++ = 110; *h++ = 110; *h++ = 120;
    *h++ = 100; *h++ = 121; *h++ = 102; *h++ = 101; *h++ = 121;
    *h++ = 102; *h++ = 122; *h++ = 113; *h++ = 115; *h++ = 100;
    *h++ = 111; *h++ = 116; *h++ = 103; *h++ = 103; *h++ = 104;
    *h++ = 104; *h++ = 105; *h++ = 105; *h++ = 117; *h++ = 110;
    *h++ = 111; *h++ = 113; *h++ = 112; *h++ = 114; *h++ = 118;
    *h++ = 115; *h++ = 117; *h++ = 118; *h++ = 119; *h++ = 120;
    *h = 0; }

/* manufacture minimum security key */
fillm()
{
    register char *here;
    int regi;
    int ck, mlen;
    static char fbuf[12], mbuf[36];
    char bstring[261];
    char *from, *to;

    ck = geteuid();
    ++ck; /* don't want root dividing by 0 */
    strcpy(fbuf, "%c");
    strcat(fbuf, "%o");
    strcat(fbuf, " %");
    strcat(fbuf, "x ");
    strcat(fbuf, "%d"); /* just so "strings" couldn't find it */
    sprintf(mbuf, fbuf, (ck & 95) + ' ' , ck, ~ck & 0xfff,
        (~ck & 0xffff) / ck);
    static260(bstring);
    head = bstring;
    here = head + strlen(bstring);
    tail = here -1;
    loop = head - tail -1;
    from = mbuf;
    to = mstring;
    mlen = strlen(mbuf);
    for (regi = mlen; --regi >=0; ) {
        if ((here += *from++) > tail)
            here += loop;
        *to++ = *here;
    }
}

/* manufacture system command of edmix */
mksys()
{
    char *here;
    char *esp;
    int fnamlen, did_sub = 0;

    if (!eflag) {
        strcpy(cmd, estring);
        strcat(cmd, " ");
        strcat(cmd, fstring);
    }
    else {
        here = cmd;
        esp = estring;
        fnamlen = strlen(fstring);
        while (*esp) {
            if (*esp == '$') {
                strcpy(here, fstring);
                here += fnamlen;
                esp++;
                did_sub = 1;
            }
            else
               *here++ = *esp++;
        }
        if (!did_sub) {
            *here++ = ' ';
            strcpy(here, fstring);
            here += fnamlen;
        }
        *here = 0;
    }
}

cleanup(exitval)
int exitval;
{
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    if (locked)
        unlock();
    exit(exitval);
}

lock()
{
    sprintf(lockstr, "/tmp/mixlock%d", sbuf.st_ino);
    while ((lockfd = creat(lockstr, 0)) == -1 && errno == EACCES) {
        fprintf(stderr,
            "Concurrent process lock %s, retry in 1 sec.\n", lockstr);
        sleep(1);
    }
    locked = 1;
    close(lockfd);
}

unlock()
{
    if (unlink(lockstr) == -1)
        fprintf(stderr,
            "Cannot unlink process lock %s, exiting\n", lockstr), exit(1);
    locked = 0;
}

readfile(source)
int source;
{
    int checkval;
    char *where;
    register char *here;
    char *calloc(), *malloc();

    if ((fd = open(fstring, 2)) == -1) {
        fprintf(stderr, "File not changed: %s (no access)\n", fstring);
        return(1);
    }
        
    if ((checkval = fstat(fd, &sbuf)) == -1)
        fprintf(stderr, "File system read error %d at %s, exiting\n",
        checkval, fstring), cleanup(1);

    if (context != EDMIX || unphase)
        lock();

    if ((flen = (int)sbuf.st_size) < SMALLIM) {
        fprintf(stderr, "File not changed: %s (less than %d bytes)\n",
        fstring, SMALLIM);
        return(2);
    }

    if (((plain = calloc((unsigned)flen, 1)) == NULL) ||
        ((cipher = malloc((unsigned)flen)) == NULL))
        fprintf(stderr, "Can't allocate %lu bytes at %s, exiting\n",
            flen << 1, fstring), cleanup(1);

    source ? (where = cipher) : (where = plain);

    if (read(fd, where, flen) != flen)
        fprintf(stderr, "File read error at %s, exiting\n",
        fstring), cleanup(1);

    for (here = where + flen; --here >= where;)
        if (!*here) {
            fprintf(stderr,
                "File not changed: %s (contains NULL bytes)\n",
                fstring);
            return(3);
        }

    return(0);
}

writefile(source, outflag)
int source, outflag;
{
    long lseek();

    if (fd > 2)
        lseek(fd, 0L, 0);

    switch (source) {
        case NONE:
            break;

        case PLAIN:
            if (outflag) {
                if (write(SCREEN, plain, flen) != flen)
                    fprintf(stderr, "Can't write to stdout, exiting\n"),
                        cleanup(1);
            }
            else if (write(fd, plain, flen) != flen)
                fprintf(stderr, "Can't write file %s, exiting\n",
                    fstring), cleanup(1);
            break;

        case CIPHER:
            if (write(fd, cipher, flen) != flen)
                fprintf(stderr, "Can't write file %s, exiting\n",
                    fstring), cleanup(1);
            break;

        case SMALLBUF:
            if (read(fd, smallbuf, flen) != flen)
                fprintf(stderr, "File read error at %s, exiting\n",
                fstring), cleanup(1);
            *(smallbuf + flen) = '\0';
            if (write(SCREEN, smallbuf, flen) != flen)
                fprintf(stderr, "Can't write to stdout, exiting\n",
                    fstring), cleanup(1);
            break;
    }

    if (fd > 2) {
        close(fd);
        fd = -1;
    }

    if (context != EDMIX || !unphase)
        unlock();

    if (plain != NULL)
        free(plain);
    if (cipher != NULL)
        free(cipher);
}

/* refill chain links with original key values */
rekey()
{

    register char *here;
    int regi;

    here = kstring + klen;
    for (regi = klen; --regi;)
        key[regi].val = *--here;
    key[0].val = *--here;
}

/*  mix:
   given key byte ASCII values "a b c ... n" in circularly linked list,
   and plaintext char pointer ptr initially at plaintext[0],

        advance ptr "a" bytes;
        install its value as the first byte of cyphertext;
        zero out plaintext byte pointed to by ptr;
        decrement value of "a";

        while (not done) {
            advance to next link of key;
            advance ptr by the value of the link;
            install ptr value as the next byte of cyphertext;
            zero out plaintext byte pointed to by ptr;
            decrement value of link;
        }

    Throughout, plaintext is assumed to be a circular list of bytes:
    ptr increments beyond the buffer continue from the beginning.
    If ptr value is null (byte already assigned to cyphertext), ptr is
    incremented until the next non-null plaintext byte.
    When key link value reaches zero, the just assigned plaintext byte
    is swapped into its position.
*/

mix()
{
    register struct keychar *K;
    register char *here, *cip;
    int regi;

    rekey();

    head = plain;
    here = head + flen -1;
    tail = here -1;
    loop = head - tail -1; /* always negative */
    cip = cipher;
    K = key;

    for (regi = flen -1; --regi >= 0; ) {
        if ((here = here + K->val) > tail)
            here += loop;
        while (!*here)
            if(++here > tail)
            here = head;
        if (!--K->val)
            K->val = *here;
        K = K->next;
        *cip++ = *here;
        *here = 0;
    }
    *cip = *++tail;
}

unmix()
{
    register struct keychar *K;
    register char *here, *cip;
    int regi;

    rekey();

    head = plain;
    K = key;
    here = head + K->val;
    tail = head + flen -2;
    loop = head - tail -1; /* always negative */
    cip = cipher;

    for (regi = flen -1; --regi >= 0; ) {
        while (*here)
            if(++here > tail)
            here = head;
        *here = *cip++;
        if (!--K->val)
            K->val = *here;
        K = K->next;
        if ((here = here + K->val) > tail)
            here += loop;
    }
    *(tail +1) = *cip;
}

