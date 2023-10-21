#include "help.h"
#define MAXMATCHES	24

/*
 * modified slightly, R. Perry 9/21/86
 */

 /* ************************************************************
  * 			H   E   L   P    !!
  *		     =========================
  *
  * 	This program emulates the VMS help facility in the UNIX
  *  environment.  The main routine HELP1 looks up help and 
  *  subtopics for help.  The help texts for various topix 
  *  are tree-structured.  A directory is defined as a "help"
  *  directory, it has four types of files in it:
  *		main help text: 	.HLP
  *		manual page name:	.MANUAL
  *		subtopic texts:		<topicname>.HLP
  *		subtopic directories:	<topicname>
  *
  *	Subtopic names must start with an alphanumeric
  *  character.  Preferably all subtopics will start with
  *  lowercase letters.
  *
  *	The routine help1 is recursive, it descends the	tree
  *  structure.
  */

  main(argc, argv)
  	int argc;
	char *argv[];
  {
	int i,j,k;
	int longlen;
	char yesno[10];
	char *opts;
		
	strcpy(progname,argv[0]);
	helpdir = NULL;
	getwd(olddir);
	dumb_flag = 0;
	list_flag = 0;
	frst_flag = 1;
	col_flag  = 1;

	argc--;
	if (*(*++argv) == '-') {		/* must be options */
		for (opts = *argv; *opts != '\0'; opts++) {
			switch(*opts) {
			   case '-' :	
			   	break;

			   case 'd' :	
			   	if (argc > 1) {
					helpdir = *++argv;
					argc--;
				}
				break;

			   case 'l' :
			   	list_flag = 1;
				col_flag = 0;
				break;

			   case 'q' :
			   	dumb_flag = 1;
				break;

			   case 'C' :
			   	col_flag = 1;
				break;

			   case 'n' :
			   	col_flag = 0;

			   default:
			        fprintf(stderr,"%s: %c: bad option.\n",
					progname,*opts);
				break;
			}
		}
		argc--; argv++;
	}

	if (helpdir == NULL) helpdir = HELPDIR;

	if (chdir(helpdir) < 0) {
		fprintf(stderr,"%s: %s: help directory not found.\n",
			progname,helpdir);
		exit(1);
	}

	if (argc >= 1) {		/* treat vector as a help path */
#ifdef DEBUG
	    fprintf(stderr,"help path vector, argc=%d, *argv=%s.\n",
		    argc,*argv);
#endif
	    help1( "", argv, 0);

	}
	else	help1( "", NULL, 0);

	exit(0);
 }




 /* **************************************************************
  * printhelp: given a string, pop .HLP on the end on do 
  *	a more on it.
  *
  *	This routine sends a help file to more(1).  A string
  *  is passed in, which is the name of a help.  If the string
  *  is nil, then just use the name HLP.
  *	The return value is -1 if no help file is accessible, 
  *  0 if the more(1) command was called okay with fkoff();
  */

  printhelp(hs, path)
  	char *hs;
  {
	char filename[MAXNAMELEN], comm[MAXNAMELEN + 20];

	if (hs == NULL) strcpy(filename,HELPEX);
	else if (strlen(hs) < 1) strcpy(filename, HELPEX);
	else {
		strcpy(filename, hs);
		strcat(filename,HELPEX);
	}

	if ( access(filename, R_OK) < 0 ) {
		printf("\nSorry, no help text for %s.\n",
			(hs==NULL)?"this topic":hs );
		return(-1);
	}

	if (path != NULL) printf("\n%s\n\n",path);
/*	if (path != NULL) printf("\n HELP: %s\n",path); */

	fkoff(VIEWPROGRAM,VIEWPROGOPTS1,VIEWPROGOPTS2,filename,NULL);

	putchar('\n');
	return(0);
  }



