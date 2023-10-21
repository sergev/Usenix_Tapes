/***************************************************************
**  errors.h -- header file for portable error facility
**
**  Purpose:	Error facility declarations
**
**  Date:	 June 30, 1986
**  Author:	 S.M. Orlow, Systex, Inc.
**  Contractor:  K.E. Gorlen, Computer Systems Lab, NIH
***************************************************************/
#ifndef ERRORSH
#define ERRORSH

#include <stream.h>

const int MAX_MSG_ARG =	8;      /* max numbers of args per msg */
const int MAX_FACILITIES = 32;  /* max no. of facilities */
const int SYS_LAST_ERROR = 34;	/* last UNIX system error */

/* severity levels */
enum severity_level { SUCCESS = 0,
		      INFORMATION = -1,
		      INFO = -1,
		      WARNING = -2,
		      ERROR = -3,
		      FATAL = -4,
		      DEFAULT = 1 };

/* Error Code Definition */
const int FAC_SHIFT =    12;	    // right shift count to get facility code
const int OFFSET_MASK =  0xfff;     // mask for offset portion of error 
const int FAC_MASK =     0xfff000;  // mask for facility portion 

inline int FACILITY_CODE(int ER) { return ((ER&FAC_MASK)>>FAC_SHIFT); }
inline int OFFSET_INDEX(int ER)  { return (ER&OFFSET_MASK); }

class ErrSpecs {
public:
	int severity;		/* severity of this error */
	char* args;		/* error msg args: I=int, S=string,D=double */
	char* text;		/* printf format string for error text */
	};


class ErrFac {
public:
	char* longname;		/* facility long name for the error msgs */
	ErrSpecs* errlist;	/* all errors for this facility */
	int last;		/* last error in this facility */
	};
#endif /* ERRORSH */
