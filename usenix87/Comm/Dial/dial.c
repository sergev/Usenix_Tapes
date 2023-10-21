/* dial -- dial a remote computer using a state transition table
 *
 * Copyright (c) 1986, Oliver Laumann, Technical University of Berlin.
 * Not derived from licensed software.
 *
 * Permission is granted to freely use, copy, modify, and redistribute
 * this software, provided that no attempt is made to gain profit from it,
 * the author is not construed to be liable for any results of using the
 * software, alterations are clearly marked as such, and this notice is
 * not modified.
 */

#include <stdio.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TIMEOUT     10
#define DELAY       0
#define BUFLEN      1024
#define EXIT        ((struct state *)0)
#define LOCKFN      "/usr/spool/uucp/LCK..%s"
#define DIALLIB     "/usr/local/lib/dial"
#define REMOTE      "/etc/remote"

#define FULL        0      /* match() return values */
#define PART        1
#define NONE        2

#define ISSPACE(x) (x == ' ' || x == '\t')
#define ISOCT(x)   (x >= '0' && x <= '7')
#define ISDEC(x)   (x >= '0' && x <= '9')

char *script;
char *line;
int escape = '~';
char *prompt = "\r\n:";
struct timeval timeout = {TIMEOUT, 0};
struct timeval delay = {DELAY, 0};
char obuf[BUFLEN];
char *obp = obuf;
char rbuf[BUFLEN];
char *rbp = rbuf;
int vflag;
int pflag;
int ttyf;
struct sgttyb ottyb;
struct sgttyb consb;
struct tchars otc;
struct ltchars oltc;
short coflags;
int ttydone;
int consdone;
char *myname;
char **argp;
int numargs;
char lockfile[BUFLEN];
int locked;
int undef;
char *dialdir;
int baud = -1;
int bdconst[] = {
    B50, B75, B110, B134, B150, B200, B300, B600,
    B1200, B1800, B2400, B4800, B9600, EXTA, EXTB
};
int bdnum[] = {
    50, 75, 110, 134, 150, 200, 300, 600,
    1200, 1800, 2400, 4800, 9600, 19200, 38400, 0
};

#define S_STATE  1

struct state {
    struct state *s_next;
    char *s_string;
    int s_len;
    char *s_name;
    char *s_output;
    int s_outlen;
    struct timeval s_timeout;
    struct timeval s_delay;
    int s_stat;
} *stab;

extern errno;
extern char **environ;
char *malloc(), *alloc(), *memsav(), *getenv(), *tgetstr(), *fnexpand();
struct state *enter(), *receive(), *next_state(), *user();
struct passwd *getpwnam(), *getpwuid();
long atol();

main (ac, av) char **av; {
    register struct state *next;
    register char *p;
    int Exit();

    myname = ac < 1 ? "dial" : av[0];
    if (ac < 1) {
help:   err ("Use: %s [-v] [-p] [-lline] [script-file [args...]]\n", myname);
    }
    while (--ac) {
	p = *++av;
	if (*p == '-' && p[1] != '\0') {
	    if (*++p == 'v') {
		++vflag;
	    } else if (*p == 'p') {
		++pflag;
	    } else if (*p == 'l') {
		line = ++p;
	    } else goto help;
	} else if (!script) {
	    script = p;
	} else {
	    argp = av;
	    numargs = ac;
	    break;
	}
    }
    if (script) {
	dialdir = getenv ("DIALDIR");
	get_script ();
	check_script ();
    }
    if (!line || line[0] == '\0')
	err ("No tty line specified.\n");
    signal (SIGHUP, Exit);
    signal (SIGINT, Exit);
    open_line ();
    if (!stab)
	say ("Connected.\n");
    set_tty ();
    next = (stab ? stab : user ());
    while (next != EXIT)
	next = enter (next);
    if (vflag)
	say ("State `exit' -- done.\n");
    Exit (0);
}

