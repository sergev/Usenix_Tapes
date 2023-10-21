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

/* 
  String class implementation
 */


#include <std.h>
#include <regex.h>
#include <String.h>
// #include <String.i.h>
#include <ctype.h>

// error handling


void default_String_error_handler(char* msg)
{
  cerr << "Fatal String error. " << msg << "\n";
  exit(1);
}

one_arg_error_handler_t String_error_handler = default_String_error_handler;

one_arg_error_handler_t set_String_error_handler(one_arg_error_handler_t f)
{
  one_arg_error_handler_t old = String_error_handler;
  String_error_handler = f;
  return old;
}

void String::error(char* msg)
{
  (*String_error_handler)(msg);
}

//  globals

_Srep  _nil_Srep = { 0, 0, 1, { 0 } }; // nil strings point here
String _nilString;                      // nil SubStrings point here

#define SHOULD_BE_INLINE

/*
  allocation fcts  

  Because of the way they are used, these can't
  be done conveniently by overloading 'new'

  The padded version is called to try to allocate
  a reasonable amount of storage for any "dynamic-looking" string.
  Unpadded version used instead for original assignments & constructors
  to conserve space. 

  Both allocate a minimum of MINSIZE chars, where MINSIZE is close 
  to the smallest chunk that malloc will deal with given the _Srep overhead.
*/

#define SHORT_SIZE  16

#define PAD         2
#define MINSIZE     4
#define MAXSIZE     (1 << (SHORT_SIZE - 1))

static int calc_size(int l, int pad)
{
  int siz;
  if (pad != 0)
    siz = l * PAD + MINSIZE;
  else if (l < MINSIZE)
    siz = MINSIZE;
  else 
    siz = l;
  if (siz > MAXSIZE)
    (*String_error_handler)("Requested length out of range");
  return siz;
}

SHOULD_BE_INLINE static _Srep*  new_Srep(int l, int pad = 0)
{
  int siz = calc_size(l, pad);
  _Srep* z = (_Srep *) (new char [sizeof(_Srep) + siz]);
  z->len = l;
  z->sz  = siz + 1;
  z->ref = 1;
  return z;
}

// resize is used very sparingly for now, to avoid any
// peculiarities in realloc fcts. It is only used to
// expand the most recently allocated string. This
// will be changed when GNU CC malloc/realloc fcts
// are in place.

SHOULD_BE_INLINE static _Srep* resize_Srep(_Srep* x, int l, int pad = 0)
{
  int siz = calc_size(l, pad);
  _Srep* z = (_Srep*)(realloc((void*)x, sizeof(_Srep) + siz));
  z->len = l;
  z->sz = siz + 1;
  z->ref = 1;
  return z;
}

/*
 the following inline fcts are specially designed to work
 in support of String classes, and are not meant as generic replacements
 for libc "str" functions.

 inline copy fcts -  we assume 'bcopy' overhead isn't worth it for Strings.
 I like left-to-right from->to arguments.
*/

SHOULD_BE_INLINE static void ncopy(const char* from, char* to, short n) // inline strncpy
{
  if (from != 0) while(n-- > 0 && (*to++ = *from++));
}

SHOULD_BE_INLINE static void ncopy0(const char* from, char* to, short n) //null terminate
{
  if (from != 0) while(n-- > 0 && (*to++ = *from++));
  *to = 0;
}

SHOULD_BE_INLINE static void scopy(const char* from, char* to) // inline strcpy
{
  if (from != 0) while(*to++ = *from++);
}

SHOULD_BE_INLINE static void revcopy(const char* from, char* to, short n) // copy left
{
  if (from != 0) while (n-- > 0) *to-- = *from--;
}

SHOULD_BE_INLINE static int slen(const char* t) // inline  strlen
{
  if (t == 0)
    return 0;
  else
  {
    const char* a = t;
    while (*a++ != 0);
    return a - 1 - t;
  }
}

int String::cmp(const char* b)
{
  if (b == 0)
      return rep->len;
  else
  {
    signed char diff = 0;
    const char* a = rep->s;
    while ((diff = *a - *b++) == 0 && *a++ != 0);
    return diff;
  }
}

int String::cmp(String& y)
{
  signed char diff = 0;
  const char* a = rep->s;
  const char* b = y.rep->s;
  while ((diff = *a - *b++) == 0 && *a++ != 0);
  return diff;
}

int String::cmp(SubString& y)
{
  signed char diff = 0;
  const char* a = rep->s;
  const char* b = &(y.S->rep->s[y.pos]);
  int n;
  if (rep->len == y.len)
  {
    n = rep->len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
  }
  else if (rep->len < y.len)
  {
    n = rep->len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
    if (diff == 0)
      return -1;
  }
  else
  {
    n = y.len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
    if (diff == 0)
      return 1;
  }
  return diff;
}

int SubString::cmp(SubString& y)
{
  signed char diff = 0;
  const char* a = &(S->rep->s[pos]);
  const char* b = &(y.S->rep->s[y.pos]);
  int n;
  if (len == y.len)
  {
    n = len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
  }
  else if (len < y.len)
  {
    n = len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
    if (diff == 0)
      return -1;
  }
  else
  {
    n = y.len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
    if (diff == 0)
      return 1;
  }
  return diff;
}

int SubString::cmp(const char* b)
{
  if (b == 0)
    return len;
  else
  {
    signed char diff = 0;
    const char* a = &(S->rep->s[pos]);
    int n = len;
    while (n-- > 0 && (diff = *a++ - *b++) == 0);
    if (diff == 0 && *b != 0)
      return -1;
    else
      return diff;
  }
}

/*
 index fcts
*/

