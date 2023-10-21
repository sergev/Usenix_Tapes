#define NULL 0
#define MAXSTRING 132

#include <ctype.h>
#include <rms.h>
#include <stdio.h>
#include <stsdef.h>
#include <ssdef.h>
#include <descrip.h>

struct dsc$descriptor_s desc,desc1;

main (argc,argv)
    int argc;
    char **argv;

{   char target[MAXSTRING],todir[MAXSTRING],dirlist[MAXSTRING];
    char dirname[MAXSTRING],*startname,*endname;
    int i,context;

    if (!getsymbol("CDPATH",dirlist)) {
	return(SS$_NORMAL);
    }
    else {
	do {
	    startname = strrchr(dirlist,' ');
	    if (!startname) {
		strcpy(todir,dirlist);
		*dirlist = '\0';
	    }
	    else {
		strcpy(todir,startname+1);
		do {
		   *(startname--) = '\0';
		} while (startname >= dirlist && startname == ' ');
	    }
	    strcat(todir,"*.dir");
	    context = 0;
	    while (findfile(todir,target,&context)) {
		endname = strchr(target,' ');
		*endname = '\0';
		startname = strrchr(target,']')+1;
		endname = strrchr(target,'.');
		i = endname-startname;
		strncpy(dirname,startname,i);
		dirname[i] = '\0';
		*(startname-1) = '\0';
		strcat(target,".");
		strcat(target,dirname);
		strcat(target,"]");
		setlogical(dirname,target);
	    }
	} while (*dirlist);
    }
}
findfile(file_name,result_name,context)
char *file_name,*result_name;
int *context;
{   int status;

    setdesc(&desc,file_name,strlen(file_name));
    setdesc(&desc1,result_name,MAXSTRING);
    status = lib$find_file(&desc,&desc1,context,0,0,0,0);
    if (status & STS$M_SUCCESS) {
	return(1);
    }
    else {
	return(0);
    }
}
getlogical(logical_name,value)
char *logical_name,*value;
{   int valuelen;
    char cap_name[MAXSTRING];

    upshift(cap_name,logical_name);
    setdesc(&desc,cap_name,strlen(cap_name));
    setdesc(&desc1,value,MAXSTRING-1);
    valuelen = 0;
    lib$sys_trnlog(&desc,&valuelen,&desc1,0,0,0);
    value[valuelen] = '\0';
    if (!strcmp(cap_name,value)) {
	return(0);
    }
    return(1);

}
getsymbol(symbol_name,value)
char *symbol_name,*value;
{   int valuelen,status;
    char cap_name[MAXSTRING];

    upshift(cap_name,symbol_name);
    setdesc(&desc,cap_name,strlen(cap_name));
    setdesc(&desc1,value,MAXSTRING-1);
    valuelen = 0;
    status = lib$get_symbol(&desc,&desc1,&valuelen,0);
    if (status & STS$M_SUCCESS) {
	value[valuelen] = '\0';
	return(1);
    }
    else {
	return(0);
    }
}
setdesc(descr,str,strlen)
struct dsc$descriptor_s *descr;
char *str;
int strlen;
{
    descr->dsc$w_length = strlen;
    descr->dsc$a_pointer = str;
    descr->dsc$b_class = DSC$K_CLASS_S;	/* String desc class */
    descr->dsc$b_dtype = DSC$K_DTYPE_T;	/* Ascii string type */
}
setlogical(logical_name,value)
char *logical_name,*value;
{   int valuelen;
    char cap_name[MAXSTRING];

    upshift(cap_name,logical_name);
    setdesc(&desc,cap_name,strlen(cap_name));
    setdesc(&desc1,value,strlen(value));
    lib$set_logical(&desc,&desc1,0);
}
upshift(upname,lwname)
char *upname,*lwname;
{
    while(*(upname++) = toupper(*(lwname++)));
}
