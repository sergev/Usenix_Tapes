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
  Some of the following algorithms are based on those from 
  MIT C-Scheme bignum.c, which is
      Copyright (c) 1987 Massachusetts Institute of Technology

  with other guidance from Knuth, vol. 2

  Thanks to the creators of the algorithms.
*/

#include <Integer.h>
#include <std.h>
#include <ctype.h>

#define SHOULD_BE_INLINE

// The following three #defines should be checked for accuracy
// before compiling

#define SHORT_SIZE     16
#define LONG_SIZE      32
#define LONG_PER_SHORT 2


#define SHIFT          SHORT_SIZE
#define RADIX          ((unsigned long)(1 << SHIFT))
#define MAXNUM         ((unsigned long)((RADIX - 1)))
#define MINNUM         (RADIX >> 1)

#define MINLONG        ((unsigned long)(1 << (LONG_SIZE - 1)))
#define MAXLONG        (MINLONG - 1)

#define MINLEN         (LONG_PER_SHORT + 1)
#define MAXLEN         MAXNUM
#define MAXBITS        (MAXLEN * SHIFT)


// these should not be changed, since a few inlines in the .h
// file cheat, and use actual values

#define POSITIVE       1
#define NEGATIVE       0


// The following isolate potential trouble spots in unsigned ops

SHOULD_BE_INLINE static unsigned short extract(unsigned long x)
{
  return x & MAXNUM;
}

SHOULD_BE_INLINE static unsigned short down(unsigned long x)
{
  return (x >> SHIFT) & MAXNUM;
}

SHOULD_BE_INLINE static unsigned long up(unsigned long x)
{
  return x << SHIFT;
}

SHOULD_BE_INLINE static unsigned long u_div(unsigned long x, unsigned long y)
{
  return ((x / y) & MAXNUM);
}

SHOULD_BE_INLINE static unsigned long u_mod(unsigned long x, unsigned long y)
{
  return x % y;
}

SHOULD_BE_INLINE static unsigned long u_lsh(unsigned long x, unsigned long y)
{
  return x << y;
}

SHOULD_BE_INLINE static unsigned long u_rsh(unsigned long x, unsigned long y)
{
  return x >> y;
}

SHOULD_BE_INLINE static int is_short(unsigned long x)
{
  return (x <= MAXNUM);
}


// static globals

_Irep   _nil_Irep = { 1, 0, 1, 1, { 0 } };
Integer _tmpInteger(0);                 // temporary operands kept here


// error handling

void default_Integer_error_handler(char* msg)
{
  cerr << "Fatal Integer error. " << msg << "\n";
  exit(1);
}

one_arg_error_handler_t Integer_error_handler = default_Integer_error_handler;

one_arg_error_handler_t set_Integer_error_handler(one_arg_error_handler_t f)
{
  one_arg_error_handler_t old = Integer_error_handler;
  Integer_error_handler = f;
  return old;
}

void Integer::error(char* msg)
{
  (*Integer_error_handler)(msg);
}

// miscellaneous utilities


// copy .s fields, zero-pad if necessary

SHOULD_BE_INLINE static void nshortcpy(const unsigned short* from, short oldlen, 
                             unsigned short* to,   short newlen)
{
  short i = 0;
  if (from != 0)
  {
    for (; i < oldlen; ++i)
      *to++ = *from++;
  }
  for(; i < newlen; ++i)
    *to++ = 0;
}

SHOULD_BE_INLINE static void zeropad(unsigned short* s, short oldlen, 
                                     short newlen)
{
  for(short i = oldlen; i < newlen; ++i)
    s[i] = 0;
}

// figure out max length of result of +, -, etc.

static int calc_len(int len1, int len2, int pad)
{
  int newl;
  if (len1 >= len2)
    newl = len1;
  else
    newl = len2;
  if (pad)
    newl++;
  if (newl < MINLEN)
    newl = MINLEN;
  return newl;
}

// ensure len & sgn are correct

void Integer::checklength()
{
  int l = rep->len;
  const unsigned short* p = &(rep->s[l]);
  while (l > 0 && *--p == 0) --l;
  if ((rep->len = l) == 0) rep->sgn = POSITIVE;
}

static _Irep*  new_Irep(int l)
{
  int siz = l;
  if (siz < MINLEN)
    siz = MINLEN;
  else if (siz >= MAXLEN)
    (*Integer_error_handler)("Requested length out of range");

  _Irep* z = (_Irep *) (new char [sizeof(_Irep) + (siz - 1) * sizeof(short)]);
  z->len = l;
  z->sz  = siz;
  z->ref = 1;
  z->sgn = POSITIVE;
  return z;
}

void Integer::setlength(int newlen)
{
  if (newlen < MINLEN)
    newlen = MINLEN;

  if (rep == &_nil_Irep)
  {
    rep = new_Irep(newlen);
    zeropad(rep->s, 0, newlen);
  }
  else if (rep->ref > 1)
  {
    rep->ref--;
    _Irep* newrep = new_Irep(newlen);
    nshortcpy(rep->s, rep->len, newrep->s, newrep->sz);
    newrep->sgn = rep->sgn;
    rep = newrep;
  }
  else if (newlen > rep->sz)
  {
    _Irep* newrep = new_Irep(newlen);
    nshortcpy(rep->s, rep->len, newrep->s, newrep->sz);
    newrep->sgn = rep->sgn;
    delete rep;
    rep = newrep;
  }
  else
  {
    zeropad(rep->s, rep->len, newlen);
    rep->len = newlen;
  }
}

void Integer::make_unique()
{
  if (rep->ref > 1 || rep == &_nil_Irep)
  {
    if (rep != &_nil_Irep)
      rep->ref--;
    _Irep* oldrep = rep;
    rep = new_Irep(oldrep->len);
    rep->sgn = oldrep->sgn;
    nshortcpy(oldrep->s, oldrep->len, rep->s, rep->sz);
  }
}

