/*
 * mailbox
 * $Header: /lurch/interviews/src/bin/mailbox/RCS/mailbox.c,v 1.20 88/01/20 10:23:34 craig Exp $
 */

#include <InterViews/world.h>
#include <InterViews/sensor.h>
#include <InterViews/interactor.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>
#include <InterViews/frame.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/glue.h>

#include <CC/string.h>
#include <CC/stdio.h>
#include <CC/ctype.h>
#include <sys/param.h>
#include <sys/stat.h>

extern char * index( char *, char );
extern char * getenv( const char * );
extern int lstat(const char*, struct stat*);

World * world;
char mailpath[128];

const char * MAILPATH = "/usr/spool/mail/";
const char * DOMAIN = ".stanford";

struct Viewer {
    Viewer( Interactor * i ) { view = i; next = nil; }
    Interactor * view;
    Viewer * next;
};

class MailBox;

class MailView : public Interactor {
protected:
    MailBox * mailbox;
public:
    MailView( MailBox *, Painter * );
    void Delete();

    void Draw() { Redraw( 0, 0, xmax, ymax ); }
    void Update() { Draw(); }
};

class MailBeep : public MailView {
    int lastcount;
public:
    MailBeep( MailBox * m ) : ( m,nil ) { lastcount = 0; }
    void Update();
};

class MailText : public MailView {
public:
    MailText( MailBox * m, Painter * p ) : ( m,p ) { }
    void Redraw(Coord,Coord,Coord,Coord);
};

class MailFlag : public MailView {
    boolean showcount;
    int lastcount;
    Painter * highlight;
public:
    MailFlag( MailBox *, Painter *, boolean showcount=false );
    void Delete() { delete highlight; }
    void Update();
    void Redraw(Coord,Coord,Coord,Coord);
};

const int MaxItemCount = 100;
const int MaxItemSize = 100;

enum Status { All, New, Unread };

class MailBox : public Frame {
    Viewer * views;

    Pattern * gray;
    char * mailboxpath;
    int lastsize;
    Status status;
    int count;
    char *items[MaxItemCount];
public:
    MailBox(
	char * path, Font * font, int delay, Status stat,
	boolean john=false, boolean arturo=false, boolean craig=false,
	boolean showcount=false,
	int rows=0, int columns=0,
	int width=0, int height=0
    );
    void Delete();
    void Handle(Event&);

    char * GetItem(int i) { return items[i]; }
    int GetCount() { return count; }

    void Attach( Interactor * );
    void Detach( Interactor * );
    void Notify();

    void Run();
    void Tick();
    void Scan();
};

MailView::MailView( MailBox * m, Painter * p ) : ( nil, p ) {
    mailbox = m;
    m->Attach( this );
}

void MailView::Delete() {
     mailbox->Remove(this);
}

void MailText::Redraw(Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect( canvas, x1, y1, x2, y2 );
    int height = output->GetFont()->Height();
    Coord h;
    int i;
    for (
	i = 0, h = ((ymax+1) % height) / 2;
	i < mailbox->GetCount() && h <= (ymax+1)-height;
	++i, h += height
    ) {
	output->MoveTo( 0, h );
	output->Text( canvas, mailbox->GetItem(i) );
    }
}

void MailBeep::Update() {
    int count = mailbox->GetCount();
    if ( count > lastcount ) {
	world->RingBell( 0 );
	world->RingBell( 0 );
    }
    lastcount = count;
}

MailFlag::MailFlag( MailBox * m, Painter * p, boolean count ) : ( m, p ) {
    showcount = count;
    lastcount = 0;
    highlight = new Painter(output);
    highlight->SetColors( output->GetBgColor(), output->GetFgColor() );
    shape->width = p->GetFont()->Width("M") + 6;
    shape->height = p->GetFont()->Height();
    shape->Rigid( 0, vfil/1000, 0, vfil );
}

