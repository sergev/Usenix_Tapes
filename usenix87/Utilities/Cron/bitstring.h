/* bitstring.h - bit string manipulation macros
 * vix 26feb87 [written]
 * vix 03mar87 [fixed stupid bug in setall/clearall]
 * vix 25mar87 [last-minute cleanup before mod.sources gets it]
 */

#ifndef	_bitstring_defined
#define	_bitstring_defined

/*
 * there is something like this in 4.3, but that's licensed source code that
 * I'd rather not depend on, so I'll reinvent the wheel (incompatibly).
 */

/*
 * except for the number of bits per int, and the other constants, this should
 * port painlessly just about anywhere.  please #ifdef any changes with your
 * compiler-induced constants (check the CC man page, it'll be something like
 * 'vax' or 'mc68000' or 'sun' or some such).  also please mail any changes
 * back to me (ucbvax!dual!ptsfa!vixie!paul) for inclusion in future releases.
 */

/*
 * (constants used internally -- these can change from machine to machine)
 */
			/*
			 * how many bits in the unit returned by sizeof ?
			 */
#define	_bit_charsize	8

			/*
			 * what type will the bitstring be an array of ?
			 */
#define	_bit_type	unsigned int

			/*
			 * how many bits in an int ?
			 */
#define	_bit_intsiz	(sizeof(_bit_type) * _bit_charsize)

			/*
			 * an int of all '0' bits
			 */
#define	_bit_0s		((_bit_type)0)

			/*
			 * an int of all '1' bits
			 */
#define	_bit_1s		((_bit_type)~0)

/*
 * (macros used internally)
 */
	/*
	 * how many int's in a string of N bits?
	 */
#define	_bit_size(N) \
	((N / _bit_intsiz) + ((N % _bit_intsiz) ? 1 : 0))

	/*
	 * which int of the string is bit N in?
	 */
#define	_bit_intn(N) \
	((N) / _bit_intsiz)

	/*
	 * mask for bit N in it's int
	 */
#define	_bit_mask(N) \
	(1 << ((N) % _bit_intsiz))

/*
 * (macros used externally)
 */
	/*
	 * declare (create) Name as a string of N bits
	 */
#define	bit_decl(Name, N) \
	_bit_type Name[_bit_size(N)];

	/*
	 * declare (reference) Name as a bit string
	 */
#define	bit_ref(Name) \
	_bit_type Name[];

	/*
	 * is bit N of string Name set?
	 */
#define	bit_test(Name, N) \
	((Name)[_bit_intn(N)] & _bit_mask(N))

	/*
	 * set bit N of string Name
	 */
#define	bit_set(Name, N) \
	{ (Name)[_bit_intn(N)] |= _bit_mask(N); }

	/*
	 * clear bit N of string Name
	 */
#define	bit_clear(Name, N) \
	{ (Name)[_bit_intn(N)] &= ~_bit_mask(N); }

	/*
	 * set bits 0..N in string Name
	 */
#define	bit_setall(Name, N) \
	{	register _bit_i; \
		for (_bit_i = _bit_size(N)-1; _bit_i >= 0; _bit_i--) \
			Name[_bit_i]=_bit_1s; \
	}

	/*
	 * clear bits 0..N in string Name
	 */
#define	bit_clearall(Name, N) \
	{	register _bit_i; \
		for (_bit_i = _bit_size(N)-1; _bit_i >= 0; _bit_i--) \
			Name[_bit_i]=_bit_0s; \
	}

#endif	_bitstring_defined
