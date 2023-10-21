#include <stdio.h>
char *me;
/* cp.c - implements a version of UNIX cp */
char target_name[128];
#ifndef MAIN
cp
#else
main
#endif
(argc,argv)
int argc;
register char *argv[];
{
	static char *usage = "cp : usage cp file1 [file2 . . fileN] target\r\n";
    char target[128],*fname_part();
    register int i;
	me = argv[0];
    if (argc < 3)
    {
	write(2,usage,strlen(usage));
	return(-1);
    }
    strcpy(target, argv[argc-1]);
    /* kill trailing backslashes */
    if (target[i = strlen(target) - 1] == '\\')
	target[i] = '\0';
    if (argc == 3)
    {
	if (target[1] == ':' && !target[2])
	    strcat(target,fname_part(argv[1]));
	/* if the target doesn't exist and it's not a directory then rename */
	if (access(target,0) && !dirp(target))
	{
	    fprintf(stderr,"copying %s to %s\n",argv[1],target);
	    filecopy(target,argv[1]);
	}
	else
	{
	    /* if the target is a directory copy to same name that directory */
	    if (dirp(target))
	    {
		int len;
		strcpy(target_name,target);
		if (target_name[(len = strlen(target_name))-1] != '\\')
		{
		    target_name[len = strlen(target_name)] = '\\';
		    target_name[len+1] = '\0';
		}
		strcat(target_name,fname_part(argv[1]));
		fprintf(stderr,"copying %s to %s\n",argv[1],target_name);
		filecopy(target_name,argv[1]);
	    }
	    else
	    {
		fprintf(stderr,"copying %s to %s\n",argv[1],target);
		filecopy(target,argv[1]);
	    }
	}
	return(0);
    }
    /* handle special case of a drive designation */
    if (target[(i = strlen(target))-1] != ':')
	if (!dirp(target))
	{
	    fprintf(stderr,"cp : %s isn't a directory\n",target);
	    return(-1);
	}
    for (i = 1; i < argc-1; i++)
    {
	int len;
	strcpy(target_name,target);
	if (target_name[(len = strlen(target_name))-1] != '\\')
	{
	    target_name[len = strlen(target_name)] = '\\';
	    target_name[len+1] = '\0';
	}
	strcat(target_name,fname_part(argv[i]));
	if (!filep(argv[i]))
	{
	    fprintf(stderr,"cp : %s isn't a file\n",argv[i]);
	    continue;
	}
	fprintf(stderr,"copying %s to %s\n",argv[i],target_name);
	filecopy(target_name,argv[i]);
    }
	return 0;
}
