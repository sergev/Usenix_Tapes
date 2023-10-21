#define NULL 0
#define MAXSTRING 132

#include <rms.h>
#include <stdio.h>
#include <stsdef.h>
#include <ssdef.h>
#include <descrip.h>

struct dsc$descriptor_s desc,desc1;

main (argc,argv)
    int argc;
    char **argv;

{   char dirlist[MAXSTRING],todir[MAXSTRING],*last_dir;

    if (!getlogical("dir_stack",dirlist)) {
	invalid_pop();
	return(1);
    }
    else {
	last_dir = strchr(dirlist,',');
	if (!last_dir) {
	    strcpy(todir,dirlist);
	    *dirlist = '\0';
	}
	else {
	    *last_dir = '\0';
	    strcpy(todir,dirlist);
	    strcpy(dirlist,last_dir+1);
	}
	chdir_and_new_stack(todir,dirlist);
    }
    
}
dellogical(logical_name)
char *logical_name;
{   char cap_name[MAXSTRING];

    upshift(cap_name,logical_name);
    setdesc(&desc,cap_name,strlen(cap_name));
    lib$delete_logical(&desc,0);
}
delsymbol(symbol_name)
char *symbol_name;
{   int tbl;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    tbl = 2;
    lib$delete_symbol(&desc,&tbl);
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

    setdesc(&desc,symbol_name,strlen(symbol_name));
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
invalid_pop()
{
    fprintf(stderr,"empty stack\n");
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
{   char cap_name[MAXSTRING];

    upshift(cap_name,logical_name);
    setdesc(&desc,cap_name,strlen(cap_name));
    setdesc(&desc1,value,strlen(value));
    lib$set_logical(&desc,&desc1,0);
}
setsymbol(symbol_name,value)
char *symbol_name,*value;
{   int tbl;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    setdesc(&desc1,value,strlen(value));
    tbl = 2;
    lib$set_symbol(&desc,&desc1,&tbl);
}
upshift(upname,lwname)
char *upname,*lwname;
{
    while (*(upname++) = toupper(*(lwname++)));
}
chdir_and_new_stack(todir,pushstack)
char *todir,*pushstack;
{
    if (*pushstack) {
	setlogical("dir_stack",pushstack);
    }
    else {
	dellogical("dir_stack");
    }

    printf("%s\n",todir);
    chdir(todir,1);
}
