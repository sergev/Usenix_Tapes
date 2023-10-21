/*
 * Bitmap - Bit map for InterViews
 */

#include <InterViews/bitmap.h>
#include <InterViews/transformer.h>
#include <InterViews/Xwindow.h>
#include <InterViews/Xoutput.h>
#include <InterViews/paint.h>
#include <InterViews/canvas.h>

extern void bcopy( void * src, void * dst, int count );
extern void bzero( void * dst, int count );

#ifdef X10
#   define DoFreePixmap(p) XFreePixmap(p)
#endif

#ifdef X11
#   define DoFreePixmap(p) XFreePixmap(_World_dpy, p)

#   define BitmapSize(w, h) \
	((w)*(h)*XBitmapUnit(_World_dpy) + XBitmapPad(_World_dpy) - 1) >> 3
#endif

Bitmap::Bitmap( short * d, int w, int h ) {
    Init();
    width = w;
    height = h;
    int size = BitmapSize( w, h );
    data = new short[ size/2 ];
    if ( d != nil ) {
	bcopy( d, data, size );
    } else {
	bzero( data, size );
    }
}

Bitmap::Bitmap( Bitmap * b ) {
    Init();
    width = b->width;
    height = b->height;
    if ( b->data != nil ) {
	int size = BitmapSize(width,height);
	data = new short[ size/2 ];
	bcopy( b->data, data, size );
    }
}

void Bitmap::Init() {
    x0 = 0;
    y0 = 0;
    width = 0;
    height = 0;
    data = nil;
    bitmap = nil;
    pixmap = nil;
    foreground = black;
    background = white;
}

Bitmap::~Bitmap() {
    FreeMaps();
    delete data;
}

int Bitmap::Width() { return width; }

int Bitmap::Height() { return height; }

void Bitmap::SetColors(Color * fg, Color * bg ) {
    if ( fg != foreground ) {
	if ( pixmap != nil ) {
	    DoFreePixmap(pixmap);
	    pixmap = nil;
	}
	foreground = fg;
    }
    if ( bg != background ) {
	if ( pixmap != nil ) {
	    DoFreePixmap(pixmap);
	    pixmap = nil;
	}
	background = bg;
    }
}

Color * Bitmap::GetFgColor() {
    return foreground;
}

Color * Bitmap::GetBgColor() {
    return background;
}

void Bitmap::Transform(Transformer * t) {

    int x1 = 0, y1 = 0;
    int x2 = 0, y2 = height-1;
    int x3 = width-1, y3 = height-1;
    int x4 = width-1, y4 = 0;

    t->Transform( x1, y1 );
    t->Transform( x2, y2 );
    t->Transform( x3, y3 );
    t->Transform( x4, y4 );

    t->Invert();

    int xmax = max(x1,max(x2,max(x3,x4)));
    int xmin = min(x1,min(x2,min(x3,x4)));
    int ymax = max(y1,max(y2,max(y3,y4)));
    int ymin = min(y1,min(y2,min(y3,y4)));

    Bitmap * b = new Bitmap( nil, xmax-xmin+1, ymax-ymin+1 );
    for ( int x = xmin; x <= xmax; ++x ) {
	for ( int y = ymin; y <= ymax; ++y ) {
	    int xx = x;
	    int yy = y;
	    t->Transform( xx, yy );
	    b->Poke( Peek(xx,yy), x-xmin, y-ymin );
	}
    }
    delete data;
    data = b->data;
    width = b->width;
    height = b->height;
    b->data = nil;
    delete b;
    FreeMaps();
}

void Bitmap::Scale(float sx, float sy) {
    Transformer * t = new Transformer;
    t->Scale(sx,sy);
    Transform(t);
    delete t;
}

void Bitmap::Rotate(float angle) {
    Transformer * t = new Transformer;
    t->Rotate(angle);
    Transform(t);
    delete t;
}

void Bitmap::SetOrigin(int x, int y) {
    x0 = x;
    y0 = y;
}

void Bitmap::GetOrigin(int &x, int &y ) {
    x = x0;
    y = y0;
}

void Bitmap::FlipHorizontal() {
    for ( int x = 0; x < width/2; ++x ) {
	for ( int y = 0; y < height; ++y ) {
	    boolean bit = Peek( x, y );
	    Poke( Peek( width-x-1, y ), x, y );
	    Poke( bit, width-x-1, y );
	}
    }
    FreeMaps();
}

