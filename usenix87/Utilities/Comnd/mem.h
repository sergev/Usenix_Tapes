/* mem.h	Names and typedefs that I use regularly.

	1984	Mark E. Mallett

*/

#ifndef	MEM_INC				/* If not already included */

#define	MEM_INC	0			/* Avoid duplicate includes */

#define	AZTEC	1			/* For Aztec compiler specific stuff */
#define	ISCPM	1			/* For CPM */


#define	isalpha(c) (isupper(c) || islower(c))

typedef	char		BYTE;
typedef	int		BOOL;
typedef	int		WORD;
typedef	long		LONG;
typedef	int		AITYPE;		/* type which can hold both an int
					   and an address value.  */

#define	IND	static		/* For variables with no allocation
				    dependancies (don't have to be
				    on the stack) */

/**//* Various constants */


#define	TRUE	1
#define	FALSE	0
#define NUL	'\000'

#endif
