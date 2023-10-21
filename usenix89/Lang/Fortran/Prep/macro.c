/* MACRO.c
 *
 *   The routines in this file support the macro processing facilities
 * of PREP.  The style is similar to that of c #define macros, except
 * that : is used instead of #define and ; terminates the macro.  
 *   Recursive definitions are permitted, but will cause an abort
 * (and possibly a memory allocation error) on expansion.  For each
 * line submitted to expand_macros, a count of is kept for each
 * stored macro indicating how many times it has been expanded in
 * the current line.  When this exceeds MAX_CALLS, the program 
 * assumes a macro definition is recursive and stops.  Macros
 * are expanded starting with the one with the longest name, so that
 * if the definitions
 *
 * : >=		.ge. ;
 * : >		.gt. ;
 *
 * are in effect, >= will be changed to .ge. rather than .gt.=.  This
 * is only a potential problem when macro names are not fully
 * alphanumeric, since "arg" will not be flagged if "r" is defined.
 *   If a definition contains no test ( : name ; ) then name is
 * removed from the list if present.  This can be used for undefining
 * macro defs.
 *
 * 11/4/86 P.R.OVE
 */


#include "prep.h"

#define	MAX_MACROS	1000
#define MAX_CALLS	100	/* if exceeded, assume recursive */
#define MAXCHAR		127	/* max ascii char allowed in names (for bm) */

/* macro structure */
struct Macro {
	char	*name ;		/* macro name */
	int	namelength ;	/* macro name length */
	char	*text ;		/* text with parm codes */
	int	parmcount ;	/* number of parms */
	int	callcount ;	/* recursion check counter */
	int	alpha ;		/* 1 if an alphanumeric border exists */
	unsigned short	*skip1, *skip2 ; /* Boyer-Moore search tables */
} macro[MAX_MACROS], *macrop ;

int	defined_macros = 0 ;	/* number of defined macros */

/* function types */
char	*expand_macros(), *mac_expand(), *search(), *strmatch() ;
int	define_macro() ;




/* Macro processor.
 *
 *   This routine defines and expands macros.  The definition phase
 * is invoked when a leading : is found in the record.  Text is
 * then taken until the terminating ; is found.  Text following the
 * ; is ignored.  Multiline macros are permitted: they will be
 * converted to at least as many lines in the fortran program.
 * Failure to have a terminating ; will define the entire program
 * to be a macro.
 *   A NULL pointer is returned if a macro has been defined.  Otherwise
 * a pointer to the buffer with the expanded text is returned (even if
 * no macros have been expanded).  The buffer is temporary and should
 * be eliminated by the caller.
 */
 
char	*mac_proc()
{
int	i, j, size ;
char	*text, *def ;


/* see if this is a definition (look for leading :) */
for ( i=0, text=NULL; in_buff[i] != NULL; i++ ) {
	if ( in_buff[i] == BLANK | in_buff[i] == TAB ) continue ;
	if ( in_buff[i] == ':' ) text = &in_buff[i] ;
	break ;
}

if ( text == NULL ) {
/* expand macro if not a definition */
	if ( defined_macros == 0 ) {
		GET_MEM( text, strlen(in_buff) ) ;
		strcpy( text, in_buff ) ;
		return( text ) ;
	}
	else return( expand_macros( in_buff ) ) ;

}
else {

/* macro definition, get characters until ; */
	GET_MEM( def, strlen(text)+10 ) ;
	strcpy( def, text ) ;
	for ( j=1;; j++ ) {

		switch ( def[j] ) {

		case ';':	def[j+1] = NULL ;
				define_macro( def ) ;
				free( def ) ;
				return( NULL ) ;
			
		case NULL :	def[j] = '\n' ;
				def[j+1] = NULL ;
				if ( NULL == get_rec() )
					abort("MACRO: EOF in macro def") ;
				size = strlen(def) + strlen(in_buff) + 10 ;
				if ( NULL == (def=realloc(def,size)) )
					abort("MACRO: realloc error") ;
				strcat( def, in_buff ) ;
		}
	}
}
}