void Integer::copy(const unsigned short* news, int newlen, int newsgn)
{
  if (rep == &_nil_Irep)
  {
    rep = new_Irep(newlen);
    nshortcpy(news, newlen, rep->s, rep->sz);
  }
  else if (rep->ref > 1)
  {
    rep->ref--;
    rep = new_Irep(newlen);
    nshortcpy(news, newlen, rep->s, rep->sz);
  }
  else if (newlen > rep->sz)
  {
    _Irep* newrep = new_Irep(newlen);
    nshortcpy(news, newlen, newrep->s, newrep->sz);
    delete rep;
    rep = newrep;
  }
  else
    nshortcpy(news, newlen, rep->s, rep->sz);

  rep->len = newlen;
  rep->sgn = newsgn;

  checklength();
}


static SHOULD_BE_INLINE void cvt_long(unsigned short* s, unsigned long u)
{
  s[0] = extract(u);  
  for (int i = 1; i < LONG_PER_SHORT; ++i)
  {
    u = down(u);
    s[i] = extract(u);  
  }
}

void Integer::copy(long x)
{
  unsigned short tmp[LONG_PER_SHORT];
  unsigned long u;
  int newsgn;
  if (newsgn = (x >= 0))
    u = x;
  else
    u = -x;

  cvt_long(tmp, u);
  copy(tmp, LONG_PER_SHORT, newsgn);
}

void mk_tmpInteger(long x)
{
  unsigned long u;
  if (_tmpInteger.rep->sgn = (x >= 0))
    u = x;
  else
    u = -x;
  cvt_long(_tmpInteger.rep->s, u);
  _tmpInteger.checklength();
}

long Integer::operator long()
{ 
  if (rep->len > LONG_PER_SHORT)
    return (rep->sgn == POSITIVE) ? MAXLONG : MINLONG;
  else
  {
    unsigned long a = rep->s[LONG_PER_SHORT - 1];
    if (a >= MINNUM)
      return (rep->sgn == POSITIVE) ? MAXLONG : MINLONG;
    else
    {
      for (int i = LONG_PER_SHORT - 2; i >= 0; --i)
        a = up(a) + rep->s[i];
      return (rep->sgn == POSITIVE)? a : -((long)a);
    }
  }
}

int Integer::fits_in_long()
{
  int l = rep->len;
  if (l < LONG_PER_SHORT)
    return 1;
  else if (l > LONG_PER_SHORT)
    return 0;
  else if (rep->s[LONG_PER_SHORT - 1] < MINNUM)
    return 1;
  else if (rep->sgn == NEGATIVE && rep->s[LONG_PER_SHORT - 1] == MINNUM)
  {
    for (int i = 0; i < LONG_PER_SHORT - 1; ++i)
      if (rep->s[i] != 0)
        return 0;
    return 1;
  }
  else
    return 0;
}

SHOULD_BE_INLINE static int do_cmp(const unsigned short* x, const unsigned short* y, int l)
{
  int diff = 0;
  const unsigned short* xs = &(x[l]);
  const unsigned short* ys = &(y[l]);
  while (l-- > 0 && (diff = *--xs - *--ys) == 0);
  return diff;
}
  
int Integer::cmp(Integer& y)
{
  int diff = rep->sgn - y.rep->sgn;
  if (diff == 0)
  {
    diff = rep->len - y.rep->len;
    if (diff == 0)
      diff = do_cmp(rep->s, y.rep->s, rep->len);
    return (rep->sgn == POSITIVE)? diff : -diff;
  }
  else 
    return diff;
}

int Integer::ucmp(Integer& y)
{
  int diff = rep->len - y.rep->len;
  if (diff == 0)
    diff = do_cmp(rep->s, y.rep->s, rep->len);
  return diff;
}

int Integer::cmp(long  y)
{
  if (y == 0)                   // get rid of zero case first
  {
    if (rep->len == 0)
      return 0;
    else if (rep->sgn == NEGATIVE)
      return -1;
    else
      return 1;
  }
  int ysgn = y >= 0;
  int diff = rep->sgn - ysgn;
  if (diff == 0)
  {
    diff = rep->len - LONG_PER_SHORT;
    if (diff <= 0)
    {
      unsigned short tmp[LONG_PER_SHORT];
      unsigned long uy = (y >= 0)? y : -y;
      cvt_long(tmp, uy);
      diff = do_cmp(rep->s, tmp, LONG_PER_SHORT);
    }
    return (rep->sgn == POSITIVE)? diff : -diff;
  }
  else 
    return diff;
}


int Integer::ucmp(long  y)
{
  if (y == 0)
    return rep->len;
  int diff = rep->len - LONG_PER_SHORT;
  if (diff <= 0)
  {
    unsigned short tmp[LONG_PER_SHORT];
    unsigned long uy = (y >= 0)? y : -y;
    cvt_long(tmp, uy);
    diff = do_cmp(rep->s, tmp, LONG_PER_SHORT);
  }
  return diff;
}


void Integer::us_multiply(unsigned short h)
{
  setlength(rep->len+1);
  if (h != 1)
  {
    unsigned short* s = rep->s;
    unsigned long prod = 0;
    unsigned long m = (unsigned long)h;
    int l = rep->len;
    for (int i = 0; i < l; ++i)
    {
      prod = m * (unsigned long)(s[i]) + down(prod);
      s[i] = extract(prod);
    }
    s[i] = down(prod);
  }
  checklength();
}

