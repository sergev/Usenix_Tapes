/*
 * Interface to handling errors from window system.
 */

#include <InterViews/defs.h>
#include <InterViews/reqerr.h>
#include <InterViews/Xinput.h>

#ifdef X10
#   define XSetErrorHandler XErrorHandler
#   define minor_code func
#   define resourceid window
#   define errdisplay
#endif

typedef void XHandler(void*, XErrorEvent*);

extern void XSetErrorHandler(XHandler*);

static ReqErr* errhandler = nil;

ReqErr::ReqErr () {
}

ReqErr::~ReqErr () {
    if (errhandler == this) {
	errhandler = nil;
    }
}

void ReqErr::Error () {}

void DoXError (void* errdisplay, register XErrorEvent* e) {
    register ReqErr* r;

    r = errhandler;
    if (r != nil) {
	r->msgid = e->serial;
	r->code = e->error_code;
	r->request = e->request_code;
	r->minor = e->minor_code;
	r->id = e->resourceid;
#ifdef X10
	extern const char* XErrDescrip(int);
	extern void strncpy(char*, const char*, int);
	strncpy(r->message, XErrDescrip(r->code), sizeof(r->message));
#endif
#ifdef X11
	extern void XGetErrorText(void*, int, char*, int);
	XGetErrorText(errdisplay, r->code, r->message, sizeof(r->message));
#endif
	r->Error();
    }
}

ReqErr* ReqErr::Install () {
    ReqErr* r;

    if (errhandler == nil) {
	XSetErrorHandler(DoXError);
    }
    r = errhandler;
    errhandler = this;
    return r;
}