int _Stringcindex(const char* s, int sl, int start, char c)
{
  if (s != 0 && sl > 0)
  {
    if (start >= 0)
    {
      const char* a = &(s[start]);
      const char* lasta = &(s[sl]);
      while (a < lasta) if (*a++ == c) return a - 1 - s;
    }
    else
    {
      const char* a = &(s[sl + start + 1]);
      while (--a >= s) if (*a == c) return a - s;
    }
  }
  return -1;
}


int _Stringsindex(const char* s, int sl, int start, const char* t, int tl = -1)
{
  if (tl < 0)
    tl = slen(t);
  if (s != 0 && t != 0)
  {
    if (start >= 0)
    {
      const char* lasts = &(s[sl - tl]);
      const char* lastt = &(t[tl]);
      const char* p = &(s[start]);

      while (p <= lasts)
      {
        const char* x = p++;
        const char* y = t;
        while (*x++ == *y++) if (y >= lastt) return p - 1 - s;
      }
    }
    else
    {
      const char* firsts = &(s[tl - 1]);
      const char* lastt =  &(t[tl - 1]);
      const char* p = &(s[sl + start + 1]); 

      while (--p >= firsts)
      {
        const char* x = p;
        const char* y = lastt;
        while (*x-- == *y--) if (y < t) return x + 1 - s;
      }
    }
  }
  return -1;
}


/*
 * other utilities
 */

String& String::copy(const char* news, int newlen = -1)
{
  if (newlen < 0)
    newlen = slen(news);

  if (newlen == 0)
  {
    if (rep != &_nil_Srep && --rep->ref == 0)
      delete rep;
    rep = &_nil_Srep;
  }
  else if (rep == &_nil_Srep)
  {
    rep = new_Srep(newlen);
    ncopy0(news, rep->s, newlen);
  }
  else if (rep->ref > 1)
  {
    --rep->ref;
    rep = new_Srep(newlen, PAD);
    ncopy0(news, rep->s, newlen);
  }
  else if (newlen >= rep->sz)
  {
    _Srep* newrep = new_Srep(newlen, PAD);
    ncopy0(news, newrep->s, newlen);
    delete rep;
    rep = newrep;
  }
  else
  {
    ncopy0(news, rep->s, newlen);
    rep->len = newlen;
  }
  return *this;
}

void String::make_unique()
{
  if (rep->ref > 1 || rep == &_nil_Srep)
  {
    if (rep != &_nil_Srep)
      rep->ref--;
    _Srep* oldrep = rep;
    rep = new_Srep(oldrep->len);
    scopy(oldrep->s, rep->s);
  }
}

/*
 concatenation- These are all redundant so that call-by-value fcts
 won't have to go through two constructors on return
*/

String operator+(String& x,SubString&  y)
{
  String p;
  p.rep = new_Srep(x.rep->len + y.len, PAD);
  ncopy(x.rep->s, p.rep->s, x.rep->len);
  ncopy0(&(y.S->rep->s[y.pos]), &(p.rep->s[x.rep->len]), y.len);
  return p;
}

String operator+(String& x,const char* t)
{
  int  tl = slen(t);
  String p;
  p.rep = new_Srep(x.rep->len + tl, PAD);
  ncopy(x.rep->s, p.rep->s, x.rep->len);
  ncopy0(t, &(p.rep->s[x.rep->len]), tl);
  return p;
}

String operator+(String& x,char c)
{
  String p;
  p.rep = new_Srep(x.rep->len + 1, PAD);
  ncopy(x.rep->s, p.rep->s, x.rep->len);
  ncopy0(&c, &(p.rep->s[x.rep->len]), 1);
  return p;
}

String operator+(SubString& x, String& y)
{
  String p;
  p.rep = new_Srep(x.len + y.rep->len, PAD);
  ncopy(&(x.S->rep->s[x.pos]), p.rep->s, x.len);
  ncopy0(y.rep->s, &(p.rep->s[x.len]), y.rep->len);
  return p;
}

String SubString::operator+(SubString&  y)
{
  String p;
  p.rep = new_Srep(len + y.len, PAD);
  ncopy(&(S->rep->s[pos]), p.rep->s, len);
  ncopy0(&(y.S->rep->s[y.pos]), &(p.rep->s[len]), y.len);
  return p;
}

String SubString::operator+(const char* t)
{
  int tl = slen(t);
  String p;
  p.rep = new_Srep(len + tl, PAD);
  ncopy(&(S->rep->s[pos]), p.rep->s, len);
  ncopy0(t, &(p.rep->s[len]), tl);
  return p;
}

String SubString::operator+(char c)
{
  String p;
  p.rep = new_Srep(len + 1, PAD);
  ncopy(&(S->rep->s[pos]), p.rep->s, len);
  ncopy0(&c, &(p.rep->s[len]), 1);
  return p;
}

String operator+(String& x,String& y)
{
  String p;
  p.rep = new_Srep(x.rep->len + y.rep->len, PAD);
  ncopy(x.rep->s, p.rep->s, x.rep->len);
  ncopy0(y.rep->s, &(p.rep->s[x.rep->len]), y.rep->len);
  return p;
}

String& String::append(const char* t, int tl = -1)
{
  if (tl < 0)
    tl = slen(t);
  if (t == 0 || tl <= 0)
    return *this;

  int sl = rep->len + tl;

  if (rep == &_nil_Srep)
  {
    rep = new_Srep(tl, PAD);
    ncopy0(t, rep->s, tl);
  }
  else if (rep->ref > 1)
  {
    --rep->ref;
    _Srep* newrep = new_Srep(sl, PAD);
    scopy(rep->s, newrep->s);
    ncopy0(t, &(newrep->s[rep->len]), tl);
    rep = newrep;
  }
  else if (sl >= rep->sz)
  {
    _Srep* newrep = new_Srep(sl, PAD);
    scopy(rep->s, newrep->s);
    ncopy0(t, &(newrep->s[rep->len]), tl);
    delete rep;
    rep = newrep;
  }
  else
    ncopy0(t, &(rep->s[rep->len]), tl);


  rep->len = sl;
  return *this;
}


