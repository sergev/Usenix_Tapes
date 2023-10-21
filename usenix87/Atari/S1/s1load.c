/*
        Program to build a binary file from S1 input file
*/

#include "stdio.h"
#include "ctype.h"

#define MEMSIZE 16384   /* size of part of memory used */
#ifdef OS9
#define BINARY "w"     /* open parameter for binary file */
#else
#define BINARY "wb"     /* open parameter for binary file */
#endif

char mem[MEMSIZE], line[530], *p, *q, x;
unsigned int check;

main(argc,argv)
int argc;
char **argv;
{
    int c, i, d, extadr = 0;
    FILE *fd, *ofd;
    unsigned int s1addr, s1len, s1data, s1end, v;
    unsigned int count = 0, base = 0xffff, roof = 0x0000;
    unsigned int low = 0x0000, high, offset = 0x0000;
    unsigned int lowaddr = 0x0000, highaddr = 0xffff;

    printf("\n");
    if (argc < 3)
    {
        printf("usage: s1load infile outfile ");
        printf("[lowaddr [highaddr [offset]]]\n");
        exit(1);
    }
    if ((ofd = fopen(argv[2], BINARY)) == NULL)
    {
        printf("can't open %s\n", argv[2]);
        exit(1);
    }
    if (argc > 2)
    {
        p = argv[3];
        low = lowaddr = gethex(4);
    }
    if (argc > 3)
    {
        p = argv[4];
        highaddr = gethex(4);
    }
    if (argc > 4)
    {
        p = argv[5];
        offset = gethex(4);
    }
    if ((high = low + MEMSIZE - 1) < low)
        high = 0xffff;
    for (c = 0; c < 530; line[c++] = 0);
reader:
    if ((fd = fopen(argv[1], "r")) == NULL)
    {
        printf("can't open %s\n", argv[1]);
        exit(1);
    }
    while(fgets(p = line, 530, fd) != NULL)
    {
        while((x = *p++) != 'S')
            if (x == '\n')
                goto nextloop;
        if ((d = *p++) == '9')
            break;
        if ((d < '1') || (d > '3'))
            continue;
        check = 0;
        s1len = gethex(2) - 3;
        if (d > '1')
        {
            extadr |= (i = (d - '1'));
            s1len -= i;
            c = gethex (i + i);
        }
        s1addr = gethex(4) + offset;
        for (q = p, i = 0; i <= s1len; ++i)
            c = gethex(2);
        if ((check &= 0xff) != 0xff)
        {
            if (low == lowaddr)
                printf("warning- checksum %02x in error:\n  %s",
                    check, line);
        }
        s1end = s1addr + s1len - 1;
        if (low == lowaddr)
        {
            printf(" s1addr %04x  s1len %02x\n", s1addr, s1len + 3);
            if (s1addr < base)
                base = s1addr;
            if (s1end > roof)
                roof = s1end;
        }
        if ((s1addr <= high) && (s1end >= low))
        {
            if (s1addr < low)
            {
                q += (2 * (low - s1addr));
                s1addr = low;
            }
            if (high < s1end)
                s1end = high;
            count += (s1end - s1addr + 1);
            for (p = q; s1addr <= s1end; ++s1addr)
                mem[s1addr - low] = gethex(2);
        }
nextloop:;
    }
    fclose(fd);
writer:
    if (((v = highaddr - low + 1) > MEMSIZE) || (!v))
        v = MEMSIZE;
    printf("writing addresses %04x thru %04x to disk\n", low, low + v - 1);
    for (i = 0; i < v; ++i)
        putc(mem[i], ofd);
    if (high < highaddr)
    {
        for (c = 0; c < 530; line[c++] = 0);
        v = low;
        low += MEMSIZE;
        if (low > v)
        {
            if ((high += MEMSIZE) < low)
                high = 0xffff;
            if (high > highaddr)
                high = highaddr;
            if (roof < low)
                goto writer;
            else
                goto reader;
        }
    }
    fclose(ofd);
    if (count)
        printf("\n%d bytes loaded\n", count);
    if ((base != 0xffff) || roof)
        printf(" input low address = %04x high address = %04x\n", base, roof);
    printf("output low address = %04x high address = %04x\n",
        lowaddr, highaddr);
    if (offset)
        printf(" address offset = %04x\n", offset);
    if (extadr)
    {
        if (extadr & 2)
            printf(" S3 addresses truncated to low-order 16 bits\n");
        if (extadr & 1)
            printf(" S2 addresses truncated to low-order 16 bits\n");
    }
    exit(0);
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
    check += ((i & 0xff) + (i >> 8));
    return (i);
}

