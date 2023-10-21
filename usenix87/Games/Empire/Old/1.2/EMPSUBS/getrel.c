#define D_NATSTR
#include        "empdef.h"

getrel(c1, c2)
int     c1, c2;
{
        register        word, shift;

        if( getnat(c1) == -1 ) return(-1);
        word = c2;
        word >>= 3;
        shift = 14 - ((c2 % 7) << 1);
        return((nat.nat_relate[word]>>shift) & 03);
}

putrel(c1, c2, rel)
int     c1, c2, rel;
{
        register        word, shift;

        if( getnat(c1) == -1 ) return(-1);
        word = c2;
        word >>= 3;
        shift = 14 - ((c2 % 7) << 1);
        nat.nat_relate[word] &= ~(3<<shift);
        nat.nat_relate[word] |= rel<<shift;
        putnat(c1);
        return(0);
}
