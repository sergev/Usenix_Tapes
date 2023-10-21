#include "errors.h"
#include "errlib.h"
#include <stdio.h>
#include <string.h>

extern int errno;

typedef union anyv {
		char c;
		int i;
		long l;
		long ll[2];
		float f;
		double d;
		char* s;
		} AnyVal;

ErrFac errfac[MAX_FACILITIES];	// error reporting facilities
int severity;				// current severity
char msg[132];				// current error msg buffer
FILE* fout = stderr;			// current error FILE
int print_severity = WARNING;		// print threshhold
int exit_severity  = ERROR;		// exit threshhold
#define format_severity 1
#define format_facility 2
static char* sevmsg[5] = { "success","info","warning","error","fatal" };
int format_cntl    = 3;	// format control: 0=~facility&~severity,
				        // 1=~facility, 2=~severity, 3=all
int dump_cntl = 0;			// dump control: not implemented

static char* bad_fac_msg = "*** Invalid Facility (%d) -- from error (%d)\n";
static char* bad_err_msg="*** Error No.(%d) out of range -- from error (%d)\n";

char* argv;

void seterropt(int esev, int psev, int, int fmt, const FILE* filep) {
	exit_severity = esev;
	print_severity = psev;
	format_cntl = fmt;
	if ( filep == 0 )
	  fout = stderr;
         else
          fout = filep;
	}

void geterropt(int& esev,int& psev,int& dump,int& fmt,FILE*& filep) {
	esev = exit_severity;
	psev = print_severity;
	fmt = format_cntl;
	dump = dump_cntl;
	filep = fout;
	}

void StrCpy(char* p,char* q) {
	int flag = 1;
	while ( *q != '\0' ) {
	  if ( flag&&(*q == '\001') ) {
	    *(p++) = '%';
	    ++q;
	    flag = 0;
	    }
           else
            *(p++) = *(q++);
          };
        *p = '\0';
	}

void printerror(const char* prefix) {
	char facnameprefix[80];
	char severityprefix[80];
	if ( format_cntl&format_facility ) 
	  sprintf(facnameprefix,"%s: ",errfac[FACILITY_CODE(errno)].longname);
         else
	  facnameprefix[0] = '\0';
        if ( format_cntl&format_severity )
	  sprintf(severityprefix,"%s: ",sevmsg[-severity]);
	 else
          severityprefix[0] = '\0';
	fprintf(fout,"%s%s%s%s\n",prefix,facnameprefix,severityprefix,msg);
	}

int seterror(int err,int sev ... ) {
// printf("seterror> Error No: %d  Severity: %d\n",err,sev);
	errno = err;
	AnyVal args[MAX_MSG_ARG];
        char buf[132],buf0[132];

	/* facility code */
	int fac_code = FACILITY_CODE(err);

	if ( (fac_code < 0)||(fac_code > MAX_FACILITIES) ) { // bad facility
	   sprintf(buf,bad_fac_msg,fac_code,err);
	   severity = ERROR;
	   }
	  else { // valid facility
           ErrFac fac = errfac[fac_code];

/*
printf("seterror> Facility Code: %d  Facility Name: %s  Last Error: %d\n",
	fac_code,fac.longname,fac.last);
*/
	   /* error number */
	   int err_code = OFFSET_INDEX(err);
	   int first_code = err&FAC_MASK;

	   if ( (err < first_code)||(err > fac.last) ) { // bad error
	     sprintf(buf,bad_err_msg,err_code,err);
	     severity = ERROR;
	     }
	    else { // valid error code
	     severity = (sev < 0)? sev:fac.errlist[err_code].severity;
	     strcpy(buf,fac.errlist[err_code].text);
	     char* pp = &buf[0];
	     while ( *pp != '\0' ) {	// remove all control '%'
		if ( *pp == '%' ) {
			if ( *(pp+1) == '%' ) 
			++pp;			// skip "%%"
			else
			*pp = '\001';		// change '%' to '^A'
		};
		++pp;
		};
     
	     char* p = fac.errlist[err_code].args;
	     int nargs = strlen(p);		// number of arguments
	     int* argv = new int[nargs+1];	// argument list
	     argv = &sev; 			// *argv[0] = sev
	     ++argv;				// *argv is now first arg
     
	     int inx = 0;
	     while ( *p != '\0' ) { // set up next arg
		switch ( *(p++) ) {
		  case 'I':
	            args[inx++].i = *(argv++);
    	      	    break;
		  case 'S':
	            args[inx++].s = (char*)*(argv++);
	            break;
		  case 'D':
	            args[inx++].d = *(double*)argv;
	            argv += 2;
	            break;
	          default : // ignored
	            break;
	          };// end switch
	     StrCpy(buf0,buf);
	     sprintf(buf,buf0,args[inx-1]);
	     };// end while
	    strcpy(msg,buf);
	     };
	    };

	if ( severity <= print_severity )  // print error msg
	  printerror("");

	if ( severity <= exit_severity ) exit(1);
	return severity;
	}
	