void Integer::us_divide(unsigned short h, unsigned short& rem)
{
  unsigned long carry = 0;
  if (h == 0)
    error("attempted division by zero");

  make_unique();
  if (h != 1)
  {
    unsigned short* s = rep->s;
    int l = rep->len;
    unsigned long carry = 0;
    unsigned long m = (unsigned long) h;
    for (int k = l - 1; k >= 0; --k)
    {
      unsigned long d = up(carry) + (unsigned long)(s[k]);
      s[k] = extract(u_div(d, m));
      carry = u_mod(d, m);
    }
    rem = extract(carry);
  }
  else
    rem = 0;
  checklength();
}


Integer& Integer::addto(int changesgn, Integer& y, int changeysgn)
{
  Integer yy = y;               // in case this == y;
  int mysgn, ysgn;
  if (rep->len == 0)
    mysgn = POSITIVE;
  else if (changesgn)
    mysgn = !rep->sgn;
  else
    mysgn = rep->sgn;
  if (yy.rep->len == 0)
    ysgn = POSITIVE;
  else if (changeysgn)
    ysgn = !yy.rep->sgn;
  else
    ysgn = yy.rep->sgn;
  int comp = ucmp(yy);
  int samesign = mysgn == ysgn;
  if (samesign)
  {
    setlength(calc_len(rep->len, yy.rep->len, samesign));
    unsigned short* s = rep->s;
    unsigned short* ys = yy.rep->s;
    int l = rep->len;
    int yl = yy.rep->len;
    unsigned long sum = 0;
    for (int i = 0; i < yl; ++i)
    {
      sum += (unsigned long)(s[i]) + (unsigned long)(ys[i]);
      s[i] = extract(sum);
      sum = down(sum);
    }
    for(; i < l && sum != 0; ++i)
    {
      sum += (unsigned long)(s[i]);
      s[i] = extract(sum);
      sum = down(sum);
    }
  }
  else if (comp == 0)
    copy(0);
  else
  {
    make_unique();
    unsigned short* s = rep->s;
    unsigned short* ys = yy.rep->s;
    int l = rep->len;
    int yl = yy.rep->len;
    unsigned long hi = 1;
    for (int i = 0; i < yl; ++i)
    {
      unsigned long cmpl = MAXNUM - (unsigned long)(ys[i]);
      hi += (unsigned long)(s[i]) + cmpl;
      s[i] = extract(hi);
      hi = down(hi);
    }
    for (; i < l && hi == 0; ++i)
    {
      hi = (unsigned long)(s[i]) + MAXNUM;
      s[i] = extract(hi);
      hi = down(hi);
    }
  }
  if (comp < 0)
    rep->sgn = ysgn;
  else
    rep->sgn = mysgn;
  checklength();
  return *this;
}

Integer& Integer::addto(int changesgn, long y, int changeysgn)
{
  int mysgn, ysgn;
  if (rep->len == 0)
    mysgn = POSITIVE;
  else if (changesgn)
    mysgn = !rep->sgn;
  else
    mysgn = rep->sgn;
  unsigned long uy;
  if (y == 0)
  {
    uy = y;
    ysgn = POSITIVE;
  }
  else if (y < 0)
  {
    uy = -y;
    if (changeysgn)
      ysgn = POSITIVE;
    else
      ysgn = NEGATIVE;
  }
  else
  {
    uy = y;
    if (changeysgn)
      ysgn = NEGATIVE;
    else
      ysgn = POSITIVE;
  }
  int comp = ucmp(y);
  int samesign = mysgn == ysgn;
  if (samesign)
  {
    setlength(calc_len(rep->len, LONG_PER_SHORT, samesign));
    unsigned short* s = rep->s;
    unsigned long sum = 0;
    int l = rep->len;
    for (int i = 0; i < LONG_PER_SHORT; ++i)
    {
      unsigned long yi = extract(uy);
      uy = down(uy);
      sum += (unsigned long)(s[i]) + yi;
      s[i] = extract(sum);
      sum = down(sum);
    }
    for(; i < l && sum != 0; ++i)
    {
      sum += (unsigned long)(s[i]);
      s[i] = extract(sum);
      sum = down(sum);
    }
  }
  else if (comp == 0)
    copy(0);
  else
  {
    make_unique();
    unsigned short* s = rep->s;
    int l = rep->len;
    unsigned long hi = 1;
    for (int i = 0; i < LONG_PER_SHORT; ++i)
    {
      unsigned long yi = extract(uy);
      uy = down(uy);
      unsigned long cmpl = MAXNUM - yi;
      hi += (unsigned long)(s[i]) + cmpl;
      s[i] = extract(hi);
      hi = down(hi);
    }
    for (; i < l && hi == 0; ++i)
    {
      hi = (unsigned long)(s[i]) + MAXNUM;
      s[i] = extract(hi);
      hi = down(hi);
    }
  }
  if (comp < 0)
    rep->sgn = ysgn;
  else
    rep->sgn = mysgn;
  checklength();
  return *this;
}


Integer& Integer::operator *= (Integer& y)
{
  Integer yy = y;               // in case this==y
  int al = rep->len;
  int bl = y.rep->len;
  int rl = al + bl;
  setlength(rl);
  rep->sgn = (rep->sgn == y.rep->sgn);
  unsigned short* as = rep->s;
  unsigned short* bs = yy.rep->s;
  for (int i = al - 1; i >= 0; --i)
  {
    unsigned long ai = (unsigned long)(as[i]);
    as[i] = 0;
    if (ai != 0)
    {
      unsigned long prod = 0;
      int k = i;
      for (int j = 0; j < bl; ++j, ++k)
      {
        prod += ai * (unsigned long)(bs[j]) + (unsigned long)(as[k]);
        as[k] = extract(prod);
        prod = down(prod);
      }
      for(; k < rl && prod != 0; ++k)
      {
        prod += (unsigned long)(as[k]);
        as[k] = extract(prod);
        prod = down(prod);
      }
    }
  }
  checklength();
  return *this;
}

