/*
 * Graphics implementation on top of X
 */

#include <InterViews/bitmap.h>
#include <InterViews/painter.h>
#include <InterViews/world.h>
#include <InterViews/Xcolor.h>
#include <InterViews/Xfont.h>
#include <InterViews/Xoutput.h>
#include <InterViews/Ximage.h>
#include <string.h>

extern void bcopy(void*, void*, int);

extern int _World_nplanes;

Painter* stdpaint;
Color* black;
Color* white;
Pattern* solid;
Pattern* clear;
Pattern* lightgray;
Pattern* gray;
Pattern* darkgray;
Brush* single;
Font* stdfont;

static int spat[] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

static int cpat[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#ifdef X10

Color::Color (int r, int g, int b) {
    XColor c;

    c.red = r;
    c.green = g;
    c.blue = b;
    if (XGetHardwareColor(&c)) {
	pixel = c.pixel;
	red = c.red;
	green = c.green;
	blue = c.blue;
    } else {
	pixel = -1;
    }
}

Color::Color (int index, int r, int g, int b) {
    red = r;
    green = g;
    blue = b;
    pixel = index;
}

Color::Color (int index) {
    pixel = index;
}

Color::Color (const char* name) {
    XColor closest, exact;

    if (_World_nplanes == 1) {
	if (strcmp(name, "white") == 0) {
	    pixel = white->pixel;
	    red = 255; blue = 255; green = 255;
	} else {
	    pixel = black->pixel;
	    red = 0; green = 0; blue = 0;
	}
    } else if (XGetColor(name, &closest, &exact)) {
	pixel = closest.pixel;
	red = closest.red;
	green = closest.green;
	blue = closest.blue;
    } else {
	pixel = -1;
    }
}

Color::~Color () {
    /* nothing to do */
}

boolean Color::Valid () {
    return pixel != -1;
}

Pattern::Pattern (int p[patternHeight]) {
    short bp[patternHeight];
    register int* src;
    register short* dst;
    register int i;

    src = p;
    dst = bp;
    for (i = 0; i < patternHeight; i++) {
#	ifndef noswap /* X orders bits wrong */
	    register int j;
	    register unsigned s1, s2;

	    *dst = 0;
	    s1 = 1;
	    s2 = 1 << (patternWidth-1);
	    for (j = 0; j < patternWidth; j++) {
		if ((s1 & *src) != 0) {
		    *dst |= s2;
		}
		s1 <<= 1;
		s2 >>= 1;
	    }
#	else
	    *dst = *src;
#	endif
	++src;
	++dst;
    }
    info = XStoreBitmap(patternWidth, patternHeight, bp);
}

Pattern::Pattern (int dither) {
    register int i, seed;
    int r[4];
    short p[patternHeight];

#   ifndef noswap
	register unsigned s1, s2;

	s1 = 1;
	s2 = 1 << (patternHeight-1);
	seed = 0;
	for (i = 0; i < patternWidth; i++) {
	    if ((s1 & dither) != 0) {
		seed |= s2;
	    }
	    s1 <<= 1;
	    s2 >>= 1;
	}
#   else
	seed = dither;
#   endif
    for (i = 0; i < 4; i++) {
	r[i] = seed & 0xf;
	r[i] |= r[i] << 4;
	r[i] |= r[i] << 8;
	r[i] |= r[i] << 16;
	seed >>= 4;
    }
    for (i = 0; i < patternHeight; i++) {
	p[i] = r[i%4];
    }
    info = XStoreBitmap(patternWidth, patternHeight, p);
}

Pattern::Pattern (Bitmap*) {
    /* unimplemented */
}

Pattern::~Pattern () {
    XFreeBitmap(info);
}

Brush::Brush (int p, int w) {
    width = w;
    info = (void*)(XMakePattern(p, 16, 1));
}

Brush::~Brush () {
    /* nothing to do */
}

Font::Font (const char* name) {
    id = XGetFont(name);
    Init();
}

Font::Font (const char* name, int len) {
    char tmp[256];

    strncpy(tmp, name, len);
    tmp[len] = '\0';
    id = XGetFont(tmp);
    Init();
}

void Font::Init () {
    register XFontInfo* i;

    if (id != nil) {
	i = new XFontInfo;
	XQueryFont(id, *i);
	height = i->height;
	if (i->fixedwidth) {
	    i->widths = nil;
	} else {
	    i->widths = XFontWidths(id);
	}
	info = i;
    } else {
	height = -1;
    }
}

Font::~Font () {
    register XFontInfo* i = (XFontInfo*) info;

    if (id != nil) {
	XFreeFont(id);
	if (i->widths != nil) {
	    delete i->widths;
	}
	delete i;
    }
}

int Font::Baseline () {
    register XFontInfo* i = (XFontInfo*) info;

    return i->baseline;
}

int Font::Height () {
    return height;
}

inline int CharWidth (register XFontInfo* info, char c) {
    register int i;

    i = c;
    if (i >= info->firstchar && i <= info->lastchar) {
#	if vax
	    if (_World_nplanes == 1) {
		/* X10 QVSS bug -- widths are 1 off */
		--i;
	    }
#	endif
	i = info->widths[i - info->firstchar];
    } else {
	i = 0;
    }
    return i;
}

int Font::Width (const char* s) {
    register XFontInfo* i = (XFontInfo*) info;
    register char* p;
    register int w;

    if (i->fixedwidth) {
	return strlen(s) * i->width;
    } else {
	w = 0;
	for (p = (char*) s; *p != '\0'; p++) {
	    w += CharWidth(i, *p);
	}
	return w;
    }
}

int Font::Width (const char* s, int len) {
    register XFontInfo* i = (XFontInfo*) info;
    register char* p;
    register int w, n;

    if (i->fixedwidth) {
	return len * i->width;
    } else {
	w = 0;
	for (p = (char*) s, n = len; *p != '\0' && n > 0; p++, n--) {
	    w += CharWidth(i, *p);
	}
	return w;
    }
}

int Font::Index (const char* s, int offset, boolean between) {
    return Index(s, strlen(s), offset, between);
}

int Font::Index (const char* s, int len, int offset, boolean between) {
    register XFontInfo* i = (XFontInfo*) info;
    register int n, cw, coff;

    if (offset < 0 || *s == '\0' || len == 0) {
	return 0;
    }
    if (i->fixedwidth) {
	cw = i->width;
	n = offset / cw;
	coff = offset % cw;
    } else {
	register int w = 0;
	register const char* p;
	for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
	    cw = CharWidth(i, *p);
	    w += cw;
	    if (w > offset) {
		break;
	    }
	}
	coff = offset - w + cw;
    }
    if (between && coff > cw/2) {
	++n;
    }
    return min(n, len);
}