get_script () {
    FILE *f = NULL;
    char fn[BUFLEN], lbuf[BUFLEN], buf[BUFLEN];
    register char *p, *s;
    register struct state *sp, *tmp;
    register c, delim, lno = 0;
    struct stat st;

    if (!strcmp (script, "-")) {
	f = stdin;
	script = "stdin";
    } else {
	if (stat (script, &st) || (st.st_mode & S_IFMT) == S_IFDIR
			       || (f = fopen (script, "r")) == NULL) {
	    if (dialdir) {
		sprintf (fn, "%s/%s", dialdir, script);
		f = fopen (fn, "r");
	    }
	    if (f == NULL) {
		sprintf (fn, "%s/%s", DIALLIB, script);
		if ((f = fopen (fn, "r")) == NULL)
		    err ("Cannot open `%s'.\n", script);
	    }
	}
    }
    while (1) {
	++lno;
	if (fgetstr (f, lbuf, BUFLEN-1) == EOF)
	    break;
	if (!expand_args (lbuf, buf)) goto error;
	for (p = buf; ISSPACE(*p); ++p) ;
	if (*p == '#' || *p == '\0')
	    continue;
	if (p == buf) {
	    if (getkey (p))
		continue;
	} else if (!stab) goto error;
	tmp = (struct state *)alloc (sizeof (struct state));
	tmp->s_stat = (p == buf ? S_STATE : 0);
	for (s = p; *s && !ISSPACE(*s); ++s) ;
	if (*s == '\0') goto error;
	*s = '\0';
	tmp->s_name = memsav (p, s-p+1);
	for (++s; ISSPACE(*s); ++s) ;
	if (delim = (*s == '"')) ++s;
	for (p = s; *s && (delim ? *s != '"' : !ISSPACE(*s)); ++s) ;
	if (delim) {
	    if (*s == '\0') goto error;
	    *s++ = '\0';
	}
	c = *s;
	*s = '\0';
	tmp->s_len = cook (p, p);
	tmp->s_string = memsav (p, tmp->s_len);
	tmp->s_outlen = 0;
	tmp->s_timeout = timeout;
	tmp->s_delay = delay;
	if (c) {
	    for (++s; ISSPACE(*s); ++s) ;
	    if (*s) {
		if (delim = (*s == '"')) ++s;
		for (p = s; *s && (delim ? *s != '"' : !ISSPACE(*s)); ++s) ;
		if (delim) {
		    if (*s == '\0') goto error;
		    *s++ = '\0';
		}
		*s = '\0';
		if (tmp->s_stat & S_STATE) {
		    if (!set_par (tmp, p)) goto error;
		} else {
		    tmp->s_outlen = cook (p, p);
		    tmp->s_output = memsav (p, tmp->s_outlen);
		}
	    }
	}
	if (stab)
	    sp = sp->s_next = tmp;
	else
	    stab = sp = tmp;
	sp->s_next = 0;
    }
    fclose (f);
    return;
error:
    err ("%s: syntax error in line %d.\n", script, lno);
}

expand_args (from, to) char *from, *to; {
    register char *p, *q, *t, *s = to, *endp = to+BUFLEN-1;
    register n;
    FILE *f;

    for (p = from; *p; ++p) {
	if (s == endp)
	    break;
	if (*p == '$') {
	    if (p > from && p[-1] == '\\') {
		s[-1] = '$';	
	    } else if (ISDEC(p[1])) {
		if (*++p - '0' <= numargs) {
		    for (q = argp[*p - '0']; s < endp && *q; *s++ = *q++)
			;
		}
	    } else if (p[1] == '*') {
		for (n = 0; n < numargs; ++n) {
		    for (q = argp[n]; s < endp && *q; *s++ = *q++)
			;
		    if (n < numargs-1 && s < endp)
			*s++ = ' ';
		}
		++p;
	    } else if (p[1] == '{') {
		++p;
		for (q = ++p; *p && *p != '}'; ++p) ;
		if (*p == '\0') return (0);
		*p = '\0';
		if (*q == '~' || *q == '/') {
		    t = fnexpand (q);
		    if ((f = fopen (t, "r")) == NULL)
			err ("Cannot open `%s'.\n", t);
		    while ((n = getc (f)) != EOF && s < endp)
			*s++ = n;
		    fclose (f);
		} else if (t = getenv (q)) {
		    while (s < endp && *t)
			*s++ = *t++;
		}
	    } else *s++ = '$';
	} else *s++ = *p;
    }
    *s = '\0';
    return (1);
}

