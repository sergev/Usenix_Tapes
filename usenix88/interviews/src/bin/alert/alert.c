/*
 *  alert - displays a message in a dialog box at centre screen
 */

#include <InterViews/world.h>
#include <InterViews/frame.h>
#include <InterViews/sensor.h>
#include <InterViews/interactor.h>
#include <InterViews/shape.h>
#include <InterViews/painter.h>
#include <InterViews/box.h>
#include <InterViews/glue.h>
#include <InterViews/button.h>
#include <InterViews/paint.h>

#include <stdio.h>
#include <string.h>

extern char * index( const char *, char );

const char * DEFAULTLABEL = "OK, OK ...";

World * world;

class Message : public Interactor {
    const char* text;
public:
    Message (const char *s, Painter * p);
    void Redraw(Coord, Coord, Coord, Coord);
};

class Alert : public Frame {
    ButtonState * quit;
public:
    Alert( Painter *p, const char * label );
    void Delete();
    void Pop();
    void Redraw( Coord, Coord, Coord, Coord );
};

Message::Message(const char *s, Painter *p) : (nil, p) {
    text = s;
    shape->width = output->GetFont()->Width(s);
    shape->height = output->GetFont()->Height();
    shape->Rigid(0, hfil, 0, 0);
}

void Message::Redraw(Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->MoveTo(0, (ymax-shape->height)/2);
    output->Text(canvas, text);
}

Alert::Alert( Painter * p, const char * label ) : (p, nil, 4) {
    quit = new ButtonState( false );
    HBox * button = new HBox(
	new HGlue( output, 0, 0, hfil ),
	new PushButton( label, quit, true, output ),
    );

    VBox * messages = new VBox;
    messages->Insert( new VGlue( output, 0, 0, 200 ) );
    char buffer[ 256 ];
    while ( fgets(buffer, 255, stdin) != nil ) {
	char * nl = index(buffer,'\n');
	if ( nl != nil ) {
	    *nl = '\0';
	}
	char * string = new char[ strlen(buffer) + 1 ];
	strcpy( string, buffer );
	messages->Insert( new Message( string, output ) );
    }
    messages->Insert( new VGlue( output, 20, 0, 200 ) );
    messages->Insert( button );

    HBox * text = new HBox(
	new HGlue( output, 40, 0, 0 ),
	messages,
	new HGlue( output, 40, 0, 0 )
    );
    VBox * contents = new VBox;
    contents->Insert( new VGlue( output, 20, 0, 100 ) );
    contents->Insert( text );
    contents->Insert( new VGlue( output, 20, 0, 100 ) );

    Insert( contents );
}

void Alert::Delete () {
    delete quit;
}

void Alert::Redraw( Coord x1, Coord y1, Coord x2, Coord y2 ) {
    MonoScene::Redraw(x1, y1, x2, y2);
    output->Rect(canvas, 0, 0, xmax, ymax );
    output->Rect(canvas, 2, 2, xmax-2, ymax-2 );
    output->Rect(canvas, 3, 3, xmax-3, ymax-3 );
}

void Alert::Pop() {
    
    world->Insert( this,
	(world->Width() - shape->width) / 2,
	(world->Height() - shape->height) / 2
    );
    world->RingBell(0);

    Event e;
    int done;
    while ( quit->GetValue(done), done != true ) {
	world->Read( e );
	e.target->Handle(e);
    }

    world->Remove( this );
}
	
main( int argc, char * argv[] ) {
    const char * font = nil;
    const char * label = DEFAULTLABEL;
    for (int i = 1; i < argc; ++i) {
	char c;
	char * arg = argv[i];
	if (sscanf(arg, "font=%c", &c) == 1 ) {
	    font = arg+5;
	} else if (sscanf(arg, "button=%c", &c) == 1 ) {
	    label = arg+7;
	} else if (arg[0] == '-') {
	    ; // ignore for now
	} else {
	    fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], arg);
	    fprintf(stderr,
		"usage: %s [font=name] [button=label]\n", argv[0]
	    );
	    exit( 1 );
	}
    }
    world = new World;
    Painter * painter = new Painter( stdpaint );
    Font * textfont = font == nil ? stdfont : new Font( font );
    if (textfont->Valid()) {
	painter->SetFont( textfont );
    }
    Alert * alert = new Alert( painter, label );
    alert->Pop();
    exit(0);
}
