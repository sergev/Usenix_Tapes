/*
 * digital clockface class
 */

#include <string.h>

#include "dclock.h"
#include "dface.h"
#include "digit.h"
#include "clocktime.h"

const int Frame = 2;
const int FadeDelay = 10000;

void DFace::DrawFace() {
    output->ClearRect(canvas, Frame, Frame, xmax-Frame, ymax-Frame);
}

void DFace::DrawFrame() {
    Painter * p;
    if (selected) {
	p = output;
    } else {
	p = grayer;
    }
    p->FillRect(canvas, 0, 0, Frame-1, ymax-Frame);
    p->FillRect(canvas, 0, ymax-Frame+1, xmax-Frame, ymax);
    p->FillRect(canvas, xmax-Frame+1, Frame, xmax, ymax);
    p->FillRect(canvas, Frame, 0, xmax, Frame-1);
}

void DFace::DrawColon() {
    output->FillPolygon( canvas, colon[0].x, colon[0].y, colon[0].count );
    output->FillPolygon( canvas, colon[1].x, colon[1].y, colon[1].count );
}

void DFace::DrawAMPM(Painter *painter) {
    if ( AMPMmode != BLANK ) {
	if ( AMPMmode == AM ) {
	    painter->FillPolygon( canvas, A.x, A.y, A.count );
	} else {
	    painter->FillPolygon( canvas, P.x, P.y, P.count );
	}
	painter->FillPolygon( canvas, M.x, M.y, M.count );
    }
}

void DFace :: DrawDate ()
{
    if (showDate && date.len != 0) {
	Painter *painter = output;
	Coord dateYPos = ymax - font->Height() - Frame;
	Coord dateXPos = Frame + 2;

	int availWidth = xmax - Frame*2 - dateXPos - 2;
	int dayWidth = font->Width( date.text, 3 );
	int dayDateWidth = font->Width( date.text, 10 );
	int wholeWidth = font->Width( date.text, date.len );

	painter->ClearRect(
	    canvas,
	    Frame, ymax - font->Height() - Frame,
	    xmax-Frame, ymax - Frame
	);
	painter->MoveTo( dateXPos, dateYPos );
	if ( wholeWidth < availWidth ) {
	    painter->Text( canvas, date.text, date.len );	// whole
	} else if ( dayDateWidth < availWidth ) {
	    painter->Text( canvas, date.text, 10 );		// day & date
	} else if ( dayWidth < availWidth ) {
	    painter->Text( canvas, date.text, 3 );		// day only
	} else {
	    ;						// nothing
	}
    }
}

void DFace :: DrawBorder ()
{
    if (showDate && showTime) {
	int ypos = ymax - font->Height() - 2;
	output->Line(canvas, 1, ypos, xmax-1, ypos);
    }
}

void DFace :: Tick() {
    int nextTick = clock->NextTick();
    if (nextTick > 0) {
	input->CatchTimer(nextTick + 1, 0); // this extra second makes sure the
    } else {				    // timeout is after the next minute
	int h, m, s;
	char date[50];
	clock->GetTime( date, h, m, s );
	Set( date, h, m );
    }
}

void DFace::Set( char *today, int hours, int minutes ) {
    int h = hours;
    if (mode==CIVIL) {
	if ( hours > 12) {
	    h -= 12;
	} else if ( hours == 0 ) {
	    h = 12;				// midnight is 12:xx
	}
    }

    Event e;
    Sensor * wait = new Sensor;
    wait->CatchTimer(0,0);
    Listen( wait );
    int lasttime;
    int fade = FadeDelay * (1 << (min(4,max(0,FadeRate))) );

    boolean done = false;
    while (showTime && ! done) {
	Read(e);
	lasttime = e.timestamp;

	done=true;

	if ( (mode==CIVIL) && (h<10) ) {
	    done &= ht->Set( -1 );		// blank digit
	} else {
	    done &= ht->Set( h/10 );
	}
	done &= hu->Set( h%10 );
	done &= mt->Set( minutes/10 );
	done &= mu->Set( minutes%10 );

	wait->CatchTimer( 0, 0 );
	Read(e);
	wait->CatchTimer( 0, lasttime+fade - e.timestamp );
    }

    Listen(input);
    delete wait;

    if ( showTime && mode==CIVIL ) {
	AMPMMODE newAMPM = (hours>=12) ? PM : AM;

	if ( AMPMmode == BLANK ) {
	    AMPMmode = newAMPM;
	    DrawAMPM(output);
	} else if ( AMPMmode != newAMPM ) {
	    DrawAMPM(invertor);				// erase old
	    AMPMmode = newAMPM;
	    DrawAMPM(output);				// draw new
	}
    }
    if (showDate && strcmp(date.text, today) != 0) {
	strcpy(date.text, today);
	date.len = strlen(today);
	DrawDate();
	if (showBorder) {
	    DrawBorder();
	}
    }
}

