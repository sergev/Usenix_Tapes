/***************************************************************
**  ErrFac.h -- header file for Class ErrorFacility
**
**  Purpose:     Class of Error Reporting Facilities
**
**  Date:	 June 30, 1986
**  Author:	 S.M. Orlow, Systex, Inc.
**  Contractor:  K.E. Gorlen, Computer Systems Lab, NIH
***************************************************************/

#ifndef ERRFACH
#define ERRFACH

#include <stream.h>
#include "errors.h"

class ErrorSpecs {
public:
	int severity;		/* severity of this error */
	char* args;		/* error msg args: I=int, S=string,D=double */
	char* text;		/* printf format string for error text */

	ErrorSpecs(int err_sev,char* err_args,char* err_text);
	void printOn(ostream&);	
	};

typedef ErrorSpecs* ErrorSpecsPt;

class ErrorFacility {
	int Code;		// facility code 
	char* shortname;	// facility short name for prefixing 
	char* basename;		// facility basename for source files
	char* longname;		// facility long name for the error msgs 
	ErrorSpecsPt* errlist;	// all errors for this facility 
	int last;		// last error in this facility
	int capacity;		// no. of errors in facility 
	int size;		// current size of error list 
	void resize(int newcapacity);
public:
	ErrorFacility(char* long_name,char* short_name,
			int fac_code,char* base_name);
	void add(ErrorSpecs&);
	void h_init(ostream&);
	void h_define(ostream&,char*,int);
	int lastError()		{ return last; }
	int code()		{ return Code; }
	void printOn(ostream&);
	};
#endif /* ERRFACH */
