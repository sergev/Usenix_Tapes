#ifndef lock_h
#define lock_h

#include <InterViews/frame.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>

extern World* world;

struct passwd {
    char *pw_name;
    char *pw_passwd;
    int pw_uid;
    int pw_gid;
    char *pw_age;
    char *pw_comment;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};
     
class ScreenLock : public Interactor {
    Frame* lockdialog;
    char username[ 128 ];
    char userpasswd[ 128 ];
    char rootpasswd[ 128 ];
    struct passwd pwentry;
public:
    ScreenLock(Painter*);

    Cursor* LockCursor();

    void InsertInto(Scene* s) {
	s->Insert(lockdialog,
	    (world->Width() - lockdialog->shape->width)/2,
	    (world->Height() - lockdialog->shape->height)/2
	);
    }
    void RemoveFrom(Scene* s) { s->Remove(lockdialog); }

    void Activate();
};

#endif