void String::_del(int pos, int len)
{
  if (pos <= 0 || rep->len <= 0 || pos + len > rep->len)
    return;
  
  _Srep* nrep = rep;
  int nlen = rep->len - len;
  if (rep->ref > 1)
  {
    rep->ref--;
    nrep = new_Srep(nlen, PAD);
    ncopy(rep->s, nrep->s, pos);
  }
  scopy(&(rep->s[pos + len]), &(nrep->s[pos]));
  nrep->len = nlen;
  rep = nrep;
}


void SubString::assign(const char* news, int newlen = -1)
{
  if (newlen < 0)
    newlen = slen(news);

  _Srep* targ = S->rep;
  int oldlen = len;

  if (targ == &_nil_Srep || pos < 0 || pos >= targ->len)
    return;

  int sl = targ->len - oldlen + newlen;

  if (targ->ref > 1)
  {
    targ->ref--;
    _Srep* oldtarg = targ;
    targ = new_Srep(sl, PAD);
    ncopy(oldtarg->s, targ->s, pos);
    ncopy(news, &(targ->s[pos]), newlen);
    scopy(&(oldtarg->s[pos + oldlen]), &(targ->s[pos + newlen]));
  }
  else if (sl >= targ->sz)      // avoid realloc
  {
    _Srep* oldtarg = targ;
    targ = new_Srep(sl, PAD);
    ncopy(oldtarg->s, targ->s, pos);
    ncopy(news, &(targ->s[pos]), newlen);
    scopy(&(oldtarg->s[pos + oldlen]), &(targ->s[pos + newlen]));
    delete oldtarg;
  }
  else if (oldlen == newlen)
    ncopy(news, &(targ->s[pos]), oldlen);
  else if (newlen < oldlen)
  {
    ncopy(news, &(targ->s[pos]), newlen);
    scopy(&(targ->s[pos + oldlen]), &(targ->s[pos + newlen]));
    targ->len = sl;
  }
  else
  {
    revcopy(&(targ->s[targ->len]), &(targ->s[sl]), targ->len-pos-oldlen +1);
    ncopy(news, &(targ->s[pos]), newlen);
    targ->len = sl;
  }
  S->rep = targ;
}

void String::_gsub(const char* pat, int pl, const char* r, int rl)
{
  int sl = rep->len;
  if (sl <= 0 || pl <= 0 || sl < pl)
    return;

  const char* s = rep->s;

  _Srep* nrep = new_Srep(2 * sl); // guess size
  char* x = nrep->s;

  int si = 0;
  int xi = 0;
  int remaining = sl;

  while (remaining >= pl)
  {
    int pos = _Stringsindex(&(s[si]), remaining, 0, pat, pl);
    if (pos < 0)
      break;
    else
    {
      int mustfit = xi + remaining + rl - pl;
      if (mustfit >= nrep->sz)
      {
        nrep = resize_Srep(nrep, mustfit, PAD);
        x = nrep->s;
      }
      ncopy(&(s[si]), &(x[xi]), pos);
      ncopy(r, &(x[xi + pos]), rl);
      si += pos + pl;
      remaining -= pos + pl;
      xi += pos + rl;
    }
  }

  ncopy0(&(s[si]), &(x[xi]), remaining);
  nrep->len = xi + remaining;

  if (--rep->ref == 0)
  {
    if (nrep->len <= rep->sz)   // fit back in if possible
    {
      rep->ref = 1;
      rep->len = nrep->len;
      scopy(nrep->s, rep->s);
      delete(nrep);
      return;
    }
    else
      delete(rep);
  }
  rep = nrep;
}

void String::_gsub(Regex& pat, const char* r, int rl)
{
  int sl = rep->len;
  if (sl <= 0)
    return;

  const char* s = rep->s;

  _Srep* nrep = new_Srep(sl * 2, PAD); // guess len, readjust as necessary
  char* x = rep->s;

  int si = 0;
  int xi = 0;
  int remaining = sl;
  int  pos, pl;

  while (remaining > 0)
  {
    pos = pat.search(s, sl, pl, si);
    if (pos < 0 || pl <= 0)
      break;
    else
    {
      int mustfit = xi + remaining + rl - pl;
      if (mustfit >= nrep->sz)
      {
        nrep = resize_Srep(nrep, mustfit, PAD);
        x = nrep->s;
      }
      ncopy(&(s[si]), &(x[xi]), pos);
      ncopy(r, &(x[xi + pos]), rl);
      si += pos + pl;
      remaining -= pos + pl;
      xi += pos + rl;
    }
  }

  ncopy0(&(s[si]), &(x[xi]), remaining);
  nrep->len = xi + remaining;

  if (--rep->ref == 0)
  {
    if (nrep->len <= rep->sz)   // fit back in if possible
    {
      rep->ref = 1;
      rep->len = nrep->len;
      scopy(nrep->s, rep->s);
      delete(nrep);
      return;
    }
    else
      delete(rep);
  }

  rep = nrep;
}


int _Stringdecomp(String& x, String& l, String& m, 
                        String& r, int p, int patlen)
{
  if (p >= 0 && patlen >= 0 && patlen <= x.rep->len)
  {
    String tmp = x;             // in case x = l/m/r
    int sl = tmp.rep->len;
    l.copy(tmp.rep->s, p);
    m.copy(&(tmp.rep->s[p]), patlen);
    r.copy(&(tmp.rep->s[p + patlen]), sl - (p + patlen));
    return 1;
  }
  else
    return 0;
}

