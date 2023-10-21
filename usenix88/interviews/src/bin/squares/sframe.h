/*
 * A frame surrounds the squares view with banner and scroll bars.
 */

#ifndef sframe_h
#define sframe_h

#include <InterViews/frame.h>
#include <InterViews/menu.h>

class SquaresFrame : public BorderFrame {
    friend class SquaresMetaView;

    typedef enum { NEWSQ, ZOOMIN, ZOOMOUT, NEWVIEW, SETUP, QUIT } Choice;
    class SquaresMenuItem : public TextItem {
    public:
	Choice choice;

	SquaresMenuItem (const char* s, Choice c, Painter* p) : (p, s) {
	    choice = c;
	}
    };

    class SquaresView* view;
    class SquaresMetaView* meta;
    Menu* menu;
    Menu* quit;
    int hwidth, vwidth;
    boolean below, right;

    void Item (Menu* m, const char* s, Choice c) {
	m->Insert(new SquaresMenuItem(s, c, output));
    }
    void Init(SquaresView*);
    void MakeFrame();
public:
    SquaresFrame(SquaresView*, Painter*);
    SquaresFrame(SquaresFrame*, Painter*);
    void Handle(Event&);
};

#endif