Integer& Integer::operator *= (long y)
{
  int al = rep->len;
  int bl = LONG_PER_SHORT;
  int rl = al + bl;
  setlength(rl);
  int ysgn = y >= 0;
  unsigned long uy = ysgn? y : -y;
  rep->sgn = (rep->sgn == ysgn);
  unsigned short* as = rep->s;
  unsigned short bs[LONG_PER_SHORT];
  cvt_long(bs, uy);

  for (int i = al - 1; i >= 0; --i)
  {
    unsigned long ai = (unsigned long)(as[i]);
    as[i] = 0;
    if (ai != 0)
    {
      unsigned long prod = 0;
      int k = i;
      for (int j = 0; j < bl; ++j, ++k)
      {
        prod += ai * (unsigned long)(bs[j]) + (unsigned long)(as[k]);
        as[k] = extract(prod);
        prod = down(prod);
      }
      for(; k < rl && prod != 0; ++k)
      {
        prod += (unsigned long)(as[k]);
        as[k] = extract(prod);
        prod = down(prod);
      }
    }
  }
  checklength();
  return *this;
}


void divide(Integer& x, Integer& y, Integer&  quotient, Integer&  remainder)
{
  Integer q, r, b;

  if (y.rep->len == 0)
    x.error("attempted division by zero");

  int samesign = x.rep->sgn == y.rep->sgn;
  int comp = x.ucmp(y);

  if (comp < 0)
  {
    q = 0;
    r = x;
  }
  else if (comp == 0)
  {
    q = 1;
    r = 0;
  }
  else if (y.rep->len == 1)
  {
    unsigned short rem;
    q = x;
    q.us_divide(y.rep->s[0], rem);
    r = long(rem);
  }
  else
  {

// an adptation of code from knuth & from scheme

    r = x;
    b = y;

    int rl = r.rep->len;
    int bl = b.rep->len;
    int ql = rl - bl + 1;
    q.setlength(ql);

    unsigned short prescale = RADIX / (1 + b.rep->s[b.rep->len - 1]);
    r.us_multiply(prescale);
    b.us_multiply(prescale);

    unsigned short* rs = r.rep->s;
    unsigned short* bs = b.rep->s;
    unsigned short* qs = q.rep->s;

    Integer temp;               // for temporary products
    int tl = bl+1;
    temp.setlength(tl);
    unsigned short* ts = temp.rep->s;

    unsigned long d1 = (unsigned long)(bs[bl - 1]); // for estimating divisors
    unsigned long d2 = (unsigned long)(bs[bl - 2]);

    int l = ql - 1;

    for (; l >= 0; --l)
    {
      int i = l + bl;

// guess q

      unsigned long qhat;
      if (d1 == (unsigned long)(rs[i]))
        qhat = RADIX - 1;
      else
      {
        unsigned long lr = up((unsigned long)rs[i]) + (unsigned long)(rs[i-1]);
        qhat = u_div(lr, (unsigned long)d1);
      }

// adjust q
//  extra-cautious about overflow

      for(;;)
      {
        unsigned long prod = d1 * qhat;
        ts[0] = extract(prod);
        prod = down(prod) + d2 * qhat;
        ts[1] = extract(prod);
        ts[2] = extract(down(prod));
        if (do_cmp(ts, &(rs[i-2]), 3) > 0)
          --qhat;
        else
          break;
      };

// Seperately multiply and subtract --
// This is a bit slower that the knuth combined multiply-and-subtract method,
// but it avoids any conceivable problems with sign extension &
// overflow that could differ across machines (for all I know)
// when using the two's complement trick.
// Its really not much slower anyway, since the off-by-one
// subtract loop is combined with the adjust-remainder loop, and
// everything is inline.

// multiply qhat * b => temp

      unsigned long prod = 0;
      for (int k = 0; k < bl; ++k)
      {
        prod = qhat * (unsigned long)(bs[k]) + down(prod);
        ts[k] = extract(prod);
      }
      ts[k] = down(prod);

// if off by one, subtract b from temp, then temp from r

      if (do_cmp(ts, &(rs[l]), tl) > 0)
      {
        qhat--;
        unsigned long hi = 1;
        unsigned long hi_t = 1;
        int j = l;
        for (int k = 0; k < bl; ++k)
        {
          unsigned long cmpl = MAXNUM - (unsigned long)(bs[k]);
          hi_t += (unsigned long)(ts[k]) + cmpl;
          cmpl = MAXNUM - (unsigned long)(extract(hi_t));
          hi_t = down(hi_t);
          hi += (unsigned long)(rs[j]) + cmpl;
          rs[j] = extract(hi);
          hi = down(hi);
          ++j;
        }
        for (; k < tl && hi_t == 0; ++k)
        {
          hi_t = (unsigned long)(ts[k]) + MAXNUM;
          unsigned long cmpl = MAXNUM - (unsigned long)(extract(hi_t));
          hi_t = down(hi_t);
          hi += (unsigned long)(rs[j]) + cmpl;
          rs[j] = extract(hi);
          hi = down(hi);
          ++j;
        }
        for (; j < rl && hi == 0; ++j)
        {
          hi = (unsigned long)(rs[j]) + MAXNUM;
          rs[j] = extract(hi);
          hi = down(hi);
        }
      }
      else // otherwise, just adjust r
      {
        unsigned long hi = 1;
        int j = l;
        for (int k = 0; k < tl; ++k)
        {
          unsigned long cmpl = MAXNUM - (unsigned long)(ts[k]);
          hi += (unsigned long)(rs[j]) + cmpl;
          rs[j] = extract(hi);
          hi = down(hi);
          ++j;
        }
        for (; j < rl && hi == 0; ++j)
        {
          hi = (unsigned long)(rs[j]) + MAXNUM;
          rs[j] = extract(hi);
          hi = down(hi);
        }
      }

      qs[l] = extract(qhat);

    }

    unsigned short junk;
    r.us_divide(prescale, junk);

  }

  q.rep->sgn = samesign;
  r.rep->sgn = x.rep->sgn;
  q.checklength();
  r.checklength();
  quotient = q;
  remainder = r;
}