SubString::SubString(String* x, int first, int l)
{
  if (first < 0 || l <= 0 || first + l > x->rep->len)
  {
    S = &_nilString; pos = len = 0;
  }
  else
  {
    S = x; pos = first; len = l;
  }
}


void Regex::initialize(const char* t, int tlen, int fast, int bufsize, 
                       const char* transtable)
{
  buf = new re_pattern_buffer;
  reg = new re_registers;
  if (fast)
    buf->fastmap = new char[256];
  else
    buf->fastmap = 0;
  buf->translate = transtable;
  if (tlen > bufsize)
    bufsize = tlen;
  buf->allocated = bufsize;
  buf->buffer = new char [buf->allocated];
  char* msg = re_compile_pattern(t, tlen, buf);
  if (msg != 0)
    (*String_error_handler)(msg);
  else if (fast)
    re_compile_fastmap(buf);
}

Regex::Regex(const char* t, int fast = 0, int bufsize = 40, 
             const char* transtable = 0)
{
  initialize(t, slen(t), fast, bufsize, transtable);
}

Regex::Regex(String& x, int fast = 0, int bufsize = 40, 
             const char* transtable = 0)
{
  initialize(x.rep->s, x.rep->len, fast, bufsize, transtable);
}

Regex::~Regex()
{
  delete(buf->buffer);
  delete(buf->fastmap);
  delete(buf);
  delete(reg);
}

int Regex::search(const char* s, int len, int& matchlen, int startpos = 0)
{
  int matchpos, pos, range;
  if (startpos >= 0)
  {
    pos = startpos;
    range = len - startpos;
  }
  else
  {
    pos = len + startpos;
    range = -pos;
  }
  matchpos = re_search_2(buf, 0, 0, s, len, pos, range, reg, len);
  if (matchpos >= 0)
    matchlen = reg->end[0] - reg->start[0];
  else
    matchlen = 0;
  return matchpos;
}


/*
 * substring extraction 
 */


SubString String::at(int first, int len)
{
  return SubString(this, first, len);
}

SubString String::at(String& y, int startpos = 0)
{
  int first = _Stringsindex(rep->s, rep->len, startpos, y.rep->s, y.rep->len);
  return SubString(this, first,  y.rep->len);
}

SubString String::before(String& y, int startpos = 0)
{
  int last = _Stringsindex(rep->s, rep->len, startpos, y.rep->s, y.rep->len);
  return SubString(this, 0, last);
}

SubString String::after(String& y, int startpos = 0)
{
  int first = _Stringsindex(rep->s, rep->len, startpos, y.rep->s, y.rep->len);
  return SubString(this, first + y.rep->len, rep->len - (first+y.rep->len));
}

SubString String::at(char c, int startpos = 0)
{
  int first = _Stringcindex(rep->s, rep->len, startpos, c);
  return SubString(this, first, 1);
}

SubString String::before(char c, int startpos = 0)
{
  int last = _Stringcindex(rep->s, rep->len, startpos, c);
  return SubString(this, 0, last);
}

SubString String::after(char c, int startpos = 0)
{
  int first = _Stringcindex(rep->s, rep->len, startpos, c);
  return SubString(this, first + 1, rep->len - (first + 1));
}

SubString String::at(const char* t, int startpos = 0)
{
  int tlen = slen(t);
  int first = _Stringsindex(rep->s, rep->len, startpos, t, tlen);
  return SubString(this, first, tlen);
}

SubString String::before(const char* t, int startpos = 0)
{
  int last = _Stringsindex(rep->s, rep->len, startpos, t);
  return SubString(this, 0, last);
}

SubString String::after(const char* t, int startpos = 0)
{
  int tlen = slen(t);
  int first = _Stringsindex(rep->s, rep->len, startpos, t, tlen);
  return SubString(this, first + tlen, rep->len - (first + tlen));
}


SubString String::at(SubString& y, int startpos = 0)
{
  int first = _Stringsindex(rep->s, rep->len, startpos, 
                            &(y.S->rep->s[y.pos]), y.len);
  return SubString(this, first, y.len);
}

SubString String::before(SubString& y, int startpos = 0)
{
  int last = _Stringsindex(rep->s, rep->len, startpos, 
                           &(y.S->rep->s[y.pos]), y.len);
  return SubString(this, 0, last);
}

SubString String::after(SubString& y, int startpos = 0)
{
  int first = _Stringsindex(rep->s, rep->len, startpos, 
                            &(y.S->rep->s[y.pos]), y.len);
  return SubString(this, first + y.len, rep->len - (first + y.len));
}

SubString String::at(Regex& r, int startpos = 0)
{
  int first, mlen;
  first = r.search(rep->s, rep->len, mlen, startpos);
  return SubString(this, first, mlen);
}


SubString String::before(Regex& r, int startpos = 0)
{
  int first, mlen;
  first = r.search(rep->s, rep->len, mlen, startpos);
  return SubString(this, 0, first);
}

SubString String::after(Regex& r, int startpos = 0)
{
  int first, mlen;
  first = r.search(rep->s, rep->len, mlen, startpos);
  return SubString(this, first + mlen, rep->len - (first + mlen));
}

SubString String::before(int pos)
{
  return SubString(this, 0, pos);
}

SubString String::after(int pos)
{
  return SubString(this, pos + 1, rep->len - (pos + 1));
}

/*
 * substitution
 */


void String::gsub(String& pat, String& r)
{
  _gsub(pat.rep->s, pat.rep->len, r.rep->s, r.rep->len);
}

