/*
 * Indicator : a view of a Log
 * $Header: indicator.h,v 1.1 87/06/17 11:27:49 calder Exp $
 */

#ifndef indicator_h
#define indicator_h

#include <InterViews/interactor.h>

class Indicator;
class Log;

typedef float (*FF)(float);
typedef boolean (*IPB)(Indicator*);

class Indicator : public Interactor {
protected:
    Log * log;
    float scale;
    int currentScale;
    int scaleCount;
    float * scales;
    FF filter;
    IPB scaleup;
    IPB scaledown;

    float Limit(float f) { return (f>1.0) ? 1.0 : (f<0.0) ? 0.0 : f; }
    float Process(float f) { return filter!=nil?(*filter)(f)/scale:f/scale; }
    void FixScaling();
public:
    Indicator( Log * );
    Log * GetLog() { return log; }
    void Scale(float s) { scale = s; }
    float GetScale() { return scale; }
    void Scaling(int count, int init, float * scales);
    void Scalers(IPB up, IPB down) { scaleup = up; scaledown = down; }
    void Filter(FF f) { filter = f; }
    void Update() { Draw(); }
    void Draw() { Redraw( 0, 0, xmax, ymax ); }
};

class Readout : public Indicator {
    char * format;
    int textx, texty;
    float multiplier;
    float shift;
public:
    Readout( Log *, const char * sample, const char * form );
    void Redraw( Coord, Coord, Coord, Coord );
};

class Bar : public Indicator {
    Painter * patterns[3];
public:
    Bar( Log *, int width);
    ~Bar();
    void Redraw( Coord, Coord, Coord, Coord );
};

class Pointer : public Indicator {
    int last1, last2;
    int thumb;
    void Show1( float val );
    void Show2( float val1, float val2 );
    void Show();
public:
    Pointer( Log * );
    void Update();
    void Redraw( Coord, Coord, Coord, Coord );
};

class Graph : public Indicator {
    Painter * dots;
    float jump;
    int drawn;

    void Plot( int x, float val );
    void Dot( int x, float val );
public:
    Graph( Log *, float jump =0.5 );
    ~Graph();
    void Resize();
    void Update();
    void Redraw( Coord, Coord, Coord, Coord );
};

#endif