/* Process the macro definition in the argument string.
 * A macro has the form:
 *
 * : name( parm1, parm2, ... )	text with parms ;
 *
 * In a definition the delimeter must follow the name
 * without whitespace.  In the source code this requirement is
 * relaxed.  Alphanumeric macros must be not be next to an alpha or 
 * number character or they will not be recognized.
 *
 * This routine puts the macro string into a more easily handled
 * structure, replacing parms in the text with n, where n is a
 * binary value (128 to 128+MAX_TOKENS).
 *
 * The macro is placed in a structure of the form:
 *
 * struct Macro {
 *	char	*name ;
 *	char	namelength ;
 *	char	*text ;
 *	int	parmcount ;
 *	int	callcount ;
 *	unsigned short	*skip1, *skip2 ;
 * } macro[MAX_MACROS], *macrop ;
 *
 * where the text string has binary symbols where the parms were.
 * Returns the macro index.  The number of macros defined is stored
 * in global variable defined_macros.  Skip1 and skip2 are Boyer-Moore
 * search tables.
 * 
 * The macros are entered in order of their name length, so that
 * the macro expander will expand those with long names first.
 *
 * If no text is present the macro is removed from the list.
 */

int	define_macro(string)
char	*string ;
{
struct	Macro spare_macro ;
char	*pntr, *pntr1, *name, *parms[MAX_TOKENS],
	*parm, *text,
	*open_parens, *close_parens ;
int	i, j, l ;

/* macrop is a pointer to the macro structure that will be used */
	if ( defined_macros >= MAX_MACROS ) {
		sprintf(errline,"DEFINE_MACRO: too many macros: %s",string);
		abort( errline ) ;
	}
	macrop = &macro[defined_macros] ;
	defined_macros++ ;

/* get the name */
	name = strtokp( string, ":; \n\t(" ) ;	/* pointer to the name */
	macrop->namelength = strlen(name) ;
	GET_MEM( macrop->name, macrop->namelength ) ;
	strcpy( macrop->name, name ) ;
	macrop->alpha = isalnum( *macrop->name ) ||
			isalnum( *(macrop->name + macrop->namelength - 1) ) ;

/* set up the Boyer-Moore skip tables */
	if ( macrop->namelength > 1 ) makeskip( macrop ) ;
	else {
		macrop->skip1 = NULL ;
		macrop->skip2 = NULL ;
	}
	
/* get the parameters */
	for ( i=0; i<MAX_TOKENS; i++ ) parms[i] = NULL ;
	open_parens = strmatch(string,name) + macrop->namelength ;
	if ( NULL == line_end( open_parens ) ) {
		sprintf( errline, "DEFINE_MACRO: unterminated: %s", string ) ;
		abort( errline ) ;
	}

	/* get the text storage here to avoid memory allocation tangles */
	text = open_parens ;
	GET_MEM( macrop->text, strlen(text) ) ;

	if ( strchr( "([{\'\"", *open_parens ) ) {
		if ( NULL == ( close_parens = mat_del( open_parens ) ) ) {
			sprintf(errline,"DEFINE_MACRO: missing delimeter: %s",
				string ) ;
			abort( errline ) ;
		}
		text = close_parens + 1 ;
		i = (int)(close_parens - open_parens) - 1 ;
		pntr = open_parens + 1 ;
		*close_parens = NULL ;
		for ( i=0, pntr1 = pntr; i<MAX_TOKENS; i++, pntr1 = NULL ) {
			if ( NULL == ( parm = strtokp( pntr1, ", \t" ) ) )
				break ;
			GET_MEM( parms[i], strlen(parm) ) ;
			strcpy( parms[i], parm ) ;
		}
	}

	
/* get the text, plugging in binary codes for parameters */

	/* remove leading whitespace */
	if ( NULL == (text=line_end( text )) ) {
		sprintf( errline, "DEFINE_MACRO: unterminated: %s", string ) ;
		abort( errline ) ;
	}

	/* remove the trailing ';' but NOT whitespace */
	for ( i=strlen(text)-1; i>=0; i-- ) {
		if ( text[i] == ';' ) { text[i] = NULL ; break ; }
	}

	/* if the text is snow white at this stage, delete the entry
	 * and any other entries with the same name, then return.
	 */
	if ( NULL == line_end(text) ) {
		for ( i=defined_macros-2; i>=0; i-- ) {
			if ( NULL == strcmp( macrop->name, macro[i].name ) ) {
				mac_del(i) ;
				macrop = &macro[defined_macros-1] ;
			}
		}
		mac_del(defined_macros-1) ;
		return(-1) ;
	}

	strcpy( macrop->text, text ) ;
	text = macrop->text ;

	for ( i=0; i<MAX_TOKENS && NULL != (parm = parms[i]); i++ ) {

		/* replace parm by code, if not next to an alpha or number */
		l = strlen(parm) ;
		for ( pntr=text; NULL != (pntr1=strmatch(pntr,parm));
		pntr=pntr1+1 ) {
			if ( !( isalnum(*(pntr1-1)) && isalnum(*pntr1) ) &&
			     !( isalnum(*(pntr1+l-1)) && isalnum(*(pntr1+l)))) {
			     	*pntr1 = i + 128 ;
				strcpy( pntr1 + 1, pntr1 + strlen(parm) ) ;
			}
		}
	}


/* count parms and free up temporary storage */
	macrop->parmcount = 0 ;
	for ( i=0; i<MAX_TOKENS && NULL != parms[i]; i++ ) {
		free( parms[i] ) ;
		macrop->parmcount++ ;
	}

/* rearrange the macro table so it is sorted by name length */
	for ( i=0; i<defined_macros-1; i++ ) {
		if ( macrop->namelength < macro[i].namelength ) {
			mac_copy( &spare_macro, macrop ) ;
			for ( j=defined_macros-1; j>i; j-- )
				mac_copy( &macro[j], &macro[j-1] ) ;
			mac_copy( &macro[i], &spare_macro ) ;
			break ;
		}
		/* replace if name already exists */
		if ( macrop->namelength == macro[i].namelength &&
		     NULL == strcmp( macrop->name, macro[i].name ) ) {
			mac_swap( &macro[i], macrop ) ;
			mac_del( defined_macros - 1 ) ;
			break ;
		}
	}
	
/* return the index of the new macro */
	return(i) ;
}