void MailFlag::Update() {
    int count = mailbox->GetCount();
    if ( count > lastcount ) {
	int i;
	const int bignum = 10000;
	lastcount = 0;
	Draw();
	Sync();
	for ( i=0; i<bignum; ++i ) { }
	lastcount = count;
	Draw();
	Sync();
	for ( i=0; i<bignum; ++i ) { }
	lastcount = 0;
	Draw();
	Sync();
	for ( i=0; i<bignum; ++i ) { }
	lastcount = count;
	Draw();
	Sync();
    } else {
	lastcount = count;
	Draw();
    }
}

void MailFlag::Redraw(Coord x1, Coord y1, Coord x2, Coord y2 ) {
    Painter * p = (lastcount > 0) ? highlight : output;
    Font * f = p->GetFont();
    int height = f->Height();
    p->ClearRect( canvas, x1, y1, x2, y2 );
    if ( ymax+1 < height ) {
	;
    } else if ( showcount ) {
	char c;
	if ( mailbox->GetCount() <= 0 ) {
	    c = '-';
	} else if ( mailbox->GetCount() > 9 ) {
	    c = '*';
	} else {
	    c = '0' + mailbox->GetCount();
	}
	p->MoveTo( xmax/2 - f->Width("M")/2, ymax/2 - height/2 );
	p->Text( canvas, &c, 1 );
    } else if ( ymax+1 < 4*height ) {
	p->MoveTo( xmax/2 - f->Width("M")/2, ymax/2 - height/2 );
	p->Text( canvas, "M" );
    } else {
	p->MoveTo( xmax/2 - f->Width("M")/2, ymax/2 + height );
	p->Text( canvas, "M" );
	p->MoveTo( xmax/2 - f->Width("a")/2, ymax/2 );
	p->Text( canvas, "a" );
	p->MoveTo( xmax/2 - f->Width("i")/2, ymax/2 - height );
	p->Text( canvas, "i" );
	p->MoveTo( xmax/2 - f->Width("l")/2, ymax/2 - 2*height );
	p->Text( canvas, "l" );
    }
}

MailBox::MailBox(
    char * path, Font * font, int delay, Status stat,
    boolean john, boolean arturo, boolean craig, boolean showcount,
    int rows, int cols, int width, int height
) : ( new Painter(stdpaint), new Interactor, 2 ) {

    mailboxpath = path;
    status = stat;
    lastsize = 0;

    for ( int i=0; i<MaxItemCount; ++i ) {
	items[i] = nil;
    }

    count = 0;

    output->SetFont( font );
    gray = new Pattern( 0x5a5a );

    input = new Sensor(onoffEvents);
    input->CatchTimer( delay,0 );
    input->Catch( KeyEvent );

    views = nil;

    if ( !craig ) {
	(void) new MailBeep( this );
    }

    HBox * contents = new HBox;
    if ( !john ) {
	contents->Insert( new MailFlag( this, output, showcount ) );
    }
    if ( !john && !arturo ) {
	contents->Insert( new VBorder );
    }
    if ( !arturo ) {
	MailText * newtext = new MailText( this, output );
	if ( rows!=0 || cols!=0 ) {
	    newtext->shape->height = rows * output->GetFont()->Height();
	    newtext->shape->width = cols * output->GetFont()->Width("m");
	}
	contents->Insert( new HGlue( output, 2, 0, 0 ) );
	contents->Insert( new VBox(
	    new VGlue( output, 2, 0, 0 ),
	    newtext,
	    new VGlue( output, 2, 0, 0 )
	));
	contents->Insert( new HGlue( output, 2, 0, 0 ) );
    }

    Insert( contents );
    if ( width!=0 && height!=0 ) {
	shape->width = width;
	shape->height = height;
    }
    output->SetPattern( gray );
}