void String::gsub(String& pat, SubString&  r)
{
  _gsub(pat.rep->s, pat.rep->len, &(r.S->rep->s[r.pos]), r.len);
}

void String::gsub(SubString&  pat, String& r)
{
  _gsub(&(pat.S->rep->s[pat.pos]), pat.len, r.rep->s, r.rep->len);
}

void String::gsub(SubString&  pat, SubString&  r)
{
  _gsub(&(pat.S->rep->s[pat.pos]), pat.len, &(r.S->rep->s[r.pos]), r.len);
}

void String::gsub(String& pat, const char* r)
{
  _gsub(pat.rep->s, pat.rep->len, r, slen(r));
}

void String::gsub(const char* pat, String& r)
{
  _gsub(pat, slen(pat), r.rep->s, r.rep->len);
}

void String::gsub(const char* pat, const char* r)
{
  _gsub(pat, slen(pat), r, slen(r));
}

void String::gsub(Regex& pat, String& r)
{
  _gsub(pat, r.rep->s, r.rep->len);
}

void String::gsub(Regex& pat, const char* r)
{
  _gsub(pat, r, slen(r));
}

void String::gsub(Regex& pat, SubString&  r)
{
  _gsub(pat, &(r.S->rep->s[r.pos]), r.len);
}

/*
 * deletion
 */

void String::del(int startpos, int len)
{
  _del(startpos, len);
}

void String::del(String& y, int startpos = 0)
{
  int p = _Stringsindex(rep->s, rep->len, startpos, y.rep->s, y.rep->len);
  _del(p, y.rep->len);
}

void String::del(SubString& y, int startpos = 0)
{
  int p = _Stringsindex(rep->s, rep->len, startpos, 
                        &(y.S->rep->s[y.pos]), y.len);
  _del(p, y.len);
}

void String::del(const char* t, int startpos = 0)
{
  int tlen = slen(t);
  int p = _Stringsindex(rep->s, rep->len, startpos, t, tlen);
  _del(p, tlen);
}

void String::del(char c, int startpos = 0)
{
  int p = _Stringcindex(rep->s, rep->len, startpos, c);
  _del(p, 1);
}

void String::del(Regex& r, int startpos = 0)
{
  int first, mlen;
  first = r.search(rep->s, rep->len, mlen, startpos);
  _del(first, mlen);
}

/*
 decomposition
 */

int decompose(String& x, String& l, String& m, String& r, int p, int patlen)
{
  return _Stringdecomp(x, l, m, r, p, patlen);
}

int decompose(String& x, String& l, String& m, String& r,
               String& pat, int startpos = 0)
{
  int p = _Stringsindex(x.rep->s, x.rep->len, startpos,
                        pat.rep->s, pat.rep->len);
  return _Stringdecomp(x, l, m, r, p, pat.rep->len);
}

int decompose(String& x, String& l, String& m, String& r,
               SubString& pat, int startpos = 0)
{
  int p = _Stringsindex(x.rep->s, x.rep->len, startpos, 
                        &(pat.S->rep->s[pat.pos]), pat.len);
  return _Stringdecomp(x, l, m, r, p, pat.len);
}

int decompose(String& x, String& l, String& m, String& r,
               const char* pat, int startpos = 0)
{
  int patlen = slen(pat);
  int p = _Stringsindex(x.rep->s, x.rep->len, startpos, pat, patlen);
  return _Stringdecomp(x, l, m, r, p, patlen);
}


int decompose(String& x, String& l, String& m, String& r,
               Regex& pat, int startpos = 0)
{
  int p, mlen;
  p = pat.search(x.rep->s, x.rep->len, mlen, startpos);
  return _Stringdecomp(x, l, m, r, p, mlen);
}

/*
 * split fcts - can't conveniently factor out common code in
 * different versions, so they're pretty redundant...
 */

int split(String& src, String results[], int n, const char* t)
{
  String x = src;
  const char* s = x.rep->s;
  int sl = x.rep->len;
  int tlen = slen(t);
  int i = 0;
  int pos = 0;
  while (i < n && pos < sl)
  {
    int p = _Stringsindex(s, sl, pos, t, tlen);
    if (p < 0)
      p = sl;
    results[i].copy(&(s[pos]), p - pos);
    i++;
    pos = p + tlen;
  }
  return(i);
}

int split(String& src, String results[], int n, String& sep)
{
  String x = src;
  _Srep* xs = x.rep;
  const char* s = xs->s;
  int sl = xs->len;
  int i = 0;
  int pos = 0;
  while (i < n && pos < sl)
  {
    int p = _Stringsindex(s, sl, pos, sep.rep->s, sep.rep->len);
    if (p < 0)
      p = sl;
    results[i].copy(&(s[pos]), p - pos);
    i++;
    pos = p + sep.rep->len;
  }
  return(i);
}



int split(String& src, String results[], int n, Regex& r)
{
  String x = src;
  char* s = x.rep->s;
  int sl = x.rep->len;
  int i = 0;
  int pos = 0;
  int p, matchlen;
  while (i < n && pos < sl)
  {
    p = r.search(s, sl, matchlen, pos);
    if (p < 0)
      p = sl;
    results[i].copy(&(s[pos]), p - pos);
    i++;
    pos = p + matchlen;
  }
  return(i);
}


String join(String src[], int n, String& separator)
{
  String sep = separator;
  int xlen = 0;
  for (int i = 0; i < n; ++i)
    xlen += src[i].rep->len;
  xlen += (n - 1) * sep.rep->len;

  String x;
  x.rep = new_Srep(xlen);

  int j = 0;
  
  for (i = 0; i < n - 1; ++i)
  {
    ncopy(src[i].rep->s, &(x.rep->s[j]), src[i].rep->len);
    j += src[i].rep->len;
    ncopy(sep.rep->s, &(x.rep->s[j]), sep.rep->len);
    j += sep.rep->len;
  }
  ncopy0(src[i].rep->s, &(x.rep->s[j]), src[i].rep->len);
  return x;
}