boolean Font::Valid () {
    return height != -1;
}

boolean Font::FixedWidth () {
    register XFontInfo* i = (XFontInfo*) info;

    return i->fixedwidth;
}

void InitPaint (World* w) {
    char* stdfontname;

    black = new Color(0);
    white = new Color(1);
    stdfontname = w->GetGlobalDefault("InterViews.stdfont");
    if (stdfontname == nil) {
	stdfontname = "6x13p";
    }
    stdfont = new Font(stdfontname);
    solid = new Pattern(spat);
    clear = new Pattern(cpat);
    lightgray = new Pattern(0x8020);
    gray = new Pattern(0xa5a5);
    darkgray = new Pattern(0xfafa);
    single = new Brush(0xffff);
    stdpaint = new Painter;
}

#endif

#ifdef X11

Color::Color (int r, int g, int b) {
    XColor c;

    c.red = r;
    c.green = g;
    c.blue = b;
    if (XAllocColor(_World_dpy, _World_cmap, &c)) {
	pixel = c.pixel;
	red = c.red;
	green = c.green;
	blue = c.blue;
    } else {
	pixel = -1;
    }
}

Color::Color (int index, int r, int g, int b) {
    pixel = index;
    red = r;
    green = g;
    blue = b;
}

Color::Color (int index) {
    XColor def;

    def.pixel = index;
    if (XQueryColor(_World_dpy, _World_cmap, def)) {
	pixel = index;
	red = def.red;
	green = def.green;
	blue = def.blue;
    } else {
	pixel = -1;
    }
}

/*
 * For some reason, X requires special casing for black and white
 * (at least this is true on one-plane systems).
 */

Color::Color (const char* name) {
    XColor c, exact;
    int ok;

    if (strcmp(name, "black") == 0) {
	c.pixel = XBlackPixel(_World_dpy, _World_screen);
	ok = XQueryColor(_World_dpy, _World_cmap, c);
    } else if (strcmp(name, "white") == 0) {
	c.pixel = XWhitePixel(_World_dpy, _World_screen);
	ok = XQueryColor(_World_dpy, _World_cmap, c);
    } else {
	ok = XLookupColor(_World_dpy, _World_cmap, name, c, exact);
    }
    if (ok) {
	pixel = c.pixel;
	red = c.red;
	green = c.green;
	blue = c.blue;
    } else {
	pixel = -1;
    }
}

Color::~Color () {
    /* nothing to do */
}

boolean Color::Valid () {
    return pixel != -1;
}

/*
 * Create a Pixmap for stippling from the given array of data.
 * The assumption is that the data is 16x16 and should be expanded to 32x32.
 */