/* *************************************************************
 * printtopix: print the topics available in this directory
 *		in a nice format.
 *
 *	This routine does a directory of help options, 
 *  and prints them out in a manner similar to that of ls(1).
 *  All filenames which start with anything other than numbers 
 *  or letters are not kept.  Extensions are stripped off.
 *	
 *	The number of subtopics found is returned, along with
 *  a pointer to a null-terminated vector of string pointers.
 *  If the mode is non-zero, it means just use the strings stored
 *  in topix.  If mode==0, then re-allocate space and re-check
 *  the directory.  If mode
 */

 printtopix( topix, mode, supress)
 	char *topix[];
	int mode, supress;
 {
	int i,j,k,l;
	int longlen;
	int  namewidth, totalnames, rowcnt, colcnt;
	char *malloc(), *nbuf;
	char row[TERMWID+1];
	char *thisname, *index(), *rindex();
	char *s1, *s2, **nxtname, *nxcnt;
	struct direct *readdir(), *filedat;
	DIR *dirp, *opendir();

	if ( mode ) {
		for(nxtname=topix, i=0; *nxtname++ != NULL; i++ );
		totalnames = i;
		goto inputtopix;
	}

	/* if mode is zero, then allocate space and search the directory */
	nbuf = malloc( MAXNAMES * MAXNAMELEN );
	nxcnt=nbuf;
	dirp = opendir(".");
	if (dirp == NULL) {
		fprintf(stderr,"%s: Cannot open help directory.\n",progname);
		return(-1);
	}

	for(nxtname=topix, i=0; i < (MAXNAMES-1) ; ) {
		filedat = readdir(dirp);
		if (filedat == NULL) break;
		thisname = filedat->d_name;

/*		if ( !(isalnum(*thisname)) )    /* if not in [0-9A-Za-z] */
/*				continue;	/* do the next one.      */
/*		if ( (s1 = index(thisname,'.')) != NULL) {		 */

		if ( (s1 = rindex(thisname,'.')) != NULL) {
			if (strcmp(s1, HELPEX) == 0) *s1 = '\0';
			else continue;
/*			else if ( strcmp(s1, MANEX) == 0) continue;	 */
		}
		if (strlen(thisname) >= MAXNAMELEN)
			*(thisname+MAXNAMELEN-1) = '\0';
		/* copy in data from this loop */
		strcpy(nxcnt,thisname);
		*nxtname++ = nxcnt; 
		/* update pointers for next loop */
		nxcnt += strlen(thisname) + 1;
		i++;
	}
	*nxtname++ = NULL;
	totalnames = i;
	closedir(dirp);

	if (totalnames == 0) return(0);
	
	/* sort the names in ascending order with exchange algorithm */
	for(i=0; i < totalnames-1; i++)
		for(j=i+1; j <totalnames; j++)
			if (strcmp(topix[i],topix[j]) > 0) {
				thisname = topix[i];
				topix[i] = topix[j];
				topix[j] = thisname;
			}

 inputtopix:
	if (supress) return(totalnames);

	longlen = 0;
	for(i=0; i < totalnames; i++ )
		longlen = ((k=strlen(topix[i]))>longlen)?k:longlen;

	/* here print the names out in nice columns */
	namewidth = longlen + COLUMNSPACE;
	rowcnt = TERMWID / namewidth;
	colcnt = (totalnames + (rowcnt-1)) / rowcnt ;
	if (colcnt <= 0) colcnt = 1;

	if (col_flag && rowcnt > 1) {
		printf("\n Subtopics:\n\n");
		for(i=0; i < colcnt ; i++ ) {
			for(k=0; k < TERMWID; row[k++] = ' ');
			row[k] = '\0';
			for(j=0, s1 = row; 
		    	    (i+j) < totalnames; 
		    	    j += colcnt) {
				row[strlen(row)] = ' ';
				strcpy(s1,topix[i+j]);
				s1 = s1 + namewidth;
			}
			printf("    %s\n",row);
		}
		putchar('\n');
	}
	else {
		for(i=0; i < totalnames; i++)
			printf("%s\n",topix[i]);
 	}

	return(totalnames);
 }



 /* ****************************************************************
  * help1: descend recursive help tree and provide some
  *	     user services.
  *
  *	This routine is the heart of the new UNIX help facility.
  *  It climbs recursively around an n-tree of documentation files
  *  and directories.  The routine printhelp() outputs a file of
  *  help text, the routine printtopix prints out all subtopics.
  *
  *	This routine can operate in interactive mode, or not.  The
  *  basic cycle of operation is:
  *		if (not list-only-mode) print help.
  *		if (not quiet-mode)	print list.
  *		if (not interactive)	return.
  *		print prompt and do commands.
  *		return.
  *
  *	There are a number of commands available in the
  *  interactive mode, they are:
  *
  *		blank line:	up recursive level.
  *		subtopic name:	recurse to subtopic.
  *		?:		list topics again.
  *		*:		get list of commands
  *		$:		get info on topix
  *		^D:		quit help program.
  *		.:		man page (if any).
  *		#		list topics again
  *		
  */

  help1(ppt,svec,skip)
  	char *ppt;				/* prompt */
	char *svec[];
	int  skip;

  {

	int i,j,k, err;
	int no_subs;
	char answer[MAXLINELEN];
	char fullpath[MAXNAMELEN + 11];
	char yesno[10];
	char *topix[MAXNAMES], *mx[MAXMATCHES];
	int  matchv();
	char *index(), *getenv();
	char *s1, *s2, *s3, *rest;
	char *wvec[MAXMATCHES];
	char cmdbuf[90];

	if ( svec != NULL  && *svec != NULL) {
	    printtopix( topix, 0, 1);
	    if ( (k = matchv(*svec, topix, mx)) == 0) {
		printf("Sorry, no help path to %s %s\n\n",ppt,*svec);
		return(-1);
	    }
	    j=0;
	    for(i=0, err = -1; mx[i] != NULL; ) {
		if (i > 0)
		  j = takeit("Next help path: %s %s\nTry it?",ppt,mx[i]);
		if (j == 1) { i++; continue; }
		if (j < 0)  break;
		strcpy(fullpath,ppt);
		strcat(fullpath," ");
		strcat(fullpath,mx[i]);
		if ( chdir(mx[i]) < 0) {
		    if ( *(svec + 1) != NULL ) {
			i++;
			continue;
		    }
		    if (list_flag) 
			return(-1);
		    else
			printhelp( mx[i],fullpath);
			putchar('\n');
		    if (!dumb_flag) help1(ppt,NULL,1);
		    err = 0;
		}
		else {
		    if ( help1(fullpath,(svec+1),0) >= 0) {
			err = 0;
			chdir("..");
			putchar('\n');
			if (!dumb_flag) help1(ppt,NULL,1);
		    }
		    else chdir("..");
		}
		i++;
	    }
	    return(err);
	}

	if ( !list_flag && !skip) 
	    if (printhelp(NULL, ppt) < 0) {
		return(-1);
	    }

	if ( !list_flag && !dumb_flag && ( access(MANEX, R_OK) >= 0) && !skip)
		printf("\n Manual page is available.\n");

	if ( !dumb_flag ) {
	    if ( printtopix( topix, 0, skip) <= 0 ) {
		no_subs = 1;
	    }
	    else no_subs= 0;
	}

	if ( dumb_flag  ||  list_flag ) return(0);

	while ( 1 ) {
		if (frst_flag) {
			printf("(type '*' for commands)\n");
			frst_flag = 0;
		}
		if ( strlen(ppt) < 1 ) printf("%s",PROMPT);
		else		 printf("%s %s",ppt+1,SUBPROMPT);
		if ( fgets(answer, MAXLINELEN-1, stdin) == NULL ) {
			putchar('\n');
			exit(0);
/*			printf("\nbye...\n"); */
/*			exit(1);	      */
		}

		/* first remove any leading blanks or tabs */
		for(s1=answer; *s1 == ' ' || *s1 == '	'; s1++);

		/* chop off all of answer after first word. */
		s2 = index(s1, ' ');
		if (s2 == 0) s2 = index(s1, '\n');
		if (s2) { *s2 = '\0'; rest = s2+1; }
		else rest = s1 + (strlen(s1) - 1);
		makewvec(rest,wvec);

		if ( strlen(s1) == 0 )		/*  on blank line, */
			break;			/* pop up one level*/

		switch (*s1) {
	    	    case '?':			/* ?: print stuff again */
		    	printhelp(NULL, ppt);
			if ( access(MANEX, R_OK) >= 0 )
			    printf("\n Manual page is available.\n");
	    		if (!no_subs) printtopix(topix, 1,0);
			else	printf("\nSorry, no subtopics.\n\n");
			break;

		    case '#':
			if (!no_subs) printtopix( topix, 1,0);
			else    printf("\nSorry, no subtopics.\n\n");
			break;

		    case '*':			/* *: list commands */
			putchar('\n');
		        for(i=0; helpcmds[i] != NULL; i++) {
				printf("	%s\n",helpcmds[i]);
			}
			putchar('\n');
			break;

		    case '$':			/* $: find out about files */
		    	s2 = s1 + 1;
			if (no_subs) {
				printf("\nSorry, no subtopics.\n\n");
		    		break;
			}
			strcpy(cmdbuf, INFOHEAD);
			strcat(cmdbuf,s2);
			strcat(cmdbuf,"* ");
			strcat(cmdbuf, INFOTAIL);
			printf("\nFile information: \n");
			system(cmdbuf);
			putchar('\n');
			break;
		        

	    	    case '.':			/* .: do manpage if any */
		    	s2 = s1 + 1;
			if (no_subs) {
				printf("\nSorry, no subtopics.\n\n");
		    		break;
			}
			if ( *s2 == '\0' ) {
				mx[0] = "";  mx[1] = NULL; k = 1;
			}
			else if ( (k = matchv( s2, topix, mx)) == 0 ) {
			   printf("\nSorry, no topics match %s\n",s2);
			   printf("(list commands with '*', topics with '#')\n");
			}
			for( i=0; i < k; i++ ) {
				strcpy(cmdbuf,mx[i]);
				strcat(cmdbuf,MANEX);
				if (access(cmdbuf, R_OK|X_OK ) < 0) {
				        strcpy(cmdbuf,mx[i]);
					strcat(cmdbuf,MAN_SUBEX);
					if (access(cmdbuf,R_OK|X_OK) < 0) {
					    printf("\nSorry, %s for %s.\n\n",
					      "No manual reference available",
					      (strlen(s2)==0)?ppt:mx[i]);
					    continue;
					}
				}
				if (i > 0) {
 				     k=
				     takeit("Next man page: %s.\nTake it? ",
					mx[i]);
				     if (k ==  1) continue;
				     if (k == -1) break;
				}
				fkoff(SHELLPROG,SHELLOPTS,cmdbuf,NULL);
					/* source contents of .MANUAL file */
			}
			break;


		    default:			/* must be a topic spec */
		        if ( no_subs )  {
				printf("\nSorry, no subtopics.\n\n");
		    		break;
			}
			if ( (k = matchv( s1, topix, mx)) == 0 ) {
			   printf("\nSorry, no help for %s.\n",s1);
			   printf("(list commands with '*', topics with '#')\n");
			   break;	/* leave switch */
			}

#ifdef DEBUG
			fprintf(stderr,"diag: wvec= '%s' '%s' ...\n",
				wvec[0],(wvec[0]!=NULL)?(wvec[1]):NULL);
#endif

		        for(i=0; i < k; i++) {
			    s3 = mx[i];

			    if (i > 0) {
			        j=takeit("\nNext %s subtopic: %s\nTake it? ",
					 ppt,s3);
				if (j ==  1) continue;
				if (j == -1) break;
			    }

			    if ( chdir(s3) >= 0 ) {  /* directory  subtopic */
			    	strcpy(fullpath,ppt);
			    	strcat(fullpath," ");
			    	strcat(fullpath,s3);
			    	help1(fullpath,wvec,0);		/* recurse */
				if ( strcmp(getwd(newdir),helpdir) )
					chdir("..");
			    }			/* else text subtopic */
			    else  {
				strcpy(fullpath, ppt);
				strcat(fullpath, " ");
				strcat(fullpath, s3);
				printhelp(s3, fullpath);
			    }
			}

		}	/* end of switch */

	}	/* end of while(1) */

	if (!no_subs && (*topix != NULL) ) free( *topix );

	return(0);

  }	/* end of help1 */


