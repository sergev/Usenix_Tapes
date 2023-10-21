/*
 * Segment class for digital clock
 */

#ifndef segment_h
#define segment_h

enum Seg { SegA, SegB, SegC, SegD, SegE, SegF, SegG };

class Segment {
    SegPoints p;			// polygon points for this segment
    Seg whichSeg;			// for indexing into segment data array
    float Xorg;				// fractional amount from canvas origin
    float Yorg;
    Canvas *canvas;			// canvas of DFace
    int fullFade;
    int fade;				// 0 = off; fullFade = on
    static Painter *fadePainter[17];	// painters for fading

    Pattern *MakePattern(int seed);
    void InitPainters(Painter *seedPainter);

    void FadeUp() {
	if ( fade < fullFade ) {
	    fade += FadeStep;
	    this->Draw();
	}
    }
    void FadeDown() {
	if ( fade > 0 ) {
	    fade -= FadeStep;
	    this->Draw();
	}
    }
    boolean IsOn () {
	return ( fade == fullFade );
    }
    boolean IsOff() {
	return ( fade == 0 );
    }
    void Draw () {
	fadePainter[fade]->FillPolygon(canvas, p.x, p.y, p.count);
    }

public:
    Segment( Seg, float, float, Painter *painter);
    ~Segment();
    void Resize(Canvas *canvas, int height);
    void Redraw() {
	if (! this->IsOff() ) {	// don't redraw if off
	    Draw();
	}
    }
    boolean On() {
	if ( ! this->IsOn() ) {
	    this->FadeUp();
	}
	return ( this->IsOn() );
    }
    boolean Off() {
	if ( ! this->IsOff() ) {
	    this->FadeDown();
	}
	return ( this->IsOff() );
    }
};

#endif
