#ifndef DOFILEH
#define DOFILEH

#include <stream.h>

extern istream* open_istream(char* fname);
extern ostream* open_ostream(char* fname);
extern ostream* append_ostream(char* fname);
extern char* ext_filename(char* fname);
extern char* base_filename(char* fname);
#endif