char *fnexpand (s) char *s; {
    static char buf[BUFLEN];
    struct passwd *pw;
    register char c, *p, *q = s+1;

    if (*s != '~')
	return (s);
    for (p = q; (c = *p) && c != '/'; ++p) ;
    *p = '\0';
    if (p == q) {
	if (!(pw = getpwuid (getuid ())))
	    err ("Cannot get home directory.\n");
    } else if (!(pw = getpwnam (q)))
	err ("Unknown user: %s.\n", q);
    if (c) {
	sprintf (buf, "%s/%s", pw->pw_dir, ++p);
	return (buf);
    } else return (pw->pw_dir);
}

set_par (sp, s) struct state *sp; char *s; {
    register char *p;

    for (p = s; *p && *p != ','; ++p) ;
    if (*p == ',')
	*p++ = '\0';
    if (*s) {
	if (!isnum (s)) return (0);
	set_time (&sp->s_timeout, s);
    }
    if (*p) {
	if (!isnum (p)) return (0);
	set_time (&sp->s_delay, p);
    }
    return (1);
}

set_time (tp, s) struct timeval *tp; char *s; {
    register char *p;
    register long div;

    for (p = s; *p && *p != '.'; ++p) ;
    if (*p == '.')
	*p++ = '\0';
    tp->tv_sec = atoi (s);
    if (p) {
	p[6] = '\0';
	tp->tv_usec = atol (p);
	div = 1;
	while (*p) {
	    div *= 10;
	    ++p;
	}
	tp->tv_usec *= 1000000L / div;
    } else tp->tv_usec = 0;
}

getkey (s) char *s; {
    register char *k, *p, *q;

    for (p = s; *p; ++p)
	if (*p == '=' && !(p > s && p[-1] == '\\'))
	    break;
    if (*p == '\0') return (0);
    for (*p++ = '\0'; ISSPACE(*p); ++p) ;
    for (q = p; *q && !ISSPACE(*q); ++q) ;
    *q = '\0';
    for (k = s; *k && !ISSPACE(*k); ++k) ;
    *k = '\0';
    if (!strcmp (s, "line")) {
	if (*p == '\0')
	    err ("Bad value for keyword `line'.\n");
	if (!line)
	    line = memsav (p, strlen (p)+1);
    } else if (!strcmp (s, "timeout")) {
	if (!isnum (p))
	    err ("Bad value for keyword `timeout'.\n");
	set_time (&timeout, p);
    } else if (!strcmp (s, "delay")) {
	if (!isnum (p))
	    err ("Bad value for keyword `delay'.\n");
	set_time (&delay, p);
    } else if (!strcmp (s, "escape")) {
	if (strlen (p) != 1)
	    err ("Bad value for keyword `escape'.\n");
	escape = *p & 0177;
    } else if (!strcmp (s, "prompt")) {
	prompt = memsav (p, strlen (p)+1);
    } else err ("Invalid keyword `%s'.\n", s);
    return (1);
}

check_script () {
    register struct state *sp;

    for (sp = stab; sp; sp = sp->s_next) {
	if (!(sp->s_stat & S_STATE)) continue;
	if (!sp->s_next || (sp->s_next->s_stat & S_STATE))
	    err ("No transitions for state `%s'.\n", sp->s_name);
    }
}

struct state *enter (sp) struct state *sp; {
    if (vflag)
	say ("Entering state %s.\n", sp->s_name);
    if (sp->s_delay.tv_sec || sp->s_delay.tv_usec) {
	flushbuf ();
	if (select (0, (int *)0, (int *)0, (int *)0, &sp->s_delay) == -1) {
	    perror ("select");
	    Exit (1);
	}
    }
    sendstr (sp->s_string, sp->s_len);
    return (receive (sp));
}

