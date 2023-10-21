/*
 * a demonstration of Text, Layout and TextBlock
 */

const char * PAGESFILE = "PAGESTEXT";
const char * DEBUG = "DEBUG";
const char * DEFAULTFILE = "pages.text";

#include <InterViews/Text/layout.h>
#include <InterViews/Text/text.h>
#include <InterViews/Text/textpainter.h>
#include <InterViews/world.h>
#include <InterViews/frame.h>
#include <InterViews/sensor.h>
#include <CC/string.h>
#include <CC/stdio.h>

#include "pages.h"

extern const char * getenv(const char *);
extern void exit(int);

extern Text* Build( FILE *, Layout * );

main( int argc, char *argv[] ) {
    World * world = new World(argv[0]);
    const char * textfile = DEFAULTFILE;
    if ( argc > 1 ) {
	textfile = argv[1];
    }
    if ( getenv(PAGESFILE) != nil ) {
	textfile = getenv(PAGESFILE);
    }
    FILE * f = fopen(textfile,"r");
    if ( f == nil ) {
	fprintf(stderr, "%s : can't open file \"%s\"\n", argv[0], textfile);
	exit(1);
    }

    Composition * text = new Composition(nil);
    TextPainter * stdtextpaint;
    if ( getenv(DEBUG) == nil ) {
	stdtextpaint = new TextPainter( stdpaint );
    } else {
	stdtextpaint = new TextPainter( stdpaint );
	stdtextpaint->Greyed();
    }
    Layout * layout = new Layout( text, nil, stdtextpaint );
    text->Build( Build(f,layout) );
    text->Reshape();

    Frame * frame = new Frame(stdpaint,	new Pages(layout, 4), 2);
    frame->SetName(argv[0]);
    world->Insert( frame );

    Event e;
    while ( true ) {
	world->Read(e);
	e.target->Handle(e);
    }
}
