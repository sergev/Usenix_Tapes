/*
 * extensions to the shell
 *
 * George K. Rosenberg
 * 17 Mar 80
 *
 * modified:
 *	24 Apr 80	GKR
 *	8 Jun 80	GKR
 */

#include "extend.h"
#include "defs.h"
#include "errno.h"
#include <a.out.h>

#define	EXISTS(loc)	((char *)(&(head.loc)+1) <= limit)
#define	RESET(flag)	(flag = setting == 2 ? !flag : setting)


extern errno;
MSG nonascii;
MSG sep_id;
MSG badseek;
MSG badarg;
MSG defpthd;

int pthderr;

MSG onnam = "on";
MSG offnam = "off";
MSG cd;
MSG _chdir;
MSG toggle;
MSG slashnam = "slash";
int slashok = SLASH;
int implcd = IMPLCD;



/* customized interpreters */
custom(command,file,argv,key)
char *command;
char *file;
register char *argv[];
{
	register char **av;
	register char *path;
	char **arglist;

	if (key)
		++lock1;

	itexists = 1;

	getstak(length(file));

	for (av=argv; *av++ != 0;) ;

	arglist = av = (char **)alloc((av-argv+1) * sizeof *argv) ;

	*av++ = command;
	*av++ = file;
	while (*av++ = *++argv) ;

	path = getpath(command);

	while (path = execs(path,arglist)) ;

	if (key)
		--lock1;

}



/* dot file evaluator */
char *dotfile(arglist,getp)
register char *arglist[];
char *(*getp)();
{
	auto struct argenv ae;
	register fd;
	register char **av;

	if (arglist[0] == 0)
		return 0;	/* degenerate case */

	pthderr = 0;

	fd = pthdopen((*getp)(arglist[0]),arglist[0]);

	if (fd == -1)
		return pthderr ? restricted : notfound ;

	if ((filetype(fd)&BINtyp) != 0) {
		close(fd);
		return nonascii;
	}

	if (lseek(fd,0L,0) != 0) {
		close(fd);
		return badseek;
	}

	for (av = arglist-1; *++av != 0;) ;

	ae.ae_cmd = shrp.ae_cmd ;
	ae.ae_list = shrp.ae_list ;
	ae.ae_nargs = shrp.ae_nargs ;
	ae.ae_id = shrp.ae_id ;

	shrp.ae_cmd = arglist[0] ;
	shrp.ae_list = arglist ;
	shrp.ae_nargs = av-arglist-1 ;
	shrp.ae_id = ++uniqid ;

	assnum(&shrpnarg,shrp.ae_nargs);
	assnum(&shrpid,shrp.ae_id);

	execexp(0,fd);

	shrp.ae_cmd = ae.ae_cmd ;
	shrp.ae_list = ae.ae_list ;
	shrp.ae_nargs = ae.ae_nargs ;
	shrp.ae_id = ae.ae_id ;

	assnum(&shrpnarg,shrp.ae_nargs);
	assnum(&shrpid,shrp.ae_id);

	return 0;

}


filetype(fd)
{
	/* this routine may move the I/O pointer */
	int type;
	static union header head;
	int r;
	register i;
	register char *limit;

	type = 0;

	r = read(fd,&head,sizeof head);

	if (r <= 0)
		return type;

	for (i=r; --i >= 0;)
		if (head.bytes[i] & ~'\177') {
			type |= BINtyp;
			break;
		}

#ifdef RTS
	limit = &(head.bytes[r]);
	if (
		EXISTS(csh) &&
		head.csh == CSHMAGIC &&
		(type&BINtyp) == 0
	)
		type |= CSHtyp ;
#ifdef vax
	if (EXISTS(UNIX.magic11))
		switch (head.UNIX.magic11) {

			case A_MAGIC1:
			case A_MAGIC2:
				type |= UNIX7typ|UNIX6typ ;
				break;

			case A_MAGIC3:
				type |= UNIX7typ|UNIX6typ|SEPIDtyp ;
				break;

			case A_MAGIC4:
				type |= UNIX7typ|OVERtyp ;
				break;

		}

	/* v6 and v7 absolute a.out files may some day differ here */
	if (EXISTS(UNIX.flag11))
		if (head.UNIX.flag11 == 0)
			type &= ~UNIX6typ ;
	/*				 ... but they don't now
		else
			type &= ~UNIX7typ ;
	/* the above else clause might be commented out */

	/*
	 * the UNIX v6 a.out files at
	 * University of Pittsburgh IDIS
	 * have a 1 at .unused11
	 * to distinguish them from v7 a.out files
	 * this location is unused (i.e. 0) in standard UNIX v6
	 */
	if (
		EXISTS(UNIX.unused11) &&
		head.UNIX.unused11 == 1
	)
		type &= ~UNIX7typ ;
#endif
#ifdef eleven
	if (
		EXISTS(rt11.rtmagic) &&
		head.rt11.rtmagic == RT11MAGIC
	)
		type |= RT11typ ;
#endif
#endif
	return type;
}


