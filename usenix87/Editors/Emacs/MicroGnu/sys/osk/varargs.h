/* varargs.h for os9/68k by Robert A. Larson	*/
/* version 0 for os9/68k C 2.0 04/20/86		*/
/* varargs is a "portable" way to write a routine that takes a variable */
/* number of arguements.  This implemination agrees with both the 4.2bsd*/
/* and Sys V documentation of varargs.  Note that just because varargs.h*/
/* is used does not mean that it is used properly.			*/
/* Ignore the "expression with little effect" warnings.  (Seems to be a */
/* compiler bug.)							*/

#define va_alist	_va_arg1, _va_arg2, _va_arg3

#define va_dcl		unsigned _va_arg1, _va_arg2, _va_arg3;

typedef	struct {
	  unsigned _va_at;  /* number of arguments used, 0, 1, or more  */
                            /* (double as first arg counts as 2)        */
	  union {
	    struct {
	      unsigned _va_uns1, _va_uns2;
	    } _va_uuns;
	    double _va_udouble;
	  } _va_union;
	  char *_va_pointer;
        } va_list;

#define	va_start(pvar)	( (pvar)._va_at = 0,				\
			  (pvar)._va_union._va_uuns._va_uns1 = _va_arg1,\
			  (pvar)._va_union._va_uuns._va_uns2 = _va_arg2,\
			  (pvar)._va_pointer = (char *) &_va_arg3	\
			)

#define va_arg(pvar, type)	(				\
	((pvar)._va_at++) ? (					\
		((pvar)._va_at == 2) ? (			\
			(sizeof(type) == 8) ? (			\
				*(((type *)((pvar)._va_pointer))++)	\
			) : (type)(				\
				(pvar)._va_union._va_uuns._va_uns2	\
			)					\
		) : (						\

			*(((type *)((pvar)._va_pointer))++)	\
		)						\
	) : (							\
		(sizeof(type) == 8) ? (type)(			\
			(pvar)._va_at++,			\
			(pvar)._va_union._va_udouble		\
		) : (type)(					\
			(pvar)._va_union._va_uuns._va_uns1	\
		)						\
	)							\
)

#define va_end(pvar)				/* va_end is simple */
