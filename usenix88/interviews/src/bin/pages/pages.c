/*
 * Pages - demo for Text and Layout
 */

#include "pages.h"

#include <InterViews/Text/layout.h>
#include <InterViews/Text/text.h>
#include <InterViews/Text/textpainter.h>

#include <InterViews/deck.h>
#include <InterViews/sensor.h>
#include <InterViews/paint.h>
#include <InterViews/scroller.h>
#include <InterViews/box.h>
#include <InterViews/border.h>
#include <InterViews/glue.h>
#include <InterViews/scene.h>

#include <ctype.h>

static const int WIDTH = 200;
static const int HEIGHT = 300;

Pages::Pages( Layout* l, int c, Painter * p ) : ( nil, p ) {
    hit = nil;
    after = nil;
    edit = nil;

    highlighter = new TextPainter( output );
    highlighter->Inverted();

    layout = l;
    deck = new Deck;
    count = c;

    for ( int i = 1; i <= count; ++i ) {
	TextBlock * leftblock = new TextBlock( WIDTH, HEIGHT, this );
	TextBlock * rightblock = new TextBlock( WIDTH, HEIGHT, this );
	layout->Chain( leftblock );
	layout->Chain( rightblock );

	HBox * text = new HBox;
	text->Insert( new HGlue(stdpaint,5,0) );
	text->Insert( leftblock );
	text->Insert( new HGlue(stdpaint,5,0) );
	text->Insert( new VBorder );
	text->Insert( new HGlue(stdpaint,5,0) );
	text->Insert( rightblock );
	text->Insert( new HGlue(stdpaint,5,0) );

	VBox * page = new VBox(
	    new VGlue(stdpaint,10,0),
	    text,
	    new VGlue(stdpaint,10,0)
	);
	deck->Insert( page );
    }

    Insert( new VBox( deck, new HBorder, new HScroller(deck) ) );

    sensor = new Sensor;
    sensor->Catch( DownEvent );
    sensor->Catch( KeyEvent );
    layout->Listen(sensor);
}

Pages::~Pages() {
    delete highlighter;
    delete sensor;
}

void Pages::Handle(Event& e) {
    Text * newhit;
    Text * newafter;
    Word * w;
    if ( e.eventType == KeyEvent ) {
	if ( e.len > 0 && hit != nil ) {
	    switch (e.keystring[0]) {
	    case '\002':
		if ( edit != nil ) {
		    edit->Backward();
		}
		break;
	    case '\006':
		if ( edit != nil ) {
		    edit->Forward();
		}
		break;
	    case '\010':
	    case '\177':
	    case '\004':
	    case '\027':
		if ( edit != nil ) {
		    edit->Delete();
		    edit->Parent()->Reshape();
		} else if ( hit != nil ) {
		    Composition * parent = hit->Parent();
		    if ( parent != nil ) {
			layout->Unpaint(hit);
			parent->Remove(hit);
			delete hit;
			hit = nil;
			layout->Show(parent);
		    }
		}
		break;
	    case ' ':
	    case '\n':
	    case '\r':
	    case '\t':
		w = new Word("",0);
		if ( edit == nil ) {
		    edit = new EditWord(editbuffer,255);
		    layout->Unpaint(hit);
		} else {
		    edit->Done();
		}
		hit->Parent()->InsertAfter(hit,w);
		hit = w;
		edit->Edit(w);
		break;
	    default:
		if ( isgraph(e.keystring[0]) && edit != nil ) {
		    edit->Insert( e.keystring, e.len );
		    edit->Parent()->Reshape();
		}
		break;
	    }
	    if ( edit != nil ) {
		layout->Show(edit->Parent());
	    }
	}
    } else if (e.eventType==DownEvent && layout->Hit(e,newhit,newafter)) {
	if ( edit != nil ) {
	    edit->Done();
	    if ( newhit == edit ) {
		newhit = hit;
		layout->Paint( hit, highlighter );
	    }
	    delete edit;
	    edit = nil;
	    layout->Show(hit);
	}
	if ( newhit != hit ) {
	    if ( hit != nil ) {
		layout->Unpaint( hit );
		layout->Show( hit );
	    }
	    if ( newhit != nil ) {
		layout->Paint( newhit, highlighter );
		layout->Show( newhit );
	    }
	    hit = newhit;
	    after = newafter;
	}
    } else {
	e.target->Handle(e);
    }
}