DFace::DFace(
    boolean showDate,
    boolean showBorder,
    boolean showTime,
    TMode timeMode,
    boolean inverted,
    int width, int height
) : (nil, new Painter)
{
    clock = new Clock();

    mode = timeMode;
    AMPMmode = BLANK;
    A.count = AData.count;
    P.count = PData.count;
    M.count = MData.count;
    colon[0].count = ColonData[0].count;
    colon[1].count = ColonData[1].count;

    invertor = new Painter(output);
    grayer = new Painter(output);
    grayer->SetPattern( new Pattern( 0x5a5a ) );
    if (inverted) {
	output->SetColors(output->GetBgColor(), output->GetFgColor());
    } else {
	invertor->SetColors(invertor->GetBgColor(), invertor->GetFgColor());
    }

    ht = new Digit( HTx, ALLy, output );
    hu = new Digit( HUx, ALLy, output );
    mt = new Digit( MTx, ALLy, output );
    mu = new Digit( MUx, ALLy, output );

    input = new Sensor(onoffEvents);
    input->CatchTimer(0, 0);
    input->Catch(KeyEvent);

    font = output->GetFont();
    this->showDate = showDate;
    this->showBorder = showBorder;
    this->showTime = showTime;
    selected = false;
    done = false;

    date.len = 0;

    shape->Rect(width, height);
    shape->Rigid(hfil, hfil, vfil, vfil);
}

void DFace::Delete() {
    delete clock;
    delete ht;
    delete hu;
    delete mt;
    delete mu;
    delete invertor;
}

void DFace::Resize() {
    int w = xmax;
    int h = ymax;
    if (showDate) {
	h -= font->Height();		// adjust vertical size for date
    }

    for ( int i=0; i<colon[0].count; i++ ) {		// resize colon
	colon[0].x[i] = Coord(ColonData[0].x[i] * w );
	colon[0].y[i] = Coord(ColonData[0].y[i] * h );
	colon[1].x[i] = Coord(ColonData[1].x[i] * w );
	colon[1].y[i] = Coord(ColonData[1].y[i] * h );
    }
    for ( i=0; i<12; i++ ) {				// resize AM/PM
	A.x[i] = Coord( AData.x[i] * w );
	A.y[i] = Coord( AData.y[i] * h );
	P.x[i] = Coord( PData.x[i] * w );
	P.y[i] = Coord( PData.y[i] * h );
	M.x[i] = Coord( MData.x[i] * w );
	M.y[i] = Coord( MData.y[i] * h );
    }

    if (showTime) {
	ht->Resize(canvas, h);
	hu->Resize(canvas, h);
	mt->Resize(canvas, h);
	mu->Resize(canvas, h);
    }
}

void DFace::Redraw(Coord left, Coord bottom, Coord right, Coord top)
{
    output->Clip(canvas, left, bottom, right, top);
    Draw();
    output->NoClip();
}

void DFace::Draw()
{
    DrawFace();
    DrawFrame();
    if (showDate) {
	DrawDate();
    }
    if (showTime) {
	DrawColon();
	DrawAMPM(output);
	ht->Redraw();
	hu->Redraw();
	mt->Redraw();
	mu->Redraw();
    }
    if (showBorder) {
	DrawBorder();
    }
}

void DFace :: Handle (Event &event)
{
    switch(event.eventType) {
	case OnEvent:
	    selected = true;
	    DrawFrame();
	    break;
	case OffEvent:
	    selected = false;
	    DrawFrame();
	    break;
	case KeyEvent:
	    if (event.len > 0 && event.keystring[0] == 'q') {
		done = true;
	    }
	    break;
	case TimerEvent:
	    Tick();
	    break;
    }
}

void DFace :: Run ()
{
    while (!done) {
	Event event;

	Read(event);
	Handle(event);
	Tick();
    }
}
