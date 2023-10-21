/* stripped-down version of Paul Calder's grayscale class */

#include <InterViews/paint.h>

extern int vdi_AssocPattern(Pattern*);

const int NSEEDS = 17;

const int GraySeed[NSEEDS] = {
	0x0000, 0x8000, 0x8020, 0xA020, 0xA0A0, 0xA4A0, 0xA4A1, 0xA5A1,
	0xA5A5, 0xA5B5, 0xE5B5, 0xF5B5, 0xF5F5, 0xF5F7, 0xFDF7, 0xFFF7,
	0xFFFF
};

Pattern* MakePattern (int seedno) {
    Pattern * pat;
    int dat[patternHeight];
    unsigned int Row[4];
    int i, seed;

    seed = GraySeed[seedno];
    for (i=0; i<=3; i++) {
	    Row[i] = seed & 0xF;
	    Row[i] |= Row[i]<<4;
	    Row[i] |= Row[i]<<8;
	    Row[i] |= Row[i]<<16;
	    seed >>= 4;
    }
    for (i=0; i <= patternHeight-1; i++) {
	    dat[i] = Row[i%4];
    }
    pat = new Pattern(dat);
    return (Pattern*) vdi_AssocPattern(pat);
}

void grays__init() {}