void MailBox::Handle(Event &e) {
    switch( e.eventType ) {
    case TimerEvent:
	Tick();
	break;
    case OnEvent:
	output->SetPattern( solid );
	Redraw( 0, 0, xmax, ymax );
	break;
    case OffEvent:
	output->SetPattern( gray );
	Redraw( 0, 0, xmax, ymax );
	break;
    case KeyEvent:
	if ( e.target == this && e.keystring[0] == 'q' ) {
	    e.target = nil;
	}
	break;
    default:
	if ( e.target != nil ) {
	    e.target->Handle(e);
	}
	break;
    }
}

void MailBox::Delete() {
    while ( views != nil ) {
	Viewer * doomed = views;
	delete views->view;
	views = views->next;
	delete doomed;
    }
    delete gray;
}

void MailBox::Attach( Interactor * i ) {
    Viewer * newViewer = new Viewer(i);
    newViewer->next = views;
    views = newViewer;
}

void MailBox::Detach( Interactor * i ) {
    Viewer * prev = nil;
    for ( Viewer * v = views; v != nil; v = v->next ) {
	if ( v->view == i ) {
	    if ( prev == nil ) {
		views = v->next;
	    } else {
		prev->next = v->next;
	    }
	    delete v;
	    break;
	} else {
	    prev = v;
	}
    }
}

void MailBox::Notify() {
    for ( Viewer * viewer = views; viewer!=nil; viewer = viewer->next ) {
	viewer->view->Update();
    }
}

void MailBox::Run() {
    Event e;
    for (;;) {
	Read(e);
	Handle(e);
	if ( e.target == nil ) {
	    break;
	}
    }
}

void MailBox::Tick() {
    struct stat statBuffer;
    if ( lstat(mailboxpath,&statBuffer) >= 0 ) {
	if ( statBuffer.st_size != lastsize ) {
	    count = 0;
	    Scan();
	    lastsize = statBuffer.st_size;
	    Notify();
	}
    } else if ( lastsize > 0 ) {
	count = 0;
	lastsize = 0;
	Notify();
    }
}

void MailBox::Scan() {
    FILE * f = fopen( mailboxpath, "r");
    char line[256];
    char info[256];
    char mail[256];
    while ( fgets( line, 255, f ) != 0 ) {
        if ( sscanf( line, "From %s", info ) > 0 ) {
	    if ( info[0] == ':' ) {
		continue;
	    }
	    char * ip = info;
	    while ( ip[0] == '<' || ip[0] == '(' ) {
		ip ++;
	    }
	    if ( ip[0] == '@' ) {
		ip ++;
		char * colonp = index ( ip, ':' );
		if ( colonp != nil ) {
		    ip = colonp + 1;		    // strip off prefix
		}
	    }
	    char * atp = index( ip, '@' );
	    if ( atp != nil ) {			    // has a host address
		char * dotp = index( atp, '.' );    // has a domain
		if ( dotp != nil ) {
		    for ( int i = 0; i < strlen(DOMAIN); ++i ) {
			char c = dotp[i];
			c = isupper(c) ? c - 'A' + 'a' : c;
			if ( c != DOMAIN[i] ) {
			    break;
			}
		    }
		    if ( i == strlen(DOMAIN) ) {
			*dotp = '\0';		    // matched, strip domain
		    }
		}
	    }
	    for ( char * end = & ip[strlen( ip ) - 1];
		    *end == '>' || *end == ')';
		    end -- ) {
		*end = '\0';
	    }
	    strcpy( mail, ip );
	} else if ( *mail != '\0' &&
		    sscanf( line, "Subject: %s", info ) > 0 ) {
	    char * p = index( line, '\n' );
	    if ( p != 0 ) {
		*p = '\0';
	    }
	    strcat( mail, " << " );
	    strcat( mail, line+9 );
	} else if ( *mail != '\0' &&
		    sscanf( line, "Status: %s", info ) > 0 ) {
	    switch ( status ) {
	    case New:
		if ( index(info, 'O') != nil ) {
		    *mail = '\0';
		}
		break;
	    case Unread:
		if ( index(info, 'R') != nil ) {
		    *mail = '\0';
		}
		break;
	    default:
		break;
	    }
        } else if ( *line == '\n' && *mail != '\0' ) {
	    delete items[MaxItemCount-1];
	    for ( int i=MaxItemCount-2; i>=0; --i ) {
		items[i+1] = items[i];
	    }
	    items[0] = new char[MaxItemSize];
	    strcpy( items[0], mail );
	    *mail = '\0';
	    ++count;
        }
    }
    fclose( f );
}

