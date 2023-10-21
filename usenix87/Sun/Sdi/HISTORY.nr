.\" feed me through tbl and the me macros
.tp
.\" Add the 'po' line if sending to normal printer.
.\"po 1i
.sz 18
.ce 99
History of the SDI code.
.ce 0
.lp
.sz 13
.TS
tab(:);
c cw(4i) c
l l l.
Date:What:Hours
_
Dec. 31, '86:T{
Hacked up a little program to draw expanding and
contracting circles.  This program still exists (called wormtest), and
allowed the tuning of blast aesthetics, and makes pretty output itself.
T}:6
Jan. 1, '87:T{
Organized blasting code onto a linked list, tested it, in preparation
for missile code.  Fooled around with multiple window presentation
and subframes.
T}:8
Jan. 3, '87:T{
Put in missle launching code.
T}:4
Jan. 4, '87:T{
Tried various versions of missile drawing.  Sent lots of mail to
bugs@sun complaining about their vector algorithms not agreeing with
the documentation. Finally adopted my own increment-with-correction
code, which still makes noticable mistakes but not often enough to 
rise above fixit threshold.
T}:8
Jan. 5, '87:T{
First playable game in a single window.  Played it, discovered that at high missle and
blast densities the notifier prioritized mouse events such that it was
impossible to get a blast out ahead of a fast moving missle.
Started thinking about a solution.
T}:4
Jan. 6, '87:T{
Tried various notification schemes, mostly in toy programs to see if
they did the job.  Discovered that asynchronous notification could
not be relied upon, and that there were bugs in the sun suntools
input_event_handler.  However, by a combination of ugly hacks and double
firewalls got something that worked with fcntrl NOBLOCK.  Still occasionally
dropped mouseclicks, but all that were seen were seen immediately.
T}:8
Jan. 8, '87:T{
Started getting annoyed with dropped mouseclicks, looked into other schemes:
like changing the notifier priorities (not tried at this time, be used eventually), like registering asynchronous
event handlers (tried).  Nothing tried worked.
Gprofed a long run, and discovered all the time was going into
rops (as is reasonable).  Reorganized code to update everything
at fixed intervals (blast_delay), rather than setting separate
notifications for everything.  Fixed intervals meant I could lock
and batch all rops.  Big performance improvement.
Now there was really lots of cpu
time left (on a Sun-3/75-4) even with the busiest screen, and so
I could probably return to synchronous.  Tried it.  Upped polling
time to 150 milliseconds, and it worked.  #ifdef'd (ASYNCH) the asynch
code in case it was ever needed again.
T}:10
Jan. 9, '87:T{
Added second playing window, for missiles launching up.  As planned, the
missile launching and drawing code worked backwards and forwards.
T}:6
Jan. 10, '87:T{
Played with city melting.  Fell asleep at 4am with it not working.
T}:8
Jan. 11, '87:T{
Got melting working right away.  Started working on GAME OVER blowup
code.  Wasted lots of time on the genmessage program thinking Vax
bit/byte order, worked immediately after I made the code explicit
about Endians.
T}:8
Jan. 12, '87:T{
Started working on panels.  Split panel into separate control window.
Got the basic buttons in.  Fooled
around with making every button work whenever pushed (e.g.
new game in the middle of a game.)  Suspend/Resume did
not work.
T}:6
Jan. 13, '87:T{
Cleaned up some global switches that even I wasn't sure about, and
got suspend/resume and game over working clean under all conditions.
Fixed some races in notification of end-of-game under unusual conditions.
T}:4
Jan. 15, '87:T{
Cleaned up the structure, created argv.c, helpers.c, and global.c.
Worked on resize problem.  (Had to go back to 3.0 manuals because
3.2 lacks an examples of popup menus which grab the input).
T}:6
Jan. 16, '87:T{
Got resize popup to work.  Now the game could not be thwarted by any
inadvertent user action.  Put game on file server, sent mail to a
few friends announcing availability.
T}:6
Jan. 17, '87:T{
A solid game!  Played a few rounds.  Tuned launch distribution
and missile speed to make game more playable at higher levels.
Added srandom() call to main.c
T}:4
Feb. 3, '87:T{
Added lasers, prettier screen arrangement, continuous mode.
T}:6
Feb. 4, '87:T{
Ground away at some nagging intermittent bugs.  Discovered a dangling
pointer problem in 'doto_missiles' and 'doto_blasts', and writing to
arrays out of bounds in 'melt' and 'start_blast'.  Now, I can't make
it crash (but someone will, I'm sure.)
T}:2
Feb. 6, '87:T{
More clean up.
T}:3
Feb. 7, '87:T{
Drew a new city, much nicer.  Added 'end-game', skill level, score files,
improved melting speed.
T}:6
Feb. 9, '87:T{
Added 'advice'.  (Fought with bug in using window_loop to display text subwindows.  The bug won.)  Gprofed the world again, added LINE_GRAIN to intersect.c
to cut down excessive cpu there.  Back to good old mem_rop eating
all my time (about which I can do nothing.)
T}:4
Feb. 12, '87:T{
Worked on active icons.  Sure wish I could get directly to that pixwin--keep
getting flicker.  Hand drew 24 miniature game frame, wrote icontest to
animate them.  Looks bad.
T}:4
Feb. 16, '87:T{
Drew a different set of 8 icons for animation.  Subtle, just lights flickering
and two quiet missiles.  Still flicker, but good enough to put into game.
T}:2
Feb. 21, '87:T{
Intense notifier hacking to make response good even at very high levels of missiles and blasts.  Finally went to my own notifier scheduler, and turning
asynchronicity on and off at each round.
T}:4
Feb. 22, '87:T{
Finished notifier hacking.  Seems good, but now a little too easy too play.
Reduced the blast size of a blown up missile so self-sustaining blast walls
do not occur.
T}:4
Feb. 27, '87:T{
Added the source and history buttons.  Fun!
T}:2
March 6, '87:T{
Added the -t and -l options for the Maryland open house.
T}:1
March 15, '87:T{
Commented uncommented routines.
T}:2
March 30, '87:T{
Put in changes suggested by my Beta testers on the net (thanks everyone!):
(a) slight delay after click-to-start,
(b) saving and restoring games,
(c) look in /usr/game/lib/sdi_scores before tmp,
(d) initial position of the three windows,
(e) explain scoring better in the man entry, add a 'man' button,
(f) improve source, history, and man fiddling,
(g) disable menus for launch and city frames.
T}:6
March 31, '87:T{
Put in rocks.  Added shift and meta button modifications.
T}:4
April 1, '87:T{
An all too appropriate date for discovering and working around yet another
Sun notification bug (in SunOs 3.2) which causes the middle mouse button down transition
to be lost when the input focus is transfered into the canvas and not
refused.  Wrote the keytest tool to graphically illustrate the problem
and mailed it to bugs@sun.  Wrote a workaround into sdi.
T}:3
April 2, '87:T{
Fixed a lurking bug in the -r option.
T}:1
April 4, '87:T{
Added the dynamic cursor.
T}:2
April 5,6,7,8,9,10,11, '87:T{
Tuning, playing, massive re-arrangment of control panel items into sub
windows, a little at a time.
T}:8
April 12, '87:T{
Pie-menu version of 'Things to Read'.
T}:4
April 13 thru 23, '87:T{
Little tunings.
T}:6
May 1 thru 5, '87:T{
Background border, message when saving to existing file, pie dynamic cursor.
Spent many, many hours wasted trying to get a window to properly reshape
when it has a panel at the bottom.  Could not do it.  (The 'foe on the
ground' slider really belongs on the bottom of the launch frame.)
T}:16
.TE
.hl
.lp
NOTES.
.np
Almost all programming was done at home after 10pm at night.
.np
Very little time was spent not at the terminal.  There were no design
documents, not even scrawled notes.  The only large offline time
was spent reading Foley and van Dam for ideas on clipping vectors,
and it was useless time.
.np
Included in the above times is some mail and news reading interleaved
with waiting for compiles.  I don't know how to separate that out.  It might
be as much as half the time.
.np
Also included in the above times is some general Sun manual and code
reading, paticularily about the notifier.  This is probably not
more than 2-3 hours, however.