String join(String src[], int n, const char* sep)
{
  int seplen = slen(sep);
  int xlen = 0;
  for (int i = 0; i < n; ++i)
    xlen += src[i].rep->len;
  xlen += (n - 1) * seplen;

  String x;
  x.rep = new_Srep(xlen);

  int j = 0;
  
  for (i = 0; i < n - 1; ++i)
  {
    ncopy(src[i].rep->s, &(x.rep->s[j]), src[i].rep->len);
    j += src[i].rep->len;
    ncopy(sep, &(x.rep->s[j]), seplen);
    j += seplen;
  }
  ncopy0(src[i].rep->s, &(x.rep->s[j]), src[i].rep->len);
  return x;
}
  
/*
 misc
*/

    
String reverse(String& x)
{
  String y = x;
  if (y.rep->len > 0)
  {
    y.make_unique();
    char* a = y.rep->s;
    char* b = &(y.rep->s[y.rep->len - 1]);
    while (a < b)
    {
      char t = *a;
      *a++ = *b;
      *b-- = t;
    }
  }
  return y;
}


String replicate(char c, int n)
{
  String w;
  w.rep = new_Srep(n);
  char* p = w.rep->s;
  while (n-- > 0)
    *p++ = c;
  *p = 0;
  return w;
}

String replicate(const char* t, int n)
{
  String w;
  int tlen = slen(t);
  w.rep = new_Srep(n * tlen);
  char* p = w.rep->s;
  while (n-- > 0)
  {
    ncopy(t, p, tlen);
    p += tlen;
  }
  *p = 0;
  return w;
}

String replicate(String& y, int n)
{
  String w;
  w.rep = new_Srep(n * y.rep->len);
  char* p = w.rep->s;
  while (n-- > 0)
  {
    ncopy(y.rep->s, p, y.rep->len);
    p += y.rep->len;
  }
  *p = 0;
  return w;
}

String upcase(String& y)
{
  String x = y;
  x.make_unique();
  char* e = &(x.rep->s[x.rep->len]);
  char* p = x.rep->s;
  for (; p < e; ++p)
    if (*p >= 'a' && *p <= 'z') *p = *p - 'a' + 'A';
  return x;
}

String downcase(String& y)
{
  String x = y;
  x.make_unique();
  char* e = &(x.rep->s[x.rep->len]);
  char* p = x.rep->s;
  for (; p < e; ++p)
    if (*p >= 'A' && *p <= 'Z') *p = *p - 'A' + 'a';
  return x;
}

   
/*
 * IO
 */


File& File::get(String& x, int n, char terminator = '\n')
{
  if (!readable())
  {
    error();
    return *this;
  }

  char ch;
  stat = n;
  String w;
  w.rep = new_Srep(n);
  char* s = w.rep->s;

  if (n > 0 && (get(ch)))
  {
    if (ch == terminator)
      unget(ch);
    else
    {
      *s++ = ch; --n;
      while (n > 0 && (get(ch)))
      {
        if (ch == terminator)
        {
          unget(ch);
          break;
        }
        else
        {
          *s++ = ch; --n;
        }
      }
    }
  }

  *s = 0;
  x = w;
  return failif((stat != 0) && ((stat -= n) == 0));
}

File& File::getline(String& x, int n, char terminator = '\n')
{
  if (!readable())
  {
    error();
    return *this;
  }

  char ch;
  stat = n;
  String w;
  w.rep = new_Srep(n);
  char* s = w.rep->s;

  while (n > 0 && (get(ch)))
  {
    --n;
    if ((*s++ = ch) == terminator)
      break;
  }

  *s = 0;
  x = w;
  return failif((stat != 0) && ((stat -= n) == 0));
}


istream& operator>>(istream& s, String& x)
{
  char ch;
  int i = 0;
  String w;
  w.rep = new_Srep(20);        // guess length;
  while (s.get(ch) && isspace(ch));
  while (s.good())
  {
    if (i >= w.rep->sz - 1)
      w.rep = resize_Srep(w.rep, i+1, PAD);
    w.rep->s[i++] = ch;
    s.get(ch);
    if (isspace(ch))
      break;
  }
  w.rep->s[i] = 0;
  w.rep->len = i;
  x = w;
  s.failif(i == 0);
  return s;
}


ostream& operator<<(ostream& s, SubString& x)
{ 
  const char* a = &(x.S->rep->s[x.pos]);
  const char* lasta = &(a[x.len]);
  while (a < lasta)
    s.put(*a++);
  return(s);
}


/*
 some built-in Regular expressions
*/

Regex RXwhite("[ \n\t]+", 1);
Regex RXint("-?[0-9]+", 1);
Regex RXdouble("-?\\(\\([0-9]+\\.[0-9]*\\)\\|\\([0-9]+\\)\\|\\(\\.[0-9]+\\)\\)\\([eE][---+]?[0-9]+\\)?", 1, 200);
Regex RXalpha("[A-Za-z]+", 1);
Regex RXlowercase("[a-z]+", 1);
Regex RXuppercase("[A-Z]+", 1);
Regex RXalphanum("[0-9A-Za-z]+", 1);
Regex RXidentifier("[A-Za-z_][A-Za-z0-9_]*", 1);


