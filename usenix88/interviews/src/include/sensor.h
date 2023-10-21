/*
 * Sensors describe input events of interest.
 */

#ifndef sensor_h
#define sensor_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

typedef enum {
    MotionEvent,	/* mouse moved */
    DownEvent,		/* button pressed */
    UpEvent,		/* button released */
    KeyEvent,		/* key pressed, intepreted as ascii */
    OnEvent,		/* now input focus */
    OffEvent,		/* no longer input focus */
    ChannelEvent,	/* input pending on channel */
    TimerEvent,		/* time out on read */
} EventType;

static const int LEFTMOUSE = 0;
static const int MIDDLEMOUSE = 1;
static const int RIGHTMOUSE = 2;

/*
 * The flag type should be boolean, but C++ doesn't allow enum bitfields.
 */
typedef unsigned char EventFlag;

struct Event {
    class Interactor* target;
    int timestamp;
    EventType eventType;
    Coord x, y;			/* mouse position relative to target */
    EventFlag control : 1;	/* true if down */
    EventFlag meta : 1;
    EventFlag shift : 1;
    EventFlag shiftlock : 1;
    EventFlag leftmouse : 1;
    EventFlag middlemouse : 1;
    EventFlag rightmouse : 1;
    unsigned char button;	/* button pressed or released, if any */
    short len;			/* length of ASCII string */
    char* keystring;		/* ASCII interpretation of event, if any */
    int channel;		/* set of channels ready */
    char keydata[sizeof(int)];	/* for simple mappings */
private:
    friend class Sensor;
    friend class Interactor;

    class World* w;		/* world in which event occurred */
    Coord wx, wy;		/* mouse position relative to world */

    void GetButtonInfo(void*);
public:
    void GetAbsolute(Coord&, Coord&);
    void GetAbsolute(World*&, Coord&, Coord&);
};

class Sensor : public Resource {
    friend class Interactor;
    friend class Scene;

    unsigned mask;
    unsigned down[8];
    unsigned up[8];
    unsigned channels;
    int maxchannel;
    boolean timer;
    int sec, usec;
public:
    Sensor();
    Sensor(Sensor*);
    ~Sensor();
    void Catch(EventType);
    void CatchButton(EventType, int);
    void CatchChannel(int);
    void CatchTimer(int, int);
    void Ignore(EventType);
    void IgnoreButton(EventType, int);
    void IgnoreChannel(int);
    void CatchRemote();
    void IgnoreRemote();
    boolean Interesting(void*, Event&);
};

extern Sensor* allEvents;
extern Sensor* onoffEvents;
extern Sensor* updownEvents;
extern Sensor* noEvents;

#endif
