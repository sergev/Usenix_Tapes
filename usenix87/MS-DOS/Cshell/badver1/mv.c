/* :ts=2 */
#include <stdio.h>
char *savestr();
#ifndef MAIN
extern char *fname_part(),*savestr();
extern char *me;
#else
char *me;
#endif
/* mv.c - implements a version of UNIX mv */
#ifdef MAIN
main
#else
mv
#endif
(argc,argv)
	int argc;
	register char *argv[];
{
	static char *usage = "mv : usage mv file1 [file2 . . fileN] target\r\n";
	static char target_name[128];
	char target[128],*fname_part();
	register int i;
	int len;
#ifndef MAIN
	me = argv[0];	/* referenced in routines common with cp.c */
#endif
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
		/* if the target doesn't exist and it's not a directory then rename */
		if (!filep(target))
		{
			if (!dirp(target))
			{
				fprintf(stderr,"rename : moving %s to %s\n"
				,argv[1],argv[2]);
				rename(argv[1],argv[2]);
			}
			else
			{
			/* if the target is a directory copy to same name that directory */
				strcpy(target_name,target);
				if (target_name[(len = strlen(target_name))-1] != '\\')
				{
					target_name[len = strlen(target_name)] = '\\';
					target_name[len+1] = '\0';
				}
				strcat(target_name,fname_part(argv[1]));
#ifdef DEBUG
				fprintf(stderr,"interdirectory copy same name : moving %s to %s\n"
#else
				fprintf(stderr,"moving %s to %s\n"
#endif
				,argv[1],target_name);
				if (-1 != maybecopy(target_name,argv[1]))
					unlink(argv[1]);
			}
		}
		else
		{
		/* target exists , and isn't a directory */
			char *tpath,*spath;
			char tpathsaved = 0,spathsaved = 0;
			char *path_part();
			/* find path parts of source and target */
			if (tpath = path_part(target))
			{
				tpath = savestr(tpath);
				tpathsaved++;
			}
			else
				tpath = ".";	/* current directory */
			if (spath = path_part(argv[1]))
			{
				spath = savestr(spath);
				spathsaved++;
			}
			else
				spath = ".";	/* current directory */
			if (0 == strcmp(tpath,spath))
			{
			/* if source and target are in the same directory */
#ifdef DEBUG
				fprintf(stderr,"intradirectory delete then rename : moving %s to %s\n"
#else
				fprintf(stderr,"moving %s to %s\n"
#endif
				,argv[1],target);
				unlink(target);
				rename(argv[1],target);
			}
			else
			{
#ifdef DEBUG
				fprintf(stderr,"interdirectory file to file: moving %s to %s\n"
#else
				fprintf(stderr,"moving %s to %s\n"
#endif
				,argv[1],target);
				if (-1 != maybecopy(target,argv[1]))
					unlink(argv[1]);
			}
			if (tpathsaved)
				free(tpath);
			if (spathsaved)
				free(spath);
		}
		return(0);
	}
	/* handle special case of a drive designation */
	if (target[(i = strlen(target))-1] != ':')
		if (!dirp(target))
		{
			fprintf(stderr,"mv : %s isn't a directory\n",target);
			return(-1);
		}
	for (i = 1; i < argc-1; i++)
	{
		strcpy(target_name,target);
		if (target_name[(len = strlen(target_name))-1] != '\\')
		{
			target_name[len = strlen(target_name)] = '\\';
			target_name[len+1] = '\0';
		}
		strcat(target_name,fname_part(argv[i]));
		if (!filep(argv[i]))
		{
			fprintf(stderr,"mv : %s isn't a file\n",argv[i]);
			continue;
		}
		fprintf(stderr,"moving %s to %s\n",argv[i],target_name);
		if (-1 != maybecopy(target_name,argv[i]))
			unlink(argv[i]);
	}
	return 0;
}

maybecopy(target,source)
	char *target,*source;
{
	char *drive_part();
	static char targetdrive[3], sourcedrive[3];
	strcpy(targetdrive,drive_part(target));
	strcpy(sourcedrive,drive_part(source));
	if (0 == strcmp(targetdrive,sourcedrive))
	{
		unlink(target);
		rename(source,target);
		return 0;
	}
	return (filecopy(target,source));
}
