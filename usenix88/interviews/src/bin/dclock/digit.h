/*
 * Digit class for digital clock
 */

#ifndef digit_h
#define digit_h

class Digit {
    Segment * Segs[7];
    float Xorg;
    float Yorg;
public :
    Digit( float, float, Painter * );
    ~Digit();
    boolean Set( int );
    void Resize(Canvas *canvas, int height);
    void Redraw();
};

#endif
