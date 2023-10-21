/*
 * Hash table class for storing painters.  On X10, SetColor, SetFont, etc.
 * operations require a client-server ipc round-trip and hence are
 * inefficient.  To avoid this, we keep a hash table of painters that have
 * preset colors, patterns, etc.  When it's time to Draw a graphic, we
 * use a painter whose graphics state matches the graphic's own.  If none
 * exists, then we create a new painter, set its graphics state to match
 * the graphic's, and use it to draw the graphic.  The new painter is stored
 * in the hash table for future use.
 */

#ifndef hash_h
#define hash_h

class Entry;
class Graphic;
class Painter;

class HashTable {
protected:
    Entry** entries;
    int count;

    virtual boolean Match(void* target, void* entry);
    virtual int Hash(void*);
public:
    HashTable(int);
    ~HashTable();

    void Insert(void* tag, void* value);
    void Remove(void* tag);
    void* Find(void* tag);
};

class GraphicToPainter : HashTable {
    boolean Match(void*, void*);
    int Hash(void*);
    class Canvas* canvas;
    class BoxObj* box;
public:
    GraphicToPainter(int);
    ~GraphicToPainter();
    void Insert(Graphic*, Painter*&);
    void Remove(Graphic* g) { HashTable::Remove(g); }
    Painter* Find(Graphic* g);
    void Clip(Canvas*, class Coord, Coord, Coord, Coord);
    void NoClip();
};

#endif
