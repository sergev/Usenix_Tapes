#
struct bufr {
        int bu_fd;              /* UNIX file descriptor */
        int bu_nchars;          /* # bytes in buffer */
        char *bu_pos;           /* position in file */
        long bu_floc;           /* current position in file */
        long bu_uloc;           /* unix current position */
        int bu_flags;           /* flags about buffer */
#define MOD     1               /* set if buffer needs writing */
        char bu_buf[512];
};

int openb (name, buf, mode)
char *name;
int mode;
struct bufr *buf;
{
        int fd;
        if (mode & ~1) return (-1);
        fd = open (name, mode);
        if (fd >= 0) initb (fd, buf);
        return (fd);
}

int creatb (name, buf, mode)
char *name;
int mode;
struct bufr *buf;
{
        int fd;
        fd = creat (name, mode);
        if (fd >= 0)    initb (fd, buf);
        return (fd);
}

int initb (fd, buf)
struct bufr *buf;
{
        register struct bufr *bp;
        if (fd > 15 || fd < 0) return (-1);
        bp = buf;
        bp->bu_fd = fd;
        bp->bu_pos = &bp->bu_buf;
        bp->bu_floc = 0;
        bp->bu_uloc = 0;
        bp->bu_nchars = 0;
        bp->bu_flags = 0;
        return (0);
}

int putbc (chr, buf)
char chr;
struct bufr *buf;
{
        register struct bufr *bp;
        bp = buf;
        if (bp->bu_pos >= &bp->bu_buf[512] ||
          bp->bu_pos > &bp->bu_buf[bp->bu_nchars]) {
                flushb(bp);
                bp->bu_pos = &bp->bu_buf;
                bp->bu_nchars = 0;
        }
        *bp->bu_pos = chr;
        bp->bu_flags =| MOD;
        if (bp->bu_pos++ - &bp->bu_buf >= bp->bu_nchars) bp->bu_nchars++;
        return (1);
}

int getbc (buf)
struct bufr *buf;
{
        register struct bufr *bp;
        register int ct;
        bp = buf;
        if (bp->bu_pos >= bp->bu_buf + bp->bu_nchars) {
                flushb (bp);
                bp->bu_floc =+ bp->bu_nchars;
                chkseekb (bp);
                ct = read (bp->bu_fd, bp->bu_buf, 512);
                if (ct < 0) ct = 0;
                bp->bu_uloc =+ ct;
                bp->bu_nchars = ct;
                bp->bu_pos = &bp->bu_buf;
                if (ct == 0) return (-1);
        }
        return (*bp->bu_pos++ & 0377);
}

/*
        get word from file -- error return is ambiguous as
        -1 is also valid data return
*/
int getbw (buf)
struct bufr *buf;
{
        register int a, b;
        a = getbc (buf);
        if (a < 0) return a;
        b = getbc (buf);
        if (b < 0) return b;
        return a | b<<8;
}

flushb (buf)
struct bufr *buf;
{
        register struct bufr *bp;
        bp = buf;
        if (bp->bu_nchars > 0 && (bp->bu_flags&MOD)) {
                chkseekb (bp);
                write (bp->bu_fd, bp->bu_buf, bp->bu_nchars);
                bp->bu_flags =& ~MOD;
                bp->bu_pos = &bp->bu_buf;
                bp->bu_nchars = 0;
        }
}

closeb (buf)
struct bufr *buf;
{
        register struct bufr *bp;
        bp = buf;
        close (bp->bu_fd);
        bp->bu_fd = -1;
}

seekb (buf, adr)
struct bufr *buf;
long adr;
{
        register struct bufr *bp;
        bp = buf;
        if (adr >= bp->bu_floc &&
           adr < bp->bu_floc + bp->bu_nchars) {
                bp->bu_pos = bp->bu_buf + adr - bp->bu_floc;
                return;
        }
        flushb (bp);
        bp->bu_floc = adr;
        chkseekb (bp);
        bp->bu_pos = &bp->bu_buf;
        bp->bu_nchars = 0;
}

chkseekb (buf)
struct bufr *buf;
{
        register struct bufr *bp;
        bp = buf;
        if (bp->bu_floc != bp->bu_uloc) {
                lseek (bp->bu_fd, bp->bu_floc-bp->bu_uloc, 1);
                bp->bu_uloc = bp->bu_floc;
        }
}

long getlocb (buf)
struct bufr *buf;
{
        register struct bufr *bp;
        long t;
        bp = buf;
        t = bp->bu_floc;
        t =+ bp->bu_pos - &bp->bu_buf;
        return t;
}