/*
 include this when these are made inline in String.h

extern _Srep  _nil_Srep;
extern String _nilString;

extern int _Stringcindex(const char*, int, int, char);
extern int _Stringsindex(const char*, int, int, const char*, int = -1);
extern int re_match_2 (re_pattern_buffer*, char*, int, 
                       char*, int, int, re_registers*, int);
extern int re_search_2 (re_pattern_buffer*, char*, int, 
                        char*, int, int, int, re_registers*, int);
*/

SHOULD_BE_INLINE String::String()
{ 
  rep = &_nil_Srep;
}

SHOULD_BE_INLINE String::String(String& x)
{ 
  rep = x.rep; rep->ref++;
}

SHOULD_BE_INLINE String::String(const char* t)
{
  rep = &_nil_Srep; copy(t);
}

SHOULD_BE_INLINE String::String(const char* t, int tlen)
{
  rep = &_nil_Srep; copy(t, tlen);
}

SHOULD_BE_INLINE String::String(SubString& y)
{
  rep = &_nil_Srep; copy(&(y.S->rep->s[y.pos]), y.len);
}

SHOULD_BE_INLINE String::String(char c)
{
  rep = &_nil_Srep; char ch = c; copy(&ch, 1);
}

SHOULD_BE_INLINE String::~String()
{ 
  if (rep != &_nil_Srep && --rep->ref == 0) delete rep;
}

SHOULD_BE_INLINE String& String::operator =  (String& y)
{ 
  y.rep->ref++;
  if (rep != &_nil_Srep && --rep->ref == 0) delete rep;
  rep = y.rep;
  return *this; 
}

SHOULD_BE_INLINE String& String::operator=(const char* t)
{
  return copy(t);
}

SHOULD_BE_INLINE String& String::operator=(SubString&  y)
{
  return copy(&(y.S->rep->s[y.pos]), y.len);
}

SHOULD_BE_INLINE String& String::operator=(char c)
{
  char ch = c;  return copy(&ch, 1);
}

SHOULD_BE_INLINE SubString::SubString(SubString& x)
{ 
  S = x.S; pos = x.pos;   len = x.len; 
}

SHOULD_BE_INLINE void operator=(SubString& x, String& y)
{
  x.assign(y.rep->s, y.rep->len);
}

SHOULD_BE_INLINE void SubString::operator=(SubString&  y)
{
  assign(&(y.S->rep->s[y.pos]), y.len);
}

SHOULD_BE_INLINE void SubString::operator=(char c)
{
  char ch = c;  assign(&ch, 1);
}

SHOULD_BE_INLINE void SubString::operator=(const char* t)
{
  assign(t);
}

SHOULD_BE_INLINE String& String::operator+=(String& y)
{
  return append(y.rep->s, y.rep->len);
}

SHOULD_BE_INLINE String& String::operator+=(SubString&  y)
{
  return append(&(y.S->rep->s[y.pos]), y.len);
}

SHOULD_BE_INLINE String& String::operator+=(const char* t)
{
  return append(t);
}

SHOULD_BE_INLINE String& String::operator+=(char c)
{
  char ch = c;  return append(&ch, 1);
}


SHOULD_BE_INLINE int String::length()
{ 
  return rep->len;
}

SHOULD_BE_INLINE char  String::operator [] (int i) 
{ 
  return rep->s[i];
}

SHOULD_BE_INLINE int String::index(char c, int startpos = 0)
{
  return _Stringcindex(rep->s, rep->len, startpos, c);
}

SHOULD_BE_INLINE int String::contains(char c)
{
  return _Stringcindex(rep->s, rep->len, 0, c) >= 0;
}

SHOULD_BE_INLINE int SubString::contains(char c)
{
  return _Stringcindex(&(S->rep->s[pos]), len, 0, c) >= 0;
}

SHOULD_BE_INLINE int String::index(const char* t, int startpos = 0)
{   
  return _Stringsindex(rep->s, rep->len, startpos, t);
}

SHOULD_BE_INLINE int String::index(String& y, int startpos = 0)
{   
  return _Stringsindex(rep->s, rep->len, startpos, y.rep->s, y.rep->len);
}

SHOULD_BE_INLINE int String::index(SubString& y, int startpos = 0)
{   
  return _Stringsindex(rep->s, rep->len, startpos,&(y.S->rep->s[y.pos]),y.len);
}

SHOULD_BE_INLINE int String::contains(const char* t)
{   
  return _Stringsindex(rep->s, rep->len, 0, t) >= 0;
}

SHOULD_BE_INLINE int String::contains(String& y)
{   
  return _Stringsindex(rep->s, rep->len, 0, y.rep->s, y.rep->len) >= 0;
}

SHOULD_BE_INLINE int String::contains(SubString& y)
{   
  return _Stringsindex(rep->s, rep->len, 0, &(y.S->rep->s[y.pos]), y.len) >= 0;
}

SHOULD_BE_INLINE int SubString::contains(const char* t)
{   
  return _Stringsindex(&(S->rep->s[pos]), len, 0, t) >= 0;
}

SHOULD_BE_INLINE int SubString::contains(String& y)
{   
  return _Stringsindex(&(S->rep->s[pos]), len, 0, y.rep->s, y.rep->len) >= 0;
}

SHOULD_BE_INLINE int SubString::contains(SubString&  y)
{   
  return _Stringsindex(&(S->rep->s[pos]),len,0,&(y.S->rep->s[y.pos]),y.len)>=0;
}

SHOULD_BE_INLINE int String::contains(Regex& r)
{
  return re_search_2(r.buf,0,0,rep->s,rep->len,0,rep->len,r.reg,rep->len)>=0;
}

SHOULD_BE_INLINE int SubString::contains(Regex& r)
{
  return re_search_2(r.buf,0,0, &(S->rep->s[pos]), len,0,len, r.reg, len)>=0;
}

