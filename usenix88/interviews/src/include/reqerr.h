/*
 * Handling errors from window server.
 */

#ifndef reqerr_h
#define reqerr_h

class ReqErr {
public:
    int msgid;
    int code;
    int request;
    int minor;
    void* id;
    char message[1024];

    ReqErr();
    ~ReqErr();
    ReqErr* Install();
    virtual void Error();
};

#endif
