/* Set an environment variable in the DOS environment
**
** Author:
**        uucp: ihnp4!watmath!watdcsu!broehl  Bernie Roehl
**
** Based on Code and techniques written by:
**        uucp: ..{ihnp4|seismo|ucbvax|harvard|allegra}!uwvax!uwmacc!pwu
**        arpa: uwmacc!pwu@uwvax.ARPA
**
** Modified for MicroSoft `C':
**        uucp: ihnp4!watmath!watale!broehl  Michael Shiels
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

static unsigned orig_env;
static unsigned orig_size;

unsigned char peekb(seg,offset)
short seg,offset;
{
  char far *fptr;  /* far pointer (segment + offset) to character */
  /* if you forgot to use /ze option when you compile you'll get an
  ** error here about the '*'
  */

  FP_SEG(fptr) = seg;
  FP_OFF(fptr) = offset;
  return *fptr;
}

void pokeb(seg,offset,what)
short seg,offset;
char what;
{
  char far *fptr;  /* far pointer (segment + offset) to character */

  FP_SEG(fptr) = seg;
  FP_OFF(fptr) = offset;
  *fptr = what;
}

unsigned short peekw(seg,offset)
short seg,offset;
{
  unsigned far *fptr;  /* far pointer (segment + offset) to short */

  FP_SEG(fptr) = seg;
  FP_OFF(fptr) = offset;
  return *fptr;
}

static match(s, p, env)
char *s, *p; unsigned env;
{
	while (peekb( env, p ))
	{
		if( *s != peekb( env, p++ ) )
			return 0;  /* mismatch ! */
		if( *s++ == '=' ) /* hit the = sign */
			return 1;  /* match up to = */
		if( *s == '\0' && peekb( env, p) == '=' )
			return 1;
		}
	return *s == '\0';  /* if *s is also null, match => return 1 */
}

static nextenv(s)
char *s;
{
	while (peekb( orig_env, s++ ))
		;
	return s;
}


static find_orig()
	{
	unsigned pspv, header;
	pspv = _psp;
	while (pspv != peekw( pspv, 0x0C ))
		pspv = peekw( pspv, 0x0C );

	/* we now take advantage of the undocumented structure of MSDOS
	   memory allocation blocks */

	header = pspv - 1;  /* the paragraph *before* an allocated block is a header */
	if (peekb( header, 0 ) != 'M')  /* something's wrong, somewhere */
		return -1;
	if (peekw( header, 1 ) != pspv)  /* something else is wrong, somewhere */
		return -2;

	/* Really arcane magic: address of oldest pspv, plus its size, plus 1,
	   happens to point to the DOS environment.  I sense evil spirits... */

	orig_env = peekw( header, 1 ) + peekw( header, 3 ) + 1;
	header = orig_env - 1;  /* get its memory allocation block */
	if (peekb( header, 0 ) != 'M')  /* something's wrong, somewhere */
		return -3;
	orig_size = peekw( header, 3 );  /* and get the environment size from it */
	return 0;  /* all is well with the world */
	}

static set_var(s)
	char *s;
	{
	short p, src;
	for (p = 0; peekb( orig_env, p ); p = nextenv(p, orig_env))
		if (match(s, p, orig_env))
			break;
	if (peekb( orig_env, p )) {  /* it was there; p points to it */
		if (peekb( orig_env, src = nextenv(p, orig_env) )) {  /* there's stuff after it */
			while (peekb( orig_env, src ) || peekb(orig_env, src + 1 ))
				pokeb( orig_env, p++, peekb( orig_env, src++ ) );
			pokeb( orig_env, p++, '\0' );
			}
		/* else p points to the last envvar, which is we;re losing anyway */
		}
	/* p now points to the end of the living environment */
	/* the inner + 2 below may not be necessary, but why take the chance? */
	if ((p + strlen(s) + 2) / 16 + 1 >= orig_size)
		return -1;  /* no room */
	if (src = strcspn(s, '='))  /* is there an '=' in it? */
	{
		while (*s)
			pokeb( orig_env, p++, *s++ );
		pokeb( orig_env, p++, '\0' );  /* terminate the string */
		pokeb( orig_env, p++, '\0' );  /* and the environment */
		return 0; /* string added */
	}
	else
	{
		pokeb( orig_env, p++, '\0' );  /* terminate the string */
		pokeb( orig_env, p++, '\0' );  /* and the environment */
		return 0; /* string added */
	}
}

putdenv(s)
	char *s;
	{
	if (find_orig())  /* find the original environment */
		return -1;
	return set_var(s);
	}

/* For the record, an MSDOS memory block header looks like this: */
		
struct _mb {
	char flag;        /* M for normal, Z for last one */
	unsigned owner;   /* Segment address of owner's pspv */
	unsigned size;    /* size of the block in paragraphs */
	};
	
