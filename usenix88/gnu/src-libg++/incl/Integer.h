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

#ifndef INTEGERH
#define INTEGERH

overload pow;
overload dec;
overload oct;
overload hex;
overload lg;
overload gcd;
overload abs;
overload sqr;
overload rnd;
overload sqrt;
overload sign;
overload bitset;
overload bitclear;
overload bittest;

#include <stream.h>


struct _Irep                    // internal Integer representations
{
  unsigned short  sgn;          // 1 means >= 0; 0 means < 0 
                                //   (space wasted for portable alignment)
  unsigned short  len;          // current length
  unsigned short  sz;           // allocated space
  unsigned short  ref;          // reference count
  unsigned short  s[1];         // represented as ushort array starting here
};


class Integer
{
  _Irep*          rep;

  void            copy(long);
  void            copy(const unsigned short*, int, int);
  void            checklength();
  Integer&        addto(int, Integer& y, int);
  Integer&        addto(int, long, int);
  void            us_multiply(unsigned short);
  void            us_divide(unsigned short, unsigned short&);
  Integer&        bitop(Integer&, int);
  Integer&        bitop(long, int);
  friend void     mk_tmpInteger(long y);

public:
                  Integer();
                  Integer(long);
                  Integer(Integer&);

                  ~Integer();

  Integer&        operator =  (Integer& y);
  Integer&        operator =  (long y);

  friend int      operator == (Integer& x, Integer& y);
  friend int      operator == (Integer& x, long y);

  friend int      operator != (Integer& x, Integer& y);
  friend int      operator != (Integer& x, long y);

  friend int      operator <  (Integer& x, Integer& y);
  friend int      operator <  (Integer& x, long y);

  friend int      operator <= (Integer& x, Integer& y);
  friend int      operator <= (Integer& x, long y);

  friend int      operator >  (Integer& x, Integer& y);
  friend int      operator >  (Integer& x, long y);

  friend int      operator >= (Integer& x, Integer& y);
  friend int      operator >= (Integer& x, long y);

  friend Integer  operator +  (Integer& x, Integer& y);
  friend Integer  operator +  (Integer& x, long y);
  friend Integer  operator +  (long x, Integer& y);

  friend Integer  operator -  (Integer& x, Integer& y);
  friend Integer  operator -  (Integer& x, long y);
  friend Integer  operator -  (long x, Integer& y);

  friend Integer  operator *  (Integer& x, Integer& y);
  friend Integer  operator *  (Integer& x, long y);

  friend Integer  operator /  (Integer& x, Integer& y);
  friend Integer  operator /  (Integer& x, long y);

  friend Integer  operator %  (Integer& x, Integer& y);
  friend Integer  operator %  (Integer& x, long y);

  friend Integer  operator << (Integer& x, Integer& y);
  friend Integer  operator << (Integer& x, long y);

  friend Integer  operator >> (Integer& x, Integer& y);
  friend Integer  operator >> (Integer& x, long y);

  friend Integer  operator &  (Integer& x, Integer& y);
  friend Integer  operator &  (Integer& x, long y);

  friend Integer  operator |  (Integer& x, Integer& y);
  friend Integer  operator |  (Integer& x, long y);

  friend Integer  operator ^  (Integer& x, Integer& y);
  friend Integer  operator ^  (Integer& x, long y);

  Integer         operator -  ();
  Integer         operator ~  ();

  void            operator ++ ();
  void            operator -- ();

  Integer&        operator += (Integer& y);
  Integer&        operator += (long y);
  Integer&        operator -= (Integer& y);
  Integer&        operator -= (long y);
  Integer&        operator *= (Integer& y);
  Integer&        operator *= (long y);
  Integer&        operator /= (Integer& y);
  Integer&        operator /= (long y);
  Integer&        operator %= (Integer& y);
  Integer&        operator %= (long y);
  Integer&        operator <<=(Integer& y);
  Integer&        operator <<=(long y);
  Integer&        operator >>=(Integer& y);
  Integer&        operator >>=(long y);
  Integer&        operator &= (Integer& y);
  Integer&        operator &= (long y);
  Integer&        operator |= (Integer& y);
  Integer&        operator |= (long y);
  Integer&        operator ^= (Integer& y);
  Integer&        operator ^= (long y);

// builtin Integer functions

/*
  friend Integer& operator <? (Integer& x, Integer& y); // min
  friend Integer& operator >? (Integer& x, Integer& y); // max
*/
  friend void     divide(Integer& x, Integer& y, Integer& q, Integer& rem);
  friend Integer  abs(Integer& x);              // absolute value
  friend long     lg (Integer& x);              // floor log base 2 of abs(x)
  friend Integer  sqr(Integer& x);              // square
  friend Integer  sqrt(Integer& x);             // floor of square root
  friend Integer  rnd(Integer& x);              // uses libc rand()
  friend Integer  gcd(Integer& x, Integer& y);  // greatest common divisor
  friend int      even(Integer& y);             // true if x is even
  friend int      odd(Integer& y);              // true if x is odd
  friend int      sign(Integer& y);             // returns -1, 0, +1
  friend Integer  pow(Integer& x, Integer& y);  // x to the y power
  friend Integer  pow(Integer& x, long y);
  friend void     bitset(Integer& x, long b);   // set b'th bit
  friend void     bitclear(Integer& x, long b); // clear b'th bit
  friend int      bittest(Integer& x, long b);  // return b'th bit

// coercion & conversion

  long            operator long();
  int             fits_in_long();

  friend char*    Itoa(Integer& x, int base = 10, int width = 0);
  friend Integer  atoI(const char* s);
  
  friend istream& operator >> (istream& s, Integer& y);
  friend ostream& operator << (ostream& s, Integer& y);

// miscellany

  int             cmp(Integer& y);
  int             cmp(long y);
  int             ucmp(Integer& y);
  int             ucmp(long y);
  void            make_unique();
  void            setlength(int);
  void            error(char* msg);
};


// Integer&functions that need not be declared friends

char*             dec(Integer& x, int width = 0);
char*             oct(Integer& x, int width = 0);
char*             hex(Integer& x, int width = 0);


// related functions on longs

long              lg(long x); 
long              gcd(long x, long y);
long              pow(long x, long y);
long              abs(long y);
long              sqr(long y);
long              sqrt(long y);
int               even(long y);
int               odd(long y);
int               sign(long y);
void              bitset(long& x, int b);
void              bitclear(long& x, int b);
int               bittest(long x, int b);


// error handlers

extern  void default_Integer_error_handler(char*);
extern  one_arg_error_handler_t Integer_error_handler;

extern  one_arg_error_handler_t 
        set_Integer_error_handler(one_arg_error_handler_t f);

#endif // INTEGERH