/* MAC_COPY
 *
 * Copy macro p2 into p1 (just changing pointers)
 */
mac_copy( p1, p2 )
struct Macro *p1, *p2 ;
{
	p1->name = p2->name ;
	p1->namelength = p2->namelength ;
	p1->text = p2->text ;
	p1->parmcount = p2->parmcount ;
	p1->callcount = p2->callcount ;
	p1->alpha = p2 ->alpha ;
	p1->skip1 = p2->skip1 ;
	p1->skip2 = p2->skip2 ;
}



/* MAC_SWAP
 *
 * Exchange macro contents.
 */
mac_swap( p1, p2 )
struct Macro *p1, *p2 ;
{
struct Macro mac ;

	mac_copy( &mac, p1 ) ;
	mac_copy( p1, p2 ) ;
	mac_copy( p2, &mac ) ;
}



/* MAC_DEL
 *
 * Remove a macro, specified by index, and shift the table.
 */

/* the skip parameters may be null if the name is short */
#define FREE(s)		if ( NULL != s ) free(s)

mac_del( i )
int	i ;
{
int	j ;

	if ( i >= defined_macros ) return ;	/* index not defined */

	FREE( macro[i].name ) ;
	FREE( macro[i].text ) ;
	FREE( (char *)macro[i].skip1 ) ;
	FREE( (char *)macro[i].skip2 ) ;
	for ( j=i; j<defined_macros-1; j++ )
		mac_copy( &macro[j], &macro[j+1] ) ;

	defined_macros-- ;
}



