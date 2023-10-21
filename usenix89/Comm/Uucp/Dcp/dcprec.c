/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* file recieve routines */
#include "dcp.h"
static unsigned char rpacket[MAXPACK];
/*<FF>*/
/*********************** MISC SUB SUB PROTOCOL *************************/
/*
**
**schkdir
** scan the dir
*/
schkdir()
{
char c;
c = scandir();
if(c =='Q')
{
return('Y');
}
if(c =='S')
{
sprintf(rpacket,"HN");
if((*sendpkt)(rpacket,0,1)) return(0);
}
return('B');
}

/*<FF>*/
/*
 *
 *      endp() end protocol
 *
*/
endp()
{
sprintf(rpacket,"HY");
(*sendpkt)(rpacket,0,2); /* dont wait for ACK */
(*closepk)();
return('P');
}

/*<FF>*/
/***********************RECIEVE PROTOCOL**********************/
/*
 *  r d a t a
 *
 *  Receive Data
 */
rdata()
{
        int len;
        if((*getpkt)(rpacket,&len)) return(0);
        if(len == 0)
        {
        close(fp);
        sprintf(rpacket,"CY");
        if((*sendpkt)(rpacket,0,1)) return(0);
		if(debug) printmsg("transfer complete");
        return('F');
        }
    write(fp,rpacket,len);/* Write the data to the file */
    return('D');/* Remain in data state */
}


/*<FF>*/
/*
 *  r f i l e
 *
 *  Receive File Header
 */
rfile()
{
        int len,i;
        char *nptr,filenam1[132]; /*Holds the converted file name */
        char *fromptr,*toptr;
        toptr=tofile; fromptr=fromfile; nptr=filenam1;
        if((*getpkt)(rpacket,&len)) return(0);
        if((rpacket[0]&0x7f) == 'H') { return('C');}
        strncpy(filenam1,rpacket,len);

/* Convert upper case to lower */
for (nptr=filenam1; *nptr != '\0'; nptr++)
if (*nptr >= 'A' && *nptr <= 'Z')  *nptr |= 040;
nptr = filenam1;

toptr=tofile; nptr=filenam1; sscanf(&nptr[2],"%s %s ",fromptr,toptr);

toptr = tofile;
#ifndef DG
if((i=mindex(toptr,'.')) > 0) {nptr=filenam1; len=strlen(toptr);
/* MASH the file for PC and VMS */
sprintf(nptr,"%.1s%.4s%.3s",toptr,&toptr[i+1],&toptr[len-3]); }
#else
sprintf(nptr,"%s",toptr);
#endif

if ((fp=creat(nptr,0775)) == -1) { /* Try to open a new file */
  if(debug) printmsg("cannot create %s",nptr); /* Give up if can't */
  return('A'); }

if(debug) printmsg("Receiving %s as %s",fromptr,nptr);
sprintf(rpacket,"SY");
if((*sendpkt)(rpacket,0,1)) return(0);
return('D'); /* Switch to data state */
}

/*<FF>*/
/*
 *  r i n i t
 *
 *  Receive Initialization
 */
rinit()
{
if((*openpk)()) return(0);
return('F');
}
