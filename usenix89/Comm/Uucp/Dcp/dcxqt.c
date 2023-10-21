/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
#include "dcp.h"

/* A command formatter for DCP. RH Lamb */
/* sets up stdin and stdout on various machines */
/* There is NO command checking so watch what you send and who you */
/* let accsess your machine. "C rm /usr/*.*" could be executed. */
dcxqt()
{
int i;
char command[60],input[60],output[60],line[132];
while(dscandir())
{
sprintf(line,"%s",cfile);
fw=open(line,0);
strcpy(cfile,line);
if(debug > 2) printmsg("dcxqt:%s %d",cfile,fw);
input[0] = '\0';
output[0] = '\0';
while(getline(fw,line)==FALSE)
{
if(debug > 2) printmsg("dcxqt:%s",line); 
switch(line[0])
{
        case 'U':       break;
        case 'I':       sprintf(input,"%s",&line[2]); break;
        case 'O':       sprintf(output,"%s",&line[2]); break;
        case 'C':       strcpy(command,&line[2]); break;
        case 'R':       break;
        default :       break;
}
}
#ifndef DG
nodot(input); nodot(output); 
#endif
#ifdef PC
i=strlen(command); strcpy(line,command);
if(strlen(input)==0 && strlen(output)==0) goto syskp;
if(strlen(input)==0) {sprintf(&line[i]," >%s",output); goto syskp;}
if(strlen(output)==0) {sprintf(&line[i]," <%s",input); goto syskp;}
sprintf(&line[i]," <%s >%s",input,output);
syskp: { }
#endif PC
#ifdef DG
if(strlen(input)==0) strcpy(input,"@null");
if(strlen(output)==0) strcpy(output,"@null");
sprintf(line,"proc/def/block/inp=%s/out=%s ",input,output);
i=strlen(line);
strcpy(&line[i],command);
#endif
#ifndef VMS
if(debug > 1) printmsg("dcxqt:%s",line);
if(system(line)) return(-1);
#endif 
#ifdef VMS
/* hacks to make VMS recognize files */
if((i=strlen(input))==0) strcpy(input,"nl:");
else {input[i]='.'; input[i+1]=';'; input[i+2]='\0'; }
/* Change file record format here. MUST have a var.fdl file in this dir  */
unlink("tmp.tmp");
sprintf(line,"CONVERT/FDL=VAR %s tmp.tmp",input);
if(debug > 3) printmsg("dcxqt: %s",line);
spawn(line,"nl:","nl:");
sprintf(input,"tmp.tmp");
/* END record format conversion hack. Please fix */
if((i=strlen(output))==0) strcpy(output,"nl:");
else {output[i]='.'; output[i+1]=';'; output[i+2]='\0';}
/* hack to make VMS ignore "!" bye quoting "mit-eddie!lids" */
i=mindex(command,' '); strncpy(line,command,i+1);
line[i+1]='"'; strcpy(&line[i+2],&command[i+1]); i=strlen(line);
line[i]='"'; line[i+1]='\0';
if(debug > 1) printmsg("dcxqt:%s in:%s out:%s",line,input,output);
/* the "spawn" function seems to lose blocks of a file from time to time*/
/* The full file is transfered ok but its lost here. please FIX */
/* Cause: Incorrect file attributes. rmail wants stream_LF, spawn wants
Varable. Current hack is to user "CONVERT/FDL=VAR". */
spawn(line,input,output);
#endif
close(fw);
unlink(cfile);
unlink(input);
unlink(output);
}
return(0);
}

#ifndef DG
/* remove dots for PC or VAX file names */
nodot(string)
char *string;
{
int i,len;
char msg[80];
len=strlen(string); if((i=mindex(string,'.'))== -1) return(0);
sprintf(msg,"%.1s%.4s%.3s",string,&string[i+1],&string[len-3]);
strcpy(string,msg);
return(1);
}
#endif

/*<FF>*/
/*
**
**dscandir
** scan the directory
*/
#ifdef PC
int near dir_open();
#endif
#ifdef DG
#include <sys/types.h>
#include <sys/dir.h>
#endif

dscandir()
{
int fn,len,i;
char cname[40],tmp[132];
#ifndef DG
sprintf(cname,"x*");
if(dir_open(cname,tmp) != 0) return(0);/* asm DOS call for dir */
#ifdef PC
strcpy(cfile,&tmp[30]); /* filename starts at DTA+30 */
#else
strcpy(cfile,tmp); /*VMS */
#endif
return(-1);
#else
struct direct dirbuf;
sprintf(cname,"x."); 
len=strlen(cname);
if((fn=open(spooldir,0)) <= 0) return(0);
while(read(fn,(char *)&dirbuf,sizeof(dirbuf))>0) {
if(debug>4) printmsg("dcxqt:dir file = %s cfile = %s",dirbuf.d_name,cname);
        if(strncmp(dirbuf.d_name,cname,len)==0) {
                strcpy(cfile,dirbuf.d_name);
                close(fn);
                return(-1);
        }
}
close(fn);
return(0);
#endif
}