/* Expand the macros in the argument string.  Returns a pointer
 * to the expanded string, which is likely to be huge.  The memory
 * should be freed as soon as possible.  The macros are expanded
 * starting with the one with the highest index.  Recursive macro
 * definitions will be flagged, but may cause a termination due to
 * allocation failure before doing so.  Caution must be exercised
 * to avoid accidental recursive definitions involving
 * more than one macro:
 *	: h	i+x ;
 *	: i(y)	func(y) ;
 *	: func	h ;
 * This will generate the successive strings (from a = func(x)):
 *	a = h(x)
 *	a = i+x(x)
 *	a = func()+x(x)
 *	a = h()+x(x) .... and so on.  Beware.
 * The string is deallocated by this routine.
 */

/* macros to check for being next to an alpha */
#define ALPHA_BEFORE(s)	( (s!=text) && (isalnum(*(s-1)) && isalnum(*( s ))) )
#define ALPHA_AFTER(s)	(               isalnum(*( s )) && isalnum(*(s+1))  )
#define NEXT_TO_ALPHA(s,l)	( ALPHA_AFTER(s+l-1) || ALPHA_BEFORE(s) )

char	*expand_macros(string)
char	*string ;
{
char	*pntr, *candidate, *text, *stop ;
int	i, hit, l ;

/* Allocate some initial storage */
	GET_MEM( text, strlen(string) ) ;
	strcpy( text, string ) ;

/* clear the recursion check counters */
	for ( i=0; i<defined_macros; i++ ) macro[i].callcount = 0 ;

/* search for macros */
	do {
	stop = text + strlen(text) - 1 ; /* length changed in mac_expand */

	for ( i=defined_macros-1; i>=0; i-- ) {
		hit = 0 ;		
		l = macro[i].namelength ;
	 	quoted( text, text ) ;	/* reset the quote function */
	 	
	/* See if macro[i] is in the present string.  If the "edges"
	 * of the macro name are alphanumeric, don't accept the string
	 * if the adjacent character is also alphanumeric.  This avoids
	 * having variables such as "arg" flagged if "r" is defined.
	 * Potential macros are also rejected if quoted with '.
	 */
		for ( pntr=text;; pntr=candidate+1 ) {
			if ( macro[i].namelength == 1 )
				candidate = strchr( pntr, macro[i].name[0] ) ;
			else
				candidate = search( pntr, stop, &macro[i] ) ;
			if ( candidate == NULL ) break ;

			/* see if its not an illusion, easiest checks 1st */
			if ( macro[i].alpha && NEXT_TO_ALPHA(candidate,l) )
				continue ;
			if ( quoted( candidate, NULL ) ) continue ;

			/* got one */
			hit = 1 ;
			text = mac_expand( text, candidate, i ) ;
			break ;
		}
		if ( hit != 0 ) break ;	/* start over if one was found */
	}
	} while( hit != 0 ) ; 


	return( text ) ;
}



/* Expand a single macro in a text string, freeing the old storage
 * and returning a pointer to the new string.  Name points to the
 * macro in the string and index is the macro index.
 */

