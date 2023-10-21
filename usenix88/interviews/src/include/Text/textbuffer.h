/*
 * Interface to simple text buffer manager.
 */

#ifndef textbuffer_h
#define textbuffer_h

#include <InterViews/interactor.h>

class Line {
public:
    int width;
    char* data;
    char* attributes;
    char attributesSet;
};

typedef Line* LinePtr;

class TextBuffer : public Interactor {
    Painter* highlight;
    Coord curx, savcurx;
    Coord cury, savcury;
    int charx, chary, savcharx, savchary;
    Coord lastx;
    Coord lasty;
    int lastc;
    int top, bot;
    int height, width;
    int charheight, charwidth;
    boolean underline, savunderline;
    boolean inverse, savinverse;
    char curAttributes, savAttributes;
    char norCharSet, altCharSet, savCharSet;
    boolean prescroll;
    boolean showcursor, outline;
    char* spaces;
    LinePtr* data;

    void Init();
    Coord MapCharX (int cx) { return cx*charwidth; }
    Coord MapCharY (int cy) { return ymax - (cy+1)*charheight + 1; }
    void SetCurPos();
public:
    TextBuffer(Sensor* in = stdsensor, Painter* out = stdpaint);
    TextBuffer(
	int rows, int cols, Sensor* in = stdsensor, Painter* out = stdpaint
    );
    void Delete();
    void Draw();
    void Resize();
    void Redraw(Coord, Coord, Coord, Coord);
    int Row(Coord y);
    int Column(Coord x);
    void GetSize (int& rows, int& cols) { rows = height; cols = width; }
    int GetHeight () { return height; }
    void CursorOn();
    void CursorOff();
    void OutlineCursor (boolean onoff) { outline = onoff; }
    void BatchedScrolling (boolean onoff) { prescroll = onoff; }

    void ClearLines(int where, int count);
    void ClearScreen();
    void CursorDown(int);
    void CursorLeft(int);
    void CursorRight(int);
    void CursorUp(int);
    void EraseBOL();
    void EraseEOL();
    void EraseLine();
    void EraseEOS();
    void EraseScreen(int);
    void Goto(int row, int col);
    void ScrollDown(int where, int count);
    void ScrollUp(int where, int count);
    void ForwardScroll();
    void ReverseScroll();
    void InsertLines(int);
    void DeleteLines(int);
    void InsertCharacters(int);
    void DeleteCharacters(int);
    void PreScroll(const char* bufstart, const char* bufend);
    void CheckScroll (const char* buf, int index, int len) {
	if (prescroll) {
	    PreScroll(&buf[index], &buf[len]);
	}
    }
    void Flush();
    void SavePos();
    void AddChar(char c);
    void CarriageReturn();
    void BackSpace();
    void Tab();
    void Underline(boolean);
    void Inverse(boolean);
    void Bold(boolean);
    void Blink(boolean);
    void UseAlt(boolean);
    void SetRegion (int t, int b) { top = t - 1; bot = b; }
    void SaveCursor();
    void RestoreCursor();
    void SetNorCharSet (int c) { norCharSet = c; }
    void SetAltCharSet (int c) { altCharSet = c; }
};

#endif
