// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  
*/


#ifndef STRINGH
#define STRINGH

#include <stream.h>

struct _Srep                     // internal String representations
{
  unsigned short    len;         // string length 
  unsigned short    sz;          // allocated space
  unsigned short    ref;         // reference count;
  char              s[1];        // the string starts here 
                                 // (at least 1 char for trailing null)
                                 // allocated & expanded via non-public fcts
};


class String;
class SubString;


struct re_pattern_buffer;       // defined elsewhere
struct re_registers;

class Regex
{
  friend class       String;
  friend class       SubString;

  re_pattern_buffer* buf;
  re_registers*      reg;

  void               initialize(const char* t, int tlen, int fast, 
                                int bufsize, const char* transtable);

public:
                     Regex(const char* t, 
                           int fast = 0, 
                           int bufsize = 40, 
                           const char* transtable = 0);

                     Regex(String& x, 
                           int fast = 0, 
                           int bufsize = 40, 
                           const char* transtable = 0);

                     ~Regex();

  int                search(const char* s, int len, 
                            int& matchlen, int startpos = 0);
};


class SubString
{
  friend class      String;

  String*           S;
  unsigned short    pos;
  unsigned short    len;

                    SubString(String* x, int p, int l);
                    SubString(SubString& x);

  void              assign(const char* s, int sl = -1);
  int               cmp(SubString& y);
  int               cmp(const char* t);
  
public:
                    ~SubString() {}

  void              operator =  (SubString&  y);
  void              operator =  (const char* t);
  void              operator =  (char        c);
  
  String            operator +  (SubString&  y);
  String            operator +  (const char* t);
  String            operator +  (char        c);

  int               operator == (SubString&  y);
  int               operator == (const char* t);
  
  int               operator != (SubString&  y);
  int               operator != (const char* t);
  
  int               operator <= (SubString&  y);
  int               operator <= (const char* t);
  
  int               operator <  (SubString&  y);
  int               operator <  (const char* t);
  
  int               operator >= (SubString&  y);
  int               operator >= (const char* t);
  
  int               operator >  (SubString&  y);
  int               operator >  (const char* t);

  int               contains(char        c);
  int               contains(String&     y);
  int               contains(SubString&  y);
  int               contains(const char* t);
  int               contains(Regex&       r);

  int               matches(Regex&  r);

  int               length();
  const char*       operator char*();

  friend ostream&   operator<<(ostream& s, SubString& x);
};


class String
{
  friend class      SubString;

  _Srep*            rep;

  String&           copy(const char*, int = -1);
  String&           append(const char*, int = -1);
  int               cmp(String&);
  int               cmp(const char*);
  int               cmp(SubString&);
  void              _gsub(const char*, int, const char*, int);
  void              _gsub(Regex&, const char*, int);
  void              _del(int, int);

public:

// constructors & assignment

                    String();
                    String(String&     x);
                    String(SubString&  x);
                    String(const char* t);
                    String(const char* t, int len);
                    String(char c);

                    ~String();

  String&           operator =  (String&     y);
  String&           operator =  (const char* y);
  String&           operator =  (char        c);
  String&           operator =  (SubString&  y);
  friend void       operator =  (SubString&  x, String&     y);

// concatenation

  friend String     operator +  (String&     x, String&     y);     
  friend String     operator +  (String&     x, const char* t);
  friend String     operator +  (String&     x, char        c);
  friend String     operator +  (String&     x, SubString&  y);     
  friend String     operator +  (SubString&  x, String&     y);     

  String&           operator += (String&     y);
  String&           operator += (const char* t);
  String&           operator += (char        c);
  String&           operator += (SubString&  y);

// relational operators

  friend int        operator == (String&     x, String&     y);
  friend int        operator == (String&     x, const char* t);
  friend int        operator == (String&     x, SubString&  y);
  friend int        operator == (SubString&  x, String&     y);
  
  friend int        operator != (String&     x, String&     y);
  friend int        operator != (String&     x, const char* t);
  friend int        operator != (String&     x, SubString&  y);
  friend int        operator != (SubString&  x, String&     y);
  
  friend int        operator <= (String&     x, String&     y);
  friend int        operator <= (String&     x, const char* t);
  friend int        operator <= (String&     x, SubString&  y);
  friend int        operator <= (SubString&  x, String&     y);
  
  friend int        operator <  (String&     x, String&     y);
  friend int        operator <  (String&     x, const char* t);
  friend int        operator <  (String&     x, SubString&  y);
  friend int        operator <  (SubString&  x, String&     y);
  
  friend int        operator >= (String&     x, String&     y);
  friend int        operator >= (String&     x, const char* t);
  friend int        operator >= (String&     x, SubString&  y);
  friend int        operator >= (SubString&  x, String&     y);
  