static XPixmap MakeStipplePixmap (int* p) {
    XPixmap dst;
    Xgc g;
    register int i, j;
    register unsigned s1, s2;

    dst = XCreatePixmap(_World_dpy, _World_root, 32, 32, 1);
    g = XCreateGC(_World_dpy, dst, 0, nil);
    XSetForeground(_World_dpy, g, 0);
    XSetFillStyle(_World_dpy, g, FillSolid);
    XFillRectangle(_World_dpy, dst, g, 0, 0, 32, 32);
    XSetForeground(_World_dpy, g, 1);
    for (i = 0; i < 16; i++) {
	s1 = p[i];
	s2 = 1;
	for (j = 0; j < 16; j++) {
	    if ((s1 & s2) != 0) {
		XDrawPoint(_World_dpy, dst, g, 16-1-j, i);
		XDrawPoint(_World_dpy, dst, g, 16-1-j+16, i);
		XDrawPoint(_World_dpy, dst, g, 16-1-j, i+16);
		XDrawPoint(_World_dpy, dst, g, 16-1-j+16, i+16);
	    }
	    s2 <<= 1;
	}
    }
    XFreeGC(_World_dpy, g);
    return dst;
}

Pattern::Pattern (int p[patternHeight]) {
    info = MakeStipplePixmap(p);
}

Pattern::Pattern (int dither) {
    register int i, seed;
    int r[16];

    seed = dither;
    for (i = 0; i < 4; i++) {
	r[i] = seed & 0xf;
	r[i] |= r[i] << 4;
	r[i] |= r[i] << 8;
	r[i] |= r[i] << 16;
	seed >>= 4;
	r[i+4] = r[i];
	r[i+8] = r[i];
	r[i+12] = r[i];
    }
    info = MakeStipplePixmap(r);
}

Pattern::~Pattern () {
    XFreePixmap(_World_dpy, info);
}

Brush::Brush (int p, int w) {
    int r[16];
    register int i;

    width = w;
    for (i = 0; i < 16; i++) {
	r[i] = p;
    }
    info = MakeStipplePixmap(r);
}

Brush::~Brush () {
    XFreePixmap(_World_dpy, info);
}

Font::Font (const char* name) {
    info = XLoadQueryFont(_World_dpy, name);
    Init();
}

Font::Font (const char* name, int len) {
    char tmp[256];

    strncpy(tmp, name, len);
    info = XLoadQueryFont(_World_dpy, tmp);
    Init();
}

inline boolean IsFixedWidth (XFontStruct* i) {
    return i->min_bounds.width == i->max_bounds.width;
}

void Font::Init () {
    register XFontStruct* i = (XFontStruct*)info;
    if (i != nil) {
	id = i->fid;
	height = i->max_bounds.ascent + i->max_bounds.descent;
    } else {
	height = -1;
    }
}

Font::~Font () {
    if (info != nil) {
	XFreeFont(_World_dpy, (XFontStruct*)info);
    }
}

int Font::Baseline () {
    return 0;
}

int Font::Height () {
    return height;
}

int Font::Width (const char* s) {
    return XTextWidth((XFontStruct*)info, s, strlen(s));
}

int Font::Width (const char* s, int len) {
    register const char* p, * q;

    q = &s[len];
    for (p = s; *p != '\0' && p < q; p++);
    return XTextWidth((XFontStruct*)info, s, p - s);
}

int Font::Index (const char* s, int offset, boolean between) {
    return Index(s, strlen(s), offset, between);
}

int Font::Index (const char* s, int len, int offset, boolean between) {
    register XFontStruct* i = (XFontStruct*)info;
    register const char* p;
    register int n, w;
    int coff, cw;

    if (offset < 0 || *s == '\0' || len == 0) {
	return 0;
    }
    if (IsFixedWidth(i)) {
	cw = i->min_bounds.width;
	n = offset / cw;
	coff = offset % cw;
    } else {
	w = 0;
	for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
	    cw = XTextWidth(i, p, 1);
	    w += cw;
	    if (w > offset) {
		break;
	    }
	}
	coff = offset - w + cw;
    }
    if (between && coff > cw/2) {
	++n;
    }
    return min(n, len);
}

boolean Font::Valid () {
    return height != -1;
}

boolean Font::FixedWidth () {
    register XFontStruct* i = (XFontStruct*)info;
    return IsFixedWidth(i);
}

void InitPaint (World* w) {
    extern int atoi(const char*);
    char* v;

    black = new Color("black");
    white = new Color("white");
    v = w->GetGlobalDefault("InterViews.stdfont");
    stdfont = new Font(v == nil ? "fixed" : v);
    solid = new Pattern(spat);
    clear = new Pattern(cpat);
    lightgray = new Pattern(0x8020);
    gray = new Pattern(0xa5a5);
    darkgray = new Pattern(0xfafa);
    v = w->GetGlobalDefault("InterViews.linewidth");
    single = new Brush(0xffff, v == nil ? 0 : atoi(v));
    stdpaint = new Painter;
}

#endif