Integer operator / (Integer& x, Integer& y)
{
  Integer q, r;
  divide(x,y,q,r);
  return q;
}

Integer& Integer::operator /= (Integer& y)
{
  Integer q, r;
  divide(*this,y,q,r);
  return *this = q;
}

Integer operator % (Integer& x, Integer& y)
{
  Integer q, r;
  divide(x,y,q,r);
  return r;
}

Integer& Integer::operator %= (Integer& y)
{
  Integer q, r;
  divide(*this,y,q,r);
  *this = r;
  return *this;
}

Integer operator / (Integer& x, long y)
{
  Integer q;
  unsigned long uy = (y < 0)? -y : y;
  if (is_short(uy))
  {
    unsigned short junk;
    q = x;
    q.us_divide((unsigned short)(uy), junk);
    q.rep->sgn = (y >= 0) == x.rep->sgn;
  }
  else
  {
    mk_tmpInteger(y);
    Integer r;
    divide(x, _tmpInteger, q , r);
  }
  return q;
}

Integer operator % (Integer& x, long y)
{
  Integer r;
  unsigned long uy = (y < 0)? -y : y;
  if (is_short(uy))
  {
    unsigned short rem;
    r = x;
    r.us_divide((unsigned short)(uy), rem);
    long irem = (unsigned long)(rem) & MAXNUM; // defeat possible sign extend
    if (x.rep->sgn != POSITIVE)
      irem = -irem;
    r = irem;
  }
  else
  {
    mk_tmpInteger(y);
    Integer q;
    divide(x, _tmpInteger, q , r);
  }
  return r;
}

Integer& Integer::operator /= (long y)
{
  unsigned long uy = (y < 0)? -y : y;
  if (is_short(uy))
  {
    unsigned short junk;
    us_divide((unsigned short)(uy), junk);
    rep->sgn = (y >= 0) == rep->sgn;
  }
  else
  {
    mk_tmpInteger(y);
    Integer q, r;
    divide(*this, _tmpInteger, q , r);
    *this = r;
  }
  return *this;
}


Integer& Integer::operator %= (long y)
{
  unsigned long uy = (y < 0)? -y : y;
  if (is_short(uy))
  {
    unsigned short rem;
    us_divide((unsigned short)(uy), rem);
    long irem = (unsigned long)(rem) & MAXNUM; // defeat possible sign extend
    if (rep->sgn != POSITIVE)
      irem = -irem;
    *this = irem;
  }
  else
  {
    mk_tmpInteger(y);
    Integer q, r;
    divide(*this, _tmpInteger, q , r);
    *this = r;
  }
  return *this;
}

Integer& Integer::bitop(Integer& y, int op)
{
  int yl = y.rep->len;
  setlength(calc_len(rep->len, yl, 0));
  unsigned short* s = rep->s;
  unsigned short* ys = y.rep->s;
  int l = rep->len;
  int i = 0;
  switch (op)
  {
  case 0: // &
    for (; i < yl; ++i) s[i] &= ys[i];
    for (; i < l;  ++i) s[i] = 0;
    break;
  case 1: // |
    for (; i < yl; ++i) s[i] |= ys[i];
    break;
  case 2: // ^
    for (; i < yl; ++i) s[i] ^= ys[i];
    break;
  }
  checklength();
  return *this;
}

Integer& Integer::bitop(long y, int op)
{
  int yl = LONG_PER_SHORT;
  setlength(calc_len(rep->len, yl, 0));
  unsigned short* s = rep->s;
  unsigned short ys[LONG_PER_SHORT];
  cvt_long(ys, (unsigned long)y);
  int l = rep->len;
  int i = 0;
  switch (op)
  {
  case 0: // &
    for (; i < yl; ++i) s[i] &= ys[i];
    for (; i < l;  ++i) s[i] = 0;
    break;
  case 1: // |
    for (; i < yl; ++i) s[i] |= ys[i];
    break;
  case 2: // ^
    for (; i < yl; ++i) s[i] ^= ys[i];
    break;
  }
  checklength();
  return *this;
}

Integer  Integer::operator -()
{
  Integer x; 
  if (rep->len == 0)
    x.copy(0);
  else
    x.copy(rep->s, rep->len, !rep->sgn); 
  return x; 
}

Integer  Integer::operator ~()
{
  Integer x = *this; 
  x.make_unique();
  unsigned short* s = x.rep->s;
  int l = x.rep->len;
  for (int i = 0; i < l; ++i)
    s[i] = ~s[i];
  return x;
}

Integer& Integer::operator <<= (long y)
{
  if (rep->len == 0 || y == 0)
    return *this;

  int xl = rep->len;

  if (y >= 0)
  {
    int bw = y / SHIFT + 1;
    int sw = y % SHIFT;
    int newl = bw + xl;
    setlength(newl);
    unsigned short* s = rep->s;
    unsigned long a = 0;
    int i = newl - 1;
    for (int j = xl - 1; j >= 0; --j, --i)
    {
      a = up(a) | u_lsh((unsigned long)(s[j]),sw);
      s[i] = extract(down(a));
    }
    s[i--] = extract(a);
    for (; i >= 0; --i)
      s[i] = 0;
  }
  else
  {
    y = -y;
    int bw = y / SHIFT;
    int sw = y % SHIFT;
    int rw = SHIFT - sw;
    int newl = xl - bw;
    if (newl <= 0)
      copy(0);
    else
    {
      make_unique();
      unsigned short* s = rep->s;
      int j = bw;
      for (int i = 0; i < xl - 1; ++i, ++j)
        s[i] = extract(u_rsh((unsigned long)(s[j]),sw) | 
                       u_lsh((unsigned long)(s[j+1]), rw));
      s[i++] = extract(u_rsh((unsigned long)(s[j]), sw));
      for (; i < xl; ++i)
        s[i] = 0;
    }
  }
  checklength();
  return *this;
}