SHOULD_BE_INLINE int String::matches(Regex& r)
{
  return re_match_2(r.buf,0,0,rep->s,rep->len,0,r.reg,rep->len) == rep->len;
}

SHOULD_BE_INLINE int SubString::matches(Regex& r)
{
  return re_match_2(r.buf, 0, 0, &(S->rep->s[pos]), len, 0, r.reg, len) == len;
}

SHOULD_BE_INLINE const char* String::operator char*()
{ 
  return rep->s;
}

SHOULD_BE_INLINE int String::index(Regex& r, int startpos = 0)
{
  int mlen;  return r.search(rep->s, rep->len, mlen, startpos);
}

SHOULD_BE_INLINE const char* SubString::operator char*()
{ 
  return (char*)(String(*this)); 
}


SHOULD_BE_INLINE  int SubString::length()
{ 
  return len;
}

SHOULD_BE_INLINE  ostream& operator<<(ostream& s, String& x)
{ 
  return s.put(x.rep->s); 
}

SHOULD_BE_INLINE int operator==(String& x, String& y) 
{
  return x.rep->len == y.rep->len && x.cmp(y) == 0; 
}

SHOULD_BE_INLINE int operator!=(String& x, String& y)
{
  return x.rep->len != y.rep->len || x.cmp(y) != 0; 
}

SHOULD_BE_INLINE int operator>(String& x, String& y)
{
  return x.cmp(y) > 0; 
}

SHOULD_BE_INLINE int operator>=(String& x, String& y)
{
  return x.cmp(y) >= 0; 
}

SHOULD_BE_INLINE int operator<(String& x, String& y)
{
  return x.cmp(y) < 0; 
}

SHOULD_BE_INLINE int operator<=(String& x, String& y)
{
  return x.cmp(y) <= 0; 
}

SHOULD_BE_INLINE int operator==(String& x, SubString&  y) 
{
  return x.rep->len == y.len && x.cmp(y) == 0; 
}

SHOULD_BE_INLINE int operator!=(String& x, SubString&  y)
{
  return x.rep->len != y.len || x.cmp(y) != 0; 
}

SHOULD_BE_INLINE int operator>(String& x, SubString&  y)      
{
  return x.cmp(y) > 0; 
}

SHOULD_BE_INLINE int operator>=(String& x, SubString&  y)
{
  return x.cmp(y) >= 0; 
}

SHOULD_BE_INLINE int operator<(String& x, SubString&  y) 
{
  return x.cmp(y) < 0; 
}

SHOULD_BE_INLINE int operator<=(String& x, SubString&  y)
{
  return x.cmp(y) <= 0; 
}

SHOULD_BE_INLINE int operator==(String& x, const char* t) 
{
  return x.cmp(t) == 0; 
}

SHOULD_BE_INLINE int operator!=(String& x, const char* t) 
{
  return x.cmp(t) != 0; 
}

SHOULD_BE_INLINE int operator>(String& x, const char* t)  
{
  return x.cmp(t) > 0; 
}

SHOULD_BE_INLINE int operator>=(String& x, const char* t) 
{
  return x.cmp(t) >= 0; 
}

SHOULD_BE_INLINE int operator<(String& x, const char* t)  
{
  return x.cmp(t) < 0; 
}

SHOULD_BE_INLINE int operator<=(String& x, const char* t) 
{
  return x.cmp(t) <= 0; 
}

SHOULD_BE_INLINE int operator==(SubString& x, String& y) 
{
  return x.len == y.rep->len && y.cmp(x) == 0; 
}

SHOULD_BE_INLINE int operator!=(SubString& x, String& y)
{
  return x.len != y.rep->len || y.cmp(x) != 0;
}

SHOULD_BE_INLINE int operator>(SubString& x, String& y)      
{
  return y.cmp(x) < 0;
}

SHOULD_BE_INLINE int operator>=(SubString& x, String& y)     
{
  return y.cmp(x) <= 0;
}

SHOULD_BE_INLINE int operator<(SubString& x, String& y)      
{
  return y.cmp(x) > 0;
}

SHOULD_BE_INLINE int operator<=(SubString& x, String& y)     
{
  return y.cmp(x) >= 0;
}

SHOULD_BE_INLINE int SubString::operator==(SubString&  y) 
{
  return len == y.len && cmp(y) == 0; 
}

SHOULD_BE_INLINE int SubString::operator!=(SubString&  y)
{
  return len != y.len || cmp(y) != 0;
}

SHOULD_BE_INLINE int SubString::operator>(SubString&  y)      
{
  return cmp(y) > 0;
}

SHOULD_BE_INLINE int SubString::operator>=(SubString&  y)
{
  return cmp(y) >= 0;
}

SHOULD_BE_INLINE int SubString::operator<(SubString&  y) 
{
  return cmp(y) < 0;
}

SHOULD_BE_INLINE int SubString::operator<=(SubString&  y)
{
  return cmp(y) <= 0;
}

SHOULD_BE_INLINE int SubString::operator==(const char* t) 
{
  return cmp(t) == 0; 
}

SHOULD_BE_INLINE int SubString::operator!=(const char* t) 
{
  return cmp(t) != 0;
}

SHOULD_BE_INLINE int SubString::operator>(const char* t)  
{
  return cmp(t) > 0; 
}

SHOULD_BE_INLINE int SubString::operator>=(const char* t) 
{
  return cmp(t) >= 0; 
}

SHOULD_BE_INLINE int SubString::operator<(const char* t)  
{
  return cmp(t) < 0; 
}

SHOULD_BE_INLINE int SubString::operator<=(const char* t) 
{
  return cmp(t) <= 0; 
}

