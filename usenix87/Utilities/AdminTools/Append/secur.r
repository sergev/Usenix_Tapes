.TL
A Short Essay on Unix Security
.AU
David R. Brown
(lethe!dave, watmath!watbun!drbrown)
.AB
.PP
This note discusses a small disfeature in Unix 
in light of
the Multics experience, and shows how to block it using a 
classical Unix technique.
.AE

.SH
Introduction
.PP
On Unix systems where there are more than a few developers, an interesting
problem often occurs. To install a command in the user library,
one has to be root.  To be root gives one the power to do
serious damage accidentally, and is therefore inadvisable.
To have to interrupt the systems administrator to have him (her)
installing and deinstalling programs can make one unpopular.
.PP
Therefore numerous sites do not prohibit writing to /usr/bin or
some other site-specific bin directory.
As you might expect, this is
.B
not
a good idea.
.PP
Conversely, numerous sites do not allow developers to add to any
sharable library at all, thus making the developers little islands
of private (indeed, peculiar) tools.
.PP
Others officially refuse to allow access to shared libraries, but
unofficially allow the root password to become widely known.

.SH
Consequences
.PP
This last technique, just pretending to prohibit access, 
is by far the most common, since the administrator can
claim to be secure and the developers can still share the fruits
of their efforts.  
.PP
It is also a recipe for disaster.
.PP
Try to imagine the expression on a manager's face when he discovers
that the reorganization he just did resulted
in the loss of about six months of his boss's work...  
Imagine the expression when he can't figure out how to do a restore
without asking operations.
Finally, imagine what it make him look like to the boss who was
promised that his data was secure from his subordinates.
.PP
I got to see this happen at more than one site in more than one company.
It is humorous only to the onlooker.  It is a career-eater to the
participants.

.PP
Refusing to allow any access to the bin directories results in a maze
of private, peculiar environments, one per programmer.  If the site
is consistent in refusing access to other shared directories 
(xxx/include, xxx/lib), then when programmers try to cooperate on
a project, they find themselves artificially prevented from sharing
source and libraries.  
.PP
This tends to make a development team into a collection of independent
individuals, if not a collection of prima donnas.

.PP
Leaving write permission on /usr/bin or the like, while guarding
the superuser password, leaves a different security hole open, the
traditional "trapdoor" hole.
.PP
A person with access to a bin directory writes a utility program which,
while doing its normal task, checks to see if it is being executed by root,
and if so creates a setuid-root shell for the author.
From that day onward, the author of the trapdoor program is root, even
if the root password is changed and kept secret.

.SH
Other Misfeatures
.PP
There are some other problems, on first glance unrelated, that
arise out of the same deficiency.
.PP
Firstly, one cannot place "gates" to sharable information in
a public place unless the place is writable by all.  This makes
it difficult to allow access to a file which you have saved
away down an unsearchable filetree, unless you put links to it
in someone's home directory.  The person looking for the information
can find it, but has to remember where to look.

.PP
Also, one cannot easily have an accessible but private place for transfer
programs to put things: instead you get the UUCP public directory, 
and the security (and administration!) problem that creates.

.SH
Attempted Cures
.PP
The most common attempt to cure the above lies in creating "project"
or "public" accounts, into which project-specific and public information
is placed.
.PP
This really doesn't change a thing.  The bin directories of
the public account can be trapdoored by anyone, the data in the account
can be changed or lost by a careless mistake, and access to anyone with
the password is possible, even to private or confidential information.
.PP
In other words, the password to superuser is unnecessary to a cracker.
The password to the public account becomes sufficient for building
trapdoors.
.PP
There are other kludges to allow controlled sharing, but all are as bad or
worse than public accounts.  That does not mean that there is no real solution,
merely that it is not obvious.

.SH
Multics
.PP
Once upon a time, an operating system was written by Project MAC of M.I.T,
Bell Laboratories and the General Electric Company.  This was Multics, the
direct predecessor of Unix.
.PP
On Multics, security was a design consideration.  Sometimes too much of
a design consideration according to the Hackers of the MIT AI Lab.
Nevertheless, it was written with controled sharing in mind.
.PP
One of the design features which security considerations affected was the
permissions applied to directories.  In Multics, one didn't have read,
write and execute permissions on directories, one had search, modify
and append.
.PP
This made the directories different from ordinary files and therefore
required more special-case code (to the detriment of elegance), but it
was a useful distinction.
.PP
Search was permission to read a directory, modify permission to write
it.  Append was permission to add a file to the directory. Therefore
if on had "sa", one could see what was in the directory and add files
or links, but one could not modify what was already there.

.PP
This made it easy to share things: one said "SetAccess sa *.MyProject"
to allow anyone on your project (group) to search or append to a directory.
.PP
Programs used it to allow anyone to add a "letter" to a "forum" (the Multics
version of news) without giving them the ability to modify or remove
other person's letters.
.PP
Persons working on new commands would be granted append permission to the
"EXperimentalLibrary" to put programs out to beta-test (EXL would amount to
something like /usr/local/experimental, or perhaps /usr/local/bin).
.PP
Persons alpha-testing programs would grant append permissions to their
personal libraries ($HOME/bin) to let others add the new commands at
their leisure.

.SH
Unix
.PP
Unix does not have "sma" permissions on directories, and does not
need them.  Instead Unix has a mechanism to allow a program to 
grant permissions temporarily to its users, the "setuid" bit.

.PP
Setuid allows controlled access to things which belong to individuals 
to others.  Since a program may make quite complex decisions, it actually
allows a finer degree of control than the Multics access control lists.
.PP
Setgid (set group id) allows similar access to things which belong to a central
utility to selected groups of individuals.  For purely historical reasons 
it is not often used.

.PP
With these capabilities, it is possible on Unix to write a program which
allows the kind of controlled sharing for which Multics was designed.

.SH
Append Program
.PP
The program in question is called "append", and allows either anyone in
the same group, or optionally anyone at all
to append files to the current directory (only).
.PP
It does this very simply: It checks that the file to be added is not a
directory or special file, and links it into the current directory.
It is able to do so because it is setuid (or setgid) to the owner or
group of the directory.
.PP
Voila! Multics access control for Unix.

.SH
Improvements
.PP
Because the directory is not writable by others, append could optionally
check for a ".acl" file and allow appending only by people listed in it.
.PP
This would simulate the full set of access control directives provided
on Unix's grampa.

.SH
Caveats
.PP
One should set append on /usr/local/bin, not on any directory in the
command search-path of super-user, or you will have created one of
the security problems append is supposed to prevent.

.nf
				-- Dave (Unix hack on a 'bun) Brown
				   yetti!lethe!dave 
				   (formerly DRBrown@HI-Multics.ARPA)