MailBox * mailbox;

int cols = 45;		    // size of the text part - cols
int rows = 4;		    // .. and lines
int width = 0;		    // initial width of window
int height = 0;		    // ... height
int xpos = 0;		    // initial position of lower left corner - x
int ypos = 0;		    // ... y
int delay = 60;		    // seconds between checks of mailbox
Status status = All;	    // which messages to display
boolean count = false;	    // display a count of the mail items in flag
boolean john = false;	    // John doesn't want the flag
boolean arturo = false;	    // Arturo doesn't want the text
boolean craig = false;	    // Craig doesn't want the bell

main( int argc, char *argv[] ) {
    world = new World();
    Font * font = stdfont;

    int p1, p2;
    char * curarg;
    char buffer[128];
    for (int i = 1; i < argc; i++) {
	curarg = argv[i];
	if (sscanf(curarg, "delay=%d", &p1) == 1) {
	    delay = p1;
	} else if (sscanf(curarg, "pos=%d,%d", &p1, &p2) == 2) {
	    xpos = p1; ypos = p2;
	} else if (sscanf(curarg, "size=%d,%d", &p1, &p2) == 2) {
	    width = p1; height = p2; rows=0; cols=0;
	} else if (sscanf(curarg, "rows=%d", &p1) == 1) {
	    rows = p1; height = 0; width = 0;
	} else if (sscanf(curarg, "cols=%d", &p1) == 1) {
	    cols = p1; height = 0; width = 0;
	} else if (sscanf(curarg, "mailbox=%s", mailpath ) == 1) {
	    /* nothing else to do */
	} else if (sscanf(curarg, "font=%s", buffer ) == 1) {
	    font = new Font(buffer);
	} else if (strcmp(curarg, "count") == 0) {
	    count = true;
	} else if (strcmp(curarg, "new") == 0) {
	    status = New;
	} else if (strcmp(curarg, "unread") == 0) {
	    status = Unread;
	} else if (strcmp(curarg, "all") == 0) {
	    status = All;
	} else if (strcmp(curarg, "noflag") == 0) {
	    john = true; arturo = false;
	} else if (strcmp(curarg, "notext") == 0) {
	    arturo = true; john = false;
	} else if (strcmp(curarg, "silent") == 0) {
	    craig = true;
	} else {
	    fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], curarg);
	    fprintf(stderr, "usage: %s %s %s %s %s\n",
		argv[0], "[pos=#,#] [size=#,#] [rows=#] [cols=#] [delay=#]",
		"[new] [unread] [all]",
		"[count] [silent] [noflag] [notext]",
		"[font=name] [mailbox=path]"
	    );
	    exit(1);
	}
    }

    if ( strlen( mailpath ) == 0 ) {
	strcpy( mailpath, MAILPATH );
	strcat( mailpath, getenv("USER") );
    }

    mailbox = new MailBox(
	mailpath, font, delay, status,
	john, arturo, craig, count,
	rows, cols, width, height
    );
    if ( xpos==0 && ypos==0 ) {
	world->Insert( mailbox );
    } else {
	world->Insert( mailbox, xpos, ypos );
    }

    mailbox->Tick();
    mailbox->Run();

    world->Remove( mailbox );
    delete mailbox;
    delete world;
}
