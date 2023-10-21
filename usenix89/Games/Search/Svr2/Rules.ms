.LT 4i
.RP
.ds RH A Guide to Search
.ds LH Version 6.0
.ds CF - % -
.TL
A Guide to the Game of Search/Version 6.0
.AU
Greg Ordy
.AI
Computer Science Department
Case Western Reserve University
Cleveland, Ohio 44106
.AU
Sam Leffler
.AI
Sytek, Incorporated
1153 Bordeaux Dr.
Sunnyvale, California 94086
.AU
Dave Pare
.AI
Phonetics Lab
University of California at San Diego
La Jolla, California, 92037.
.AU
Matt Crawford
.AI
Department of Astronomy and Astrophysics
University of Chicago
Chicago, Illinois, 60637
.AB
Search, is a multiplayer action game requiring quick reflexes and an
ability to spot
.I invisible
players.  Up to 15 people may be playing
search at any given time.  Players may enter and leave the game, at any
time.  This document describes the rules and commands of the game.
.AE
.NH
INTRODUCTION
.PP
Search, is a multiplayer action game requiring quick reflexes and an
ability to spot
.I invisible
players.  Up to 15 people may be playing
search at any given time.  All players involved in a game live in the
same universe.
The goal of the game is to stay alive and collect points by shooting
alien beings and other players.
.NH
THE UNIVERSE
.PP
All players are located in the same universe.  The universe
is represented as a two dimentional X,Y coordinate system, of size
256 by 256.  The universe ``wraps'' around on itself.  That is, the coordinate
one greater than 127 is -128.  This applies to both the X and Y direction.
Wrap around implies that continued travel in one direction will
eventually bring a player to a previously traveled location.  It is never
possible to get too far away from the action!
.NH
THE BASICS
.PP
The goal of search is to accumulate points.  Points can be earned
by attacking and successfully damaging other players, or alien spacecraft.
The objects in the universe are:
.IP *
A star base.
.IP #
A normal alien spacecraft.
.IP X
An X-alien spacecraft.
.IP @
A shanker alien spacecraft.
.IP +
A torpedo.
.IP %
A piece of the planet Quartone.
.IP .
A piece of shrapnel from destroyed player's spacecraft.
.IP A-K
Another player's spacecraft.
(It is also possible to view players as lower case letters,
see the information on team playing, below.)
.PP
To exist in the universe and do battle, a player must expend energy.
A player is removed from a game if his energy reaches 0.  The
manipulation of energy will be explained later.
.NH
VIEW SCREEN
.PP
When a player enters the game, his terminal is transformed
into the console of his space ship.  The most prominent part of the
console is the view port, complete with cross hairs.  Around the view port
are several status displays, which inform the player of the the status of
his ship.
A player's current position, which is constantly updated in the
status display on the upper right portion of the screen, is defined
to be the point in the center of the cross hairs.  A player never
``see's'' himself, as the view screen is like a look out of a picture
window.  Any object which is in the center of the cross hairs is in the
same location as the player.
.PP
Below the player's position, is the player's energy display. 
The current energy level is updated every 16 time units.
To immediately display your current energy, type ``e''.
As was stated above, if a player's
energy goes to zero, they leave the game, by force.
.PP
Below the energy display is the homing radar display.
Each ship has three channels of homing radar,
which may be used however the player sees fit.
Homing radar is important in that an object locked on
a homing radar channel can be ``autopiloted'' to.
.PP
Below the homing radar display is the points display. 
The point total is updated each time it changes.
The ``p'' command may also be used to display the current point total.
.PP
Coming down the left hand side of the view screen are several more
displays.
The first display is the Invisibility display.
A player may control his invisibility with the ``i'' command.
When a player is invisible, he cannot be seen by other players,
or be autopiloted upon.
Invisibility, like everything else, costs energy -- one unit per iteration.
.PP
Below Invisibility is the Velocity display.
A player may have a velocity in either the X or Y direction.
The velocity can be either a positive or negative number.
As in the case of the Energy display,
the Velocity is updated each time it changes.
Should the values become garbled,
the ``v'' command will immediately display the current velocity components.
.PP
Below Velocity is the Time display.
The current time is only displayed with the ``c'' (for clock) command.
Units of time vary with the load on the system.
Earlier versions of search placed no maximum number of iterations
per second, but it was found that with the new socket software, the
game would move much more quickly than a human could react!
The game will interate maximum 10 times per second.
It is possible, with high system loads, that the game may go slower.
The more players, the fewer the number of iterations per any unit of real time.
.PP
The final display along the left side of the view screen is the M-Factor
display.
A player may vary the magnification factor of his view screen,
to see more of the universe.
Normal M-Factor is 1, but it can go to 2 or 3.
A player is urged to use caution when using M-Factors other than 1,
as they are very costly in energy.
.PP
Below the view screen are two lines labeled ``STATUS'' and ``MESSAGE''.
Various messages from inside the player's ship, and other players,
appear on these lines.
.PP
Finally, the last five lines of the screen are occupied by the ``PLAYERS MAP''.
This table provides a cross-index between the upper case letters
used for displaying a player's ship and the actual player.
The current status of the player
(``a'', ``d'', or ``q'' for ``alive'', ``dead'', and ``quit'') is also shown.
Certain commands require a player to be named by his/her ship name (e.g. ``A'');
this map is useful in deciding which upper case letter should be supplied.
.NH
MOVEMENT
.PP
The terminal's keypad is used to control velocity (and hence motion).  
On the screen, the positive X direction goes
toward the right, and the positive Y direction goes down.  Hitting
a key causes movement in the direction the key is positioned in,
relative to the keypad.
For example, hitting a ``6'' adds velocity (and hence movement)
in the positive X direction.  
Typing a ``7'' reduces velocity in both the X and Y directions.
Each keystroke increases a player's velocity one unit
in the specified direction.
To slow down the velocity component
in the appropriate direction must be reduced.
This is accomplished by hitting the ``reverse'' direction key.  
For example, if a ``2'' is hit when a player is
standing still, he will be begin traveling in the positive Y direction,
one unit per iteration.
Hitting an ``8'' will return the player to zero velocity,
while hitting another ``2'' will increase the velocity
to 2 in the positive Y direction.
.NH
ENERGY EXPENDITURE
.PP
Running out of energy removes a player
from the game, thus it is very important to know how it is
manipulated.
A player normally starts off with 250 units of energy
(players entering search for the first time are given 500 units as they
normally waste it quickly while learning how to maneuver).  
Certain events cause a player to lose the ability to hold energy;
here is an informal list of ``energy changers''.
.IP o
Shooting a torpedo costs 10 energy units.
.IP o
Being invisible costs 1 energy unit per iteration.
.IP o
0 velocity in both directions causes a player to 
.B gain
one unit of energy per iteration.
.IP o
A velocity of 1 in either direction neither gains nor
loses energy.
.IP o
A velocity of greater than 1 in either direction costs
1 energy unit per turn.
.IP o
Being hit with space shrapnel, from the explosion of a dead ship, costs
20 energy units.
.IP o
Each nuke modification at Quartone costs 20 energy units.
.IP o
A Magnification Factor that is greater than 1 costs energy (see below).
.PP
If one considers the above rules, the following ``rules of thumb'' may
be ascertained.
.IP o
Remaining invisibile with no movement expends no energy.
.IP o
Moving quickly with invisibility on and a high magnification factor
is close to suicide!
.PP
Being hit with a torpedo not only costs 50 energy units, but also
reduces a player's capacity to hold energy by 50 units.
This means that if a player has an energy of 250 and is hit by a torpedo,
he will have an energy of 200, and cannot ever again go above 200,
unless repairs are made at a starbase.
Starbases will be explained later.
The X-aliens also effect a player's capacity to hold energy.
If an X-alien moves across your current location (a collision),
your capacity to hold energy is decreased by 15.  
Finally, space shrapnel also reduces a player's capacity to hold energy.
.NH
WEAPONS
.PP
A player has two types of weapons, useful against other players, and aliens.
The most general is a torpedo, which can hit either a player or an alien.
A torpedo is fired by hitting the ``t'' key (or the ``5'' key) followed
by a firing direction which is entered from the keypad.  
For example, the sequence ``t2'' will shoot a torpedo from your current
position in a direction in the positive Y direction.
When a torpedo is fired, it is given the player's current position,
and velocity which is the sum of the player's components and
the ``firing'' component specified by the keystroke.
A torpedo must have non-zero velocity to be launched.  
A nuke is similar to a torpedo, but is actually a barrage of torpedoes
(see the section on nukes for more information).
.PP
The second type of weapon is the ``phaser'', which is only used against aliens.
To fire a phaser, a player must be directly on top of an alien,
and then hit the ``0'' key.
Firing a phaser costs no energy.
When an alien is hit with either a torpedo or phaser,
it moves in a new direction.
.NH
POINTS
.PP
A player receives 50 points for hitting another player.  
Hitting yourself with a torpedo (self hit) 
.B costs
75 points.  
Running into the planet Quartone not only costs 50 points,
but also removes a player from the game.
Phasering an alien is worth 4 points.
A torpedo hit on a normal alien is worth 6 points,
an X-alien 20 points, and a shanker 20 points.
.NH
SHANKERS
.PP
Shankers are aliens that fire back!!!
Some of the aliens in the universe start out looking like a normal ``#'' alien,
but are really shankers.
When a shanker is first hit with a torpedo,
it removes the ``#'' disguise and becomes an ``@'', the true shanker colors.
A shanker remembers who hit him, and when the opportunity
arises shoots at the enemy.
A shanker will remain mad at a player until it gets one hit on him,
and then the shanker will start roaming around, until next attacked.
A shanker torpedo hit reduces a player's capacity to hold energy by 50,
so they should not be taken lightly.
.NH
QUARTONE
.PP
Quartone is a planet which is located somewhere near the
coordinates <-30, -30>.    
As all objects receive a random start in the game, these coordinates
are only approximate.
Quartone has several unusual properties.
Running into Quartone costs 50 points, and causes removal from the game.
If a player is right next to Quartone, and is going slow enough,
he/she may orbit Quartone, with the ``o'' command.  
A player orbiting Quartone may not be hit with a torpedo from another player.
The reason for this is that the orbit command actually takes the
player into a low orbit over the planets surface where torpedos are burned up
by atmospheric friction.
In ``guts mode'' (see below) orbiting players are
.I not
safe from torpedos.
It may appear as if torpedos go through Quartone,
but they are really going ``behind'' it!
Torpedos shot by shankers, however, can hit Quartone
(and players in orbit around it) as they are made out of Krell metal,
the hardest substance in the Universe.
.PP
It is possible to exploit the people on quartone. 
The procedure is explained in the section on nukes.
.NH
STARBASES
.PP
There are three starbases in each game of search.
Two are located somewhere in the first quadrant <+x, +y>,
and one is located a little <-x, -y> of Quartone.
When a player has a starbase directly in the middle of his crosshairs,
he may dock, with the ``d'' command.
Each docking increases a player's capacity to hold energy by 25 units,
up to the limit of 250.
A player's energy will also be made equal to the maximum energy capacity
of his ship.
Although starbases sound like nice places to sit on,
a player should be careful -- torpedoes can hit players at starbases,
and there are usually invisible players waiting near starbases
to ambush players low on energy.
.NH
AUTOPILOT
.PP
As mentioned above, a player has three channels of homing radar,
which are used in conjunction with the autopilot.
When an object is directly in the middle of a player's crosshairs,
the player has the oportunity to ``lock onto'' that object,
using the ``s'' set command.
``s'' sets a user specified channel number to that object.
The valid set commands are therefore ``s1'', ``s2'', or ``s3''.
Once an object is locked onto, its current coordinates may be displayed by
using the ``h'' command, in conjunction with the corresponding channel
number, such as ``h2''.
Stationary objects, such as a starbase, will always have the same coordinates,
but moving objects, such as aliens or other players will normally
show a different coordinate values each time the ``h'' command is executed.
To autopilot on a set object, enter the ``a'' command,
followed by the channel number desired.
Autopilot moves you toward the target with a maximum velocity
of 2 in either direction.
Autopilot costs no energy, and is therefore an easy way to travel.
When the target is reached, the autopilot will turn off and signal
the player with a beep.
When following a moving object, the autopilot will keep up with the target,
as long as the target goes no faster than 2 in either direction.
Once a channel is set, it may be reused by ``setting over'' the previous target.
.NH
SOLAR FLARES
.PP
When a player turns on his ``invisibility'', what he is really
doing is ionizing his ships hull with random space vacuum particles (RSVP).
This ionization makes the ship ``blend'' in with the normal space void, on
other players view screens.
When an occasional solar flare occurs,
the delicate ionic potential balance is upset, and players lose their ability 
to become invisible for the duration of the flare.
When a flare occurs, each player will be informed by a message,
and their invisibility will go off.
Solar flares have a random duration, and there is no warning of
when the flare ends.
.NH
SHRAPNEL
.PP
Each ship is powered by trans-diclorobisethlyenediameme cobalt
chloride III crystals.
When a player is dealt a fatal torpedo hit,
there is always the chance that the crystals will be hit.
When these crystals are hit, then explode,
sending deadly shrapnel out from the dead ship.
If a player is hit by a piece of shrapnel, they must lose 20 energy units
to effect repairs, and, the ionic imbalance of the crystals will cause
their invisibility to go off.
.NH
GUTS MODE
.PP
When the game begins it is in the regular mode.
Players may vote to enter Galactic Undaunted Tough Searchers
mode (GUTS) by using the ``G'' command.
When all living players have registered a favorable vote, the
game enters guts mode and all players are notified of the fact.
In guts mode invisibility is not functional and orbiting the
planet does not afford protection from torpedoes.
If guts mode has not yet begun players may cancel their vote
with the ``W'' command.
If guts mode has begun the ``W'' command registers a vote to
reenter Wily Invisible Menacing Players (WIMP) mode.
When the vote becomes unanimous (whether by death or by more
democratic means) the mode of the game will change.
.NH
TEAM PLAYING
.PP
Several commands have been added to the game which allow a form of team playing.
As explaned above, the representation of the universe on the view screen
is accomplished by measuring the Ionic Mass Forces (IMF) between objects.
It is also possible to send out coded radio beacons which can be used
to locate and ``see'' objects.
Detection of ships by radio is possible under all conditions,
providing a player wishes to send the coded beacon.
Assuming several players enable each to see the other, they may remain
invisible to the rest of the universe, and work as a unit.
Four commands deal directly with team playing: j,q,l, and g.
.PP
The ``j'' command lets you select the players which may see you
all of the time.
After the ``j'' is a list of
.B lower-case
player letters.
The command is terminated with a newline (don't forget it).   
The ``q'' command lets you remove players
from the list of those who can see you at all times.   
Usage is similar to ``j'',
except that ``q.'' removes all players.  
You may display those players enabled to
see you with the ``l'' command.  Finally, the ``g'' command sends a message
only to the members of your group; it is used just like the ``b'' command.  
When viewing a player who is invisible,
but sending you a radio beacon, the player will appear as a lower case letter.
.NH
NUKES
.PP
A weapon similar to a torpedo is the nuke.  
The command used to launch a nuke is identical to that of a torpedo,
except instead of a ``t'', an ``n'' is used.
However, to launch a nuke a player's ship must be modified.
Cheap labor is usually found on the surface of Quartone,
and modifications to allow nukes can be made there.
To modify a ship,
a player must enter the orbit of Quartone and turn off invisibility.
The ``x'' command (exploit) may then be used to tell the people
on the planet to work on the ship.
As a result of poor working conditions,
each modification results in some worker deathes.
Each modification allows one nuke to be stored.
Usually only 10-12 nukes may be stored during a visit to Quartone.
Be warned that excesive carnage can lead to the generation of powerful foes.
.NH
COMMAND SUMMARY
.PP
No attempt will be made to outline or detail tactics or approaches to the game.  
Search commands, with the exception of quit, guts and wimp, are in lower case.
.IP v 9
display current velocity
.IP e 9
display current energy
.IP d 9
dock at a starbase
.IP s 9
set homing radar channel (1,2, or 3)
.IP h 9
display current position of object locked in on a homing radar
channel (1,2 or 3)
.IP a 9
autopilot to an object locked in on a homing radar channel (1,2 or 3)
.IP "@, ^L" 9
redraw the screen
.IP t 9
fire a torpedo
.IP 0 9
fire the phasers on an alien positioned directly ``underneath''
your ship
.IP c 9
display the current time (equal to the number of iterations)
.IP f 9
display your facts -- shows a player's hits (p = player hits, a =
alien hits), kills of other players, and search ``rating''
.IP o 9
go into orbit around the planet Quartone
.IP b 9
send a broadcast message (type ``b'', the message, then a carriage
return or newline); messages are truncated at 35 characters
.IP r 9
send a personal radio message -- like ``b'', but only to one person
(type ``r'', a player's upper case letter, the message, then a
carriage return or newline); messages are truncated at 35
characters
.IP "1-4,6-9" 9
add velocity in a certain direction
.IP m 9
change the magnification factor (type ``m'', followed by
1,2 or 3)
.IP Q 9
quit the game (reply with a ``y'' or ``Y'' when prompted)
.IP i 9
toggle your invisibility on or off (except in guts mode)
.IP j 9
join a group (type ``j'', the player's upper-case letters, then a 
carriage return or newline)
.IP q 9
quit players from group (type ``q'', the appropriate upper-case
letter(s), then a carriage return or newline;  a special
case is ``q.'', which removes all players)
.IP l 9
display the players in your group.
.IP g 9
broadcast a message to your group (type ``g'', followed by the
message, then a carriage return or newline);  messages are
truncated at 35 characters
.IP n 9
fire a nuke (followed by a direction in which to fire)
.IP x 9
exploit the people on Quartone
.IP G 9
registers your vote to enter ``guts mode''
.IP W 9
cancels your vote to enter, or
registers your vote to leave ``guts mode''
.NH
MISC
.PP
To try and obtain a comparative skill rating between players,
``search accounting'' was implemented.
Certain pieces of information are kept about each player's status;
.IP o
total interations,
.IP o
torpedo hits on aliens,
.IP o
torpedo hits on other players, and
.IP o
kills of other players.
.PP
A ``level'' number is derived from the accumulated statistics.
The formula for calculating this level is:
.sp 1
.ce
level = ((points*100)+(hits*100)+(kills*1000))/time
.sp 1
The ``f'' command gives a players level, within the current game;
otherwise, the accumulated information may be seen by executing
the program ``sscore''.
