/***************************************************************
**  errlib.h -- header file for portable error facility
**
**  Purpose:	Error reporting function declarations
**
**  Date:	 June 6, 1986
**  Author:	 S.M. Orlow, Systex, Inc.
**  Contractor:  K.E. Gorlen, Computer Systems Lab, NIH
***************************************************************/
#ifndef ERRLIBH
#define ERRLIBH
#include <stdio.h>

extern void errprefix(const char*);
extern void errpostfix(const char*);
extern void formaterror(const char*,char*,int);
extern void seterropt(int esev,int psev,int dump,int fmt,const FILE* filep);
extern void geterropt(int& esev,int& psev,int& dump,int& fmt,FILE*& filep);
extern void printerror(const char* prefix);
extern int seterror(int err,int sev ... );

typedef int (*perrtrap)();
extern void seterrtrap(perrtrap func,int sev);
extern void clrerrtrap(int sev);
#endif /* ERRLIBH */

