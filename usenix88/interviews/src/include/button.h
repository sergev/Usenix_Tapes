/*
 * A button is a view of some value that is normally set when
 * the button is pressed.
 */

#ifndef button_h
#define button_h

#include <InterViews/interactor.h>

class Button;
class ButtonList;

class ButtonState {
    void* value;		/* current value associated with button(s) */
    class ButtonList* views;	/* notify when set */

    void Init(void*);
    void Modify(void*);
public:
    ButtonState () { Init(nil); }
    ButtonState (int v) { Init((void*)v); }
    ButtonState (void* v) { Init(v); }
    ~ButtonState();
    void SetValue (int v) { Modify((void*)v); }
    void SetValue (void* v) { Modify(v); }
    void GetValue (int& v) { v = (int) value; }
    void GetValue (void*& v) { v = value; }
    void Attach(Button*);
    void Detach(Button*);
};

class Button : public Interactor {
    void Init(ButtonState*, void*);
protected:
    void* value;		/* value associated with this button */
    ButtonState* subject;	/* set to this->value when pressed */
    ButtonList* associates;	/* enable/disable when chosen/unchosen */
    boolean enabled;		/* can be pressed */
    boolean chosen;		/* currently toggled on */
    boolean hit;		/* currently being pushed */
public:
    Button (ButtonState* s, void* v, Painter* out) : (nil, out) { Init(s, v); }
    void Delete();
    void Disconnect();
    void SetDimensions(int width, int height);
    void Draw();
    void Attach(Button*);
    void Detach(Button*);
    void Enable();
    void Disable();
    void Choose();
    void UnChoose();
    void Handle(Event&);
    virtual void Press();
    virtual void Check();
};

class TextButton : public Button {
protected:
    const char* text;
    Painter* background;
public:
    TextButton(const char*, ButtonState*, void*, Painter*);
    void Delete();
};

class PushButton : public TextButton {
    void Init();
public:
    PushButton (
	const char* str, ButtonState* s, int v, Painter* out
    ) : (str, s, (void*)v, out) {
	Init();
    }
    PushButton (
	const char* str, ButtonState* s, void* v, Painter* out
    ) : (str, s, v, out) {
	Init();
    }
    void Redraw(Coord, Coord, Coord, Coord);
    void Update();
};

class RadioButton : public TextButton {
    void Init();
public:
    RadioButton (
	const char* str, ButtonState* s, int v, Painter* out
    ) : (str, s, (void*)v, out) {
	Init();
    }
    RadioButton (
	const char* str, ButtonState* s, void* v, Painter* out
    ) : (str, s, v, out) {
	Init();
    }
    void Redraw(Coord, Coord, Coord, Coord);
    void Update();
};

class CheckBox : public TextButton {
    void* offvalue;

    void Init(void*);
public:
    CheckBox (
	const char* str, ButtonState* s, int v, Painter* out
    ) : (str, s, nil, out) {
	Init((void*)v);
    }
    CheckBox (
	const char* str, ButtonState* s, void* v, Painter* out
    ) : (str, s, nil, out) {
	Init(v);
    }
    void Redraw(Coord, Coord, Coord, Coord);
    void Press();
    void Check();
    void Update();
};

#endif