sendstr (sbuf, len) char *sbuf; register len; {
    char buf[BUFLEN], ibuf[BUFLEN];
    register char *s, *p, *t;
    register n;
    char c;

    for (s = sbuf, p = buf; len && p < buf+BUFLEN; --len, ++p, ++s) {
	if (*s == '\\') {
	    if (s[1] == '\\') {
		*p = '\\'; ++s;
	    } else if (s[1] == '<' || s[1] == '?') {
		++s;
		flushbuf ();
		if (*s == '<') {
		    n = Read (0, ibuf, BUFLEN);
		    c = '\n';
		} else {
		    set_cons ();
		    for (t = ibuf, n = 0; n < BUFLEN; ++n, ++t) {
			Read (0, t, 1);
			if (*t == '\r') break;
		    }
		    reset_cons ();
		    if (!pflag)
			say ("\n");
		    c = '\r';
		}
		for (t = ibuf; n && *t != c && p < buf+BUFLEN; --n)
		    *p++ = *t++;
	    } else *p = '\\';
	} else *p = *s;
    }
    if (vflag) {
	say ("Sending \""); prstr (buf, p-buf); say ("\".\n");
    }
    if (p > buf)
	Write (ttyf, buf, p-buf);
}

fill (sp) struct state *sp; {
    register i;
    register char *p;
    int rbits = (1 << ttyf);

    if (rbp == rbuf+BUFLEN)
	err ("Pattern space overflow.\n");
    flushbuf ();
    i = select (ttyf+1, &rbits, (int *)0, (int *)0, &sp->s_timeout);
    if (i == -1) {
	perror ("select");
	Exit (1);
    }
    if (i == 0) {
	if (vflag)
	    say ("   Got time-out.\n");
	return (0);
    }
    i = Read (ttyf, rbp, rbuf+BUFLEN-rbp);
    p = rbp;
    do {
	*p++ &= 0177;
    } while (--i);
    if (vflag) {
	say ("   Got \""); prstr (rbp, p-rbp); say ("\".\n");
    }
    if (pflag)
	Write (1, rbp, p-rbp);
    rbp = p;
    return (1);
}

shift (n) {
    bcopy (rbuf+n, rbuf, rbp-rbuf-n);
    rbp -=n;
}

struct state *receive (sp) struct state *sp; {
    register struct state *p, *fullp, *partp;
    register r, mlen, dofill = 0;
    int len;

    while (1) {
	if ((rbp == rbuf || dofill) && !fill (sp)) {
	    rbp = rbuf;
	    for (p = sp->s_next; p && !(p->s_stat & S_STATE); p = p->s_next) {
		if (p->s_len == 1 && p->s_string[0] == '#') {
		    output (p, (char *)0, 0);
		    return (next_state (p->s_name, 1));
		}
	    }
	    return (EXIT);
	}
	dofill = 0;
	fullp = partp = 0;
	for (p = sp->s_next; p && !(p->s_stat & S_STATE); p = p->s_next) {
	    if (p->s_len == 1 && p->s_string[0] == '#')
		continue;
	    r = match (rbuf, rbp-rbuf, p->s_string, p->s_len, &len);
	    if (r == FULL && !fullp) {
		fullp = p;
		mlen = len;
	    } else if (r == PART && !partp) {
		partp = p;
	    }
	}
	if (!fullp && !partp) {
	    shift (1);
	    if (vflag)
		say ("No match -- shift(1).\n");
	    continue;
	}
	if (fullp && !partp) {
	    if (fullp->s_len == 1 && fullp->s_string[0] == '*')
		mlen = amatch (sp);
	    if (vflag)
		say ("Full match <%s> -- shift(%d).\n", fullp->s_name, mlen);
	    output (fullp, rbuf, mlen);
	    shift (mlen);
	    return (next_state (fullp->s_name, 1));
	}
	if (vflag)
	    say ("Partial match <%s>.\n", partp->s_name);
	dofill = 1;
    }
}

