/* sp.h */
/* vi: set tabstop=4 : */

/*
 * A deleted dbm entry is denoted by a dsize of zero
 */
#define IS_DELETED(C)		(C.dsize == 0)

/*
 * Because the soundex code (part of the key) includes the first character of
 * the word, we don't need to store the first character again with the content.
 * To do this we treat the first byte of the content stored in the dbm
 * specially:  we rip off the two high order bits of the first byte of
 * the content and therefore have to restrict the value of the second
 * character of the word.  We use 'a' == 0, 'z' == 25, 'A' == 26, 'Z' == 51.
 * See spchar_map[] (misc.c) for the mapping of codes 52 through 63.
 * This behaviour is isolated in tospchar() and fromspchar().
 * If spchar_map is changed you should change the man page too.
 *
 * The word can be reconstructed by extracting the first character of the word
 * from the soundex code and then looking at the first byte of the content.
 * If the UPPER_CHAR bit is on in the first byte of the content then the first
 * character of the word should be upper case.
 * The length of the content reflects the actual number of bytes stored in the
 * dbm.  Words that have been deleted from the dbm are stored with a length of
 * zero.  Because of this, words of length 1 are treated differently: they are
 * stored with a length of 1 and with the SINGLE_CHAR bit set.  Words with
 * original length > 1 will have (length - 1) bytes stored in the content.
 * Clear?
 */
#define IS_VALID(w)		(isalpha(*w) && (*(w+1) == '\0' || isalpha(*(w+1)) \
										|| tospchar(*(w+1)) != '\0'))
#define UPPER_CHAR		0200	/* 1st char of word is upper case */
#define SINGLE_CHAR		0100	/* single char word */
#define MASK_CHAR		0077	/* mask out the indicator bits */
#define QUOTE_CHAR		0064	/* (52) code for single quote */
#define AMPER_CHAR		0065	/* (53) for ampersand */
#define PERIOD_CHAR		0066	/* (54) for period */
#define SPACE_CHAR		0067	/* (55) for blank */

/*
 * Map for first byte of dbm content (special characters)
 * Terminated by a null entry
 */
struct spchar_map {
	char spchar;
	char code;
};

#define MAXDICT			10		/* Max number of dictionaries to use */
#define MAXWORDLEN		50		/* Max word length */
#define MAXWORDS		400		/* Max number of words in one sp query */
#define WORDSPACE		20480	/* Max space used words for one sp query */

/*
 * This is the default path used by sp to find dictionaries
 * Adjust for local conditions
 */
#define DEFAULT_SPPATH	"/usr/public/lib/sp/soundex:/usr/public/lib/sp/aux.soundex"

/*
 * The following must be the maximum value containable in the count part of
 * a key.
 * It must be always be less than: (the maximum positive value that can be
 * contained in an int) - 1
 * This value imposes a limit on the number of words in a dictionary having the
 * same soundex code.  For /usr/dict/words (~25K words), a count of 255 is
 * sufficient.  Larger dictionaries will need more.  In any case you can
 * always just make another dictionary and split up your words.
 * You might want to adjust MAXWORDS and WORDSPACE (above) to reflect MAXCOUNT
 * if you've got plenty of memory.
 */
#define MAXCOUNT	1023				/* 2^10 - 1 */

/*
 * The key used by dbm looks like this:
 *
 * 	<10 bits>	<5 bits>	<9 bits>
 *	counter		first char	soundex
 *
 * A soundex value is treated as a base 7 number (maximum is 666, base 7).
 */
#define KEYSIZE		3					/* in bytes */
typedef unsigned char key_t;

#define BAD_WORD	-1					/* This must be an illegal Soundex */
#define NO_MATCH	 0
#define MATCHED		 1

extern char soundex_code_map[];