/* ****************************************************************
 * takeit: ask user whether to take next topic, return t or f
 *
 *	This routine takes a message, with format, and asks the 
 *  user whether he wants to do the action, not do the action,
 *  or quit the cycle of actions.
 *	y - return  0
 *	n - return  1
 *	q - return -1
 *	? - tell what y, n, and q do.
 *      * - tell what y, n, and q do.
 *     <CR> - carriage return
 *
 *	Naturally, case is insignificant.
 */

 takeit(fmt,m1,m2,m3)
   char *fmt, *m1, *m2, *m3;
 {

	char ans[40];
	int  done, ret;
	char *fgets();

	for(done = 0; !done; ) {
		if (fmt != NULL) printf(fmt, m1, m2, m3);
		printf(" [ynq] ");
		if (fgets(ans, 39, stdin) == NULL) exit(1);  /* abort */
		isupper(*ans)?(*ans=tolower(*ans)):0;
		if (*ans == 'n') {
			done = 1;
			ret = 1;
		}
		else if (*ans == 'y' || *ans == '\n') {
			done = 1;
			ret = 0;
		}
		else if (*ans == 'q') {
			done = 1;
			ret = -1;
		}
		else printf("Answer y to get next subtopic, n to skip, q to quit.\n");
	}
	return(ret);
 }


 /* ***************************************************************
  * matchv: return all matches of a string in a vector.
  *
  *	This routine accepts a string and a vector of strings. 
  *  The string is supposed to be an abbreviation of one or more
  *  strings in the vector.  The full versions of the strings are
  *  placed in a another vector (which is in a static area) and
  *  a pointer to that vector returned.  Note that the input
  *  vector of pointers must have NULL as its terminating element.
  *  The output vector will also terminate with NULL.
  *	NOTE: The vector returned contains pointers into the input
  *  vector.  Do not, therefore, mess with the contents of the output
  *  vector.
  *
  *  If NULL is returned, nothing matched, or there was some other
  *  error.
  */

  matchv( src, vec, mx)
    char *src;
    char *vec[];
    char *mx[];
  {
	char *m[MAXMATCHES];
	char  *s1, *s2;
	int i,j, slen;

	if ( (slen = strlen(src)) == 0 ) return(NULL);  

	for(i=0, j=0; vec[i] != NULL && j < MAXMATCHES; i++)
		if ( strncmp(src,vec[i],slen) == 0   && 
		     strlen(vec[i]) >= slen )
		   		m[j++] = vec[i];
	m[j] = NULL;
	for(i=0; i <= j; mx[i] = m[i], i++);
	return(j);
  }

 /* *****************************************************************
  * fkoff: fork a process and return the exit status of the process.
  *
  *    This routine takes a command line separated into words, and
  *  uses vfork(2) and execve(2) to quickly run the program.
  *
  *  This is a simplified version of the fkoff() routine from dcon(8).
  *  Here, a program name and up to four arguments may be passed in.
  *  If one of them is null, FINE, but the fifth 
  *
  */

  fkoff(prg,arg1,arg2,arg3,arg4)
    char *prg, *arg1, *arg2, *arg3, *arg4;

  {
	char *command, *malloc(), *argvec[6];
	int pid, stat, i;

	for(i=0; i < 6; argvec[i++] = NULL);
	argvec[0] = prg;
	argvec[1] = arg1;
	argvec[2] = arg2;
	argvec[3] = arg3;
	argvec[4] = arg4;
	if (argvec[0] == NULL) return(-1);
	command = malloc( strlen(argvec[0]) + 1);
	strcpy(command,argvec[0]);
	
	pid = vfork();    /* fork 2 copies of us */
	if (pid == 0)  { /* we are child, execve the program. */
   		pid = execve(command,argvec,environ);
		_exit(1);   
	}
	else {          /* we are parent, wait for child */
		pid = wait(&stat);
		if (pid == -1) return(pid);
		stat = stat / 0400;   /* get hi byte of status */
		return(stat);
        }
  }

/* **************************************************************
 * makewvec: make a vector of words, up to 7 of them
 *
 *    This routine uses index to simply parse a string
 *  made up of (possibly) several blank-separated words.
 *  The words are stored into the slots of a vector, as pointers
 *  into the original string.  Therefore, the original string
 *  is destroyed.
 */
 makewvec(sstr, rvec)
     char *sstr;
     char *rvec[];
{
    int mcnt = 0;
    int done = 0;
    char *s1, *s2, *index();

    if (strlen(sstr) == 0) {
	rvec[0] = NULL;
    }
    else {
	/* skip leading whitespace */
	for(s1=sstr; iswhite(*s1); s1++);
	for(mcnt = 0, done = 0; !done; mcnt++) {
	    s2 = index(s1,' ');
	    if (s2 == NULL) s2 = index(s1,'	');
	    if (s2 == NULL) s2 = index(s1,'\n');
	    if (s2 != NULL) *s2 = '\0';
	    rvec[mcnt] = s1;
	    if (s2 == NULL) done = 1;
	    else {
		/* skip more white space */
		for(s1 = s2+1; iswhite(*s1); s1++);
		if (*s1 == '\0') done = 1;
	    }
	}
	rvec[mcnt] = NULL;
    }
    return(mcnt);
}
