#include "uutty.h"
/* 
** We have received something that is believed to be a password.
** It is this routine's job to combine it with the current userid,
** and determine whether the combination is acceptable.  This
** routine should work on most Unix systems, but who knows?
*/
pswd(rp)
	char *rp;
{	char *p, *q;
	int   i;
	struct passwd *pp;

	D4("pswd: r=\"%s\" ss=%d",rp,ss);
	for (p=rp; *p; ++p) {	/* Examine the chars for acceptability */
		switch(*p) {
		case ':':		/* Colons aren't legal */
			D3("Invalid char '%c' in password",*p);
			Fail;
		case '\r':
		case '\n':
			*p = 0;
			goto gotit;
		case '!':		/* Special goodie for killing daemon */
			if (p[1] == 'Q') {
				if (debug) P("%s: !Q in input, quitting [id]",getime());
				die(0);
			}
		default:
			continue;
		}
	}
gotit:			/* Make a copy of the supposed id */
	p = rp;
	q = passwd;
	while (*p && q<passwd+PASSWD)
		*q++ = *p++;
	*q = 0;
	if (debug >= 3) P("%s PASSWD=\"%s\"",getime(),passwd);
	D3("userid:\"%s\"",userid);
	D3("pswd:\"%s\"",passwd);
	pp = getpwnam(userid);
	if (pp == 0) {
		D1("Login \"%s\" incorrect.",userid);
		D3("userid \"%s\" not found.",userid);
		Fail;
	}
	if (debug >= 6) Hexdnm(pp,sizeof(*pp),"Passwd:");
	D4("pw_name  =\"%s\"",pp->pw_name);
	D4("pw_passwd=\"%s\"",pp->pw_passwd);
	D4("pw_dir   =\"%s\"",pp->pw_dir);
	D4("pw_shell =\"%s\"",pp->pw_shell);
	D5("pswd:before crypt(\"%s\",\"%s\")",passwd,pp->pw_passwd);
	p = crypt(passwd,pp->pw_passwd);
	D5("pswd: after crypt(\"%s\",\"%s\")=\"%s\"",passwd,pp->pw_passwd,p);
	if (strcmp(p,pp->pw_passwd)) {
		Pwrite("Login incorrect.\r\n");
		D3("pswd \"%s\" not correct.",passwd);
		Fail;
	}
	D3("Login: uid=%d=\"%s\" group=%d accepted.",pp->pw_uid,pp->pw_name,pp->pw_gid);
/*
** To do the next few changes, we probably need to be a super-user:
*/
#ifdef SYS5
/*
** Attempt to build a utmp structure.
*/
	errno = 0;
	up = 0;
	D5("before ttyslot()");
	i = ttyslot();	/* Identify our /etc/utmp line */
	D4("ttyslot()=%d\t[errno=%d]",i,errno);
	findutmp();
	p = 0;
	fillutmp(pp->pw_name,p,devfld,USER_PROCESS);
	D4("pswd:before pututline(%06lX)",up);
	pututline(up);
#endif
	errno = 0;
	i = chmod(device,0644);			/* Restrict terminal access to owner */
	D4("chmod(\"%s\",0%o)=%d\t[errno=%d]",device,0644,i,errno);
	D3("Change \"%s\" to user %d=%s, group %d, permissions 644.",device,pp->pw_uid,pp->pw_name,pp->pw_gid);
	i = chown(device,pp->pw_uid,pp->pw_gid);	/* Change terminal's group */
	D4("chown(\"%s\",%d,%d)=%d",device,pp->pw_uid,pp->pw_gid,i);
	D3("New group %d.",pp->pw_gid);
	i = setgid(pp->pw_gid);			/* Change terminal's owner */
	D4("setgid(%d)=%d",pp->pw_gid,i);
	if (i < 0) Fail; 
	D3("New user  %d.",pp->pw_uid);
	i = setuid(pp->pw_uid);			/* Change to login id */
	D4("setuid(%d)=%d",pp->pw_uid,i);
	if (i < 0) Fail;
	D3("New directory \"%s\"",pp->pw_dir);
	i = chdir(pp->pw_dir);			/* Move to login directory */
	D4("chdir(\"%s\")=%d",pp->pw_dir,i);
	if (i < 0) Fail;
/*
** Invoke the login shell.
*/
	D5("pswd:before exec(1,\"%s\",%lX)",pp->pw_shell,pp);
	exec(1,pp->pw_shell,pp);		/* Start up a shell */
	D5("pswd: after exec(1,\"%s\",%lX)",pp->pw_shell,pp);
	target = "?";				/* We shouldn't get here */
fail:
	D4("pswd(\"%s\") FAILED.",rp);
	if (echofl ) Awrite("\r\nLogin incorrect.");
	if (m_login) {
		Awrite(m_login);
		ss = S_LOGIN;				/* Note login prompt sent */
		D4("State %d=%s",ss,gestate());
	}
	sleep(1);
	return 0;
}
