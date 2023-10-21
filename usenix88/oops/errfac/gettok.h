/***************************************************************
**  gettok.h -- header file for gettok
**
**  Purpose:  Portable function to select tokens from a String
**
**  Date:	 June 3, 1986
**  Author:	 S.M. Orlow, Systex, Inc.
**  Contractor:  K.E. Gorlen, Computer Systems Lab, NIH
***************************************************************/

#ifndef GETTOKH
#define GETTOKH

#define white_space "\040\011\012"

extern char* gettok(char* inbuf,char* tmnl);
#endif /* GETTOKH */
