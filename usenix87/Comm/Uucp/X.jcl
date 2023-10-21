From wuphys!wucs!busch!we53!mgnetp!ihnp4!mhuxn!mhuxr!ulysses!allegra!princeton!down!honey Tue Dec  3 19:55:59 CST 1985
Article 7970 of net.unix-wizards:
Relay-Version: version B 2.10.3 4.3bsd-beta 6/6/85; site plus5.UUCP
Posting-Version: version B 2.10.2 9/18/84; site down.FUN
Path: plus5!wuphys!wucs!busch!we53!mgnetp!ihnp4!mhuxn!mhuxr!ulysses!allegra!princeton!down!honey
>From: honey@down.FUN (Peter Honeyman)
Newsgroups: net.unix-wizards
Subject: Re: Why is UUCP Notoriously Unreliable?
Message-ID: <636@down.FUN>
Date: 2 Dec 85 23:54:32 GMT
Date-Received: 3 Dec 85 19:05:48 GMT
References: <813@rlgvax.UUCP> <99500005@ima.UUCP> <3041@sun.uucp>
Organization: CS Dept., Princeton University
Lines: 41

In a recent note, Guy ended with a question: "Peter?"

The JCL cards in the X. files are as follows:

C       command to be executed

I       file name for command input

O       file name for command output

F       file required to be present before running command; optional
	second argument gives name for the file at execution time

R       name of user who issued request (relative to the host named on
	the U line)

U       second arg is name of host that passed this X. file to me;
	first arg is a user name on that host (overridden by R line)

Z       send status notification (and error output) if command failed

N       send no status notification if command failed

n       send status notification if command succeeded

B       return command input on error

e       requests command be processed with sh(1)

E       requests command be processed with exec(2)

M       return status info to the named file on the requesting host

#       comment line

Anything else is a comment line.  Does that answer the question, Guy?

	Peter Honeyman

PS:  UUCP is not notoriously unreliable in my neighborhood.  In fact,
it's not unreliable at all.


