
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