amatch (ap) struct state *ap; {
    register struct state *sp;
    register char *p;
    int notused;

    for (p = rbuf; p < rbp; ++p) {
	for (sp = ap->s_next; sp && !(sp->s_stat & S_STATE); sp = sp->s_next) {
	    if (sp->s_len == 1 &&
		    (sp->s_string[0] == '#' || sp->s_string[0] == '*'))
		continue;
	    if (match (p, rbp-p, sp->s_string, sp->s_len, &notused) != NONE)
		goto done;
	}
    }
done:
    return (p-rbuf);
}

match (s, slen, t, tlen, len) register char *s, *t; register slen, tlen;
	int *len; {
    register otlen = tlen, oslen = slen;

    if (tlen == 1 && t[0] == '*')
	return (FULL);
    while (tlen) {
	if (!slen)
	    return (PART);
	if (*t != '\\') {
	    if (*s != *t)
		if (*t != '.' || (tlen < otlen && t[-1] == '\\'))
		    return (NONE);
	    ++s; --slen;
	}
	++t; --tlen;
    }
    *len = oslen-slen;
    return (FULL);
}

struct state *next_state (s, noise) char *s; {
    register struct state *sp;

    undef = 0;
    if (!strcmp (s, "exit"))
	return (EXIT);
    if (!strcmp (s, "user")) {
	return (user ());
    }
    for (sp = stab; sp; sp = sp->s_next)
	if ((sp->s_stat & S_STATE) && !strcmp (sp->s_name, s))
	    return (sp);
    ++undef;
    if (noise)
	err ("State `%s' not found in script.\n", s);
#ifdef lint
    return (sp);
#endif
}

struct state *user () {
    char c, buf[BUFLEN];
    register n, l, gotesc = 0;
    int rbits;
    register char *p, *s;
    register struct state *sp;

    flushbuf ();
    signal (SIGINT, SIG_IGN);
    set_cons ();
    while (1) {
	rbits = (1 << 0) | (1 << ttyf);
	n = select (ttyf+1, &rbits, (int *)0, (int *)0, (struct timeval *)0);
	if (n == -1) {
	    perror ("select");
	    Exit (1);
	}
	if (rbits & (1 << 0)) {
	    if (gotesc) {
		Read (0, &c, 1);
		if (c == '.' || c == otc.t_eofc) {
		    reset_cons ();
		    say ("\nConnection closed.\n");
		    return (EXIT);
		} else if (c == oltc.t_suspc) {
		    reset_cons ();
		    kill (getpid (), SIGTSTP);
		    set_cons ();
		} else if (c == '!') {
		    reset_cons ();
		    say ("\n");
		    if (system ((s = getenv ("SHELL")) ? s : "/bin/sh") == 127)
			say ("Cannot execute shell.\n");
		    else say ("\n!\n");
		    set_cons ();
		} else if (c == '?') {
		    reset_cons ();
		    print_help ();
		    set_cons ();
		} else if (c == ' ' || c == '\r') {
		    Write (1, prompt, strlen (prompt));
		    reset_cons ();
		    if ((n = Read (0, buf, BUFLEN)) > 1) {
			buf[n-1] = '\0';
			for (p = buf; ISSPACE(*p); ++p) ;
			for (s = p; *s && !ISSPACE(*s); ++s) ;
			*s = '\0';
			if (s > p) {
			    sp = next_state (p, 0);
			    if (undef) {
				say ("`"); prstr (p, s-p);
				say ("' is undefined.\n");
			    } else {
				signal (SIGINT, Exit);
				return (sp);
			    }
			}
		    }
		    set_cons ();
		} else Write (ttyf, &c, 1);
		gotesc = 0;
	    } else {
		n = Read (0, buf, BUFLEN);
		for (p = buf, l = n; l && *p != escape; ++p, --l) ;
		if (l) {
		    if (p > buf)
			Write (ttyf, buf, p-buf);
		    ++gotesc;
		    while (--l)
			ioctl (0, TIOCSTI, ++p);
		} else Write (ttyf, buf, n);
	    }
	}
	if (rbits & (1 << ttyf))
	    Write (1, buf, Read (ttyf, buf, BUFLEN));
    }
}

