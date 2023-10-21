/*
 * Interface to shell windows.
 */

#ifndef shell_h
#define shell_h

#include <InterViews/scene.h>

class Shell : public Scene {
    class Frame* frame;
    class Banner* banner;
    class TextBuffer* text;
    class Emulator* term;

    void Init();
public:
    int pty;

    Shell(Sensor* in = nil, Painter* out = stdpaint);
    Shell(int rows, int cols, Sensor* in = nil, Painter* out = stdpaint);
    Shell(char* name, Sensor* in = nil, Painter* out = stdpaint);
    Shell(
	char* name, int rows, int cols,
	Sensor* in = nil, Painter* out = stdpaint
    );

    void Delete();
    void Draw();
    void Resize();
    void Run();
    void Write(const char*, int);
};

#endif
