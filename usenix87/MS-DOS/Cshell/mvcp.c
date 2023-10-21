#include <stdio.h>
/*
 * mvcp.c - routines common to mv and cp
 */
#include <fcntl.h>
char buffer[BUFSIZ*16];
extern char *me;
filecopy(target,source)
char *target,*source;
{
    int t,s,r;
    if (-1 == (s = open(source,O_RDONLY)))
    {
		fprintf(stderr,"%s : can't open %s\n",me,source);
		return(-1);
    }
    if (-1 == (t = open(target,O_TRUNC)))
    {
		fprintf(stderr,"%s : can't open %s\n",me,target);
		return(-1);
    }
    while(0 != (r = read(s,buffer,BUFSIZ*16)) && r != -1)
    {
		if(-1 == write(t,buffer,r))
		{
		    fprintf(stderr,"%s : error writing %s\n",me,target);
		    return(-1);
		}
    }
    close(t); 
    close(s);
	return (0);
}

#include <errno.h>
typedef struct
{
    char dos_reserved[21];
    char attribute;
    unsigned file_time;
    unsigned file_date;
    long file_size;
    char file_name[13];
} 
fcb;
fcb dir;

dirp(s)
char *s;
{
	register int junk1,junk2;
	/* handle all of the stupid special cases */
	if ((s[1] == ':' && s[2] == '\0')	/* root directory on a drive */
		|| (s[1] == '\0')				/* root directory default drive */
	)
	{
		return 1;
	}
	if (0 == strcmp(s,".."))			/* parent of this directory */
	{
		int returnval;
		char *current,*parent;
		current = getcwd(NULL,64);
		if (-1 == chdir(s)) /* go to parent */
			returnval = 0;
		else
			returnval = 1;
		parent = getcwd(NULL,64);
		chdir(current);
		free(current); free(parent);
		return returnval;
	}
    /* set the disk transfer address */
    bdos(0x1A,&dir);
    /* do a search first for the directory path */
    return (bdos(0x4E,s,0x10) == 0 && bdos(0x4E,s,0) != 0);
}

filep(s)
char *s;
{
    /* set the disk transfer address */
    bdos(0x1A,&dir);
    /* do a search first for the directory path */
    return bdos(0x4E,s,0) == 0;
}
char *fname_part(s)
register char *s;
{
    register char *r; 
    char *rindex();
    if (r = rindex(s,'\\'))
	{
		return r+1;
	}
    if (r = rindex(s,':'))
	{
		return r+1;
	}
    return s;
}
char *path_part(s)
register char *s;
{
	static char buffer[64];
    register char *r; 
    char *rindex();
	strcpy(buffer,s);	/* copy string */
	if (r = rindex(buffer,'\\'))
	{
		*++r = '\0';
		return buffer;
	}
	if (r = rindex(buffer,':'))
	{
		*++r = '\0';
		return buffer;
	}
	return NULL;
}
char *drive_part(s)
register char *s;
{
	static char buffer[64];
    register char *r; 
    char *rindex();
	strcpy(buffer,s);	/* copy string */
	if (buffer[1] == ':')
	{
		buffer[2] = '\0';
		return buffer;
	}
	return NULL;
}