char	*mac_expand( text, name, index )
char	*text, *name ;
int	index ;
{
char	*pntr, *newtext, *parm, *parms[MAX_TOKENS], *temp,
	*open_parens, *close_parens, *rest_of_text ;
int	i, j, size ;
unsigned char	 c ;

	macrop = &macro[index] ;
	if ( macrop->callcount++ > MAX_CALLS ) {
		sprintf( errline,
		"MAC_EXPAND: possible recursion involving: \'%s\' in\n%s",
			macrop->name, in_buff ) ;
		abort( errline ) ;
	}
	

/* get the parameters if there are any for this macro */
	for ( i=0; i<MAX_TOKENS; i++ ) parms[i] = NULL ;
	rest_of_text = &name[ macrop->namelength ] ;
	if ( macrop->parmcount != 0 ) {
		open_parens = &rest_of_text[ strspn( rest_of_text, " \t" ) ] ;
		if ( (NULL != strchr( "([{\'\"", *open_parens )) &&
		     (NULL != *open_parens )) {
			if (NULL == (close_parens=mat_del(open_parens)) ) {
				sprintf( errline,
				"MAC_EXPAND: missing delimeter: %s", in_buff ) ;
				abort( errline ) ;
			}
			i = (int)(close_parens - open_parens) - 1 ;
			pntr = open_parens + 1 ;
			c = *close_parens ;		/* save *close_parens */
			*close_parens = NULL ;		/* make parm block a string */
			i = tokenize( pntr, parms ) ;	/* break out the parms */
			*close_parens = (char)c ; 	/* restore text */
			rest_of_text = close_parens + 1 ;
		}
	}

	
/* find out how much memory we will need, then allocate */
	size = strlen(text) ;
	if ( NULL != ( pntr = macrop->text ) ) size += strlen(pntr) ;
	for ( i=0; NULL != (c=pntr[i]); i++ ) {
		if ( c > 127 && parms[c-128] != NULL )
			size += strlen(parms[c-128]) ;
	}
	GET_MEM( newtext, size ) ;


/* copy up to macro verbatim */
	*name = NULL ;
	strcpy( newtext, text ) ;

/* expand the macro itself if there is text */
	if ( NULL != (pntr = macrop->text) ) {
		for ( i=0, j=strlen(newtext); NULL != (c=pntr[i]); i++, j++ ) {
			if ( c > 127 ) {
				if ( parms[c-128] != NULL ) {
					strcat( newtext, parms[c-128] ) ;
					j += strlen( parms[c-128] ) - 1 ;
				}
				else j-- ;
			}
			else {		/* keep null terminated */
				newtext[j] = c ;
				newtext[j+1] = NULL ;
			}
		}
	}
	

/* finish off trailing text */
	strcat( newtext, rest_of_text ) ;
	
/* free up temporary storage and return pointer to new allocation */
	for ( i=0; i<MAX_TOKENS && NULL != parms[i]; i++ ) free( parms[i] ) ;
	free( text ) ;
	return( newtext ) ;
}




/* isalnum: returns nonzero value if the character argument belongs to the 
 * sets { a-z, A-Z, 0-9 }.
 */
 
int	isalnum( c )
char	c ;
{
	if ( c >= 97 && c <= 122 ) return (1) ;	/* a-z */
	if ( c >= 65 && c <= 90 ) return (2) ;	/* A-Z */
	if ( c >= 48 && c <= 57 ) return (3) ;	/* 0-9 */
	return(0) ;				/* miss */
}




/* Return TRUE if the pointer is quoted in the string (pntr marks
 * a position in the string).  The quote character the apostrophe.
 * If pntr is not in the the result will be meaningless.  This
 * routine keeps a static index and quote flag, so it doesn't have
 * to keep starting back at the beginning.  To reset it, call with
 * string != NULL pointer.  Subsequent calls should have string NULL,
 * and pntr >= the original string.  Since macros can be on multiple
 * lines, the quote flag is reset on newline.
 */
 
int	quoted( pntr, s )
char	*pntr, *s ;
{
static int	i, quote ;
static char	*string ;

	if ( s != NULL ) {
		i = 0 ;
		quote = FALSE ;
		string = s ;
	}
	else {
		for ( ; NULL != string[i] && &string[i] < pntr; i++ ) {
			switch ( string[i] ) {
				case '\'':	quote = !quote ; break ;
				case '\n':	quote = FALSE ;
			}
		}
	}
		
	return( quote ) ;
}