print_help () {
    char e[3];

    if (escape >= ' ' && escape <= '~')
	sprintf (e, "%c", escape);
    else
	sprintf (e, "^%c", escape ^ '@');
    say ("\n%s.       close connection and terminate.\n", e);
    say ("%s!       fork a shell.\n", e);
    say ("%s^Z      suspend %s.\n", e, myname);
    say ("%s<CR>\n", e);
    say ("%s<SPACE> leave interactive mode and enter new state.\n", e);
}

output (sp, buf, len) struct state *sp; char *buf; {
    char tbuf[BUFLEN];
    register char *s = sp->s_output, *p = tbuf;
    register i, olen = sp->s_outlen;

    if (pflag || !olen)
	return;
    while (olen) {
	if (p == tbuf+BUFLEN) break;
	if (*s == '&' && !(s > sp->s_output && s[-1] == '\\')) {
	    for (i = 0; i < len && p < tbuf+BUFLEN; ++i)
		*p++ = buf[i];
	} else if (*s != '\\') *p++ = *s;
	++s;
	--olen;
    }
    if (vflag) {
	say ("Print \""); prstr (tbuf, p-tbuf); say ("\".\n");
    } else {
	outbuf (tbuf, p-tbuf);
    }
}

outbuf (buf, len) register char *buf; register len; {
    register l;

again:
    l = obuf + BUFLEN - obp;
    if (l > len) l = len;
    bcopy (buf, obp, l);
    obp += l;
    if (len -= l) {
	flushbuf ();
	buf += l;
	goto again;
    }
}

flushbuf () {
    if (obp > obuf) {
	Write (1, obuf, obp-obuf);
	obp = obuf;
    }
}

open_line () {
    char fn[BUFLEN];
    char tbuf[1024], cbuf[1024];
    int i, f;
    struct stat statbuf;
    char **oldenv, *envp[2], *p, *pp = cbuf;

    sprintf (fn, line[0] == '/' ? "%s" : "/dev/%s", line);
    if (stat (fn, &statbuf) == -1 && errno == ENOENT) {
	sprintf (tbuf, "TERMCAP=%s", REMOTE);
	envp[0] = memsav (tbuf, strlen (tbuf));
	envp[1] = 0;
	oldenv = environ;
	environ = envp;
	if ((i = tgetent (tbuf, line)) == -1)
	    err ("Cannot open `%s'.\n", REMOTE);
	else if (i == 0)
	    err ("`%s': no such line or remote system.\n", line);
	if ((p = tgetstr ("dv", &pp)) == 0)
	    err ("No `dv' capability for system `%s'.\n", line);
	strcpy (fn, p);
	if ((baud = tgetnum ("br")) != -1) {
	    for (i = 0; bdnum[i] && baud != bdnum[i]; ++i) ;
	    if (!bdnum[i])
		err ("Invalid baud rate %d for system `%s'.\n", baud, line);
	    baud = bdconst[i];
	}
	environ = oldenv;
    }
    if (!strncmp (fn, "/dev/", 5)) {
	sprintf (lockfile, LOCKFN, fn+5);
	if ((f = open (lockfile, O_CREAT|O_EXCL, 0)) == -1) {
	    if (errno == EEXIST)
		err ("`%s' is already in use.\n", line);
	    perror (lockfile);
	    Exit (1);
	}
	locked++;
	close (f);
    }
    if ((ttyf = open (fn, O_RDWR)) == -1) {
	perror (line);
	Exit (1);
    }
    ioctl (ttyf, TIOCHPCL, (char *)0);   /* Hang up phone on last close. */
}

set_cons () {
    struct tchars tc;
    struct ltchars ltc;

    ioctl (0, TIOCGETP, &consb);
    coflags = consb.sg_flags;
    consb.sg_flags &= ~ECHO;
    consb.sg_flags &= ~CRMOD;
    consb.sg_flags |= CBREAK;
    ioctl (0, TIOCSETP, &consb);
    ioctl (0, TIOCGETC, &tc);
    otc = tc;
    tc.t_intrc = tc.t_quitc = -1;
    ioctl (0, TIOCSETC, &tc);
    ioctl (0, TIOCGLTC, &ltc);
    oltc = ltc;
    ltc.t_suspc = ltc.t_dsuspc = ltc.t_lnextc = -1;
    ioctl (0, TIOCSLTC, &ltc);
    ++consdone;
}

