
Mlocal,	P=/bin/mail, F=rlsDFMmn, S=10, R=20, A=mail -d $u
Mprog,	P=/bin/sh,   F=lsDFMe,   S=10, R=20, A=sh -c $u
Mtty,	P=/usr/stanford/bin/to, F=rlsn, S=10, R=20, A=to $u, M=5000

S10
R@			$n			errors to mailer-daemon