void bitset(Integer& x, long b)
{
  if (b >= 0)
  {
    int bw = b / SHIFT;
    int sw = b % SHIFT;
    if (x.rep->len <= bw || x.rep == &_nil_Irep || x.rep->ref > 1) 
      x.setlength(calc_len(x.rep->len, bw+1, 0));
    x.rep->s[bw] |= u_lsh(1, sw);
    x.checklength();
  }
}

void bitclear(Integer& x, long b)
{
  if (b >= 0)
  {
    int bw = b / SHIFT;
    int sw = b % SHIFT;
    if (x.rep->len <= bw || x.rep == &_nil_Irep || x.rep->ref > 1) 
      x.setlength(calc_len(x.rep->len, bw+1, 0));
    x.rep->s[bw] &= ~(u_lsh(1, sw));
    x.checklength();
  }
}

int bittest(Integer& x, long b)
{
  if (b >= 0)
  {
    int bw = b / SHIFT;
    int sw = b % SHIFT;
    return (bw < x.rep->len && (x.rep->s[bw] & u_lsh(1,sw)) != 0);
  }
  else
    return 0;
}


void bitset(long& x, long b)
{
  x |= u_lsh(1, b);
}

void bitclear(long& x, long b)
{
  x &= ~(u_lsh(1, b));
}

int bittest(long x, long b)
{
  return ((x & u_lsh(1, b)) != 0);
}


Integer gcd(Integer& x, Integer& y)
{
  Integer a, b;
  if (x.ucmp(y) >= 0)
  {
    a = abs(x);
    b = abs(y);
  }
  else
  {
    a = abs(y);
    b = abs(x);
  }

  Integer q, r;
  for(;;)
  {
    if (b.rep->len == 0) // b == 0
    {
      r = a;
      break;
    }
    else if (b.rep->len == 1 && b.rep->s[0] == 1) // fast test for b == 1
    {
      r = b;
      break;
    }
    else
    {
      divide(a, b, q, r);
      a = b;
      b = r;
    }
  }
  return r;
}


long gcd(long x, long y)
{
  long a = abs(x);
  long b = abs(y);

  long tmp;
  
  if (b > a)
  {
    tmp = a; a = b; b = a;
  }
  for(;;)
  {
    if (b == 0)
      return a;
    else if (b == 1)
      return b;
    else
    {
      tmp = b;
      b = a % b;
      a = tmp;
    }
  }
}


long lg(Integer& x)
{
  int xl = x.rep->len;
  if (xl == 0)
    return 0;

  long l = (xl - 1) * SHIFT - 1;
  unsigned short a = x.rep->s[xl-1];

  while (a != 0)
  {
    a = a >> 1;
    ++l;
  }
  return l;
}

long lg(long x)
{
  long l = 0;
  unsigned long a = abs(x);

  while (a > 1)
  {
    a = a >> 1;
    ++l;
  }
  return l;
}

  
Integer pow(Integer& x, long y)
{
  Integer r;
  if (sign(x) == 0 || y < 0)
    r = 0;
  else if (y == 0 || (x.rep->len == 1 && x.rep->s[0] == 1))
  {
    r = 1;
    if (x.rep->sgn == NEGATIVE && (y & 1))
      r.rep->sgn = NEGATIVE;
  }
  else
  {
    Integer b = x;
    r = 1;
    for(;;)
    {
      if (y & 1)
        r *= b;
      if ((y >>= 1) == 0)
        break;
      else
        b *= b;
    }
  }
  return r;
}

long  pow(long  x, long y)
{
  if (x == 0 || y < 0)
    return  0;
  else if (y == 0 || x == 1)
    return  1;
  else if (x == -1)
    return (y & 1)? -1 : 1;
  else
  {
    long r = 1;
    for(;;)
    {
      if (y & 1)
        r *= x;
      if ((y >>= 1) == 0)
        return r;
      else
        x *= x;
    }
  }
}

Integer rnd(Integer& x)
{
  extern int rand();

  if (sign(x) == 0)
    return x;

  Integer r;
  r.setlength(x.rep->len + 1);
  for (int i = 0; i < r.rep->len; ++i)
    r.rep->s[i] = extract((unsigned long)(rand()));
  r.rep->sgn = x.rep->sgn;
  Integer q, rem;
  divide(r, x, q, rem);
  return rem;
}


Integer sqrt(Integer& x)
{
  Integer r;
  
  if (sign(x) < 0)
    x.error("Attempted square root of negative Integer");
  else if (sign(x) == 0)
    r = 0;
  else
  {
    bitset(r, (lg(x) + 1) / 2 + 1); 
    Integer q;
    Integer rem;
    for(;;)
    {
      divide(x, r, q, rem);
      if (q >= r)
        break;
      else
      {
        r += q;
        r >>= 1;
      }
    }
  }
  return r;
}

long sqrt(long x)
{
  if (x <= 0)
    return 0;                   // no int error handler, so ...
  else if (x == 1)
    return 1;
  else
  {
    long r = x >> 1;
    long q;
    for(;;)
    {
      q = x / r;
      if (q >= r)
        return r;
      else
        r = (r + q) >> 1;
    }
  }
}