reset_cons () {
    if (consdone) {
	consb.sg_flags = coflags;
	ioctl (0, TIOCSETP, &consb);
	ioctl (0, TIOCSETC, &otc);
	ioctl (0, TIOCSLTC, &oltc);
	consdone = 0;
    }
}

set_tty () {
    struct sgttyb ttyb;
    ioctl (ttyf, TIOCGETP, &ttyb);
    ottyb = ttyb;
    ttyb.sg_flags &= ~ECHO;
    ttyb.sg_flags &= ~CRMOD;
    ttyb.sg_flags |= (RAW|TANDEM);
    if (baud != -1)
	ttyb.sg_ispeed = ttyb.sg_ospeed = baud;
    ioctl (ttyf, TIOCSETP, &ttyb);
    ++ttydone;
}

reset_tty () {
    if (ttydone)
	ioctl (ttyf, TIOCSETP, &ottyb);
}

Exit (c) {
    flushbuf ();
    reset_tty ();
    reset_cons ();
    if (locked) {
	if (unlink (lockfile) == -1)
	    perror (lockfile);
    }
    exit (c);
}

Write (f, buf, len) char *buf; {
    if (write (f, buf, len) != len) {
	if (errno == EIO)
	    err ("Lost line.\n");
	else {
	    perror ("write");
	    Exit (1);
	}
    }
}

Read (f, buf, len) char *buf; {
    register n;
    if ((n = read (f, buf, len)) == -1) {
	if (errno == EIO)
	    err ("Lost line.\n");
	else {
	    perror ("read");
	    Exit (1);
	}
    }
    if (n == 0)
	err ("EOF on tty.\n");
    return (n);
}

char *memsav (s, len) char *s; {
    register char *p;
    p = alloc (len);
    bcopy (s, p, len);
    return (p);
}

char *alloc (n) {
    register char *p;
    if ((p = malloc (n)) == 0)
	err ("Out of memory.\n");
    return (p);
}

cook (dst, src) char *dst, *src; {
    register char *p, *t;
    register n, c;

    for (p = dst, t = src; c = *t; ++t) {
	if (c == '\\') {
	    if (ISOCT(t[1])) {
		for (n = c = 0; n < 3; ++n) {
		    c = (c << 3) | (*++t - '0');
		    if (!ISOCT(t[1]))
			break;
		}
	    } else {
		if (t[1] == 'r') {
		    c = '\r'; ++t;
		} else if (t[1] == 'n') {
		    c = '\n'; ++t;
		} else if (t[1] == '\0')
		    break;
	    }
	}
	*p = c; ++p;
    }
    return (p - dst);
}

prstr (s, len) char *s; int len; {
    register c, l;

    for (l = len; l; --l) {
	if ((c = *s++) == '\r')
	    say ("\\r");
	else if (c == '\n')
	    say ("\\n");
	else if (c < ' ' || c > '~')
	    say ("\\%03o", c & 0377);
	else say ("%c", c);
    }
}

fgetstr (f, buf, len) register FILE *f; char *buf; {
    register c, n;
    register char *p = buf;

    n = len;
    while ((c = getc (f)) != EOF && c != '\n')
	if (n) {
	    --n; *p++ = c;
	}
    *p = '\0';
    return (c == EOF && p == buf ? EOF : p - buf);
}

isnum (s) register char *s; {
    register point = 0;

    if (*s == '\0') return (0);
    for ( ; *s; ++s) {
	if (*s == '.') {
	    if (point) return (0);
	    ++point;
	} else if (!ISDEC(*s)) return (0);
    }
    return (1);
}

/*VARARGS1*/
say (fmt, p1, p2, p3, p4) char *fmt; {
    fprintf (stdout, fmt, p1, p2, p3, p4);
}

/*VARARGS1*/
err (fmt, p1, p2, p3, p4) char *fmt; {
    fprintf (stderr, fmt, p1, p2, p3, p4);
    Exit (1);
}