/* Guts of the Boyer-Moore algorithm, using already defined skip tables.
 * Returns a pointer to the location where the text is found, else a
 * NULL pointer.
 */

char *search( start, stop, macrop )
char			*start, *stop ;		/* 1st and last in buffer */
struct Macro		*macrop ;

{
register char 	*k,		/* indexes text */
		*j ;		/* indexes pattern */
register int	skip ;		/* skip distance */
char		*patend ;	/* pointer to last char in pattern */

patend = macrop->name + macrop->namelength - 1 ;

	k = start ;
	skip = macrop->namelength - 1 ;
	while ( skip <= (stop-k) ) {

		for ( j=patend, k=k+skip; *j == *k; --j, --k )
			if ( j == macrop->name ) return(k) ;

		skip = max( macrop->skip1[ *(unsigned char *)k ],
			    macrop->skip2[ j - macrop->name ]      ) ;
	}

	/* reaching here ==> search failed */
	return(NULL) ;
}




/* Generate the skip tables for Boyer-Moore string search algorithm.
 * Skip1 is the skip depending on the character which failed to match
 * the pattern (name), and skip2 is the skip depending on how far we
 * got into the name.
 */

makeskip( macrop )
struct Macro *macrop ;
{
char	*name, *p ;
unsigned short	*skip1, *skip2 ;
int	namelength ;
int	*backtrack ;	/* backtracking table for t when building skip2 */
int	c ;		/* general purpose constant */
int	j, k, t, tp ;	/* indices into skip's and backtrack */

	
	name = macrop->name ;
	namelength = macrop->namelength ;

	/* allocate space for the skip strings */ 
	GET_MEM( p, sizeof(int) * (MAXCHAR + 1) ) ;
	skip1 = (unsigned short int *)p ;
	GET_MEM( p, sizeof(int) * namelength ) ;
	skip2 = (unsigned short int *)p ;
	
	macrop->skip1 = skip1 ;
	macrop->skip2 = skip2 ;
	
	/* allocate temporary space for the backtracking table */
	GET_MEM( p, sizeof(int) * namelength ) ;
	backtrack = (int *)p ;
	
	for (c=0; c<=MAXCHAR; ++c) skip1[c] = namelength ;

	for (k=0; k<namelength; k++) {
		skip1[name[k]] = namelength - k - 1 ;
		skip2[k] = 2 * namelength - k - 1 ;
	}

	for (j=namelength - 1,t=namelength; j >= 0; --j,--t) {
		backtrack[j] = t ;
		while (t<namelength && name[j] != name[t]) {
			skip2[t] = min(skip2[t], namelength - j - 1) ;
			t = backtrack[t] ;
		}
	}

	for (k=0; k<=t; ++k) skip2[k] = min(skip2[k],namelength+t-k) ;
	tp=backtrack[t] ;

	while( tp < namelength ) {
		while( t < namelength ) {
			skip2[t] = min( skip2[t], tp-t+namelength ) ;
			++t ;
		}
		tp = backtrack[tp] ;
	}

	free(backtrack) ;
}



/* MAC_QUERY
 *
 * Determine if a given string a defined macro.  Returns the index of
 * the macro, or -1 on failure.  The list is assumed sorted by length.
 */
int	mac_query( s )
char	*s ;
{
int	index, i, l ;

	l = strlen( s ) ;

	/* Find first macro with length l (need not be efficient here) */
	for ( index=0; index<defined_macros; index++ ) {
		if ( macro[index].namelength==l ) break ;
		if ( macro[index].namelength>l || index==defined_macros-1 )
			return(-1) ;
	}

	/* Look for a match */
	for ( i=index; macro[i].namelength==l && i<defined_macros; i++ ) {
		if ( NULL == strcmp( s, macro[i].name ) ) return(i) ;
	}

	return(-1) ;
}
