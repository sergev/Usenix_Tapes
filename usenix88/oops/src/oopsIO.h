#ifndef	OOPSIO_H
#define	OOPSIO_H

/* oopsIO.h -- declarations for OOPS Object I/O functions

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	S. M. Orlow
	Systex, Inc.

Contractor:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	oopsIO.h,v $
 * Revision 1.2  88/01/16  23:41:59  keith
 * Remove pre-RCS modification history
 * 
*/
#include "Object.h"
#include <string.h>
#include <osfcn.h>

enum oioRecordTy {		// binary object I/O record type codes
	storeOnClassRef,	// class reference and object stored
	storeOnClass,		// class and object stored
	storeOnObjectRef	// object reference stored
};

// ascii store and read for null-terminated strings
extern void checkRead(istream&);
extern void syntaxErr(istream& strm, const char* expect, char was);
extern void store_Cstring(ostream&, const char*);
extern char* read_Cstring(istream&, char*, unsigned maxlen);

/* ..._AS_BINARY macros are for use in classes with all non-class members */
#define OBJECT_ADDR  ((char*)this+sizeof(BASE))
#define OBJECT_SIZE  (sizeof(*this) - sizeof(BASE))
#define STORE_OBJECT_AS_BINARY(fd)  storeBin(fd,OBJECT_ADDR,OBJECT_SIZE)
#define READ_OBJECT_AS_BINARY(fd)   readBin(fd,OBJECT_ADDR,OBJECT_SIZE)

extern void oopsStoreBinErr();

overload readBin;
overload storeBin;

inline void storeBin(fileDescTy& fd,const char* buf,unsigned nbyte) {
	if (write(fd,buf,nbyte) < 0) oopsStoreBinErr();
	}
inline void storeBin(fileDescTy& fd,const char& val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,const unsigned char& val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,const short& val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,const unsigned short& val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,int val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,unsigned int val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,const long& val) {
	storeBin(fd,(char*)&val,sizeof(val));
	}
inline void storeBin(fileDescTy& fd,const float& val) {
	storeBin(fd,(char*)&val,sizeof(float));
	}
inline void storeBin(fileDescTy& fd,const double& val) {
	storeBin(fd,(char*)&val,sizeof(double));
	}
inline void storeBin(fileDescTy& fd,const char* val) {
	storeBin(fd,strlen(val));
	storeBin(fd,val,strlen(val));
	}
inline void storeBin(fileDescTy& fd,const short* val,unsigned size) {
	storeBin(fd,size);
	storeBin(fd,(char*)val,size*sizeof(*val));
	}
inline void storeBin(fileDescTy& fd,const int* val,unsigned size) {
	storeBin(fd,size);
	storeBin(fd,(char*)val,size*sizeof(*val));
	}
inline void storeBin(fileDescTy& fd,const long* val,unsigned size) {
	storeBin(fd,size);
	storeBin(fd,(char*)val,size*sizeof(*val));
	}
inline void storeBin(fileDescTy& fd,const float* val,unsigned size) {
	storeBin(fd,size);
	storeBin(fd,(char*)val,size*sizeof(*val));
	}
inline void storeBin(fileDescTy& fd,const double* val,unsigned size) {
	storeBin(fd,size);
	storeBin(fd,(char*)val,size*sizeof(*val));
	}

extern void readBin(fileDescTy& fd,char* buf,unsigned nbyte);

inline void readBin(fileDescTy& fd,char& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,unsigned char& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,short& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,unsigned short& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,int& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,unsigned int& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,long& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,float& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,double& val) {
	readBin(fd,(char*)&val,sizeof(val));
	}
inline void readBin(fileDescTy& fd,short* val,unsigned size) {
	readBin(fd,(char*)val,size*sizeof(*val));
	}
inline void readBin(fileDescTy& fd,int* val,unsigned size) {
	readBin(fd,(char*)val,size*sizeof(*val));
	}
inline void readBin(fileDescTy& fd,long* val,unsigned size) {
	readBin(fd,(char*)val,size*sizeof(*val));
	}
inline void readBin(fileDescTy& fd,float* val,unsigned size) {
	readBin(fd,(char*)val,size*sizeof(*val));
	}
inline void readBin(fileDescTy& fd,double* val,unsigned size) {
	readBin(fd,(char*)val,size*sizeof(*val));
	}
#endif /* OOPSIOH */
