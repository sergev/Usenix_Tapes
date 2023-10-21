.bp
.ce
APPENDIX 5
.sp 2
.ce
Designer's Notes
.sp 2
.PP
When I was about 10 or 11, I would go to the Lawrence Hall of Science
quite frequently.
There, on their time-sharing system, I would sit at a Teletype Model 33
and play trek.
.PP
Sadly, LHS replaced their system and trek went away.
It was my intention that it should not be forgotten.
I had had a copy of a slightly different version of trek written in BASIC,
but it was sadly unimplementable on my Apple computer.
.PP
When I learned of an implementation of trek written in C, I jumped at
the chance to bring it to the systems here at UC Berkeley.
At that time, the game was mostly a direct translation of the original
BASIC source.
Many of the commands and routines were either missing or faulty.
With Peter Yee, we worked together to fix up the program and managed
to get it running.
.PP
Once it was running, the time came for improvements.
I admit to pulling many concepts from the game Star Fleet Battles, most
notably the different races (Hydrans, Lyrans, etc.) and the ship names.
.PP
This version of the game represents many hours of thinking and debugging.
I hope you enjoy playing it as much as I did coding it.
.sp
.IP " " 30
-Jeff Okamoto
.sp 2
.PP
My first experience with TREK73 was also at the Lawrence Hall of Science.
I had been taking classes in Time-Sharing Basic and noticed that other
people always talked about a game called $TREK that was a real CPU hog
and was usually turned off.
Naturally I was intrigued.
Soon I was paying $2.00 an hour for the chance to play that game.
Many long hours and quite a few dollars went into playing $TREK,
so it was with a certain sadness that I learned that the DG Ecllipse on
which $TREK ran was being phased out.
I made several attempts to obtain the source before it went away, but
I was unable to get it.
.PP
Fortunately for me, Dave Pare at UC San Diego was also a fan of the game,
and more importantly, he had an outdated copy of the source from an
HP 2000.
Dave had started to implement the game in C to run under 4.2 BSD UNIX.
Expressing my interest to Dave, I was able to get a copy of Dave's code
and thus the TREK Project at Berkeley was started.
I spent endless hours tweaking with Dave's code, implementing some of
the fifteen or so commands that he had not yet translated.
.PP
At about this time, I learned that Christopher Williams, here at Berkeley,
had also tried to implement the game in C.
What is more, he had a copy of the source (in BASIC) from Berkeley
High School, and had implemented most of the commands.
Merging the work that Chris had done into my copy of Dave's work led
to a fairly complete version of the game.
There still remained a large number of bugs, poor ideas, and outright
mistakes in the code, but it ran.
.PP
Jeff Okamoto, being a fan of the game and a Star Fleet Battles
player, was greatly interested in hacking on the game to bring up to
par with the version that ran at LHS and to extend it even beyond
that.
Thus our partnership was formed and the current version of the game
represents several hundred hours of our joint work (and play).
Also represented are the suggestions, modifications and bug fixes
we received from numerous people, including (to name a few)
Matt Dillon, David Sharnoff, Joel Duisman, all at Berkeley, and
Roger Noe at Rockwell International.
.PP
It is hoped that this implementation of a classic game will bring joy
(and perhaps fond remembrances) to all who play.
.sp
.IP " " 30
Live Long and Prosper,
.br
-Peter Yee
