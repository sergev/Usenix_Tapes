From wuphys!wucs!busch!we53!mgnetp!ihnp4!houxm!mhuxj!mhuxm!mhuxn!mhuxb!mhuxr!ulysses!allegra!princeton!down!honey 11 Feb 85 
Article 207 of net.bugs.uucp:
Relay-Version: version B 2.10.2 9/17/84; site plus5.UUCP
Posting-Version: version B 2.10.2 9/18/84; site down.FUN
Path: plus5!wuphys!wucs!busch!we53!mgnetp!ihnp4!houxm!mhuxj!mhuxm!mhuxn!mhuxb!mhuxr!ulysses!allegra!princeton!down!honey
>From: honey@down.FUN (code 101)
Newsgroups: net.bugs.uucp
Subject: Re: 4.2 uucp performance problems?
Message-ID: <442@down.FUN>
Date: 11 Feb 85 15:34:09 GMT
Date-Received: 13 Feb 85 14:50:02 GMT
References: <1363@hao.UUCP> <13500001@uicsl.UUCP>
Organization: Princeton University, EECS
Lines: 16
Keywords: nap, 4.2bsd
Summary: nap() for 4.2bsd

/* "sleep" for n clock ticks */
#include <sys/time.h>
nap(n)
{
	struct timeval t;

	if (n <= 0 || n > 3000)
		return;	/* unreasonable */
	t.tv_sec = n / 60;
	t.tv_usec = ((n % 60) * 1000000) / 60;
	(void) select(32, 0, 0, 0, &t);
}

this assumes a 60 hz clock.  your mileage may vary.

	peter
