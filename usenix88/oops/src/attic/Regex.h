#ifndef REGEXH
#define REGEXH

/* Regex.h -- header file for class Regex

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
        C. J. Eppich
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

Modification History:

*/

#include "String.h"
#include "OrderedCltn.h"

#define RE_NREGS 10
#define BYTEWIDTH 8

extern void bzero(char*,int);
extern Class class_Regex;
overload Regex_reader;


/* These are the command codes that appear in compiled regular expressions, one per byte.
  Some command codes are followed by argument bytes.
  A command code can specify any interpretation whatever for its arguments.
  Zero-bytes may appear in the compiled regular expression. */
enum regexpcode
  {
    unused,
    exactn,    /* followed by one byte giving n, and then by n literal bytes */
    begline,   /* fails unless at beginning of line */
    endline,   /* fails unless at end of line */
    jump,	 /* followed by two bytes giving relative address to jump to */
    on_failure_jump,	 /* followed by two bytes giving relative address of place
		            to resume at in case of failure. */
    finalize_jump,	 /* Throw away latest failure point and then jump to address. */
    maybe_finalize_jump, /* Like jump but finalize if safe to do so.
			    This is used to jump back to the beginning
			    of a repeat.  If the command that follows
			    this jump is clearly incompatible with the
			    one at the beginning of the repeat, such that
			    we can be sure that there is no use backtracking
			    out of repetitions already completed,
			    then we finalize. */
    dummy_failure_jump,  /* jump, and push a dummy failure point.
			    This failure point will be thrown away
			    if an attempt is made to use it for a failure.
			    A + construct makes this before the first repeat.  */
    anychar,	 /* matches any one character */
    charset,     /* matches any one char belonging to specified set.
		    First following byte is # bitmap bytes.
		    Then come bytes for a bit-map saying which chars are in.
		    Bits in each byte are ordered low-bit-first.
		    A character is in the set if its bit is 1.
		    A character too large to have a bit in the map
		    is automatically not in the set */
    charset_not, /* similar but match any character that is NOT one of those specified */
    start_memory, /* starts remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    stop_memory, /* stops remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    duplicate,    /* match a duplicate of something remembered.
		    Followed by one byte containing the index of the memory register. */
  };

/* Structure to store "register" contents data in.

   Pass the address of such a structure as an argument to re_match, etc.,
   if you want this information back.

   start[i] and end[i] record the string matched by \( ... \) grouping i,
   for i from 1 to RE_NREGS - 1.
   start[0] and end[0] record the entire string matched. */

typedef struct re_registers
  {
    int start[RE_NREGS];
    int end[RE_NREGS];
  }regTy,*regPtTy;

/* This data structure is used to represent a compiled regular expression. */
typedef struct re_pattern_buffer
{
    char *buffer;	/* Space holding the compiled pattern commands. */
    int allocated;	/* Size of space that  buffer  points to */
    int used;	/* Length of portion of buffer actually occupied */
    char fastmap[1<<BYTEWIDTH];	/* Pointer to fastmap. */
			/* re_search uses the fastmap, if there is one,
			   to skip quickly over totally implausible characters */
    char fastmap_accurate;
			/* Set to zero when a new pattern is stored,
			   set to one when the fastmap is updated from it. */
    char can_be_null;   /* Set to one by compiling fastmap
			   if this pattern might match the null string.
			   It does not necessarily match the null string
			   in that case, but if this is zero, it cannot.
			   2 as value means can match null string
			   but at end of range or before a character
			   listed in the fastmap.  */
}cmpexpTy,*cmpexpPtTy;

class Regex: public String {
    cmpexpTy cmpexp;                // compiled regular expression
    void init_cmpexp()   
    // Initialize contents of cmpexp.
    { 
	char* fastmap = cmpexp.fastmap;
	cmpexp.buffer=0;
	cmpexp.fastmap_accurate=cmpexp.allocated=cmpexp.used=0;
	bzero (fastmap, (1<<BYTEWIDTH));
    }
    void re_compile_pattern();
    void re_compile_fastmap();
    int re_match(const String& str, int pos, regPtTy regs, int mstop);
    Range re_search(const String& str, int range, regPtTy regs, int mstop, int startpos);
    void cmpexp_deepenShallowCopy();
    void errArgs(char*,char*);
    void errRegex(char*);
protected:
    Regex(fileDescTy&,Regex&);
    Regex(istream&,Regex&);
    friend void Regex_reader(istream& strm, Object& where);
    friend void Regex_reader(fileDescTy& fd, Object& where);
public:
    void debug();
    Regex(const char*, unsigned oflow=DEFAULT_STRING_EXTRA);
    Regex(char&, unsigned l=1, unsigned oflow=DEFAULT_STRING_EXTRA);
    Regex(unsigned oflow=DEFAULT_STRING_EXTRA);
    Regex(const String&,unsigned oflow=DEFAULT_STRING_EXTRA);
    Regex(const SubString&, unsigned oflow=DEFAULT_STRING_EXTRA);
    Regex(const Regex&);
    Regex(const Regex&, unsigned oflow);
    ~Regex()                  { free(cmpexp.buffer); }
    void operator=(const char*);
    void operator=(const String&);
    void operator=(const SubString&);
    void operator=(const Regex&);
    void operator&=(const String&)  { shouldNotImplement("operator&"); }
    void operator&=(const SubString&)  { shouldNotImplement("operator&"); }
    void operator&=(const char*)  { shouldNotImplement("operator&"); }
    virtual void deepenShallowCopy();
    virtual const Class* isA();
    virtual void scanFrom(istream& strm);
    virtual void toAscii();
    virtual void toLower();
    virtual void toUpper();
    bool exactMatch(const String&);
    Range match(const String&, int pos=0);
    bool match(const String&, Range&, int pos=0);
    OrderedCltn* match(const String&, OrderedCltn&, int pos=0);
    bool search(const String&, Range&, int startpos=0);
    Range search(const String&, int startpos=0);
    OrderedCltn* search(const String&, OrderedCltn&, int startpos=0);
    Range search(const String&, int range, int mstop, int startpos=0);
    OrderedCltn* search(const String&, OrderedCltn&, int range, int mstop, int startpos=0);
    OrderedCltn* repeatSearch(const String&, OrderedCltn&, int range,
 int stop, int startpos=0);
    OrderedCltn* overlapSearch(const String&, OrderedCltn&, int range,
 int stop, int startpos=0);
};


#endif
