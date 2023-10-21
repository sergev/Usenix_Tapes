/* Set an environment variable */

/* Written by Bernie Roehl@watdcsu.uucp, July 1986 */

#include <stdio.h>
#include <sys.h>

static unsigned orig_env;
static unsigned orig_size;

static match(s, p, env)
	char *s, *p; unsigned env;
	{
	while (peekb(p, env)) {
		if (*s != peekb(p++, env))
			return 0;  /* mismatch ! */
		if (*s++ == '=') /* hit the = sign */
			return 1;  /* match up to = */
		}
	return *s == '\0';  /* if *s is also null, match => return 1 */
	}

static nextenv(s)
	char *s;
	{
	while (peekb(s++, orig_env))
		;
	return s;
	}


static find_orig()
	{
	unsigned psp, header;
	psp = _showcs() - 0x10;
	while (psp != peekw(0x0C, psp))
		psp = peekw(0x0C, psp);

	/* we now take advantage of the undocumented structure of MSDOS
	   memory allocation blocks */

	header = psp - 1;  /* the paragraph *before* an allocated block is a header */
	if (peekb(0, header) != 'M')  /* something's wrong, somewhere */
		return -1;
	if (peekw(1, header) != psp)  /* something else is wrong, somewhere */
		return -2;

	/* Really arcane magic: address of oldest psp, plus its size, plus 1,
	   happens to point to the DOS environment.  I sense evil spirits... */

	orig_env = peekw(1, header) + peekw(3, header) + 1;
	header = orig_env - 1;  /* get its memory allocation block */
	if (peekb(0, header) != 'M')  /* something's wrong, somewhere */
		return -3;
	orig_size = peekw(3, header);  /* and get the environment size from it */
	return 0;  /* all is well with the world */
	}

static set_var(s)
	char *s;
	{
	char *p, *src;
	for (p = 0; peekb(p, orig_env); p = nextenv(p, orig_env))
		if (match(s, p, orig_env))
			break;
	if (peekb(p, orig_env)) {  /* it was there; p points to it */
		if (peekb(src = nextenv(p, orig_env), orig_env)) {  /* there's stuff after it */
			while (peekb(src, orig_env) || peekb(src + 1, orig_env))
				pokeb(peekb(src++, orig_env), p++, orig_env);
			pokeb('\0', p++, orig_env);
			}
		/* else p points to the last envvar, which is we;re losing anyway */
		}
	/* p now points to the end of the living environment */
	/* the inner + 2 below may not be necessary, but why take the chance? */
	if ((p + strlen(s) + 2) / 16 + 1 >= orig_size)
		return -1;  /* no room */
	if (src = index(s, '='))  /* is there an '=' in it? */
		if (src[1] == '\0') { /* is there nothing after it?  (i.e. XYZ= ) */
			pokeb('\0', p++, orig_env);  /* terminate the string */
			pokeb('\0', p++, orig_env);  /* and the environment */
			return 0; /* string deleted okay */
			}
	while (*s)
		pokeb(*s++, p++, orig_env);
	pokeb('\0', p++, orig_env);  /* terminate the string */
	pokeb('\0', p++, orig_env);  /* and the environment */
	return 0; /* string added */
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
	unsigned owner;   /* Segment address of owner's PSP */
	unsigned size;    /* size of the block in paragraphs */
	};
	
