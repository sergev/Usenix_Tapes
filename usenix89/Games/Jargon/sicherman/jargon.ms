.\"	tbl | ditroff -ms
.TL
GLOSSARY OF JARGON
.ND November 14, 1982
.AU
Compiled by Guy L. Steele Jr., Raphael Finkel, Donald Woods,
Geoff Goodfellow and Mark Crispin,
with assistance from the MIT and Stanford AI communities
and Worcester Polytechnic Institute.
Some contributions were submitted via the ARPAnet
from miscellaneous sites.
.AE
.if n \{\
.ds -> \->
.ds em --
.ds 2m ---
.ds 0 '
.ds fm '
.ds mu u\}
.if t \{\
.ds -> \(->
.ds em \(em
.ds 2m \l'2m\(em'
.if \w'\(2m' .ds 2m \(2m
.ds 0 `
.ds fm \(fm
.ds mu \(mu\}
.de QU
\\$3\\*Q\\$1\\*U\\$2
..
.de IQ
\\$3\\*0\\$1'\\$2
..
.de DF
.	in 0
.	sp 0.7
.	ft B
\\$1
.	ft R
'	in 4n
..
.LP
.DF "Verb doubling:"
a standard construction is to double a verb and use it
as a comment on what the implied subject does.
Often used to terminate a conversation.
Typical examples involve WIN, LOSE, HACK, FLAME, BARF, CHOMP:
.DS
\*QThe disk heads just crashed.\*U \*QLose, lose.\*U
\*QMostly he just talked about his \*(2m crock.  Flame, flame.\*U
\*QBoy, what a bagbiter!  Chomp, chomp!\*U
.DE
.DF "Soundalike slang:"
similar to Cockney rhyming slang.
Often made up on the spur of the moment.
Standard examples:
.DS
Boston Globe \*(-> Boston Glob
Herald American \*(-> Horrid (Harried) American
New York Times \*(-> New York Slime
historical reasons \*(-> hysterical raisins
government property \- do not duplicate (seen on keys)
	\*(-> government duplicity \- do not propagate
.DE
Often the substitution will be made in such a way as to slip in
a standard jargon word:
.DS
Dr. Dobb's Journal \*(-> Dr. Frob's Journal
creeping featurism \*(-> feeping creaturism
Margaret Jacks Hall \*(-> Marginal Hacks Hall
.DE
.DF "The -P convention:"
turning a word into a question by appending the syllable
.QU P ;
from the LISP convention of appending the letter
.QU P
to denote a predicate (a Boolean-valued function).
The question should expect a yes/no answer, though it needn't.
(See T and NIL.)
.DS
At dinnertime: \*QFoodp?\*U \*QYeah, I'm pretty hungry.\*U or \*QT!\*U
\*QState-of-the-world-P?\*U (Straight) \*QI'm about to go home.\*U
	      (Humorous) \*QYes, the world has a state.\*U
.DE
[One of the best of these is a Gosperism (i.e., due to Bill Gosper).
When we were at a Chinese restaurant, he wanted to know
whether someone would like to share with him a two-person-sized
bowl of soup.
His inquiry was:
.QU "Split-p soup?"
\*-GLS]
.DF "Peculiar nouns:"
MIT AI hackers love to take various words and add the
wrong endings to them to make nouns and verbs, often by extending a
standard rule to nonuniform cases.
Examples:
.DS
	porous => porosity
	generous => generosity
Ergo:	mysterious => mysteriosity
	ferrous => ferocity
.DE
Other examples: winnitude, disgustitude, hackification.

.DF "Spoken inarticulations:"
Words such as
.QU mumble,
.QU sigh,
and
.QU groan
are spoken in places where their referent might more naturally be used.
It has been suggested that this usage derives from the
impossibility of representing such noises in a com link.
Another expression sometimes heard is
.QU complain!
.DF @BEGIN
(primarily CMU) with @END, used humorously in writing to
indicate a context or to remark on the surrounded text.
From the SCRIBE command of the same name.
For example:
.DS
@Begin(Flame)
Predicate logic is the only good programming language.
Anyone who would use anything else is an idiot.  Also,
computers should be tredecimal instead of binary.
@End(Flame)
.DE
.DF "ANGLE BRACKETS"
(primarily MIT) n. Either of the characters
.QU <
and
.QU > .
See BROKET.
.DF AOS
(aus (East coast) ay-ahs (West coast)) [based on a PDP-10
increment instruction] v. To increase the amount of something.
.QU "Aos the campfire."
Usage: considered silly.
See SOS.
.DF ARG
n. Abbreviation for
.QU argument
(to a function), used so often as to have become a new word.
.DF AUTOMAGICALLY
adv. Automatically, but in a way which, for some reason
(typically because it is too complicated, or too ugly, or perhaps
even too trivial), I don't feel like explaining to you.
See MAGIC.
Example: Some programs which produce XGP output files spool them
automagically.
.DF BAGBITER
1. n. Equipment or program that fails, usually
intermittently.  2. BAGBITING: adj. Failing hardware or software.
.QU "This bagbiting system won't let me get out of spacewar."
Usage: verges on obscenity.
Grammatically separable; one may speak of
.QU "biting the bag."
Synonyms: LOSER, LOSING, CRETINOUS, BLETCHEROUS,
BARFUCIOUS, CHOMPER, CHOMPING.
.DF BANG
n. Common alternate name for EXCL (q.v.), especially at CMU.
See SHRIEK.
.DF BAR
1. The second metasyntactic variable, after FOO.
\*QSuppose we have two functions FOO and BAR.
FOO calls BAR...\*U
2. Often appended to FOO to produce FOOBAR.
.DF BARF
[from the
.QU layman
slang, meaning
.QU vomit ]
1. interj. Term of
disgust.  See BLETCH.  2. v. Choke, as on input.  May mean to give
an error message.
\*QThe function
.IQ =
compares two fixnums or two
flonums, and barfs on anything else.\*U
3. BARFULOUS, BARFUCIOUS:
adj. Said of something which would make anyone barf, if only for
aesthetic reasons.
.DF "BELLS AND WHISTLES"
n. Unnecessary but useful (or amusing) features of
a program.
\*QNow that we've got the basic program working, let's go
back and add some bells and whistles.\*U
Nobody seems to know what
distinguishes a bell from a whistle.
.DF BIGNUMS
[from Macsyma] n. 1. In backgammon, large numbers on the dice.
2. Multiple-precision (sometimes infinitely extendable) integers
and, through analogy, any very large numbers.
3. EL CAMINO BIGNUM:
El Camino Real, a street through the San Francisco peninsula that
originally extended (and still appears in places) all the way to
Mexico City.
It was termed
.QU "El Camino Double Precision"
when
someone noted it was a very long street, and then
.QU "El Camino Bignum"
when it was pointed out that it was hundreds of miles long.
.DF BIN
[short for BINARY; used as a second file name on ITS] 1. n.
BINARY.  2. BIN FILE: A file containing the BIN for a program.
Usage: used at MIT, which runs on ITS.
The equivalent term at
Stanford is DMP (pronounced
.QU dump )
FILE.
Other names used include
SAV
.QU save ) (
FILE (DEC and Tenex), SHR
.QU share ) (
and LOW FILES (DEC), and EXE
.QU ex\*(fmee ) (
FILE (DEC and Twenex).
Also in this
category are the input files to the various flavors of linking
loaders (LOADER, LINK-10, STINK), called REL FILES.
.DF BINARY
n. The object code for a program.
.DF BIT
n. 1. The unit of information; the amount of information obtained
by asking a yes-or-no question.
.QU Bits
is often used simply to mean information, as in
.QU "Give me bits about DPL replicators."
2. [By extension from
.QU "interrupt bits"
on a computer] A reminder that
something should be done or talked about eventually.
Upon seeing
someone that you haven't talked to for a while, it's common for one
or both to say,
.QU "I have a bit set for you."
.DF BITBLT
(bit\*(fmblit) 1. v. To perform a complex operation on a large
block of bits, usually involving the bits being displayed on a
bitmapped raster screen.
See BLT.
2. n. The operation itself.
.DF "BIT BUCKET"
n. 1. A receptacle used to hold the runoff from the
computer's shift registers.
2. Mythical destination of deleted
files, GC'ed memory, and other no-longer-accessible data.
3. The physical device associated with
.QU NUL: \&.
.DF BLETCH
[from German
.QU brechen,
to vomit] 1. interj. Term of
disgust.
2. BLETCHEROUS: adj. Disgusting in design or function.
.QU "This keyboard is bletcherous!"
Usage: slightly comic.
.DF BLT
(blit, very rarely belt) [based on the PDP-10 block transfer
instruction; confusing to users of the PDP-11] 1. v. To transfer a
large contiguous package of information from one place to another.
2. THE BIG BLT: n. Shuffling operation on the PDP-10 under some
operating systems that consumes a significant amount of computer
time.  3. (usually pronounced B-L-T) n. Sandwich containing bacon,
lettuce, and tomato.
.DF BOGOSITY
n. The degree to which something is BOGUS (q.v.).
At CMU,
bogosity is measured with a bogometer; typical use: in a seminar,
when a speaker says something bogus, a listener might raise his
hand and say,
.QU "My bogometer just triggered."
The agreed-upon unit of bogosity is the microLenat (\*(muL).
.DF BOGUS
(WPI, Yale, Stanford) adj. 1. Non-functional.
.QU "Your patches are bogus."
2. Useless.
.QU "OPCON is a bogus program."
3. False.
.QU "Your arguments are bogus."
4. Incorrect.
.QU "That algorithm is bogus."
5. Silly.
.QU "Stop writing those bogus sagas."
(This word seems to
have some, but not all, of the connotations of RANDOM.)
[Etymological note from Lehman/Reid at CMU:
.QU Bogus
was originally
used (in this sense) at Princeton, in the late 60's.
It was used
not particularly in the CS department, but all over campus.
It came to Yale, where one of us (Lehman) was an undergraduate, and
(we assume) elsewhere through the efforts of Princeton alumni who
brought the word with them from their alma mater.
In the Yale
case, the alumnus is Michael Shamos, who was a graduate student at
Yale and is now a faculty member here.
A glossary of bogus words
was compiled at Yale when the word was first popularized (e.g.,
autobogophobia: the fear of becoming bogotified).]
.DF BOUNCE
(Stanford) v. To play volleyball.
\*QBounce, bounce!
Stop wasting time on the computer and get out to the court!\*U
.DF BRAIN-DAMAGED
[generalization of
.QU "Honeywell Brain Damage"
(HBD), a
theoretical disease invented to explain certain utter cretinisms in
Multics] adj. Obviously wrong; cretinous; demented.
There is an
implication that the person responsible must have suffered brain
damage, because he should have known better.
Calling something
brain-damaged is really bad; it also implies it is unusable.
.DF BREAK
v. 1. To cause to be broken (in any sense).
.QU "Your latest patch to the system broke the TELNET server."
2. (of a program) To stop
temporarily, so that it may be examined for debugging purposes.
The place where it stops is a BREAKPOINT.
.DF BROKEN
adj. 1. Not working properly (of programs).  2. Behaving
strangely; especially (of people), exhibiting extreme depression.
.DF BROKET
[by analogy with
.QU bracket:
a
.QU "broken bracket" ]
(primarily
Stanford) n. Either of the characters
.QU <
and
.QU > .
(At MIT, and
apparently in The Real World (q.v.) as well, these are usually
called ANGLE BRACKETS.)
.DF "BUCKY BITS"
(primarily Stanford) n. The bits produced by the CTRL and
META shift keys on a Stanford (or Knight) keyboard.
Rumor has it
that the idea for extra bits for characters came from Niklaus
Wirth, and that his nickname was
.QU Bucky.
.DF "DOUBLE BUCKY"
adj. Using both the CTRL and META keys.
.QU "The command to burn all LEDs is double bucky F."
.DF BUG
[from telephone terminology,
.QU "bugs in a telephone cable,"
blamed for noisy lines; however, Jean Sammet has repeatedly been heard to
claim that the use of the term in CS comes from a story concerning
actual bugs found wedged in an early malfunctioning computer] n. An
unwanted and unintended property of a program.  (People can have 
bugs too (even winners) as in
.QU "PHW is a super winner, but he has some bugs." )
See FEATURE.
.DF BUM
1. v. To make highly efficient, either in time or space, often at
the expense of clarity.  The object of the verb is usually what was
removed
.QU "I managed to bum three more instructions." ) (
but can be
the program being changed
.QU "I bummed the inner loop down to seven microseconds." ) (
2. n. A small change to an algorithm to make it more efficient.
.DF BUZZ
v. To run in a very tight loop, perhaps without guarantee of getting out.
.DF CANONICAL
adj. The usual or standard state or manner of something.
A true story:  One Bob Sjoberg, new at the MIT AI Lab, expressed
some annoyance at the use of jargon.
Over his loud objections, we
made a point of using jargon as much as possible in his presence,
and eventually it began to sink in.
Finally, in one conversation, he used the word
.QU canonical
in jargon-like fashion without thinking.
.DS
Steele: \*QAha!  We've finally got you talking jargon too!\*U
Stallman: \*QWhat did he say?\*U
Steele: \*QHe just used \*0canonical' in the canonical way.\*U
.DE
.DF CATATONIA
(kat-uh-toe\*(fmnee-uh) n. A condition of suspended animation in
which the system is in a wedged (CATATONIC) state.
.DF CDR
(ku\*(fmder) [from LISP] v. With
.QU down,
to trace down a list of elements.
.QU "Shall we cdr down the agenda?"
Usage: silly.
.DF "CHINE NUAL"
n. The Lisp Machine Manual, so called because the title is
wrapped around the cover so only those letters show.
.DF CHOMP
v. To lose; to chew on something of which more was bitten off
than one can.
Probably related to gnashing of teeth.  See
BAGBITER.
A hand gesture commonly accompanies this, consisting of
the four fingers held together as if in a mitten or hand puppet,
and the fingers and thumb open and close rapidly to illustrate a
biting action.  The gesture alone means CHOMP CHOMP (see Verb
Doubling).
.DF CLOSE
n. Abbreviation for
.QU "close (or right) parenthesis,"
used when necessary to eliminate oral ambiguity.  See OPEN.
.DF COKEBOTTLE
n. Any very unusual character.
MIT people complain about the
.QU control-meta-cokebottle
commands at SAIL, and SAIL people
complain about the
.QU altmode-altmode-cokebottle
commands at MIT.
.DF "COM MODE"
(variant: COMM MODE) [from the ITS feature for linking two or
more terminals together so that text typed on any is echoed on all,
providing a means of conversation among hackers] n. The state a
terminal is in when linked to another in this way.  Com mode has a
special set of jargon words, used to save typing, which are not
used orally:
.TS
center;
l lw(3.5i).
BCNU	Be seeing you.
BTW	By the way ...
BYE?	T{
Are you ready to unlink?  (This is the standard way to
end a com mode conversation; the other person types
BYE to confirm, or else continues the conversation.)
T}
CUL	See you later.
FOO?	T{
A greeting, also meaning R U THERE?  Often used in the
case of unexpected links, meaning also
.QU "Sorry if I butted in"
(linker) or
.QU "What's up?"
(linkee).
T}
FYI	For your information...
GA	T{
Go ahead (used when two people have tried to type
simultaneously; this cedes the right to type to the other).
T}
HELLOP	T{
A greeting, also meaning R U THERE?  (An instance of the
.QU -P
convention.)
T}
MtFBWY	May the Force be with you.  (From Star Wars.)
NIL	No (see the main entry for NIL).
OBTW	Oh, by the way ...
R U THERE?	Are you there?
SEC	Wait a second (sometimes written SEC...).
T	Yes (see the main entry for T).
TNX	Thanks.
TNX 1.0E6	Thanks a million (humorous).
<double CRLF>	T{
When the typing party has finished, he types
two CRLF's to signal that he is done; this leaves a
blank line between individual
.QU speeches
in the conversation, making it easier to re-read the preceding text.
T}
<name>:	T{
When three or more terminals are linked, each speech
is preceded by the typist's login name and a colon (or
a hyphen) to indicate who is typing.
The login name often is shortened to a unique prefix (possibly a
single letter) during a very long conversation.
T}
/\e/\e/\e	The equivalent of a giggle.
.TE
At Stanford, where the link feature is implemented by
.QU "talk loops,"
the term TALK MODE is used in place of COM MODE.  Most of the above
.QU sub-jargon
is used at both Stanford and MIT.
.DF "CONNECTOR CONSPIRACY"
[probably came into prominence with the
appearance of the KL-10, none of whose connectors match anything
else] n. The tendency of manufacturers (or, by extension,
programmers or purveyors of anything) to come up with new products
which don't fit together with the old stuff, thereby making you buy
either all new stuff or expensive interface devices.
.DF CONS
[from LISP] 1. v. To add a new element to a list.  2. CONS UP:
v. To synthesize from smaller pieces:
.QU "to cons up an example."
.DF CRASH
1. n. A sudden, usually drastic failure.  Most often said of the
system (q.v., definition #1), sometimes of magnetic disk drives.
.QU "Three lusers lost their files in last night's disk crash."
A disk crash which entails the read/write heads dropping onto the surface
of the disks and scraping off the oxide may also be referred to as a
.QU "head crash."
2. v. To fail suddenly.
.QU "Has the system just crashed?"
Also used transitively to indicate the cause of the
crash (usually a person or a program, or both).
.QU "Those idiots playing spacewar crashed the system."
Sometimes said of people.
See GRONK OUT.
.DF CRETIN
1. n. Congenital loser (q.v.).  2. CRETINOUS: adj. See
BLETCHEROUS and BAGBITING.  Usage: somewhat ad-hominem.
.DF CRLF
(cur\*(fmlif, sometimes crul\*(fmlif) n. A carriage return (CR) followed
by a line feed (LF).  See TERPRI.
.DF CROCK
[probably from
.QU layman
slang, which in turn may be derived from
.QU "crock of shit" ]
n. An awkward feature or programming technique
that ought to be made cleaner.  Example: Using small integers to
represent error codes without the program interpreting them to the
user is a crock.
Also, a technique that works acceptably but which
is quite prone to failure if disturbed in the least, for example
depending on the machine opcodes having particular bit patterns so
that you can use instructions as data words too; a tightly woven,
almost completely unmodifiable structure.
.DF CRUFTY
[from
.QU cruddy ]
adj. 1. Poorly built, possibly overly complex.
.QU "This is standard old crufty DEC software."
Hence CRUFT, n. shoddy construction.
Also CRUFT, v. [from hand cruft, pun on hand craft]
to write assembler code for something normally (and better) done by
a compiler.  2. Unpleasant, especially to the touch, often with
encrusted junk.  Like spilled coffee smeared with peanut butter and
catsup.  Hence CRUFT, n. disgusting mess.  3. Generally unpleasant.
CRUFTY or CRUFTIE n. A small crufty object (see FROB); often one
which doesn't fit well into the scheme of things.
.QU "A LISP property list is a good place to store crufties (or, random cruft)."
[Note:  Does CRUFT have anything to do with the Cruft Lab at Harvard?
I don't know, though I was a Harvard student. \*(emGLS]
.DF CRUNCH
v. 1. To process, usually in a time-consuming or complicated way.
Connotes an essentially trivial operation which is
nonetheless painful to perform.  The pain may be due to the
triviality being imbedded in a loop from 1 to 1000000000.
.QU "FORTRAN programs do mostly number crunching."
2. To reduce the size of a
file by a complicated scheme that produces bit configurations
completely unrelated to the original data, such as by a Huffman
code.  (The file ends up looking like a paper document would if
somebody crunched the paper into a wad.)  Since such compression
usually takes more computations than simpler methods such as
counting repeated characters (such as spaces) the term is doubly
appropriate.  (This meaning is usually used in the construction
.QU "file crunch(ing)"
to distinguish it from
.QU "number crunch(ing)." )
3. n. The character
.QU # .
Usage: used at Xerox and CMU, among other places.
Other names for
.QU #
include SHARP, NUMBER, HASH, PIG-PEN,
POUND-SIGN, and MESH.
GLS adds: I recall reading somewhere that
most of these are names for the # symbol IN CONTEXT.  The name for
the sign itself is
.QU octothorp.
.DF CTY
(city) n. The terminal physically associated with a computer's
operating console.
.DF CUSPY
[from the DEC acronym CUSP, for Commonly Used System Program,
i.e., a utility program used by many people] (WPI) adj. 1. (of a
program) Well-written.  2. Functionally excellent.  A program which
performs well and interfaces well to users is cuspy.  See RUDE.
.DF DAEMON
(day\*(fmmun, dee\*(fmmun) [archaic form of
.QU demon,
which has slightly
different connotations (q.v.)] n. A program which is not invoked
explicitly, but which lies dormant waiting for some condition(s) to occur.
The idea is that the perpetrator of the condition need not
be aware that a daemon is lurking (though often a program will
commit an action only because it knows that it will implicitly
invoke a daemon).
For example, writing a file on the lpt spooler's
directory will invoke the spooling daemon, which prints the file.
The advantage is that programs which want (in this example) files
printed need not compete for access to the lpt.  They simply enter
their implicit requests and let the daemon decide what to do with them.
Daemons are usually spawned automatically by the system, and
may either live forever or be regenerated at intervals.
Usage: DAEMON and DEMON (q.v.) are often used interchangeably, but seem to
have distinct connotations.  DAEMON was introduced to computing by
CTSS people (who pronounced it dee\*(fmmon) and used it to refer to
what is now called a DRAGON or PHANTOM (q.v.).  The meaning and
pronunciation have drifted, and we think this glossary reflects
current usage.
.DF "DAY MODE"
See PHASE (of people).
.DF DEADLOCK
n. A situation wherein two or more processes are unable to
proceed because each is waiting for another to do something.
A common example is a program communicating to a PTY or STY, which
may find itself waiting for output from the PTY/STY before sending
anything more to it, while the PTY/STY is similarly waiting for
more input from the controlling program before outputting anything.
(This particular flavor of deadlock is called
.QU starvation.
Another common flavor is
.QU constipation,
where each process is
trying to send stuff to the other, but all buffers are full because
nobody is reading anything.)  See DEADLY EMBRACE.
.DF "DEADLY EMBRACE"
n. Same as DEADLOCK (q.v.), though usually used only
when exactly two processes are involved.
DEADLY EMBRACE is the
more popular term in Europe; DEADLOCK in the United States.
.DF DEMENTED
adj. Yet another term of disgust used to describe a program.
The connotation in this case is that the program works as designed,
but the design is bad.  For example, a program that generates large
numbers of meaningless error messages implying it is on the point
of imminent collapse.
.DF DEMON
(dee\*(fmmun) n. A portion of a program that is not invoked
explicitly, but which lies dormant waiting for some condition(s) to occur.
See DAEMON.
The distinction is that demons are usually
processes within a program, while daemons are usually programs
running on an operating system.
Demons are particularly common in AI programs.
For example, a knowledge manipulation program might
implement inference rules as demons.
Whenever a new piece of
knowledge was added, various demons would activate (which demons
depends on the particular piece of data) and would create
additional pieces of knowledge by applying their respective
inference rules to the original piece.  These new pieces could in
turn activate more demons as the inferences filtered down through
chains of logic.
Meanwhile the main program could continue with whatever its primary task was.
.DF DIABLO
(dee-ah\*(fmblow) [from the Diablo printer] 1. n. Any letter-quality
printing device.  2. v. To produce letter-quality output from such a device.
.DF DIDDLE
v. To work with in a not particularly serious manner.
.QU "I diddled with a copy of ADVENT so it didn't double-space all the time."
.QU "Let's diddle this piece of code and see if the problem goes away."
See TWEAK and TWIDDLE.
.DF DIKE
[from
.QU "diagonal cutters" ]
v. To remove a module or disable it.
.QU "When in doubt, dike it out."
.DF DMP
(dump) See BIN.
.DF "DO PROTOCOL"
[from network protocol programming] v. To perform an
interaction with somebody or something that follows a clearly
defined procedure.  For example,
.QU "Let's do protocol with the check"
at a restaurant means to ask the waitress for the check, calculate
the tip and everybody's share, generate change as necessary, and pay the bill.
.DF DOWN
1. adj. Not working.
.QU "The up escalator is down."
2. TAKE DOWN, BRING DOWN: v. To deactivate, usually for repair work.
See UP.
.DF DPB
(duh-pib\*(fm) [from the PDP-10 instruction set] v. To plop something
down in the middle.
.DF DRAGON
n. (MIT) A program similar to a
.QU daemon
(q.v.), except that it
is not invoked at all, but is instead used by the system to perform
various secondary tasks.
A typical example would be an accounting
program, which keeps track of who is logged in, accumulates load-average
statistics, etc.
At MIT, all free TV's display a list of
people logged in, where they are, what they're running, etc. along
with some random picture (such as a unicorn, Snoopy, or the
Enterprise) which is generated by the
.QU "NAME DRAGON."
See PHANTOM.
.DF DWIM
[Do What I Mean] 1. adj. Able to guess, sometimes even correctly,
what result was intended when provided with bogus input.
Often suggested in jest as a desired feature for a complex program.
A related term, more often seen as a verb, is DTRT (Do The Right Thing).
2. n. The INTERLISP function that attempts to accomplish
this feat by correcting many of the more common errors.
See HAIRY.
.DF ENGLISH
n. The source code for a program, which may be in any
language, as opposed to BINARY.
Usage: slightly obsolete, used
mostly by old-time hackers, though recognizable in context.
At MIT, directory SYSENG is where the
.QU English
for system programs is kept, and SYSBIN, the binaries.
SAIL has many such directories, but the canonical one is [CSP,SYS].
.DF EPSILON
[from standard mathematical notation for a small quantity] 1.
n. A small quantity of anything.
.QU "The cost is epsilon."
2. adj.  Very small, negligible; less than marginal (q.v.).
.QU "We can get this feature for epsilon cost."
3. WITHIN EPSILON OF: Close enough
to be indistinguishable for all practical purposes.
.DF EXCH
(ex\*(fmchuh, ekstch) [from the PDP-10 instruction set] v. To
exchange two things, each for the other.
.DF EXCL
(eks\*(fmcul) n. Abbreviation for
.QU "exclamation point."
See BANG, SHRIEK, WOW.
.DF EXE
(ex\*(fmee)  See BIN.
.DF FAULTY
adj. Same denotation as
.QU bagbiting,
.QU bletcherous,
.QU losing,
q.v., but the connotation is much milder.
.DF FEATURE
n. 1. A surprising property of a program.  Occasionally documented.
To call a property a feature sometimes means the author of
the program did not consider the particular case, and the program
makes an unexpected, although not strictly speaking an incorrect
response.  See BUG.
.QU "That's not a bug, that's a feature!"
A bug can be changed to a feature by documenting it.  2. A well-known and
beloved property; a facility.  Sometimes features are planned, but
are called crocks by others.
An approximately correct spectrum
(these terms are all used to describe programs or portions thereof,
except for the first two, which are included for completeness):
.sp 0.5
.in +4n
CRASH  STOPPAGE  BUG  SCREW  LOSS  MISFEATURE
CROCK  KLUGE  HACK  WIN  FEATURE  PERFECTION
.in -4n
.sp 0.5
(The last is never actually attained.)
.DF FEEP
1. n. The soft bell of a display terminal (except for a VT-52!);
a beep.  2. v. To cause the display to make a feep sound.  TTY's do
not have feeps.
Alternate forms: BEEP, BLEEP, or just about
anything suitably onomatopoeic.
The term BREEDLE is sometimes
heard at SAIL, where the terminal bleepers are not particularly
.QU soft
(they sound more like the musical equivalent of sticking out one's tongue).
The
.QU feeper
on a VT-52 has been compared to the sound of a '52 Chevy stripping its gears.
.DF "FENCEPOST ERROR"
n. The discrete equivalent of a boundary condition.
Often exhibited in programs by iterative loops.  From the following
problem:
\*QIf you build a fence 100 feet long with posts ten feet
apart, how many posts do you need?\*U
(Either 9 or 11 is a better answer than the obvious 10.)
.DF FINE
(WPI) adj. Good, but not good enough to be CUSPY.  [The word FINE
is used elsewhere, of course, but without the implicit comparison
to the higher level implied by CUSPY.]
.DF "FLAG DAY"
[from a bit of Multics history involving a change in the
ASCII character set originally scheduled for June 14, 1966]
n. A software change which is neither forward nor backward
compatible, and which is costly to make and costly to revert.
.QU "Can we install that without causing a flag day for all users?"
.DF FLAKEY
adj. Subject to frequent lossages.  See LOSSAGE.
.DF FLAME v. To speak incessantly and/or rabidly on some relatively
uninteresting subject or with a patently ridiculous attitude.
FLAME ON: v. To continue to flame.  See RAVE.
This punning reference to Marvel comics' Human Torch has been lost as
recent usage completes the circle:
.QU "Flame on"
now usually means
.QU "beginning of flame."
.DF FLAP
v. To unload a DECtape (so it goes flap, flap, flap...).
Old hackers at MIT tell of the days when the disk was device 0 and
microtapes were 1, 2,... and attempting to flap device 0 would
instead start a motor banging inside a cabinet near the disk!
.DF FLAVOR
n. 1. Variety, type, kind.
.QU "DDT commands come in two flavors."
See VANILLA.  2. The attribute of causing something to be FLAVORFUL.
.QU "This convention yields additional flavor by allowing one to... ."
3. On the LispMachine, an object-oriented programming
system
.QU flavors ); (
each class of object is a flavor.
.DF FLAVORFUL
adj. Aesthetically pleasing.  See RANDOM and LOSING for antonyms.
See also the entry for TASTE.
.DF FLUSH
v. 1. To delete something, usually superfluous.
.QU "All that nonsense has been flushed."
Standard ITS terminology for aborting an output operation.
2. To leave at the end of a day's work (as
opposed to leaving for a meal).
.QU "I'm going to flush now."
.QU "Time to flush."
3. To exclude someone from an activity.
.DF FOO
1. [from Yiddish
.QU feh
or the Anglo-Saxon
.QU fooey! ]
interj. Term of disgust.  2. [from FUBAR (Fucked Up Beyond All Recognition),
from WWII, often seen as FOOBAR] Name used for temporary programs,
or samples of three-letter names.  Other similar words are BAR, BAZ
(Stanford corruption of BAR), and rarely RAG.  These have been used
in Pogo as well.  3. Used very generally as a sample name for
absolutely anything.  The old
.QU "Smokey Stover"
comic strips often
included the word FOO, in particular on license plates of cars.
MOBY FOO: See MOBY.
.DF FRIED
adj. 1. Non-working due to hardware failure; burnt out.  2. Of
people, exhausted.  Said particularly of those who continue to work
in such a state.  Often used as an explanation or excuse.
\*QYeah, I know that fix destroyed the file system, but I was fried
when I put it in.\*U
.DF FROB
1. n. (MIT) The official Tech Model Railroad Club definition is
.QU "FROB = protruding arm or trunnion,"
and by metaphoric extension
any somewhat small thing.  See FROBNITZ.  2. v. Abbreviated form of
FROBNICATE.
.DF FROBNICATE
v. To manipulate or adjust, to tweak.  Derived from
FROBNITZ (q.v.).  Usually abbreviated to FROB.  Thus one has the
saying
.QU "to frob a frob."
See TWEAK and TWIDDLE.  Usage: FROB,
TWIDDLE, and TWEAK sometimes connote points along a continuum.
FROB connotes aimless manipulation; TWIDDLE connotes gross
manipulation, often a coarse search for a proper setting; TWEAK
connotes fine-tuning.  If someone is turning a knob on an
oscilloscope, then if he's carefully adjusting it he is probably
tweaking it; if he is just turning it but looking at the screen he
is probably twiddling it; but if he's just doing it because turning
a knob is fun, he's frobbing it.
.DF FROBNITZ,
pl. FROBNITZEM (frob\*(fmnitsm) n. An unspecified physical
object, a widget.  Also refers to electronic black boxes.
This rare form is usually abbreviated to FROTZ, or more commonly to FROB.
Also used are FROBNULE, FROBULE, and FROBNODULE.  Starting
perhaps in 1979, FROBBOZ (fruh-bahz\*(fm), pl. FROBBOTZIM, has also
become very popular, largely due to its exposure via the Adventure
spin-off called Zork (Dungeon).
These can also be applied to
non-physical objects, such as data structures.
.DF FROG
(variant: PHROG) 1. interj. Term of disgust (we seem to have a
lot of them).  2. Used as a name for just about anything.  See FOO.
3. n. Of things, a crock.  Of people, somewhere inbetween a turkey
and a toad.  4. Jake Brown (FRG@SAIL).  5. FROGGY: adj. Similar to
BAGBITING (q.v.), but milder.
.QU "This froggy program is taking forever to run!"
.DF FROTZ
1. n. See FROBNITZ.  2. MUMBLE FROTZ: An interjection of very mild disgust.
.DF FRY
v. 1. To fail.  Said especially of smoke-producing hardware
failures.  2. More generally, to become non-working.  Usage: never
said of software, only of hardware and humans.  See FRIED.
.DF FTP
(spelled out, NOT pronounced
.QU fittip )
1. n. The File Transfer
Protocol for transmitting files between systems on the ARPAnet.
2.  v. To transfer a file using the File Transfer Program.
.QU "Lemme get this copy of Wuthering Heights FTP'd from SAIL."
.DF FUDGE
1. v. To perform in an incomplete but marginally acceptable way,
particularly with respect to the writing of a program.
.QU "I didn't feel like going through that pain and suffering, so I fudged it."
2. n. The resulting code.
.DF "FUDGE FACTOR"
n. A value or parameter that is varied in an ad hoc way
to produce the desired result.  The terms
.QU tolerance
and
.QU slop
are also used, though these usually indicate a one-sided leeway,
such as a buffer which is made larger than necessary because one
isn't sure exactly how large it needs to be, and it is better to
waste a little space than to lose completely for not having enough.
A fudge factor, on the other hand, can often be tweaked in more
than one direction.
An example might be the coefficients of an equation, where the
coefficients are varied in an attempt to make the equation fit certain
criteria.
.DF GABRIEL
[for Dick Gabriel, SAIL volleyball fanatic] n. An unnecessary
(in the opinion of the opponent) stalling tactic, e.g., tying one's
shoelaces or hair repeatedly, asking the time, etc.  Also used to
refer to the perpetrator of such tactics.  Also,
.QU "pulling a Gabriel,"
.QU "Gabriel mode."
.DF "GARBAGE COLLECT"
v., GARBAGE COLLECTION n. See GC.
.DF GARPLY
n. (Stanford) Another meta-word popular among SAIL hackers.
.DF GAS
[as in
.QU "gas chamber" ]
interj. 1. A term of disgust and hatred,
implying that gas should be dispensed in generous quantities,
thereby exterminating the source of irritation.
.QU "Some loser just reloaded the system for no reason!  Gas!"
2. A term suggesting
that someone or something ought to be flushed out of mercy.
.QU "The system's wedging every few minutes.  Gas!"
3. v. FLUSH (q.v.).
.QU "You should gas that old crufty software."
4. GASEOUS adj. Deserving of being gassed.  Usage: primarily used by Geoff
Goodfellow at SRI, but spreading.
.DF GC
[from LISP terminology] 1. v. To clean up and throw away useless things.
.QU "I think I'll GC the top of my desk today."
2. To recycle, reclaim, or put to another use.  3. To forget.  The
implication is often that one has done so deliberately.  4. n. An
instantiation of the GC process.
.DF GEDANKEN
[from Einstein's term
.QU gedanken-experimenten,
such as the standard proof that
.ie n E=mc^2]
.el E=mc\u\s-2\&2\s0\d]
adj. An AI project which is written up
in grand detail without ever being implemented to any great extent.
Usually perpetrated by people who aren't very good hackers or find
programming distasteful or are just in a hurry.
A gedanken thesis
is usually marked by an obvious lack of intuition about what is
programmable and what is not and about what does and does not
constitute a clear specification of a program-related concept such
as an algorithm.
.DF "GLASS TTY"
n. A terminal which has a display screen but which, because
of hardware or software limitations, behaves like a teletype or
other printing terminal.  An example is the ADM-3 (without cursor
control).  A glass tty can't do neat display hacks, and you can't
save the output either.
.DF GLITCH
[from the Yiddish
.QU glitshen,
to slide] 1. n. A sudden
interruption in electric service, sanity, or program function.
Sometimes recoverable.  2. v. To commit a glitch.  See GRITCH.
3. v. (Stanford) To scroll a display screen.
.DF GLORK
1. interj. Term of mild surprise, usually tinged with outrage,
as when one attempts to save the results of two hours of editing
and finds that the system has just crashed.  2. Used as a name for
just about anything.  See FOO.  3. v. Similar to GLITCH (q.v.), but
usually used reflexively.
.QU "My program just glorked itself."
.DF GOBBLE
v. To consume or to obtain.  GOBBLE UP tends to imply
.QU consume,
while GOBBLE DOWN tends to imply
.QU obtain.
.QU "The output spy gobbles characters out of a TTY output buffer."
.QU "I guess I'll gobble down a copy of the documentation tomorrow."
See SNARF.
.DF GORP
(CMU) [perhaps from the generic term for dried hiker's food,
stemming from the acronym
.QU "Good Old Raisins and Peanuts" ]
Another metasyntactic variable, like FOO and BAR.
.DF GRIND
v. 1. (primarily MIT) To format code, especially LISP code, by
indenting lines so that it looks pretty.  Hence, PRETTY PRINT, the
generic term for such operations.  2. To run seemingly
interminably, performing some tedious and inherently useless task.
Similar to CRUNCH.
.DF GRITCH
1. n. A complaint (often caused by a GLITCH (q.v.)).  2. v. To
complain.  Often verb-doubled:
.QU "Gritch gritch."
3. Glitch.
.DF GROK
[from the novel
.I "Stranger in a Strange Land,"
by Robert Heinlein,
where it is a Martian word meaning roughly
.QU "to be one with" ]
v. To understand, usually in a global sense.
.DF GRONK
[popularized by the cartoon strip
.QU B.C.
by Johnny Hart, but the
word apparently predates that] v. 1. To clear the state of a wedged
device and restart it.  More severe than
.QU "to frob"
(q.v.).  2. To break.
.QU "The teletype scanner was gronked, so we took the system down."
3. GRONKED: adj. Of people, the condition of feeling very
tired or sick.  4. GRONK OUT: v. To cease functioning.  Of people,
to go home and go to sleep.
.QU "I guess I'll gronk out now; see you all tomorrow."
.DF GROVEL
v. To work interminably and without apparent progress.  Often
used with
.QU over.
.QU "The compiler grovelled over my code."
Compare GRIND and CRUNCH.  Emphatic form: GROVEL OBSCENELY.
.DF GRUNGY
adj. Incredibly dirty or grubby.  Anything which has been
washed within the last year is not really grungy.  Also used
metaphorically; hence some programs (especially crocks) can be
described as grungy.
.DF GUBBISH
[a portmanteau of
.QU garbage
and
.QU rubbish? ]
n. Garbage; crap; nonsense.
.QU "What is all this gubbish?"
.DF GUN
[from the GUN command on ITS] v. To forcibly terminate a program
or job (computer, not career).
\*QSome idiot left a background
process running soaking up half the cycles, so I gunned it.\*U
.DF HACK
n. 1. Originally a quick job that produces what is needed, but
not well.  2. The result of that job.  3. NEAT HACK: A clever
technique.  Also, a brilliant practical joke, where neatness is
correlated with cleverness, harmlessness, and surprise value.
Example: the Caltech Rose Bowl card display switch circa 1961.
4. REAL HACK: A crock (occasionally affectionate).
v. 5. With
.QU together,
to throw something together so it will work.
6. To bear emotionally or physically.
.QU "I can't hack this heat!"
7.  To work on something (typically a program).  In specific sense:
.QU "What are you doing?"
.QU "I'm hacking TECO."
In general sense:
.QU "What do you do around here?"
.QU "I hack TECO."
(The former is
time-immediate, the latter time-extended.)  More generally,
.QU "I hack x"
is roughly equivalent to
.QU "x is my bag."
.QU "I hack solid-state physics."
8. To pull a prank on.  See definition 3 and HACKER (def
#6).  9. v.i. To waste time (as opposed to TOOL).
.QU "Whatcha up to?"
.QU "Oh, just hacking."
10. HACK UP (ON): To hack, but generally
implies that the result is meanings 1\-2.  11. HACK VALUE: Term used
as the reason or motivation for expending effort toward a seemingly
useless goal, the point being that the accomplished goal is a hack.
For example, MacLISP has code to read and print roman numerals,
which was installed purely for hack value.
HAPPY HACKING: A farewell.  HOW'S HACKING?: A friendly greeting
among hackers.  HACK HACK: A somewhat pointless but friendly
comment, often used as a temporary farewell.
[The word HACK doesn't really have 69 different meanings.  In fact,
HACK has only one meaning, an extremely subtle and profound one 
which defies articulation.  Which connotation a given HACK-token 
has depends in similarly profound ways on the context.  Similar 
comments apply to a couple other hacker jargon items, most notably 
RANDOM. \*(emAgre]
.DF HACKER
[originally, someone who makes furniture with an axe] n. 1. A
person who enjoys learning the details of programming systems and
how to stretch their capabilities, as opposed to most users who
prefer to learn only the minimum necessary.
2. One who programs enthusiastically, or who enjoys programming
rather than just theorizing about programming.
3. A person capable of appreciating hack value (q.v.).
4. A person who is good at programming quickly.
Not everything a hacker produces is a hack.  5. An expert at a
particular program, or one who frequently does work using it or on
it; example:
.QU "A SAIL hacker."
(Definitions 1 to 5 are correlated, and people who fit them congregate.)
6. A malicious or inquisitive
meddler who tries to discover information by poking around.  Hence
.QU "password hacker,"
.QU "network hacker."
.DF HACKISH
adj. Being or involving a hack.  HACKISHNESS n.
.DF HAIR
n. The complications which make something hairy.
.QU "Decoding TECO commands requires a certain amount of hair."
Often seen in the
phrase INFINITE HAIR, which connotes extreme complexity.
.DF HAIRY
adj. 1. Overly complicated.
.QU "DWIM is incredibly hairy."
2. Incomprehensible.
.QU "DWIM is incredibly hairy."
3. Of people, high-powered, authoritative, rare, expert, and/or
incomprehensible.  Hard to explain except in context:
.QU "He knows this hairy lawyer who says there's nothing to worry about."
.DF HAKMEM
n. MIT AI Memo 239 (February 1972).
A collection of neat mathematical and programming hacks contributed by
many people at MIT and elsewhere.
.DF HANDWAVE
1. v. To gloss over a complex point; to distract a listener;
to support a (possibly actually valid) point with blatantly faulty logic.
2. n. The act of handwaving.
.QU "Boy, what a handwave!"
The use of this word is often accompanied by gestures: both hands up,
palms forward, swinging the hands in a vertical plane pivoting at
the elbows and/or shoulders (depending on the magnitude of the
handwave); alternatively, holding the forearms still while rotating
the hands at the wrist to make them flutter.  In context, the
gestures alone can suffice as a remark.
.DF HARDWARILY
adv. In a way pertaining to hardware.
.QU "The system is hardwarily unreliable."
The adjective
.QU hardwary
is NOT used.  See SOFTWARILY.
.DF "HELLO WALL"
See WALL.
.DF HIRSUTE
Occasionally used humorously as a synonym for HAIRY.
.DF HOOK
n. An extraneous piece of software or hardware included in order
to simplify later additions or debug options.  For instance, a
program might execute a location that is normally a JFCL, but by
changing the JFCL to a PUSHJ one can insert a debugging routine at
that point.
.DF "HUMONGOUS, HUMUNGOUS"
See HUNGUS.
.DF HUNGUS
(hung\*(fmghis) [perhaps related to current slang
.QU humongous;
which one came first (if either) is unclear] adj. Large, unwieldy,
usually unmanageable.
.QU "TCP is a hungus piece of code."
.QU "This is a hungus set of modifications."
.DF IMPCOM
See TELNET.
.DF INFINITE
adj. Consisting of a large number of objects; extreme.  Used
very loosely as in:
.QU "This program produces infinite garbage."
.DF IRP
(erp) [from the MIDAS pseudo-op which generates a block of code
repeatedly, substituting in various places the car and/or cdr of
the list(s) supplied at the IRP] v. To perform a series of tasks
repeatedly with a minor substitution each time through.
\*QI guess I'll IRP over these homework papers so I can give them some
random grade for this semester.\*U
.DF JFCL
(djif\*(fmkl or dja-fik\*(fml) [based on the PDP-10 instruction that acts
as a fast no-op] v. To cancel or annul something.
.QU "Why don't you jfcl that out?"
[The license plate on Geoff Goodfellow's BMW is JFCL.]
.DF JIFFY
n. 1. Interval of CPU time, commonly 1/60 second or 1 millisecond.
2. Indeterminate time from a few seconds to forever.
.QU "I'll do it in a jiffy"
means certainly not now and possibly never.
.DF JOCK
n. Programmer who is characterized by large and somewhat brute
force programs.  The term is particularly well-suited for systems programmers.
.DF "J. RANDOM"
See RANDOM.
.DF JRST
(jerst) [based on the PDP-10 jump instruction] v. To suddenly
change subjects.  Usage: rather rare.
.QU "Jack be nimble, Jack be quick; Jack jrst over the candle stick."
.DF JSYS
(jay\*(fmsis), pl. JSI (jay\*(fmsigh) [Jump to SYStem] See UUO.
.DF KLUGE
(kloodj) alt. KLUDGE [from the German
.QU kluge,
clever] n. 1. A Rube Goldberg device in hardware or software.
2. A clever
programming trick intended to solve a particular nasty case in an
efficient, if not clear, manner.  Often used to repair bugs.  Often
verges on being a crock.  3. Something that works for the wrong
reason.  4. v. To insert a kluge into a program.
\*QI've kluged this routine to get around that weird bug, but there's
probably a better way.\*U Also KLUGE UP.  5. KLUGE AROUND: To avoid by
inserting a kluge.  6. (WPI) A feature which is implemented in a RUDE manner.
.DF LDB
(lid\*(fmdib) [from the PDP-10 instruction set] v. To extract from the middle.
.DF LIFE
n. A cellular-automaton game invented by John Horton Conway, and
first introduced publicly by Martin Gardner (Scientific American,
October 1970).
.DF "LINE FEED"
(standard ASCII terminology) 1. v. To feed the paper through
a terminal by one line (in order to print on the next line).  2. n. The
.QU character
that causes the terminal to perform this action.
.DF "LINE STARVE"
(MIT) Inverse of LINE FEED.
.DF LOGICAL
[from the technical term
.QU "logical device,"
wherein a physical
device is referred to by an arbitrary name] adj. Understood to have
a meaning not necessarily corresponding to reality.  E.g., if a
person who has long held a certain post (e.g., Les Earnest at SAIL)
left and was replaced, the replacement would for a while be known
as the
.QU "logical Les Earnest."
The word VIRTUAL is also used.  At SAIL,
.QU logical
compass directions denote a coordinate system in which
.QU "logical north"
is toward San Francisco,
.QU "logical west"
is toward the ocean, etc., even though logical north varies between
physical (true) north near SF and physical west near San Jose.
(The best rule of thumb here is that El Camino Real by definition
always runs logical north-and-south.)
.DF LOSE
[from MIT jargon] v. 1. To fail.  A program loses when it
encounters an exceptional condition.  2. To be exceptionally
unaesthetic.  3. Of people, to be obnoxious or unusually stupid (as
opposed to ignorant).  4. DESERVE TO LOSE: v. Said of someone who
willfully does the wrong thing; humorously, if one uses a feature
known to be marginal.  What is meant is that one deserves the
consequences of one's losing actions.
.QU "Boy, anyone who tries to use MULTICS deserves to lose!"
LOSE LOSE: a reply or comment on a situation.
.DF LOSER
n. An unexpectedly bad situation, program, programmer, or
person.  Especially
.QU "real loser."
.DF LOSS
n. Something which loses.
WHAT A (MOBY) LOSS!: interjection.
.DF LOSSAGE
n. The result of a bug or malfunction.
.DF LPT
(lip\*(fmit) n. Line printer, of course.
.DF LUSER
See USER.
.DF MACROTAPE
n. An industry standard reel of tape, as opposed to a MICROTAPE.
.DF MAGIC
adj. 1. As yet unexplained, or too complicated to explain.
(Arthur C. Clarke once said that magic was as-yet-not-understood science.)
.QU "TTY echoing is controlled by a large number of magic bits."
\*QThis routine magically computes the parity of an eight-bit
byte in three instructions.\*U
2. (Stanford) A feature not generally
publicized which allows something otherwise impossible, or a
feature formerly in that category but now unveiled.  Example: The
keyboard commands which override the screen-hiding features.
.DF MARGINAL
adj. 1. Extremely small.
.QU "A marginal increase in core can decrease GC time drastically."
See EPSILON.  2. Of extremely small merit.
.QU "This proposed new feature seems rather marginal to me."
3. Of extremely small probability of winning.
.QU "The power supply was rather marginal anyway; no wonder it crapped out."
4. MARGINALLY: adv. Slightly.
.QU "The ravs here are only marginally better than at Small Eating Place."
.DF MICROTAPE
n. Occasionally used to mean a DECtape, as opposed to a
MACROTAPE.  This was the official DEC term for the stuff until
someone consed up the word
.QU DECtape.
.DF MISFEATURE
n. A feature which eventually screws someone, possibly
because it is not adequate for a new situation which has evolved.
It is not the same as a bug because fixing it involves a gross
philosophical change to the structure of the system involved.
Often a former feature becomes a misfeature because a tradeoff was
made whose parameters subsequently changed (possibly only in the
judgment of the implementors).
\*QWell, yeah, it's kind of a misfeature that file names are limited to
six characters, but we're stuck with it for now.\*U
.DF MOBY
[seems to have been in use among model railroad fans years ago.
Entered the world of AI with the Fabritek 256K moby memory of MIT-AI.
Derived from Melville's
.I "Moby Dick"
(some say from
.QU "Moby Pickle" ).]
1. adj. Large, immense, or complex.
.QU "A moby frob."
2. n. The maximum address space of a machine, hence 3. n. 256K words,
the size of a PDP-10 moby.  (The maximum address space means the
maximum normally addressable space, as opposed to the amount of
physical memory a machine can have.  Thus the MIT PDP-10s each have
two mobies, usually referred to as the
.QU "low moby"
(0\-777777) and
.QU "high moby"
(1000000\-1777777), or as
.QU "moby 0"
and
.QU "moby 1."
MIT-AI has four mobies of address space: moby 2 is the PDP-6 memory, and
moby 3 the PDP-11 interface.)  In this sense
.QU moby
is often used
as a generic unit of either address space (18. bits' worth) or of
memory (about a megabyte, or 9/8 megabyte (if one accounts for
difference between 32- and 36-bit words), or 5/4 megacharacters).
4. A title of address (never of third-person reference), usually
used to show admiration, respect, and/or friendliness to a
competent hacker.
.QU "So, moby Knight, how's the CONS machine doing?"
5. adj. In backgammon, doubles on the dice, as in
.QU "moby sixes,"
.QU "moby ones,"
etc.
MOBY FOO, MOBY WIN, MOBY LOSS: standard emphatic forms.
FOBY MOO: a spoonerism due to Greenblatt.
.DF MODE
n. A general state, usually used with an adjective describing the state.
.QU "No time to hack; I'm in thesis mode."
Usage: in its jargon sense, MODE is most often said of people, though it is
sometimes applied to programs and inanimate objects.
.QU "If you're on a TTY, E will switch to non-display mode."
In particular, see DAY MODE, NIGHT MODE, and YOYO MODE; also COM MODE,
TALK MODE, and GABRIEL MODE.
.DF MODULO
prep. Except for.  From mathematical terminology: one can
consider saying that 4=22
.QU "except for the 9's"
(4=22 mod 9).
.QU "Well, LISP seems to work okay now, modulo that GC bug."
.DF MOON
n. 1. A celestial object whose phase is very important to
hackers.  See PHASE OF THE MOON.  2. Dave Moon (MOON@MC).
.DF MUMBLAGE
n. The topic of one's mumbling (see MUMBLE).
.QU "All that mumblage"
is used like
.QU "all that stuff"
when it is not quite clear what it is or how it works, or like
.QU "all that crap"
when
.QU mumble
is being used as an implicit replacement for obscenities.
.DF MUMBLE
interj. 1. Said when the correct response is either too
complicated to enunciate or the speaker has not thought it out.
Often prefaces a longer answer, or indicates a general reluctance
to get into a big long discussion.
.QU "Well, mumble."
2. Sometimes used as an expression of disagreement.
.QU "I think we should buy it."
.QU Mumble!
Common variant: MUMBLE FROTZ.
3. Yet another metasyntactic variable, like FOO.
.DF MUNCH
(often confused with
.QU mung,
q.v.) v. To transform information in a serial fashion, often requiring
large amounts of computation.  To trace down a data structure.  Related
to CRUNCH (q.v.), but connotes less pain.
.DF "MUNCHING SQUARES"
n. A display hack dating back to the PDP-1, which
employs a trivial computation (involving XOR'ing of x-y display
coordinates \- see HAKMEM items 146-148) to produce an impressive
display of moving, growing, and shrinking squares.  The hack
usually has a parameter (usually taken from toggle switches) which
when well-chosen can produce amazing effects.  Some of these,
discovered recently on the LISP machine, have been christened
MUNCHING TRIANGLES, MUNCHING W'S, and MUNCHING MAZES.
.DF MUNG
(variant: MUNGE) [recursive acronym for Mung Until No Good] v. 1.
To make changes to a file, often large-scale, usually irrevocable.
Occasionally accidental.  See BLT.  2. To destroy, usually
accidentally, occasionally maliciously.  The system only mungs
things maliciously.
.DF N
adj. 1. Some large and indeterminate number of objects;
.QU "There were N bugs in that crock!"
Also used in its original sense of a
variable name.  2. An arbitrarily large (and perhaps infinite)
number.  3. A variable whose value is specified by the current context.
.QU "We'd like to order N wonton soups and a family dinner for N\-1."
4. NTH: adj. The ordinal counterpart of N.
.QU "Now for the Nth and last time... ."
In the specific context
.QU "Nth-year grad student,"
N is generally assumed to be at least 4, and is usually 5 or more.
See also 69.
.DF "NIGHT MODE"
See PHASE (of people).
.DF NIL
[from LISP terminology for
.QU false ]
No.  Usage: used in reply to a question,
particularly one asked using the
.QU -P
convention.  See T.
.DF OBSCURE
adj. Used in an exaggeration of its normal meaning, to imply a
total lack of comprehensibility.
.QU "The reason for that last crash is obscure."
.QU "FIND's command syntax is obscure."
MODERATELY OBSCURE implies that it could be figured out but probably isn't
worth the trouble.
.DF OPEN
n. Abbreviation for
.QU "open (or left) parenthesis,"
used when necessary to eliminate oral ambiguity.  To read aloud the LISP form
(DEFUN FOO (X) (PLUS X 1)) one might say:
.QU "Open def-fun foo, open eks close, open, plus eks one, close close."
See CLOSE.
.DF PARSE
[from linguistic terminology] v. 1. To determine the syntactic
structure of a sentence or other utterance (close to the standard
English meaning).  Example:
.QU "That was the one I saw you."
.QU "I can't parse that."
2. More generally, to understand or comprehend.
.QU "It's very simple; you just kretch the glims and then aos the zotz."
.QU "I can't parse that."
3. Of fish, to have to remove the
bones yourself (usually at a Chinese restaurant).
.QU "I object to parsing fish"
means
.QU "I don't want to get a whole fish, but a sliced one is okay."
A
.QU "parsed fish"
has been deboned.  There is some
controversy over whether
.QU unparsed
should mean
.QU bony,
or also mean
.QU deboned.
.DF PATCH
1. n. A temporary addition to a piece of code, usually as a
quick-and-dirty remedy to an existing bug or misfeature.  A patch
may or may not work, and may or may not eventually be incorporated
permanently into the program.  2. v. To insert a patch into a piece
of code.
.DF PDL
(piddle or puddle) [acronym for Push Down List] n. 1. A LIFO queue
(stack); more loosely, any priority queue; even more loosely, any
queue.  A person's pdl is the set of things he has to do in the
future.  One speaks of the next project to be attacked as having
risen to the top of the pdl.
\*QI'm afraid I've got real work to do,
so this'll have to be pushed way down on my pdl.\*U
See PUSH and POP.  2. Dave Lebling (PDL@DM).
.DF PESSIMAL
[Latin-based antonym for
.QU optimal ]
adj. Maximally bad.
.QU "This is a pessimal situation."
.DF "PESSIMIZING COMPILER"
n. A compiler that produces object code that is
worse than the straightforward or obvious translation.
.DF PHANTOM
n. (Stanford) The SAIL equivalent of a DRAGON (q.v.).  Typical
phantoms include the accounting program, the news-wire monitor, and
the lpt and xgp spoolers.
.DF PHASE
(of people) 1. n. The phase of one's waking-sleeping schedule
with respect to the standard 24-hour cycle.  This is a useful
concept among people who often work at night according to no fixed
schedule.  It is not uncommon to change one's phase by as much as
six hours/day on a regular basis.
.QU "What's your phase?"
\*QI've been getting in about 8 PM lately, but I'm going to work around
to the day schedule by Friday.\*U
A person who is roughly 12 hours out of
phase is sometimes said to be in
.QU "night mode."
(The term
.QU "day mode"
is also used, but less frequently.)  2. CHANGE PHASE THE HARD
WAY: To stay awake for a very long time in order to get into a
different phase.  3. CHANGE PHASE THE EASY WAY: To stay asleep etc.
.DF "PHASE OF THE MOON"
n. Used humorously as a random parameter on which
something is said to depend.  Sometimes implies unreliability of
whatever is dependent, or that reliability seems to be dependent on
conditions nobody has been able to determine.
\*QThis feature
depends on having the channel open in mumble mode, having the foo
switch set, and on the phase of the moon.\*U
.DF PLUGH
[from the Adventure game] v. See XYZZY.
.DF POM
n. Phase of the moon (q.v.).  Usage: usually used in the phrase
.QU "POM dependent"
which means flakey (q.v.).
.DF POP
[based on the stack operation that removes the top of a stack, and
the fact that procedure return addresses are saved on the stack]
dialect: POPJ (pop-jay), based on the PDP-10 procedure return
instruction.  v. To return from a digression.  By verb doubling,
.QU "Popj, popj"
means roughly,
.QU "Now let's see, where were we?"
.DF PPN
(pip\*(fmin) [DEC terminology, short for Project-Programmer Number] n.
1. A combination
.QU project
(directory name) and programmer name,
used to identify a specific directory belonging to that user.  For
instance,
.QU FOO,BAR
would be the FOO directory for user BAR.  Since
the name is restricted to three letters, the programmer name is
usually the person's initials, though sometimes it is a nickname or
other special sequence.  (Standard DEC setup is to have two octal
numbers instead of characters; hence the original acronym.)  2.
Often used loosely to refer to the programmer name alone.
.QU "I want to send you some mail; what's your ppn?"
Usage: not used at MIT, since ITS does not use ppn's.  The equivalent
terms would be UNAME and SNAME, depending on context, but these are not
used except in their technical senses.
.DF PROTOCOL
See DO PROTOCOL.
.DF PSEUDOPRIME
n. A backgammon prime (six consecutive occupied points)
with one point missing.
.DF PTY
(pity) n. Pseudo TTY, a simulated TTY used to run a job under the
supervision of another job.
PTYJOB (pity-job) n. The job being run on the PTY.  Also a common
general-purpose program for creating and using PTYs.
This is DEC and SAIL terminology; the MIT equivalent is STY.
.DF PUNT
[from the punch line of an old joke:
.QU "Drop back 15 yards and punt" ]
v. To give up, typically without any intention of retrying.
.DF PUSH
[based on the stack operation that puts the current information
on a stack, and the fact that procedure call addresses are saved on
the stack] dialect: PUSHJ (push-jay), based on the PDP-10 procedure
call instruction.  v. To enter upon a digression, to save the
current discussion for later.
.DF QUES
(kwess) 1. n. The question mark character
.QU ? ). (
2. interj.  What?  Also QUES QUES?  See WALL.
.DF QUUX
[invented by Steele.  Mythically, from the Latin semi-deponent
verb QUUXO, QUUXARE, QUUXANDUM IRI; noun form variously QUUX
(plural QUUCES, Anglicized to QUUXES) and QUUXU (genitive plural is
QUUXUUM, four U's in seven letters).] 1. Originally, a meta-word
like FOO and FOOBAR.  Invented by Guy Steele for precisely this
purpose when he was young and naive and not yet interacting with
the real computing community.  Many people invent such words; this
one seems simply to have been lucky enough to have spread a little.
2. interj. See FOO; however, denotes very little disgust, and is
uttered mostly for the sake of the sound of it.  3. n. Refers to
one of four people who went to Boston Latin School and eventually
to MIT:
.DS
THE GREAT QUUX:  Guy L. Steele, Jr.
THE LESSER QUUX:  David J. Littleboy
THE MEDIOCRE QUUX:  Alan P. Swide
THE MICRO QUUX:  Sam Lewis
.DE
(This taxonomy is said to be similarly applied to three Frankston
brothers at MIT.)  QUUX, without qualification, usually refers to
The Great Quux, who is somewhat infamous for light verse and for the
.QU Crunchly
cartoons.  4. QUUXY: adj. Of or pertaining to a QUUX. 
.DF RANDOM
adj. 1. Unpredictable (closest to mathematical definition); weird.
.QU "The system's been behaving pretty randomly."
2. Assorted; undistinguished.
.QU "Who was at the conference?"
.QU "Just a bunch of random business types."
3. Frivolous; unproductive; undirected (pejorative).
.QU "He's just a random loser."
4. Incoherent or inelegant; not well organized.
.QU "The program has a random set of misfeatures."
.QU "That's a random name for that function."
.QU "Well, all the names were chosen pretty randomly."
5. Gratuitously wrong,
i.e., poorly done and for no good apparent reason.  For example, a
program that handles file name defaulting in a particularly useless
way, or a routine that could easily have been coded using only
three ac's, but randomly uses seven for assorted non-overlapping
purposes, so that no one else can invoke it without first saving
four extra ac's.  6. In no particular order, though deterministic.
\*QThe I/O channels are in a pool, and when a file is opened one is
chosen randomly.\*U
n. 7. A random hacker; used particularly of high
school students who soak up computer time and generally get in the way.
8. (occasional MIT usage) One who lives at Random Hall.
J. RANDOM is often prefixed to a noun to make a
.QU name
out of it (by comparison to common names such as
.QU "J. Fred Muggs" ).
The most common uses are
.QU "J. Random Loser"
and
.QU "J. Random Nurd"
.QU "Should J. Random Loser be allowed to gun down other people?" ), (
but it can be used just as an elaborate version of RANDOM in any sense.
[See also the note at the end of the entry for HACK.]
.DF RANDOMNESS
n. An unexplainable misfeature; gratuitous inelegance.  Also, a hack or
crock which depends on a complex combination of coincidences (or
rather, the combination upon which the crock depends).
\*QThis hack can output characters 40-57 by
putting the character in the accumulator field of an XCT and
then extracting 6 bits \*(em the low two bits of the XCT opcode
are the right thing.\*U
.QU "What randomness!"
.DF RAPE
v. To (metaphorically) screw someone or something, violently.
Usage: often used in describing file-system damage.  \*QSo-and-so was
running a program that did absolute disk I/O and ended up raping
the master directory.\*U
.DF RAVE
(WPI) v. 1. To persist in discussing a specific subject.  2. To
speak authoritatively on a subject about which one knows very
little.  3. To complain to a person who is not in a position to
correct the difficulty.  4. To purposely annoy another person
verbally.  5. To evangelize.  See FLAME.  Also used to describe
a less negative form of blather, such as friendly bullshitting.
.DF "REAL USER"
n. 1. A commercial user.  One who is paying
.QU real
money for his computer usage.  2. A non-hacker.  Someone using the system for
an explicit purpose (research project, course, etc.).  See USER.
.DF "REAL WORLD, THE"
n. 1. In programming, those institutions at which
programming may be used in the same sentence as FORTRAN, COBOL,
RPG, IBM, etc.  2. To programmers, the location of non-programmers
and activities not related to programming.  3. A universe in which
the standard dress is shirt and tie and in which a person's working
hours are defined as 9 to 5.  4. The location of the status quo.
5. Anywhere outside a university.
.QU "Poor fellow, he's left MIT and gone into the real world."
Used pejoratively by those not in
residence there.  In conversation, talking of someone who has
entered the real world is not unlike talking about a deceased person.
.DF RECURSION
n. See RECURSION, TAIL RECURSION.
.DF REL
See BIN.
.DF "RIGHT THING, THE"
n. That which is
.QU obviously
the correct or appropriate thing to use, do, say, etc.  Use of this
term often implies that in fact reasonable people may disagree.
.QU "Never let your conscience keep you from doing the right thing!"
.QU "What's the right thing for LISP to do when it reads \*0(.)'?"
.DF RUDE
(WPI) adj. 1. (of a program) Badly written.  2. Functionally
poor, e.g. a program which is very difficult to use because of
gratuitously poor (random?) design decisions.  See CUSPY.
.DF SACRED
adj. Reserved for the exclusive use of something (a
metaphorical extension of the standard meaning).
.QU "Accumulator 7 is sacred to the UUO handler."
Often means that anyone may look at
the sacred object, but clobbering it will screw whatever it is sacred to.
.DF SAGA
(WPI) n. A cuspy but bogus raving story dealing with N random broken people.
.DF SAV
(save) See BIN.
.DF SEMI
1. n. Abbreviation for
.QU semicolon,
when speaking.
.QU "Commands to GRIND are prefixed by semi-semi-star"
means that the prefix is
.QU ;;* ,
not 1/4 of a star.
2. Prefix with words such as
.QU immediately,
as a qualifier.
.QU "When is the system coming up?"
.QU Semi-immediately.
.DF SERVER
n. A kind of DAEMON which performs a service for the requester,
which often runs on a computer other than the one on which the
server runs.
.DF "SHIFT LEFT (RIGHT) LOGICAL"
[from any of various machines' instruction sets]
1. v. To move oneself to the left (right).
To move out of the way.
2. imper. Get out of that (my) seat!
Usage: often used without the
.QU logical,
or as
.QU "left shift"
instead of
.QU "shift left."
Sometimes heard as LSH (lish), from the PDP-10 instruction set.
.DF SHR
(share or shir)  See BIN.
.DF SHRIEK
See EXCL.  (Occasional CMU usage.)
.DF 69
adj. Large quantity.
Usage: Exclusive to MIT-AI.
\*QGo away, I have
69 things to do to DDT before worrying about fixing the bug in the
phase of the moon output routine.\*U
[Note: Actually, any number less than 100 but large enough to have
no obvious magic properties will be recognized as a
.QU "large number."
There is no denying that
.QU 69
is the local favorite.  I don't know
whether its origins are related to the obscene interpretation, but
I do know that 69 decimal = 105 octal, and 69 hexadecimal = 105
decimal, which is a nice property. \*(emGLS]
.DF SLOP
n. 1. A one-sided fudge factor (q.v.).  Often introduced to avoid
the possibility of a fencepost error (q.v.).  2. (used by compiler
freaks) The ratio of code generated by a compiler to hand-compiled
code, minus 1; i.e., the space (or maybe time) you lose because you
didn't do it yourself.
.DF SLURP
v. To read a large data file entirely into core before working on it.
.QU "This program slurps in a 1K-by-1K matrix and does an FFT."
.DF SMART
adj. Said of a program that does the Right Thing (q.v.) in a
wide variety of complicated circumstances.  There is a difference
between calling a program smart and calling it intelligent; in 
particular, there do not exist any intelligent programs.
.DF "SMOKING CLOVER"
n. A psychedelic color munch due to Gosper.
.DF SMOP
[Simple (or Small) Matter of Programming] n. A piece of code, not
yet written, whose anticipated length is significantly greater than
its complexity.  Usage: used to refer to a program that could
obviously be written, but is not worth the trouble.
.DF SNARF
v. To grab, esp. a large document or file for the purpose of
using it either with or without the author's permission.  See BLT.
Variant: SNARF (IT) DOWN.  (At MIT on ITS, DDT has a command called
:SNARF which grabs a job from another (inferior) DDT.)
.DF "SOFTWARE ROT"
n. Hypothetical disease the existence of which has been
deduced from the observation that unused programs or features will
stop working after sufficient time has passed, even if
.QU "nothing has changed."
Also known as
.QU "bit decay."
.DF SOFTWARILY
adv. In a way pertaining to software.
.QU "The system is softwarily unreliable."
The adjective
.QU softwary
is NOT used.  See HARDWARILY.
.DF SOS
1. (ess-oh-ess) n. A losing editor, SON OF STOPGAP.  2. (sahss) v.
Inverse of AOS, from the PDP-10 instruction set.
.DF SPAZZ
1. v. To behave spastically or erratically; more often, to
commit a single gross error.
.QU "Boy, is he spazzing!"
2. n. One who spazzes.
.QU "Boy, what a spazz!"
3. n. The result of spazzing.
.QU "Boy, what a spazz!"
.DF SPLAT
n. 1. Name used in many places (DEC, IBM, and others) for the
ASCII star
.QU * ) (
character.  2. (MIT) Name used by some people for
the ASCII pound-sign
.QU # ) (
character.  3. (Stanford) Name used by
some people for the Stanford/ITS extended ASCII circle-x character.
(This character is also called
.QU circle-x,
.QU blobby,
and
.QU frob,
among other names.)  4. (Stanford) Name for the semi-mythical
extended ASCII circle-plus character.  5. Canonical name for an
output routine that outputs whatever the the local interpretation
of splat is.  Usage: nobody really agrees what character
.QU splat
is, but the term is common.
.DF SUPDUP
v. To communicate with another ARPAnet host using the SUPDUP
program, which is a SUPer-DUPer TELNET talking a special display
protocol used mostly in talking to ITS sites.  Sometimes
abbreviated to SD.
.DF STATE
n. Condition, situation.
.QU "What's the state of NEWIO?"
.QU "It's winning away."
.QU "What's your state?"
.QU "I'm about to gronk out."
As a special case,
.QU "What's the state of the world?"
(or, more silly,
.QU State-of-world-P? )
means
.QU "What's new?"
or
.QU "What's going on?"
.DF STOPPAGE
n. Extreme lossage (see LOSSAGE) resulting in something
(usually vital) becoming completely unusable.
.DF STY
(pronounced
.QU sty,
not spelled out) n. A pseudo-teletype, which is
a two-way pipeline with a job on one end and a fake keyboard-tty on
the other.  Also, a standard program which provides a pipeline from
its controlling tty to a pseudo-teletype (and thence to another
tty, thereby providing a
.QU sub-tty ).
This is MIT terminology; the SAIL and DEC equivalent is PTY.
.DF SUPERPROGRAMMER
n. See
.QU wizard,
.QU hacker.
Usage: rare.  (Becoming more common among IBM and Yourdon types.)
.DF SWAPPED
adj. From the use of secondary storage devices to implement
virtual memory in computer systems.  Something which is SWAPPED IN
is available for immediate use in main memory, and otherwise is
SWAPPED OUT.  Often used metaphorically to refer to people's memories
.QU "I read TECO ORDER every few months to keep the information swapped in." ) (
or to their own availability
.QU "I'll swap you in as soon as I finish looking at this other problem." ). (
.DF SYSTEM
n. 1. The supervisor program on the computer.  2. Any
large-scale program.  3. Any method or algorithm.  4. The way
things are usually done.  Usage: a fairly ambiguous word.
.QU "You can't beat the system."
SYSTEM HACKER: one who hacks the system (in sense 1 only; for sense
2 one mentions the particular program: e.g., LISP HACKER)
.DF T
[from LISP terminology for
.QU true ]
1. Yes.  Usage: used in reply to
a question, particularly one asked using the
.QU -P
convention).  See NIL.  2. See TIME T.
.DF "TAIL RECURSION"
n. See TAIL RECURSION.
.DF "TALK MODE"
See COM MODE.
.DF TASTE
n. (primarily MIT-DMS) The quality in programs which tends to be
inversely proportional to the number of features, hacks, and kluges
programmed into it.  Also, TASTY, TASTEFUL, TASTEFULNESS.
.QU "This feature comes in N tasty flavors."
Although TASTEFUL and FLAVORFUL
are essentially synonyms, TASTE and FLAVOR are not.
.DF TECO
(tee\*(fmkoe) [acronym for Text Editor and COrrector] 1. n. A text
editor developed at MIT, and modified by just about everybody.  If
all the dialects are included, TECO might well be the single most
prolific editor in use.  Noted for its powerful pseudo-programming
features and its incredibly hairy syntax.  2. v. To edit using the
TECO editor in one of its infinite forms; sometimes used to mean
.QU "to edit"
even when not using TECO!  Usage: rare at SAIL, where
most people wouldn't touch TECO with a TENEX pole.
[Historical note: DEC grabbed an ancient version of MIT TECO many
years ago when it was still a TTY-oriented editor.  By now, TECO at
MIT is highly display-oriented and is actually a language for
writing editors, rather than an editor.  Meanwhile, the outside
world's various versions of TECO remain almost the same as the MIT
version of ten years ago.  DEC recently tried to discourage its
use, but an underground movement of sorts kept it alive.]
[Since this note was written I found out that DEC tried to force
their hackers by administrative decision to use a hacked up and
generally lobotomized version of SOS instead of TECO, and they
revolted. \*(emMRC]
.DF TELNET
v. To communicate with another ARPAnet host using the TELNET
protocol.  TOPS-10 people use the word IMPCOM since that is the
program name for them.  Sometimes abbreviated to TN.
.QU "I usually TN over to SAIL just to read the AP News."
.DF TENSE
adj. Of programs, very clever and efficient.  A tense piece of
code often got that way because it was highly bummed, but sometimes
it was just based on a great idea.  A comment in a clever display
routine by Mike Kazar:
\*QThis routine is so tense it will bring
tears to your eyes.  Much thanks to Craig Everhart and James
Gosling for inspiring this hack attack.\*U
A tense programmer is one who produces tense code.
.DF TERPRI
(tur\*(fmpree) [from the LISP 1.5 (and later, MacLISP) function to
start a new line of output] v. To output a CRLF (q.v.).
.DF THEORY
n. Used in the general sense of idea, plan, story, or set of rules.
.QU "What's the theory on fixing this TECO loss?"
.QU "What's the theory on dinner tonight?"
.QU "Chinatown, I guess." ) (
.QU "What's the current theory on letting losers on during the day?"
.QU "The theory behind this change is to fix the following well-known screw..."
.DF THRASH
v. To move wildly or violently, without accomplishing anything
useful.  Swapping systems which are overloaded waste most of their
time moving pages into and out of core (rather than performing
useful computation), and are therefore said to thrash.
.DF TICK
n. 1. Interval of time; basic clock time on the computer.
Typically 1/60 second.  See JIFFY.  2. In simulations, the discrete
unit of time that passes
.QU between
iterations of the simulation mechanism.
In AI applications, this amount of time is often left
unspecified, since the only constraint of interest is that caused
things happen after their causes.  This sort of AI simulation is
often pejoratively referred to as
.QU tick-tick-tick
simulation, especially when the issue of simultaneity of events with
long, independent chains of causes is handwaved.
.DF "TIME T"
n. 1. An unspecified but usually well-understood time, often
used in conjunction with a later time T+1.
.QU "We'll meet on campus at time T or at Louie's at time T+1."
2. SINCE (OR AT) TIME T
EQUALS MINUS INFINITY: A long time ago; for as long as anyone can
remember; at the time that some particular frob was first designed.
.DF TOOL
v.i. To work; to study.  See HACK (def #9).
.DF TRAP
1. n. A program interrupt, usually used specifically to refer to
an interrupt caused by some illegal action taking place in the user
program.  In most cases the system monitor performs some action
related to the nature of the illegality, then returns control to
the program.
See UUO.
2. v. To cause a trap.
.QU "These instructions trap to the monitor."
Also used transitively to indicate the cause of the trap.
.QU "The monitor traps all input/output instructions."
.DF TTY
(titty) n. Terminal of the teletype variety, characterized by a
noisy mechanical printer, a very limited character set, and poor print quality.
Usage: antiquated (like the TTY's themselves).
Sometimes used to refer to any terminal at all; sometimes used
to refer to the particular terminal controlling a job.
.DF TWEAK
v. To change slightly, usually in reference to a value.  Also
used synonymously with TWIDDLE.  See FROBNICATE and FUDGE FACTOR.
.DF TWENEX
n. The TOPS-20 operating system by DEC.  So named because
TOPS-10 was a typically crufty DEC operating system for the PDP-10.
BBN developed their own system, called TENEX (TEN EXecutive), and
in creating TOPS-20 for the DEC-20 DEC copied TENEX and adapted it
for the 20.  Usage: DEC people cringe when they hear TOPS-20
referred to as
.QU Twenex,
but the term seems to be catching on nevertheless.
Release 3 of TOPS-20 is sufficiently different from release 1 that some
(not all) hackers have stopped calling it TWENEX, though the written
abbreviation
.QU 20x
is still used.
.DF TWIDDLE
n. 1. tilde (ASCII 176,
.QU ~ ).
Also called
.QU squiggle,
.QU sqiggle
(sic\*(empronounced
.QU skig\*(fmgul ),
and
.QU twaddle,
but twiddle is by far the most common term.  2. A small and insignificant
change to a program.  Usually fixes one bug and generates several
new ones.  3. v. To change something in a small way.  Bits, for
example, are often twiddled.  Twiddling a switch or knob implies
much less sense of purpose than toggling or tweaking it; see FROBNICATE.
.DF UP
adj. 1. Working, in order.
.QU "The down escalator is up."
2. BRING UP: v. To create a working version and start it.
.QU "They brought up a down system."
.DF USER
n. A programmer who will believe anything you tell him.  One who
asks questions.  Identified at MIT with
.QU loser
by the spelling
.QU luser.
See REAL USER.
[Note by GLS: I don't agree with RF's definition at all.
Basically, there are two classes of people who work with a program:
there are implementors (hackers) and users (losers).  The users are
looked down on by hackers to a mild degree because they don't
understand the full ramifications of the system in all its glory.
(A few users who do are known as real winners.)  It is true that
users ask questions (of necessity).  Very often they are annoying
or downright stupid.]
.DF UUO
(you-you-oh) [short for
.QU "Un-Used Operation" ]
n. A DEC-10 system monitor call.
The term
.QU "Un-Used Operation"
comes from the fact
that, on DEC-10 systems, monitor calls are implemented as invalid
or illegal machine instructions, which cause traps to the monitor
(see TRAP).
The SAIL manual describing the available UUO's has a
cover picture showing an unidentified underwater object.
See YOYO.
[Note: DEC sales people have since decided that
.QU "Un-Used Operation"
sounds bad, so UUO now stands for
.QU "Unimplemented User Operation." ]
Tenex and Twenex systems use the JSYS machine instruction (q.v.),
which is halfway between a legal machine instruction and a UUO,
since KA-10 Tenices implement it as a hardware instruction which
can be used as an ordinary subroutine call (sort of a
.QU "pure JSR" ).
.DF VANILLA
adj. Ordinary flavor, standard.  See FLAVOR.  When used of
food, very often does not mean that the food is flavored with
vanilla extract!  For example,
.QU "vanilla-flavored wonton soup"
(or simply
.QU "vanilla wonton soup" )
means ordinary wonton soup, as opposed to hot and sour wonton soup.
.DF VAXEN
[from
.QU oxen,
perhaps influenced by
.QU vixen ]
n. pl. The plural of VAX (a DEC machine).
.DF VIRGIN
adj. Unused, in reference to an instantiation of a program.
.QU "Let's bring up a virgin system and see if it crashes again."
Also, by extension, unused buffers and the like within a program.
.DF VIRTUAL
adj. 1. Common alternative to LOGICAL (q.v.), but never used
with compass directions.  2.  Performing the functions of.  Virtual
memory acts like real memory but isn't.
.DF VISIONARY
n. One who hacks vision (in an AI context, such as the
processing of visual images).
.DF WALDO
[probably taken from the story
.QU Waldo,
by Heinlein, which is
where the term was first used to mean a mechanical adjunct to a
human limb] Used at Harvard, particularly by Tom Cheatham and
students, instead of FOOBAR as a meta-syntactic variable and
general nonsense word.  See FOO, BAR, FOOBAR, QUUX.
.DF WALL
[shortened form of HELLO WALL, apparently from the phrase
.QU "up against a blank wall" ]
(WPI) interj. 1. An indication of confusion,
usually spoken with a quizzical tone.
.QU Wall??
2. A request for further explication.
.DF WALLPAPER
n. A file containing a listing (e.g., assembly listing) or
transcript, esp. a file containing a transcript of all or part of a
login session.  (The idea was that the LPT paper for such listings
was essentially good only for wallpaper, as evidenced at SAIL where
it was used as such to cover windows.)  Usage: not often used now,
esp. since other systems have developed other terms for it (e.g.,
PHOTO on TWENEX).  The term possibly originated on ITS, where the
commands to begin and end transcript files are still :WALBEG and
:WALEND, with default file DSK:WALL PAPER.
.DF "WATERBOTTLE SOCCER"
n. A deadly sport practiced mainly by Sussman's graduate students.  It,
along with chair bowling, is the most evident manifestation of the
.QU "locker room atmosphere"
said to reign in that sphere.  (Sussman doesn't approve.)  [As of 11/82,
it's reported that the sport has given way to a new game called
.QU disc-boot,
and Sussman even participates occasionally.]
.DF WEDGED
[from
.QU "head wedged up ass" ]
adj. To be in a locked state,
incapable of proceeding without help.  (See GRONK.)  Often refers
to humans suffering misconceptions.
.QU "The swapper is wedged."
This term is sometimes used as a synonym for DEADLOCKED (q.v.).
.DF WHAT
n. The question mark character
.QU ? ). (
See QUES.  Usage: rare, used particularly in conjunction with WOW.
.DF WHEEL
n. 1. A privilege bit that canonically allows the possessor to
perform any operation on a timesharing system, such as read or
write any file on the system regardless of protections, change or
or look at any address in the running monitor, crash or reload the
system, and kill/create jobs and user accounts.  The term was
invented on the TENEX operating system, and carried over to
TOPS-20, Xerox-IFS and others.  2. A person who posses a wheel bit.
.QU "We need to find a wheel to unwedge the hung tape drives."
.DF "WHEEL WARS"
[from LOTS at Stanford University] A period during which
student wheels hack each other by attempting to log each other out
of the system, delete each other's files, or otherwise wreak havoc,
usually at the expense of the lesser users.
.DF WIN
[from MIT jargon] 1. v. To succeed.  A program wins if no
unexpected conditions arise.  2. BIG WIN: n. Serendipity.
Emphatic forms: MOBY WIN, SUPER WIN, HYPER-WIN (often used
interjectively as a reply).  For some reason SUITABLE WIN is also
common at MIT, usually in reference to a satisfactory solution to a
problem.  See LOSE.
.DF WINNAGE
n. The situation when a lossage is corrected, or when
something is winning.  Quite rare.  Usage: also quite rare.
.DF WINNER
1. n. An unexpectedly good situation, program, programmer or
person.  2. REAL WINNER: Often sarcastic, but also used as high
praise.
.DF WINNITUDE
n. The quality of winning (as opposed to WINNAGE, which is
the result of winning).
.QU "That's really great!  Boy, what winnitude!"
.DF WIZARD
n. 1. A person who knows how a complex piece of software or
hardware works; someone who can find and fix his bugs in an
emergency.  Rarely used at MIT, where HACKER is the preferred term.
2. A person who is permitted to do things forbidden to ordinary
people, e.g., a
.QU "net wizard"
on a TENEX may run programs which
speak low-level host-imp protocol; an ADVENT wizard at SAIL may
play Adventure during the day.
.DF WORMHOLE
n. A location in a monitor which contains the address of a
routine, with the specific intent of making it easy to substitute a
different routine.
The following quote comes from
.QU "Polymorphic Systems,"
vol. 2, p. 54:

\*QAny type of I/O device can be substituted for the standard device
by loading a simple driver routine for that device and installing
its address in one of the monitor's
.IQ wormholes. *
.br
\l'1i'
.br
*The term
.IQ wormhole
has been used to describe a hypothetical
astronomical situation where a black hole connects to the
.IQ "other side"
of the universe.
When this happens, information can pass
through the wormhole, in only one direction, much as
.IQ assumptions
pass down the monitor's wormholes.\*U
.DF WOW
See EXCL.
.DF XGP
1. n. Xerox Graphics Printer.  2. v. To print something on the XGP.
.QU "You shouldn't XGP such a large file."
.DF XYZZY
[from the Adventure game] adj. See PLUGH.
.DF YOYO
n. DEC service engineers' slang for UUO (q.v.).
Usage: rare at
Stanford and MIT, has been found at random DEC installations.
.DF "YOYO MODE"
n. State in which the system is said to be when it rapidly
alternates several times between being up and being down.
.DF "YU-SHIANG WHOLE FISH"
n. The character gamma (extended SAIL ASCII 11),
which with a loop in its tail looks like a fish.  Usage: used
primarily by people on the MIT LISP Machine.
Tends to elicit incredulity from people who hear about it second-hand.
.DF ZERO
v. 1. To set to zero.  Usually said of small pieces of data, such
as bits or words.  2. To erase; to discard all data from.  Said of
disks and directories, where
.QU zeroing
need not involve actually writing zeroes throughout the area being zeroed.
