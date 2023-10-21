
/*
 * SYNOPSIS:
 *	sus [username] [command string]
 *
 * EXPLANATION
 *	Another kind of su(1) program, in most cases it acts like the
 *	ordinary su program. But it has many more features build-in
 *	it can take a command string and execute it (using the standard
 *	Bourne shell) as the specified user or root. Futhermore it
 *	allows the System Administrator to define a group of people
 *	to use the program without having to give a password to change
 *	userid. This can be regarded as a weakining of the security on
 *	the UNIX system, but it hasn't been causing any problems here yet!
 *
 *	The list of users allowed to use the program without using password
 *	are kept in the file "/etc/sus.allow" and contains the login names
 *	of the choosen few one per line.
 *
 *	The superuser is always able to use all the facilities of the 
 *	program and cannot be denied by omitting the name in the namefile.
 *
 * HISTORY
 *	We have been using this utility for quite some time in our company
 *	to allow certain members of the UNIX staff to become root without
 *	knowing the root password. Furthermore it has helped us as system
 *	administrators not to run as superusers when not nescessary and
 *	thereby decreasing the chances of making fatal mistakes.
 *
 *	The name sus was just made out of convinience since it was near to
 *	su(1) and easy to type - But someone pointed out that it could be
 *	the Mnemonic of "Super-User Sans Password" I kind of like that!
 *
 * BUGS
 *	Beware when omitting the username and the first argument of the
 *	command string matches a user on the system. Sus will then take
 *	this as the intended user and the rest as the command. To avoid
 *	this situation just put in the username of the superuser.
 *
 * AUTHOR
 *	Kim Chr. Madsen, AmbraSoft A/S
 *	kimcm@olamb.UUCP or ..seismo!mcvax!diku!olamb!kimcm
 *
 * LAST MODIFIED
 *	Wed Jan 28 13:30:32 GMT-1 1987
 *
 * NOTICE
 *	Permissions to copy use and redistribute this program is hereby
 *	granted as long as the Copyright notice and this comment is kept
 *	intact.
 *
 *		    (c) Copyright 1987 Kim Chr. Madsen
 */

#include <stdio.h>
#include <pwd.h>

#define	SUS_ALLOW	"/etc/sus.allow"  /* Users who doesn't need passwords */
#define	SHELL		"/bin/sh"	  /* Default shell 		      */
#define	SUPERUSER	"root"		  /* Might be "zeus" on zilog systems */
#define	CMDSIZ		1024		  /* Max length of command string     */
#define	FALSE		0
#define	TRUE		1

typedef int	bool;

main(argc,argv,envp) 
int	argc;
char	*argv[];
char	*envp[];
{
	bool			want_sh = TRUE;
	char			*pword;
	char			*shell;
	char			cmd[CMDSIZ];
	char			prompt[25];
	char			salt[2];
	char			tmpbuf[CMDSIZ];
	char			username[CMDSIZ];
	extern char		*getenv();
	extern char		*getpass();
	extern struct passwd 	*getpwnam();
	int			groupid = 0;
	int			i;
	int			userid = 0;
	struct passwd 		*entry;

	strcpy(cmd,(char *) NULL);		    /* empty cmd buffer	   */
	strcpy(tmpbuf,(char *) NULL);		    /* Empty tmpbuf buffer */
	if (argc == 1) strcpy(username,SUPERUSER);  /* default action      */
	if (argc >= 2) strcpy(username,argv[1]);
	if (argc > 2) {
		want_sh = FALSE;
		for (i=2; i<argc; i++) 
			if ((strlen(tmpbuf) + strlen(argv[i]) + 1) < CMDSIZ) {
				strcat(tmpbuf," ");
				strcat(tmpbuf,argv[i]);
			} else {
				fprintf(stderr,"Argument(s) too long\n");
				exit(1);
			}
	}

	if ((entry = getpwnam(username)) == (struct passwd*) NULL) {
		/* Must be a command to be executed with superuser priviledges */
		strcpy(cmd,username);
		strcpy(username,SUPERUSER);
		if ((entry = getpwnam(username)) == (struct passwd*) NULL) {
			/* You had your chance ! */
			fprintf(stderr,"What no \"%s\" on the system!!!\n",SUPERUSER);
			fprintf(stderr,"Your system can be in a terrible state\n");
			exit(1);
		}
		want_sh = FALSE;
	}

	/*
	 * Build the command list into one string!
	 */

	if ((strlen(tmpbuf) + strlen(cmd) + 1) < CMDSIZ) {
		strcat(cmd," ");
		strcat(cmd,tmpbuf);
	} else {
		fprintf(stderr,"Argument(s) too large\n");
		exit(1);
	}

	/*
	 * Check to see if password is needed.
	 *	- Some users have automatic access to all categories
	 *	  of users.
	 */

	if (entry->pw_passwd == (char *) NULL) goto ok;
	if (nopasswd(getlogin())) goto ok;

	/*
	 * Get password and check it against the encrypted passwd of
	 * intended user.
	 */

	strncpy(salt,entry->pw_passwd,2);
	pword = getpass("password:");
	if (strcmp(entry->pw_passwd,crypt(pword,salt)) != 0) {
		fprintf(stderr,"Sorry Buster!\n");
		exit(1);
	}

	/*
	 * Password accepted or not nescessary continue to do the stuff
	 */
ok:
	if (setuid(entry->pw_uid) < 0) {
		fprintf(stderr,"can't set uid to %d\n",userid);
		exit(1);
	}

	if (!want_sh) {		/* Just execute the command and return */
		(void) system(cmd);
		exit(0);
	}

	/*
	 * User wants a to run a shell, check out which one if the
	 * environment variable $SHELL has been set use that one then
	 * try to use the shell defined in /etc/passwd (pw_shell) and
	 * if this fails too use the standard Bourne shell.
	 */

	if ((shell=getenv("SHELL")) == (char *) NULL) 
		if (entry->pw_shell == (char *) NULL) shell = entry->pw_shell;
		else shell = SHELL;

	/*
	 * Set the prompt to the name of the new identity or hashmark (#)
	 * for the Superuser. This works unfortunately only in Bourne Shell
	 */

	sprintf(prompt,"PS1=%s ",(argc ==1)?"#":argv[1]);

	if (putenv(prompt) != 0) {
		fprintf(stderr,"can't change environment\n");
		exit(1);
	}

	/*
	 * Ah, at last do it exec the shell...
	 */

	if (execl(shell,shell,0,envp) < 0) {
		fprintf(stderr,"can't exec %s\n",shell);
		exit(1);
	}
}

nopasswd(user)
char	*user;
{
	FILE	*suf;
	char	line[BUFSIZ];
	char	*cp;

	if (strcmp(SUPERUSER,user) == 0) return(1);

	if ((suf=fopen(SUS_ALLOW,"r")) == (FILE *) NULL) return(0);

	while (fgets(line,BUFSIZ,suf) != (char *) NULL) {
		for (cp=line; *cp != '\n'; *cp++);
		*cp = '\0';
		if (strcmp(user,line) == 0) return(1);
	}
	return(0);
}
