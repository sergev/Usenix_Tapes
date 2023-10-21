/* oopsIO -- auxiliary functions for OOPS Object I/O

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	S.M. Orlow
        Systex,Inc.
        Beltsville, MD 20705
        301-474-0111

Contractor:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov

$Log:	oopsIO.c,v $
 * Revision 1.2  88/01/16  23:41:55  keith
 * Remove pre-RCS modification history
 * 
*/
#include <ctype.h>
#include "oopsIO.h"

static char rcsid[] = "$Header: oopsIO.c,v 1.2 88/01/16 23:41:55 keith Exp $";

extern const int OOPS_DRVDCLASSRSP,OOPS_ILLEGALMFCN,OOPS_BADARGCL,
        OOPS_BADARGSP,OOPS_BADARGCLM,OOPS_BADARGSPM,OOPS_BADCLASS,OOPS_BADSPEC,
        OOPS_RDEOF,OOPS_RDFAIL,OOPS_RDSYNERR,OOPS_RDREFERR,OOPS_RDWRONGCLASS,
	OOPS_RDUNKCLASS,OOPS_RDVERSERR,OOPS_RDABSTCLASS,
	OOPS_STROV,OOPS_READBINERR,OOPS_STOREBINERR,OOPS_READBINUNDFL;
	
extern int errno;
extern char* sys_errlist[];

extern void oopsStoreBinErr()
{
	setOOPSerror(OOPS_STOREBINERR,FATAL,sys_errlist[errno]);
}

void readBin(fileDescTy& fd,char* buf,unsigned nbyte)
{
	if (nbyte > 0) {
		int retval = read(fd,buf,nbyte);
		if (retval == -1) 
			setOOPSerror(OOPS_READBINERR,FATAL,sys_errlist[errno]);
		if (retval == 0) 
			setOOPSerror(OOPS_RDEOF,FATAL);
		if (retval < nbyte)
			setOOPSerror(OOPS_READBINUNDFL,FATAL,nbyte,retval);
	}
}

void checkRead(istream& strm)	// readFrom input error checking 
{
	char context[31];
	if (strm.good()) return;
	if (strm.eof()) setOOPSerror(OOPS_RDEOF,DEFAULT);
	strm.clear();
	context[0] = '\0';
	strm.get(context,sizeof(context),-1);
	setOOPSerror(OOPS_RDFAIL,DEFAULT,context);
}

void syntaxErr(istream& strm, const char* expect, char was)
/*
	Report readFrom syntax error.
*/
{
	char context[31];
	strm.putback(was);
	context[0] = '\0';
	strm.get(context,sizeof(context));
	setOOPSerror(OOPS_RDSYNERR,DEFAULT,expect,context);
}

void store_Cstring(ostream& strm, const char* s)
{
	register unsigned char c;
	strm << "\"";
	while (c = *s++) {
		switch (c) {
			case '\n' : { strm << "\\n"; break; }	// line feed
			case '\t' : { strm << "\\t"; break; }	// horizontal tab
			case '\b' : { strm << "\\b"; break; }	// backspace
			case '\r' : { strm << "\\r"; break; }	// carriage return
			case '\f' : { strm << "\\f"; break; }	// form feed
			case '\\' : { strm << "\\\\"; break; }
			case '"'  : { strm << "\\\""; break; }
			case '{'  : { strm << "\\["; break; }
			case '}'  : { strm << "\\]"; break; }
			default   : {
				if (isprint(c)) strm.put(c);
				else strm << "\\" << (unsigned)c << "\\";
			}
		}
	}
	strm << "\"";
}

char* read_Cstring(istream& strm, char* s, unsigned maxlen)
// Read characters in C string format from strm into buffer area s.
// maxlen is the length of s, so at most maxlen-1 characters can be
// read due to the byte required for the null terminator.
{
	unsigned char c;
	register char* p = s;
	register unsigned n = maxlen;
	strm >> WS;
	strm.get(c);
	checkRead(strm);
	if (c != '"') syntaxErr(strm,"\"",c);
	while ((strm.get(c), c != '"')) {
		if (n-- == 0) setOOPSerror(OOPS_STROV,DEFAULT,maxlen);
		if (c != '\\') *p++ = c;
		else switch ((strm.get(c), c)) {
			case 'n' : { *p++ = '\n'; break; }
			case 't' : { *p++ = '\t'; break; }
			case 'b' : { *p++ = '\b'; break; }
			case 'r' : { *p++ = '\r'; break; }
			case 'f' : { *p++ = '\f'; break; }
			case '\\' : { *p++ = '\\'; break; }
			case '"' : { *p++ = '"'; break; }
			case '[' : { *p++ = '{'; break; }
			case ']' : { *p++ = '}'; break; }
			default  : {
				int i;
				strm.putback(c);
				strm >> i; *p++ = i; strm.get(c);
				if (c != '\\') syntaxErr(strm,"\\",c);
				}
			};
	}
	if (n-- == 0) setOOPSerror(OOPS_STROV,DEFAULT,maxlen);
	*p++ = '\0';
	return s;
}
