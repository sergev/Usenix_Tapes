/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* VAX/VMS support. */
#include "dcp.h"
#ifdef VMS
#include <descrip.h>
dir_open(tplt,f21)
char *tplt,*f21;
{
int len1,cont1,ii1;
char lbuf[80],*strchr();
$DESCRIPTOR(tmp11,tplt);
$DESCRIPTOR(tmp21,lbuf);
tmp11.dsc$w_length = strlen(tplt);
tmp21.dsc$w_length = sizeof(lbuf);
cont1=0;
ii1=lib$find_file(&tmp11,&tmp21,&cont1);
/*lib$signal(ii1);*/
len1=strchr(lbuf,' ') - lbuf;
lbuf[len1]='\0';
strcpy(f21,lbuf);
if(ii1==65537) return(0);
return(-1);
}

unlink(name)
char *name;
{
/* strange how VAX C does things */
delete(name);
}

spawn(command,input,output)
char *command,*input,*output;
{
int ret;
$DESCRIPTOR(tmp1,command);
$DESCRIPTOR(tmp2,input);
$DESCRIPTOR(tmp3,output);
tmp1.dsc$w_length = strlen(command);
tmp2.dsc$w_length = strlen(input);
tmp3.dsc$w_length = strlen(output);
ret=lib$spawn(&tmp1,&tmp2,&tmp3,0,0,0,0,0,0,0);
if(debug && !remote) lib$signal(ret);
}
#endif VMS

