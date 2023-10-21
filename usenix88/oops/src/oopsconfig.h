#ifndef	OOPSCONFIG_H
#define	OOPSCONFIG_H

/* oopsconfig.h -- OOPS configuration file

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	oopsconfig.h,v $
 * Revision 1.2  88/01/16  23:42:01  keith
 * Remove pre-RCS modification history
 * 
*/

// oopsconfig.h -- OOPS configuration file

/* Machine Model Implementation Status:

	ATT3B		partially implemented and tested
	MASSCOMP	implemented and tested
	RTPC		partially implemented and tested
	SUN3		implemented and tested
	VAX		implemented and tested
*/

// Machine Model (only one can be defined)

//#define ATT3B
//#define MASSCOMP
//#define RTPC
#define SUN3
//#define VAX

/* Operating System Implementation Status:

	ACIS42		implemented and tested
	AIX		partially implemented and tested
	RTUATT		implemented and tested
	RTUUCB		implemented and tested
	SYSV		partially implemented, partially tested
	UCB42BSD	implemented and tested
	ULTRIX		NOT implemented, NOT tested
*/

// Operating System (only one can be defined)

//#define ACIS42
//#define AIX
//#define RTUATT
//#define RTUUCB
//#define SYSV
#define UCB42BSD
//#define ULTRIX

inline unsigned mod_sizeof_short(unsigned i)	{
  return sizeof(short)&sizeof(short)-1 ? i%sizeof(short) : i&sizeof(short)-1; }
inline unsigned mod_sizeof_int(unsigned i)	{
  return sizeof(int)&sizeof(int)-1 ? i%sizeof(int) : i&sizeof(int)-1; }
inline unsigned mod_sizeof_long(unsigned i)	{
  return sizeof(long)&sizeof(long)-1 ? i%sizeof(long) : i&sizeof(long)-1; }
inline unsigned mod_sizeof_float(unsigned i)	{
  return sizeof(float)&sizeof(float)-1 ? i%sizeof(float) : i&sizeof(float)-1; }
inline unsigned mod_sizeof_double(unsigned i)	{
  return sizeof(double)&sizeof(double)-1 ? i%sizeof(double) : i&sizeof(double)-1; }
inline unsigned mod_sizeof_ptr(unsigned i)	{
  return sizeof(void*)&sizeof(void*)-1 ? i%sizeof(void*) : i&sizeof(void*)-1; }

#if defined(ATT3B) | defined(MASSCOMP) | defined(RTPC) | defined(SUN3) | defined(VAX)

inline unsigned div_bitsize_char(unsigned i)	{ return i >> 3; }
inline unsigned mod_bitsize_char(unsigned i)	{ return i & 7; }
inline unsigned div_sizeof_short(unsigned i)	{ return i >> 1; }
inline unsigned div_sizeof_int(unsigned i)	{ return i >> 2; }
inline unsigned div_sizeof_long(unsigned i)	{ return i >> 2; }
inline unsigned div_sizeof_float(unsigned i)	{ return i >> 2; }
inline unsigned div_sizeof_double(unsigned i)	{ return i >> 3; }
inline unsigned div_sizeof_ptr(unsigned i)	{ return i >> 2; }

const int UNINITIALIZED	=0xa5a5a5a5;	// data value to flag uninitialized variables 

// Pseudo-random number generator 
const int MAX_INT = 0x7fffffff;
inline int DRAW(long& x)	{ return (x = x*1103515245 + 12345) & MAX_INT; }

#endif

#if defined(ATT3B) | defined(MASSCOMP) | defined(VAX) | defined(RTPC) | defined(SUN3)

#define STACK_GROWS_DOWN 1

#endif

#if defined(ACIS42) | defined(MASSCOMP) | defined(UCB42BSD) | defined(ULTRIX) | defined(SUN3)
// defined if select(2) implemented
#define HAVE_SELECT

#endif

// defines for interfacing with ASTs or signals

extern int ast_level;		// AST/signal nesting level

class AST_LEVEL {
public:
	AST_LEVEL()	{ ::ast_level++; }
	~AST_LEVEL()	{ ::ast_level--; }
};

// AST/signal handlers must begin with an AST_ENTER
#define AST_ENTER	AST_LEVEL ast_level_dummy

// Test to see if an AST/signal handler is executing
#define AST_ACTIVE	(::ast_level != 0)

#if defined(MASSCOMP)

extern int setpri(int);
extern int astpause(int,int);
#define	AST_DISABLE	int prior_AST_state = setpri(127)
#define AST_ENABLE	setpri(prior_AST_state)
#define	AST_STATE	setpri(-1)
#define AST_SAVE(m)	m = prior_AST_state
#define AST_RESTORE(m)	setpri(m)
#define AST_PAUSE	astpause(0,1000)

#endif

#if defined(UCB42BSD) | defined(ACIS42)

extern int sigblock(int);
extern void sigpause(int);
extern int sigsetmask(int);
#define	AST_DISABLE	int prior_AST_state = sigblock(0xFFFFFFFF)
#define AST_ENABLE	sigsetmask(prior_AST_state)
#define	AST_STATE	sigblock(0)
#define AST_SAVE(m)	m = prior_AST_state
#define	AST_RESTORE(m)	sigsetmask(m)
#define	AST_PAUSE	sigpause(0)

#endif

#if defined(SYSV)

// Signals do not queue under System V, so they are unusable

#define	AST_DISABLE
#define AST_ENABLE
#define	AST_STATE	0
#define AST_SAVE(m)
#define	AST_RESTORE(m)
#define	AST_PAUSE

#endif

#if defined(RTUUCB) | defined(UCB42BSD) | defined(ACIS42)
#define strchr(s,c) index(s,c)
#define strrchr(s,c) rindex(s,c)
#endif

#endif