Integer atoI(const char* s)
{
  Integer r = 0;
  if (s != 0)
  {
    while (s != 0 && isspace(*s))
    if (s != 0)
    {
      char sgn = POSITIVE;
      if (*s == '-')
      {
        sgn = NEGATIVE;
        s++;
      }
      while (s != 0 && *s >= '0' && *s <= '9')
      {
        long digit = *s - '0';
        r.us_multiply(10);
        r += digit;
        ++s;
      }
      r.rep->sgn = sgn;
    }
  }
  return r;
}


extern char _form_workspace[];  // from File.cc

// this will someday be replaced with something using Obstacks

char* Itoa(Integer& x, int base = 10, int width = 0)
{
  char* e = &(_form_workspace[BUFSIZ]);
  char* s = e;
  char sgn = 0;
  Integer z;

  *--s = 0;
  
  if (sign(x) == 0)
    *--s = '0';
  else
  {
    if (sign(x) < 0)
    {
      sgn = '-';
      z = -x;
    }
    else
      z = x;
    unsigned short b = base;
    unsigned short rem;
    while (sign(z) != 0 && s > _form_workspace)
    {
      z.us_divide(b, rem);
      char ch = rem;
      if (ch >= 10)
        ch += 'a' - 10;
      else
        ch += '0';
      *--s = ch;
    }
    if (sign(z) != 0)
      z.error("formatting buffer overflow");
  }

  if (sgn && s > _form_workspace) *--s = sgn;
  int w = e - s - 1;
  while (w++ < width && s > _form_workspace)
    *--s = ' ';
  return s;
}

char* dec(Integer& x, int width = 0)
{
  return Itoa(x, 10, width);
}

char* oct(Integer& x, int width = 0)
{
  return Itoa(x, 8, width);
}

char* hex(Integer& x, int width = 0)
{
  return Itoa(x, 16, width);
}

istream& operator >> (istream& s, Integer& y)
{
  if (!s.readable())
  {
    s.error();
    return s;
  }

  char* e = &(_form_workspace[BUFSIZ - 1]);
  char* p = _form_workspace;
  char sgn = 0;
  char ch;
  s >> WS;
  while (s.good() && p < e)
  {
    s.get(ch);
    if (ch == '-')
    {
      if (sgn == 0)
        sgn = *p++ = '-';
      else
        break;
    }
    else if (ch >= '0' && ch <= '9')
      *p++ = ch;
    else
      break;
  }
  *p = 0;
  y = atoI(_form_workspace);
  return s;
}


/*

to be placed in .h file if following are changed to inlines:

extern  Integer _tmpInteger;
extern  _Irep   _nil_Irep;

*/


SHOULD_BE_INLINE Integer::Integer()
{
  rep = &_nil_Irep;
}

SHOULD_BE_INLINE Integer::Integer(long y)
{
  rep = &_nil_Irep;  copy(y);
}

SHOULD_BE_INLINE Integer::Integer(Integer&  y)
{
  rep = y.rep; rep->ref++;
}

SHOULD_BE_INLINE Integer::~Integer()
{
  if (rep != &_nil_Irep && --rep->ref == 0) delete rep;
}

SHOULD_BE_INLINE Integer&  Integer::operator = (Integer&  y)
{
  y.rep->ref++;
  if (rep != &_nil_Irep && --rep->ref == 0) delete rep;
  rep = y.rep;
  return *this;
}

SHOULD_BE_INLINE Integer&  Integer::operator = (long y)
{
  copy(y);  return *this;
}

SHOULD_BE_INLINE int operator == (Integer&  x, Integer&  y)
{
  return x.cmp(y) == 0; 
}

SHOULD_BE_INLINE int operator == (Integer&  x, long y)
{
  return x.cmp(y) == 0; 
}

SHOULD_BE_INLINE int operator != (Integer&  x, Integer&  y)
{
  return x.cmp(y) != 0; 
}

SHOULD_BE_INLINE int operator != (Integer&  x, long y)
{
  return x.cmp(y) != 0; 
}

SHOULD_BE_INLINE int operator <  (Integer&  x, Integer&  y)
{
  return x.cmp(y) <  0; 
}

SHOULD_BE_INLINE int operator <  (Integer&  x, long y)
{
  return x.cmp(y) <  0; 
}

SHOULD_BE_INLINE int operator <= (Integer&  x, Integer&  y)
{
  return x.cmp(y) <= 0; 
}

SHOULD_BE_INLINE int operator <= (Integer&  x, long y)
{
  return x.cmp(y) <= 0; 
}

SHOULD_BE_INLINE int operator >  (Integer&  x, Integer&  y)
{
  return x.cmp(y) >  0; 
}

SHOULD_BE_INLINE int operator >  (Integer&  x, long y)
{
  return x.cmp(y) >  0; 
}

SHOULD_BE_INLINE int operator >= (Integer&  x, Integer&  y)
{
  return x.cmp(y) >= 0; 
}

SHOULD_BE_INLINE int operator >= (Integer&  x, long y)
{
  return x.cmp(y) >= 0; 
}

SHOULD_BE_INLINE int sign(Integer& x)
{
  return (x.rep->len == 0) ? 0 : ( (x.rep->sgn == 1) ? 1 : -1 );
}

SHOULD_BE_INLINE int sign(long x)
{
  return (x == 0) ? 0 : ( (x > 0) ? 1 : -1 );
}

SHOULD_BE_INLINE Integer  operator +  (Integer&  x, Integer& y)
{
  Integer r = x;  r.addto(0, y, 0); return r;
}

SHOULD_BE_INLINE Integer  operator +  (Integer&  x, long y)
{
  Integer r = x;   r.addto(0, y, 0); return r;
}

SHOULD_BE_INLINE Integer  operator +  (long x, Integer&  y)
{
  Integer r = y;  r.addto(0, x, 0); return r;
}

