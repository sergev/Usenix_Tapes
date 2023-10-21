>From mtplx1!ihnp4!houxm!mhuxl!eagle!harpo!decvax!wivax!linus!utzoo!dciem!tim Wed Dec 31 18:00:00 1969
Relay-Version: version B 2.10.1 6/24/83; site washu.UUCP
Posting-Version: version B 2.10.1 6/24/83; site dciem.UUCP
Path: washu!mtplx1!ihnp4!houxm!mhuxl!eagle!harpo!decvax!wivax!linus!utzoo!dciem!tim
From: tim@dciem.UUCP
Newsgroups: net.sources
Subject: Re: UUCP analysis
Message-ID: <472@dciem.UUCP>
Date: Tue, 8-Nov-83 07:48:38 CST
Date-Received: Wed, 9-Nov-83 17:23:00 CST
References: 16@trwspp.UUCP
Organization: D.C.I.E.M, Toronto, Canada
Lines: 75

Long, long ago, in a newsgroup far, far away...
Way back in April, zemon@trwspp submitted a couple of awk scripts for
analyzing the information in the uucp LOGFILE and SYSFILE. Since this
site is in the process of setting up several new connections, we wanted
to keep track of how well the connections were being made. Since the
awk script mentioned above provided success/failue/locked counts for
each site, it seemed reasonable to extract this from the output from awk.
At this site, the LOGFILE and SYSFILE are backed-up and zero'ed every
morning. zemon's awk scripts are run just before this to get the information
on the previous day's activity, with the output concatenated to a permanent
log file. The awk script below uses this log file to provide a summary
of connection statuses.


----------------------------------------------------------------------------
# USAGE: awk -f thisfile logfilefromuucpanalysis

BEGIN	{
	nsystems = 0;
	}
/summary/,/AVAILABLE/ {
	if (insummary == 0)
		{
		insummary = 1;
		next;
		}

	if (insummary) {
		if (NF == 3 && $1 != "NO" && $1 != "UUCP") {
			system=$1
			for(i=0; i<nsystems; i++)
				if (system == systems[i])
					break;
			if (i == nsystems) {
				systems[nsystems++] = system;
				successes[system] = 0;
				failures[system] = 0;
				locked[system] = 0;
			}
		}
		if ($3=="successes")
			successes[system] += $2;
		if ($2=="failures")
			failures[system] += $1;
		if ($2=="locked")
			locked[system] += $1;
		}
	}

END	{
	for (i=0; i<nsystems; i++) {
		system = systems[i];
		printf("%-7s: %3d succ;  %3d fail;  %3d lock\n", system,			successes[system], failures[system], locked[system]);
	}
}


--------------------------------------------------------------------------

Sample output:


rhodniu:   7 succ;    0 fail;    0 lock
trigrap:  26 succ;    8 fail;    3 lock
hcr    :  14 succ;    1 fail;    0 lock
utzoo  :  25 succ;   31 fail;    1 lock
per    :  39 succ;    8 fail;    0 lock
rds    :  48 succ;    0 fail;   11 lock
utcsrgv:  23 succ;    2 fail;    1 lock
psddevl:   7 succ;   11 fail;    0 lock


--------------------------------------------------------------------------
>From this, one can vary quickly see that we are having trouble contacting
"utzoo" and "psddevl". The above represents several (8?) days connections.




