/* 
 * digital clockface class
 */

#ifndef dface_h
#define dface_h

struct CharPoints {
    int count;
    Coord x[12];
    Coord y[12];
};

enum AMPMMODE { BLANK, AM, PM };

class DFace : public Interactor {
    Clock *clock;
    TMode mode;					// civil or military
    AMPMMODE AMPMmode;
    SegPoints colon[2];				// colon shape data
    CharPoints A, P, M;				// character shape data
    Painter *invertor;				// for highlights and erasing
    Painter *grayer;				// for border
    Font *font;					// font for date
    boolean showDate;				// visibility of date
    boolean showBorder;				// visibility of date/time line
    boolean showTime;				// visibility of time
    boolean selected;				// highlight date if true
    boolean done;				// program terminator

    Digit *ht, *hu, *mt, *mu;

    struct {					// date string
	int len;
	char text[50];
    } date;

protected:
    void DrawFace();
    void DrawFrame();
    void DrawColon();
    void DrawAMPM(Painter *painter);
    void DrawDate();
    void DrawBorder();
    void Set( char *today, int hours, int minutes );
    void Tick();

public:
    DFace(
	boolean showDate,
	boolean showBorder,
	boolean showTime,
	TMode timeMode,
	boolean inverted,
	int width = 0, int height = 0
    );
    void Delete();
    void Resize();
    void Redraw(Coord left, Coord bottom, Coord right, Coord top);
    void Draw();
    void Handle(Event &event);
    void Run();
};

#endif
