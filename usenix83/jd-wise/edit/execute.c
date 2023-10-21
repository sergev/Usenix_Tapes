#include "ed.h"
#define maxsource 5
int rfile[maxsource];
int sourcelev;

run(filename,path)
	char *filename;
	char *path;
	{
	int fd;
	char fullname[FNSIZE];

	/**
	 ** things to change:
	 **	should use buffered i/o
	 **/
	if(path){
		strcpy(fullname,path);
		strcat(fullname,"/");
		strcat(fullname,filename);
		filename=fullname;
		}
	if((fd=open(filename,0))<0)
		errfunc("Can't open %s",filename);
	if(++sourcelev>=maxsource){
		sourcelev--;
		errfunc("Too many nested executes");
		}
	rfile[sourcelev]=fd;
	}

/** this code is shared with filename and should be merged */
getname()
	{
	register c;
	register char *cp;

	c=getchr();
	if(c!=' ')
		errfunc("Bad file name format");
	while((c=getchr())==' ')
		;
	if(c=='\n')
		errfunc("No file name given");
	cp=file;
	do{
		*cp++ = c;
		} while((c=getchr())!='\n');
	*cp++ = 0;
	}
