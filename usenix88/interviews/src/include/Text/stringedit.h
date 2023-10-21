/*
 * StringEdit - basic interactive editor for character strings
 */

#ifndef stringedit_h
#define stringedit_h

#include <InterViews/interactor.h>

class Event;
class Cursor;
class Painter;
class Sensor;
class Font;

static const int SEClipSize = 1024;		// class-static clipboard
static const int SEBufferSize = SEClipSize;	// per-instance work buffer

class StringEdit : public Interactor {
    static char clip[ SEClipSize ];
    static int cliplength;

    const char* old;				// original
    char* buffer;				// working storage

    Painter* highlight;
    Sensor* edit;
    Sensor* select;

    int size;					// max length of  buffer
    int length;					// current length of buffer
    int prev, next;				// selection pointers
    int hitprev, hitnext;			// initial selection
    int extra;					// characters to grow by
    int border;					// pixels around edge

    boolean selecting;				// dragging across text
    boolean caret;				// caret is visible

    int lasttime;				// last up event
    int lastx, lasty;

    boolean DoubleClick(Event&);
    boolean EmptySelection() { return next == prev+1; }
    boolean AtBegin() { return prev <= -1; }
    boolean AtEnd() { return next >= length; }

    void ShowSelection();
    void HideSelection();
    void ShowCaret();
    void HideCaret();
    void DrawCaret(int);

    void Init();
    void FixSize();
    void DoChar(char c);
    void Refresh(int from, int to, Painter*);
    int Hit(Coord);

    void AddChar(char c);
    void DeleteChar(int);

    void Clear();
    void Cut();
    void Copy();
    void Paste();

    void StartSelection(Coord);
    void ExtendSelection(Coord);
    void Select();
    void SelectWord();
    void Unselect();
public:
    StringEdit(const char* sample, Painter * = stdpaint, int border = 3);
    void Delete();

    void Redraw(Coord, Coord, Coord, Coord);
    void Handle(Event&);
    void Reshape(Shape&);

    void Extra(int);
    void Flash(int);
    void SetString(const char*, boolean select = true);
    char* GetString();
};

#endif