SHOULD_BE_INLINE Integer  operator -  (Integer&  x, Integer& y)
{
  Integer r = x;  r.addto(0, y, 1); return r;
}

SHOULD_BE_INLINE Integer  operator -  (Integer&  x, long y)
{
  Integer r = x;  r.addto(0, y, 1); return r;
}

SHOULD_BE_INLINE Integer  operator -  (long x, Integer&  y)
{
  Integer r = y;  r.addto(1, x, 0); return r;
}

SHOULD_BE_INLINE Integer&  Integer::operator += (Integer& y)
{
  return addto(0, y, 0);
}

SHOULD_BE_INLINE Integer&  Integer::operator += (long y)
{
  return addto(0, y, 0);
}

SHOULD_BE_INLINE Integer&  Integer::operator -= (Integer& y)
{
  return addto(0, y, 1);
}

SHOULD_BE_INLINE Integer&  Integer::operator -= (long y)
{
  return addto(0, y, 1);
}

SHOULD_BE_INLINE void Integer::operator ++ ()
{
  addto(0, 1, 0);
}

SHOULD_BE_INLINE void Integer::operator -- ()
{
  addto(0, 1, 1);
}

SHOULD_BE_INLINE Integer  operator *  (Integer&  x, Integer& y)
{
  Integer r = x;  r *= y; return r;
}

SHOULD_BE_INLINE Integer  operator *  (Integer&  x, long y)
{
  Integer r = x;  r *= y; return r;
}

SHOULD_BE_INLINE Integer  operator << (Integer&  x, Integer&  y)
{     // correct since max Integer bits < max long
  Integer r = x; r <<= long(y); return r;
}

SHOULD_BE_INLINE Integer  operator >> (Integer&  x, Integer&  y)
{
  Integer r = x; r <<= -(long(y)); return r;
}

SHOULD_BE_INLINE Integer  operator << (Integer&  x, long  y)
{
  Integer r = x; r <<= y; return r;
}

SHOULD_BE_INLINE Integer  operator >> (Integer&  x, long  y)
{
  Integer r = x; r <<= -y; return r;
}

SHOULD_BE_INLINE Integer&  Integer::operator <<= (Integer& y)
{
  *this <<= long(y); return *this;
}

SHOULD_BE_INLINE Integer&  Integer::operator >>= (long y)
{
  *this <<= -y; return *this;
}

SHOULD_BE_INLINE Integer&  Integer::operator >>= (Integer&  y)
{
  *this <<= -(long(y)); return *this;
}

SHOULD_BE_INLINE Integer  operator &  (Integer&  x, Integer& y)
{
  Integer r = x; r.bitop(y, 0); return r;
}

SHOULD_BE_INLINE Integer  operator &  (Integer&  x, long y)
{
  Integer r = x; r.bitop(y, 0); return r;
}

SHOULD_BE_INLINE Integer  operator |  (Integer&  x, Integer& y)
{
  Integer r = x;  r.bitop(y, 1); return r;
}

SHOULD_BE_INLINE Integer  operator |  (Integer&  x, long y)
{
  Integer r = x; r.bitop(y, 1); return r;
}

SHOULD_BE_INLINE Integer  operator ^  (Integer&  x, Integer& y)
{
  Integer r = x; r.bitop(y, 2); return r;
}

SHOULD_BE_INLINE Integer  operator ^  (Integer&  x, long y)
{
  Integer r = x;  r.bitop(y, 2); return r;
}

SHOULD_BE_INLINE Integer&  Integer::operator &= (Integer& y)
{
  return bitop(y, 0);
}

SHOULD_BE_INLINE Integer&  Integer::operator |= (Integer& y)
{
  return bitop(y, 1);
}

SHOULD_BE_INLINE Integer&  Integer::operator ^= (Integer& y)
{
  return bitop(y, 2);
}

SHOULD_BE_INLINE Integer&  Integer::operator &= (long y)
{
  return bitop(y, 0);
}

SHOULD_BE_INLINE Integer&  Integer::operator |= (long y)
{
  return bitop(y, 1);
}

SHOULD_BE_INLINE Integer&  Integer::operator ^= (long y)
{
  return bitop(y, 2);
}

/*
SHOULD_BE_INLINE Integer& operator <? (Integer& x, Integer& y)
{
  if (x.cmp(y) <= 0) return x; else return y;
}

SHOULD_BE_INLINE Integer& operator >? (Integer& x, Integer& y)
{
  if (x.cmp(y) >= 0) return x; else return y;
}
*/

SHOULD_BE_INLINE Integer  pow(Integer&  x, Integer&  y)
{
  return pow(x, long(y)); // correct since max bits < max long
}

SHOULD_BE_INLINE Integer sqr(Integer& x)
{
  Integer r = x; r *= x; return r;
}

SHOULD_BE_INLINE long sqr(long x)
{
  return x * x;
}

SHOULD_BE_INLINE Integer abs(Integer&  y)
{
  Integer r;  r.copy(y.rep->s, y.rep->len, 1);   return r;
}

SHOULD_BE_INLINE long abs(long y)
{
  return (y < 0)? -y : y;
}

SHOULD_BE_INLINE int even(Integer& y)
{
  return y.rep->len == 0 || !(y.rep->s[0] & 1);
}

SHOULD_BE_INLINE int odd(Integer& y)
{
  return y.rep->len > 0 && (y.rep->s[0] & 1);
}

SHOULD_BE_INLINE int even(long y)
{
  return !(y & 1);
}

SHOULD_BE_INLINE int odd(long y)
{
  return (y & 1);
}

SHOULD_BE_INLINE ostream& operator << (ostream& s, Integer& y)
{
  return s << Itoa(y);
}