/* see getpath (service.c) */
char *getpthd(file)
char *file;
{
	pthderr = 0;

	if (any('/',file))
		if (flags&rshflg) {
			pthderr = 1;
			return 0;
		}
		else
			if (!slashok)
				return 0;
			else
				return nullstr;
	else
		if (pthdnod.namval == 0)
			return defpthd;
		else
			return cpystak(pthdnod.namval) ;

}


pthdopen(path,name)
register char *path;
register char *name;
{
	register fd;
	int skip;

	for (;;) {
		if (path == 0)
			return -1;
		skip = !itexists && (*path == COLON || *path == '\0') ;
		path = catpath(path,name);
		if (skip)
			continue;
		if (access(curstak(),1) != -1)
			continue;
		if (errno == EACCES) {
			fd = open(curstak(),0);
			if (fd != -1)
				return fd;
		}
	}
}


#ifdef RTS
/* special run time systems */
rts(file,argv,fd)
char *file;
char *argv[];
{
	register type;
	register char *system;

	if (lock2 >= NRTS)
		return fd;

	++lock2;

	type = filetype(fd);

#ifdef vax
	system = unix7nod.namval ;
	if (system != 0 && system[0] != '\0' && type&UNIX7typ)
		if (type&SEPIDtyp) {
			lock2 = 0;
			close(fd);
			failed(file,sep_id);
		} else
			custom(system,file,argv,0);

	system = unix6nod.namval ;
	if (system != 0 && system[0] != '\0' && type&UNIX6typ)
		if (type&SEPIDtyp) {
			lock2 = 0;
			close(fd);
			failed(file,sep_id);
		} else
			custom(system,file,argv,0);
#endif

#ifdef eleven
	system = rt11nod.namval;
	if (system != 0 && system[0] != '\0' && type&RT11typ)
		custom(system,file,argv,0);
#endif

	system = cshnod.namval;
	if (system != 0 && system[0] != '\0' && type&CSHtyp)
		custom(system,file,argv,0);

	--lock2;

	if (type&BINtyp) {
		close(fd);
		fd = -2;
	}

	return fd;

}
#endif


/* toggle command */
tgl(argc,argv)
char *argv[];
{
	int setting;
	int badtok;

	if (argc <= 0) {
		prtgl(cd,implcd);
		prtgl(slashnam,slashok);
		return;
	}

	setting = 2;

	if (cf(*argv,onnam) == 0) {
		setting = 1;
		++argv;
		--argc;
	} else if (cf(*argv,offnam) == 0) {
		setting = 0;
		++argv;
		--argc;
	}

	badtok = 0;

	for (;;) {
		if (--argc < 0)
			break;
		if (cf(*argv,cd) == 0 || cf(*argv,_chdir) == 0)
			RESET(implcd);
		else if (cf(*argv,slashnam) == 0)
			RESET(slashok);
		else
			badtok = 1;
		++argv;
	}

	if (badtok)
		failed(toggle,badarg);

}


prtgl(name,value)
char *name;
{
	prs(name);
	prc(SP);
	prs(value ? onnam : offnam);
	prc(NL);
}
