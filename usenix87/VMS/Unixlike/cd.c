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

{   char curdir[MAXSTRING],testdir[MAXSTRING],todir[MAXSTRING];
    char *p1,*dirspec,*strpos();
    int i;

    if (argc <= 1) {
	getlogical("SYS$LOGIN",todir);
	if (chdir(todir,1)) {
	    invalid_dirspec(todir);
	    return(1);
	}
    }
    else {
	p1 = argv[1];
	if (strchr(p1,'[')
	 || strpos(p1,"..")
	 || strchr(p1,'/')) {
	    if (chdir(p1,1)) {
		invalid_dirspec(p1);
		return(1);
	    }
	}
	else if (getlogical(p1,todir)) {
	    if (chdir(p1,1)) {
		invalid_dirspec(p1);
		return(1);
	    }
	}
	else {
	    dirspec = strrchr(p1,':');
	    if (!dirspec) {
		*todir = '\0';
		dirspec = p1;
	    }
	    else {
		i = dirspec-p1+1;
		strncpy(todir,p1,i);
		todir[i]='\0';
		dirspec++;
	    }
	    strcat(todir,"[");
	    if (*dirspec != '.'
	     && *dirspec != '-'
	     && dirspec == p1) strcat(todir,".");
	    strcat(todir,dirspec);
	    strcat(todir,"]");
	    if (chdir(todir,1)) {
		invalid_dirspec(todir);
		return(1);
	    }
	}
    }
}
getlogical(logical_name,value)
char *logical_name,*value;
{   int valuelen;
    char cap_name[MAXSTRING],*cap_indx,*logical_indx;

    logical_indx = logical_name;
    cap_indx = cap_name;
    while (*cap_indx++ = toupper(*logical_indx++));
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
getwd(pwd)
char *pwd;
{   int pwdlen;

    setdesc(&desc,pwd,MAXSTRING-1);
    pwdlen = 0;
    sys$setddir(0,&pwdlen,&desc);
    pwd[pwdlen] = '\0';
}
invalid_dirspec(dirspec)
char *dirspec;
{
    fprintf(stderr,"invalid directory %s\n",dirspec);
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
setsymbol(symbol_name,value)
char *symbol_name,*value;
{   int valuelen;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    setdesc(&desc1,value,strlen(value));
    lib$set_symbol(&desc,&desc1,0);
}

char *strpos(str,searchstr)
char *str,*searchstr;
{   char *matchc,c,*matchstr;

    matchc = searchstr;
    c = *matchc++;
    while (c) {
	if (!*str) {
	    return(NULL);
	}
	else {
	    matchstr = str++;
	    while (c == *matchstr && *matchstr++) {
		c = *matchc++;
	    }
	    if (!c) {
	 	return(str - 1);
	    }
	    matchc = searchstr;
	    c = *matchc++;
	}
    }
    return(NULL);
}
