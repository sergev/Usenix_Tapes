/*
        Program converts straight binary file to S1 format
*/

#include "stdio.h"
#include "ctype.h"

#define BINARY "rb"     /* open parameter for binary file */
#define SIZE   32       /* number of bytes per line for S1 format */

char *p;

main (argc,argv)
int argc;
char **argv;
{
    char buf[SIZE + 1];
    FILE *fd, *ofd;
    unsigned int lowaddr = 0x0000, highaddr = 0xffff, base, z;
    unsigned int offset = 0x0000, check, i, low, m, n = SIZE;

    printf ("\n");
    if (argc < 3)
    {
        printf ("usage: s1unload infile outfile ");
        printf ("[lowaddr [highaddr [offset]]]\n");
        exit (1);
    }
    if ((fd = fopen (argv[1], BINARY)) == NULL)
    {
        printf ("can't open %s\n", argv[1]);
        exit (1);
    }
    if ((ofd = fopen (argv[2], "w")) == NULL)
    {
        printf ("can't open %s\n", argv[2]);
        exit (1);
    }
    if (argc > 2)
    {
        p = argv[3];
        lowaddr = gethex (4);
    }
    if (argc > 3)
    {
        p = argv[4];
        highaddr = gethex (4);
    }
    if (argc > 4)
    {
        p = argv[5];
        offset = gethex (4);
    }
    if (highaddr >= lowaddr)
    {
        base = low = lowaddr + offset;
        m = (highaddr - lowaddr);
        while (z = fread (buf, 1, SIZE, fd))
        {
            if ((m < SIZE) || (z < SIZE))
                n = (((m + 1) < z) ? (m + 1) : z);
            fprintf (ofd, "S1");
            check = putbyte (ofd, n + 3);
            check += putbyte (ofd, base >> 8);
            check += putbyte (ofd, base);
            base += n;
            for (i = 0; i < n; i++)
                check += putbyte (ofd, buf[i]);
            check = putbyte (ofd, (-1 - check));
            fprintf (ofd, "\n");
            if ((m < SIZE) || (z < SIZE))
                break;
            m -= SIZE;
        }
    }
    fprintf (ofd, "S9\n");
    fclose (fd);
    fclose (ofd);
    if (low != base)
    {
        printf ("low address = %04x high address = %04x\n", low, (base - 1));
        if (offset)
            printf (" address offset = %04x\n", offset);
    }
    exit (0);
}

putbyte (ofd, b)
FILE *ofd;
int b;
{
    fprintf (ofd, "%02x", (b &= 0xff));
    return (b);
}

gethex(n)
int n;
{
    char c, j = 0;
    unsigned int i = 0x0000;

    while (++j <= n)
    {
        if ((c = *p & 0x7f) < ' ')
            break;
        ++p;
        if ((c >= '0') && (c <= '9'))
            i = (i << 4) | (c - '0');
        else
            if (((c &= 0x5f) >= 'A') && (c <= 'F'))
                i = (i << 4) | (c - 'A' + 10);
    }
    return (i);
}

