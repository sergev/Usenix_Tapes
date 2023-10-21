/*
 * Pages - demo for Text and Layout
 */

#ifndef pages_h
#define pages_h

#include <InterViews/scene.h>
class Deck;
class Layout;
class TextPainter;
class Text;
class Sensor;
class Event;
class Painter;
class EditWord;

class Pages : public MonoScene {
    Deck * deck;
    Layout * layout;
    int count;

    TextPainter * highlighter;
    Sensor * sensor;

    Text * hit;
    Text * after;
    EditWord * edit;
    char editbuffer[256];
public:
    Pages( Layout*, int count, Painter * = stdpaint );
    ~Pages();
    void Handle(Event&);
};

#endif
