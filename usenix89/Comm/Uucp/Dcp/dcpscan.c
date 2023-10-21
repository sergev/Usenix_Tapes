/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* Directory scanner for DOS and DG and some unix systems */
#include "dcp.h"
/*<FF>*/
/*
**
**scandir
**
*/
#ifdef PC
int near dir_open();
#endif
#ifdef DG
#include <sys/types.h>
#include <sys/dir.h>
#endif

scandir()
{
int fn,len,i;
char cname[40],tmp[132];
#ifndef DG
sprintf(cname,"c%.4s*",rmtname);
if(dir_open(cname,tmp) != 0) return('Q');/* asm DOS call for dir */
#ifdef PC
strcpy(cfile,&tmp[30]); /* PC filename starts at DTA+30 */
#else VMS
if(debug > 3) printmsg("workfile: %s",tmp);
strcpy(cfile,tmp);  /* Normal for VMS */
#endif PC
if(fw== -1)
if((fw=open(cfile,0))==-1) return('Y');
return('S');
#else
struct direct dirbuf;
sprintf(cname,"c.%.6s",rmtname); /* sprintf(cname,"c%.4s",rmtname); */
if((i=mindex(cname,'-')) != -1) cname[i] = '?';
len=strlen(cname);
if((fn=open(spooldir,0)) <= 0) return('A');
while(read(fn,(char *)&dirbuf,sizeof(dirbuf))>0) {
        if(debug > 4) printmsg("dir file = %s cfile = %s",dirbuf.d_name,cname);
        if(strncmp(dirbuf.d_name,cname,len)==0) {
                strcpy(cfile,dirbuf.d_name);
                close(fn);
                if(fw==-1)
                        if((fw=open(cfile,0))==-1) return('Y');
                return('S');
        }
}
close(fn);
return('Q');
#endif
}
