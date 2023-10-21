
struct	hash_element {
	struct	hash_element	*hash_next;	/* pointer in linked list */
	struct	special_name	*hash_special;	/* if needs special treatment */
	short			hash_index;	/* index into chartab */
};

	/*
	 * special names need a postscript subroutine to print them as
	 * troff expects - the first request to print \(xx will generate
	 * the definition of a subroutine named Cxx and the character
	 * will be printed by invoking Cxx.
	 */
struct	special_name {
	char	*troff_name,	/* the name by which troff knows the char */
		*definition,	/* the definition string in postscript */
		sn_flags;	/* flag for various values defined below */
};

#define	SN_DEFINED	01
#define	SN_FRACTION	02	/* use special set-up common to all fractions */
#define	SN_BRACKET	04	/* use special set-up common to brackets */
#define	SN_BRK_ROUNDING	010	/* correct for rounding errors in width tables */

		/*
		 * if one of these is defined then there is some common
		 * procedure to define
		 * only one can be defined for any one name
		 */
#define	SN_ANY_MULTIPLE	(SN_FRACTION | SN_BRACKET | SN_BRK_ROUNDING )


typedef	struct	hash_element	HASH_ELEMENT;
typedef	struct	special_name	SPECIAL_NAME;

extern	SPECIAL_NAME	specnames[],
			multdefs[];

#define	HASH_SIZE	37

#define	NOHASH		(HASH_ELEMENT *)0
#define	NOSPECIAL	(SPECIAL_NAME *)0