  friend int        operator >  (String&     x, String&     y);
  friend int        operator >  (String&     x, const char* t);
  friend int        operator >  (String&     x, SubString&  y);
  friend int        operator >  (SubString&  x, String&     y);

// searching & matching

  int               index(char        c, int startpos = 0);      
  int               index(String&     y, int startpos = 0);      
  int               index(SubString&  y, int startpos = 0);      
  int               index(const char* t, int startpos = 0);  
  int               index(Regex&      r, int startpos = 0);       

  int               contains(char        c);
  int               contains(String&     y);
  int               contains(SubString&  y);
  int               contains(const char* t);
  int               contains(Regex&      r);

  int               matches(Regex& r);

// substring extraction

  SubString         at(int         pos, int len);
  SubString         at(String&     x, int startpos = 0); 
  SubString         at(SubString&  x, int startpos = 0); 
  SubString         at(const char* t, int startpos = 0);
  SubString         at(char        c, int startpos = 0);
  SubString         at(Regex&      r, int startpos = 0); 

  SubString         before(int          pos);
  SubString         before(String&      x, int startpos = 0);
  SubString         before(SubString&   x, int startpos = 0);
  SubString         before(const char*  t, int startpos = 0);
  SubString         before(char         c, int startpos = 0);
  SubString         before(Regex&       r, int startpos = 0);

  SubString         after(int         pos);
  SubString         after(String&     x, int startpos = 0);
  SubString         after(SubString&  x, int startpos = 0);
  SubString         after(const char* t, int startpos = 0);
  SubString         after(char        c, int startpos = 0);
  SubString         after(Regex&      r, int startpos = 0);

// modification

  void              del(int         pos, int len);
  void              del(String&     y, int startpos = 0);
  void              del(SubString&  y, int startpos = 0);
  void              del(const char* t, int startpos = 0);
  void              del(char        c, int startpos = 0);
  void              del(Regex&      r, int startpos = 0);

  void              gsub(String&     pat, String&     repl);
  void              gsub(const char* pat, String&     repl);
  void              gsub(String&     pat, const char* repl);
  void              gsub(const char* pat, const char* repl);
  void              gsub(Regex&      pat, String&     repl);
  void              gsub(Regex&      pat, const char* repl);
  void              gsub(String&     pat, SubString&  repl);
  void              gsub(SubString&  pat, String&     repl);
  void              gsub(SubString&  pat, SubString&  repl);
  void              gsub(Regex&      pat, SubString&  repl);

// friends & utilities

  friend int        decompose(String& x, String& l, String& m, String& r,
                              int pos, int len);
  friend int        decompose(String& x, String& l, String& m, String& r,
                              String& pat, int startpos = 0);
  friend int        decompose(String& x, String& l, String& m, String& r,
                              SubString& pat, int startpos = 0);
  friend int        decompose(String& x, String& l, String& m, String& r,
                              const char* pat, int startpos = 0);
  friend int        decompose(String& x, String& l, String& m, String& r,
                              Regex& pat, int startpos = 0);

  friend int        split(String& x, String res[], int maxn, String&     sep);
  friend int        split(String& x, String res[], int maxn, const char* sep);
  friend int        split(String& x, String res[], int maxn, Regex&       sep);

  friend String     join(String src[], int n, String& sep);
  friend String     join(String src[], int n, const char* sep);

  friend String     reverse(String& x);
  friend String     upcase(String& x);
  friend String     downcase(String& x);

  friend String     replicate(char        c, int n);
  friend String     replicate(const char* t, int n);
  friend String     replicate(String&     y, int n);

// conversion

  char              operator [] (int i);
  const char*       operator char*();

// IO

  friend ostream&   operator<<(ostream& s, String& x);
  friend istream&   operator>>(istream& s, String& x);

  friend File&      File::get    (String& x, int n, char terminator = '\n');
  friend File&      File::getline(String& x, int n, char terminator = '\n');

// miscellany

  void              make_unique();
  int               length();
  void              error(char* msg);
};



// some built in regular expressions

extern Regex RXwhite;          // = "[ \n\t]+"
extern Regex RXint;            // = "-?[0-9]+"
extern Regex RXdouble;         // = "-?\\(\\([0-9]+\\.[0-9]*\\)\\|
                               //    \\([0-9]+\\)\\|\\(\\.[0-9]+\\)\\)
                               //    \\([eE][---+]?[0-9]+\\)?"
extern Regex RXalpha;          // = "[A-Za-z]+"
extern Regex RXlowercase;      // = "[a-z]+"
extern Regex RXuppercase;      // = "[A-Z]+"
extern Regex RXalphanum;       // = "[0-9A-Za-z]+"
extern Regex RXidentifier;     // = "[A-Za-z_][A-Za-z0-9_]*"


extern void default_String_error_handler(char*);
extern one_arg_error_handler_t String_error_handler;

extern one_arg_error_handler_t 
        set_String_error_handler(one_arg_error_handler_t f);

#endif // STRINGH
