#

/****
 ****	Wednesday, 13 November 1985, 1:29 p.m.
 ****	give: change owner and group for one or more files as indicated:
 ****		usage:
 ****			give file... to user [group]
 ****		- may be used succesfully only by current owner or root
 ****		- if file is a directory, descend and change the entire tree
 ****		  beginning with (including) the named directory
 ****		compile:
 ****			cc -O -o give give.c
 ****/

main(argc,argv)
int	argc;
char	*argv[];
{
	int	i, to, uid = -1, gid = -1;
/*
 *	find end of filename list in argv[] and verify command-line syntax:
 *	must say at least: "give SOMETHING to SOMEBODY"
 */
	for ( i=0; i<argc; i++ )
		if ( !strcmp(argv[i],"to") )
		{
			to = i;
			break;
		}
	if ( (argc<4) || (to<2) || (to==(argc-1)) || (argc<4) )
		exit(printf("usage: %s File... to User [Group]\n",argv[0]));

/*
 *	get target integer user id
 */
	if ( (uid=getuser(argv[to+1])) < 0 )
		exit(printf("unknown user: `%s'...\n",argv[to+1]));

/*
 *	get target integer group id (if applicable)
 */
	if ( (to<(argc-2)) && ((gid=getgroup(argv[to+2]))<0) )
		exit(printf("unknown group: `%s'...\n",argv[to+2]));

	give(&argv[1],(to-1),uid,gid,0);
}

#include <stdio.h>

/*
 *	getuser: get integer user id: returns -1 if unknown
 */

#define	is_digit(C)	( ((C)>='0') && ((C)<='9') )
char	line[256], *c = (char *)0, *strchr();

getuser(name)
char	*name;
{
	FILE	*file;
	int	uid = -1, i, len, strlen(), intname = 1;

	len = strlen(name);

	for ( i=0; i<len; i++ )
		if ( !is_digit(name[i]) )
			intname = 0;

	if ( !(file=fopen("/etc/passwd","r")) )
		exit(printf("cannot read user name list...\n"));

	while ( fgets(line,128,file) )
		if ( intname || !strncmp(line,name,len) )
		{
			c = line;
			for ( i=0; i<2; i++ )
				if ( !(c=strchr((c+1),':')) )
					if ( intname )
						break;
					else
						goto out;
			if ( !intname || !strncmp((c+1),name,len) )
			{
				sscanf((c+1),"%d",&uid);
				break;
			}
		}
out:
	fclose(file);
	return(uid);
}

/*
 *	getgroup: get integer group id: returns -1 on error
 */

getgroup(name)
char	*name;
{
	int	i, gid = -1, intname = 1, len;
	FILE	*file;

	len = strlen(name);

	for ( i=0; i<len; i++ )
		if ( !is_digit(name[i]) )
			intname = 0;

	if ( !(file=fopen("/etc/group","r")) )
		return(-1);

	while ( fgets(line,128,file) )
		if ( intname || !strncmp(line,name,len) )
		{
			if ( !(c=strchr(line,':')) )
				goto out;
			if ( !(c=strchr((c+1),':')) )
				goto out;
			if ( intname && strncmp((c+1),name,len) )
				continue;
			sscanf((c+1),"%d",&gid);
		}
out:
	fclose(file);
	return(gid);
}

/*
 *	give: recursively descend a directory structure changing
 *	      owner and group ids.
 */

#include <sys/types.h>
#include <sys/stat.h>

#define	indent()	for ( j=0; j<pass; j++ ) printf("    ")
#define	is_dir()	((st.st_mode&S_IFDIR)==S_IFDIR)

give(names,n,uid,gid,pass)
char	**names;
int	n, uid, gid, pass;
{
	int		i, j, grp, nn = 0, stat();
	struct stat	st;
	char		**next;

	for ( i=0; i<n; i++ )
	{
		if ( stat(names[i],&st) < 0 )
		{
			indent();
			printf("`%s': cannot stat...\n",names[i]);
			continue;
		}

		if ( gid < 0 )
			grp = st.st_gid;
		else
			grp = gid;

		if ( is_dir() )
		{
			indent();
			printf("%s:\n",names[i]);
			if ( chdir(names[i]) < 0 )
			{
				indent();
				printf("cannot cd to `%s'...\n",
					names[i]);
				continue;
			}
			if ( !(nn=getflist(&next)) )
			{
				indent();
				printf("cannot ls `%s'...\n",names[i]);
			}
			else
				give(next,nn,uid,gid,(pass+1));

			if ( chdir("..") < 0 )
			{
				indent();
				printf("cannot cd to `..'...\n");
				exit(3);
			}
		}

		if ( chown(names[i],uid,grp) < 0 )
		{
			indent();
			printf("`%s': cannot change...\n",names[i]);
		}
	}
}

/*
 *	getflist: get names of files in current working directory
 */

#include <sys/dir.h>

char	ln[4096], *strcat();

getflist(names)
char	***names;
{
	int	n = 0;
	struct direct	dir;
	FILE	*file;

	if ( !(file=fopen(".","r")) )
		return(0);

	ln[0] = 0;

	while ( fread(&dir,sizeof(struct direct),1,file) )
		if ( !strcmp(dir.d_name,".") || !strcmp(dir.d_name,"..") )
			continue;
		else if ( dir.d_ino )
		{
			strcat(ln,dir.d_name);
			strcat(ln," ");
		}

	fclose(file);

	n = get_words(ln,names);

	return(n);
}

/*
 *	get_words: split a character string into individual words
 */

#define	is_white(C)	( ((C)==' ') || ((C)=='\t') )

int	get_words(line,wordlist)
char	*line;
char	***wordlist;
{
	int	i, j, k, nwords = 0, len = 0, longest = 0;
	char	*c, **words, *malloc(), was_white = 0, got_a_word = 0;

/*
 *	count words, find longest word
 */
	for ( c=line; *c; c++ )
		if ( is_white(*c) )
		{
			if ( got_a_word && !was_white )
			{
				if ( len > longest )
					longest = len;
				len = 0;
				nwords++;
			}
			was_white = 1;
		}
		else
		{
			was_white = 0;
			got_a_word = 1;
			len++;
		}

	if ( got_a_word && !was_white )
	{
		if ( len > longest )
			longest = len;
		len = 0;
		nwords++;
	}

	if ( !nwords )
		return(0);

/*
 *	allocate the array of individual words
 */
	if ( !(words=(char **)malloc(nwords*sizeof(char *))) )
	{
		printf("cannot create the word list...\n");
		return(0);
	}

	longest++;	/* plus '\0' */

	for ( i=0; i<nwords; i++ )
		if ( (words[i]=malloc(longest*sizeof(char))) == NULL )
		{
			printf("cannot allocate an individual word...\n");
			for ( --i; i>=0; i-- )
				free(words[i]);
			free((char *)words);
			return(0);
		}

/*
 *	load the array of individual words
 */
	got_a_word = was_white = j = k = 0;

	for ( i=0; line[i]!='\0'; i++ )
		if ( !is_white(line[i]) )
		{
			got_a_word = 1;
			words[j][k] = line[i];
			was_white = 0;
			k++;
		}
		else if ( got_a_word && !was_white )
		{
			words[j][k] = '\0';
			was_white = 1;
			k = 0;
			j++;
		}

	if ( got_a_word && !was_white )
		words[j][k] = '\0';
	*wordlist = words;
	return(nwords);
}