void Bitmap::FlipVertical() {
    for ( int x = 0; x < width; ++x ) {
	for ( int y = 0; y < height/2; ++y ) {
	    boolean bit = Peek( x, y );
	    Poke( Peek( x, height-y-1 ), x, y );
	    Poke( bit, x, height-y-1 );
	}
    }
    FreeMaps();
}

void Bitmap::Invert() {
    for ( int x = 0; x < width; ++x ) {
	for ( int y = 0; y < height; ++y ) {
	    Poke( !Peek(x,y), x, y );
	}
    }
    FreeMaps();
}

void Bitmap::Rotate90() {
    Bitmap * b = new Bitmap( nil, height, width );
    for ( int x = 0; x < width; ++x ) {
	for ( int y = 0; y < height; ++y ) {
	    b->Poke( Peek(x,y), width-y-1, x );
	}
    }
    delete data;
    data = b->data;
    b->data = nil;
    width = b->width;
    height = b->height;
    delete b;
    FreeMaps();
}

boolean Bitmap::Peek( int x, int y ) {
    if ( Valid(x,y) ) {
	return (data[Index(x,y)] & Bit(x,y)) != 0;
    } else {
	return false;
    }
}

void Bitmap::Poke( boolean bit, int x, int y ) {
    if ( Valid(x,y) ) {
	if ( bit ) {
	    data[Index(x,y)] |= Bit(x,y);
	} else {
	    data[Index(x,y)] &= ~Bit(x,y);
	}
    }
}

#ifdef X10

extern int XReadBitmapFile(
    const char * filename,
    int& width, int& height, short*& data,
    int *x_hot, int *y_hot
);

Bitmap::Bitmap( const char * filename ) {
    Init();
    XReadBitmapFile(filename, width, height, data, nil, nil);
}

void Bitmap::FreeMaps() {
    if ( bitmap != nil ) {
	XFreeBitmap( bitmap );
	bitmap = nil;
    }
    if ( pixmap != nil ) {
	DoFreePixmap( pixmap );
	pixmap = nil;
    }
}

void Bitmap::Draw( Canvas * c ) {
    if ( bitmap == nil && data != nil ) {
	bitmap = XStoreBitmap( width, height, data );
    }
    if ( pixmap == nil && bitmap != nil ) {
	pixmap = XMakePixmap( bitmap,
	    foreground->PixelValue(), background->PixelValue()
	);
    }
    int xx = x0 - c->left;
    int yy = (c->height - 1) - (height - 1) - (y0 - c->bottom);
    if ( pixmap != nil ) {
	XPixmapPut(c->id,
	    0, 0, xx, yy, width, height,
	    pixmap, GXcopy, AllPlanes
	);
    } else {
	XPixSet(c->id, xx, yy, width, height, background->PixelValue());
	if ( bitmap != nil ) {
	    XPixFill(c->id,
		xx, yy, width, height, foreground->PixelValue(),
		bitmap, GXcopy, AllPlanes
	    );
	}
    }
}

#endif

#ifdef X11

extern int XReadBitmapFile(
    XDisplay *, XDrawable, const char *,
    int& width, int& height, XPixmap&,
    int * x_hot, int * y_hot
);

extern XPixmap XCreateBitmapFromData(
    XDisplay*, XDrawable, void *, int, int
);

Bitmap::Bitmap( const char * filename ) {
    Init();
    XReadBitmapFile(
	_World_dpy, _World_root, filename, width, height, pixmap, nil, nil
    );
}

void Bitmap::FreeMaps() {
    if ( pixmap != nil ) {
	DoFreePixmap( pixmap );
	pixmap = nil;
    }
}

void Bitmap::Draw( Canvas * c ) {
    if ( pixmap == nil && data != nil ) {
	pixmap = XCreateBitmapFromData(
	    _World_dpy, _World_root, data, width, height
	);
    }
    if ( pixmap != nil ) {
	int xx = x0;
	int yy = (c->height - 1) - (height - 1) - y0;
	Xgc gc = XCreateGC(_World_dpy, _World_root, 0, nil);
	XSetForeground(_World_dpy, gc, foreground->PixelValue());
	XSetBackground(_World_dpy, gc, background->PixelValue());
	XCopyArea(
	    _World_dpy, pixmap, c->id, gc, 0, 0, width, height, xx, yy
	);
	c->WaitForCopy();
    }
}

#endif
