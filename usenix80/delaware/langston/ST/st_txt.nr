.ll 79
.hy 15
.de lp
.sp
.in 12
.ta 11
.ti 0
\\$1\t
..
	\" @(#)st_txt.nr	1.3 last mod 2/18/80 -- (c) psl 1979
.ce
Brief Star - Drek Playing Instructions

This document is broken up into three sections: the display,
the commands and the object.
"The Display" describes what you see on the screen while playing.
"The Commands" describes the commands that you can give to control
your ship.
"The Object" gives some vague idea of how to proceed.



THE DISPLAY
.br
-----------

The display consists of several numeric instruments and the bridge monitor.
The arrangement is something like:
.nf


   Mission Vect     +-------------------------+     Energy      E. Time
     3,  2, 98      !          O              !      999          2
		    !            .     .      !
   Base Vect        !       .         .       !     V Range     Sensors
   120,-17,358      !            !        .   !       98          127
		    !         .  !            !
   Velocity         ! .    - - - ? - - -      !     Speed       Condition
     0,  0, 5.2     !            !            !      5.2          GREEN
		    !  .         !            !
   Rotation         !     .                   !     Shields @ 33%
     0,  0,  0      ! .                       !     C.C.C. @ 66%
		    !                         !     13 A.T.U. remain
		    +-------------------------+
.fi




The Bridge Monitor
.br
-----------------

The Bridge Monitor, enclosed in a box in the middle of the display, shows
the view out the front of your ship.  You can see other ships, stars,
StarBase, torpedos, etc displayed on the screen using the following characters.
.nf
 .      star
 *      torpedo
 O      StarBase
 Y      Klingon Battleship
 #      Romulan Cruiser
 X      Vallician Warship
 @      Rigellian Space Fortress
 +      Nubian Freighter
 ?      Any ship at a distance
.fi




The instruments
.br
---------------

Mission Vector -- This is the position of something out in space that needs
looking into.  The form is x rotation, y rotation, distance.
In our example there is something just slighly off the ship's axis,
(the x rotation and y rotation are near zero),
and 98 GDUs away, (Galactic Distance Units).
The rotations are expressed in GAUs, (Galactic Angular Units);
256 GAUs are equal to 90 degrees.

Base Vector -- This is the position of StarBase relative to us.
Once again the form is x rotation, y rotation, distance.  In our example,
StarBase is 120 GAUs up, (positive x rotation), and 17 GAUs to the left,
(negative y rotation), from the way we're pointing.  It's 358 GDU away.
You'll notice that StarBase, 'O', is visible in the top part
of the bridge monitor.

Velocity Vector -- This is how the ship is moving, (translating), through
space relative to the ship's current heading.  Its form is x-rot, y-rot,
speed.  In our example the ship is moving straight forward (i.e. the
x rotation and the y rotation of our velocity vector are zero).
Our speed is 5.2 GDU/GTU, (Galactic Distance Units per Galactic Time Unit).

Rotation Vector -- This is how the ship is rotating around its own center
of mass.  Its form is x-rot, y-rot, z-rot (notice that all the other
vectors ignore z rotations since they are so infrequently used).
In our example the ship is not rotating at all.  If the figures were
16, 0, 0 then we would be rotating up (positive x rotation) at a rate
of 16 GAU/GTU which would take 64 GTUs to rotate all the way around,
(1024 GAU = 360 degrees = 2 pi radians, etc).

Energy -- This figure is the amount of energy available to be used by various
systems, (anti-matter generator, shields, phasers, etc).  The maximum energy
that can be accumulated is 999 units.  The pile and dilithium crystal
generate 50 units of energy per GTU (if they're both undamaged).  Of this,
15 units are used to run life-support systems, the electronics for the
atomic pile, and other essential automatic systems,
and 15 units are usually used to run the bridge monitor, weapons systems,
and other semi-manual instruments.
Thus energy is replenished at about 20 units per GTU.  When the ship is under
autopilot control, (see command "a"), the bridge instruments are shut down
and don't draw any energy, at these times energy is replenished
at a rate of 35 units per GTU.

Elapsed Time -- This is the number of minutes (real-time) that you have
been on your mission.  The correspondence between GTUs and
elapsed time minutes is approximately 60 to 1.

Visual Range -- This is the distance to the object closest to the center
(cross-hairs) of the bridge monitor.  In our example it is the distance to
the unidentified ship that is lined up in the crosshairs.

Sensors -- The sensors are a sophisticated form of radio receivers that
pick up the radiation from ship engines all over the galaxy.
As you approach another ship or ships the sensors climb in an
inverse square manner, which is to say that one ship 100 GDU away
will give a higher reading than two ships 200 GDU away.

Speed -- This instrument gives the "forward" speed of your ship, i.e. the
component of your velocity that lies along the axis of the ship.  This is
very handy for figuring out quickly which way you're moving.

Condition -- This indicator has three states: GREEN, YELLOW and RED.  Its
setting depends on the value of the sensors and the amount of accumulated
energy you have available.  In general Condition GREEN means that it is
quite safe for you to go to sleep for a while.  Condition YELLOW means
that danger is near and Condition RED means you had better know what you're
doing and be implementing some plan of action.

A few indicators only appear temporarily:

Shields @ 50% -- This indicates that your protective shields have been
energized to 1/2 their maximum value.  Full, (100%), shields will
withstand almost any known weapon except tractor beams which can grab
the shields as easily as they can grab the ship.  Each time your shields
are hit by a weapon they are partially used up and may need to be reset.

C.C.C. @ 33% -- This indicates that the Course-Correction-Computer has
been activated to automatically, (using the impulse engines), get rid
of any sideways motion.  A setting of 33% will neutralize 1/3 of your
drift per GTU.

12 A.T.U. remain -- This indicates that you are under Autopilot control
and 12 more GTUs of Autopilot control remain before the ship is returned
to manual control.





THE COMMANDS
.br
------------

The following are the commands recognized by Star_Drek.  Note that the
form "##x" means a number followed by "x", e.g. "3x" or "-23x".
Also note that you will not see what you're typing as you type it;
however, it will be displayed when the program acts upon it.
(Further, note that since the individual command characters can be changed
by modifying "com_map" in "st_glb.c" and/or defines in "st_def.h" your
installation may have done so and forgotten to change this document...)

Command     Action
.br
-------     ------
.lp     [DEL]
Send out emergency distress call, (used rarely when you are
either crippled or in a hurry to quit).
.lp     r
Print permanent damage report.
Permanent damages are damages which Scotty cannot repair.
.lp     ##f
Forward thrust (backward if ## is negative)
The impulse engines are capable of delivering 5 units of
thrust per GTU.  If you ask for "8f" you will immediately
get 5 units of thrust, the message "3.0 units of burn left"
will be printed, and then you will get 3 more units of thrust.
.lp     ##c
Activate course-correction-computer to ##%
.lp     *
Fire photon torpedo
Photon torpedoes,(made by anti-matter generator), can only
last for 20 time units before disintegrating.  They are
launched at a speed of 5 relative to your velocity; therefore,
if you're not moving, they will go 100 GDU before falling apart.
.lp     ##p
Fire phaser, ## is power
"p" is equivalent to "0p" which is a mild stun power.
Since phasers are not a very tight beam they require much
more energy when used at large distances.
.lp     ##x
Begin x rotation; if ## is zero, stop x rotation
Rotations are additive except for the stop rotation commands.
.lp     [up]
Same as 13x, (this is the up-arrow key).
.lp     [down]
Same as -13x, (this is the down-arrow key).
.lp     ##y
Begin y rotation; if ## is zero, stop y rotation
.lp     [left]
Same as -13y, (this is the left-arrow key).
.lp     [right]
Same as 13y, (the right-arrow key).
.lp     ##z
Begin z rotation; if ## is zero, stop z rotation
.lp     ##a
Start ## GTUs of autopilot
.lp     d
Dock at StarBase.
You must be close enough to StarBase and moving very slowly.
NOTE: This is the only honorable way to end a mission.
.lp     ##w
Warp jump.  ## is warp "factor" which determines the distance
jumped.
This is an instantaneous jump through four-space, rather than
the improbable velocity "warp drive" that the "Startrek" TV show uses.
.lp     ##t
Activate tractor-pressor.
The tractor-pressor provides an unbreakable force-field link
between your ship and the object onto which the trac-press
is locked.  This link only lasts for an instant, but during
that instant your velocity and the other object's velocity
become the same.  If the other object's mass is the same as
yours then the final velocity is the average of the original
velocities, (i.e. momentum is conserved).
.lp     ##s
Set shields to ##%.
No known enemy weapon can penetrate 100% shields at a distance of
15 GDU or greater, however
high shield settings tend to drain quite a bit of energy.
.lp     i
Execute inertial stop, (stop motion and rotation).
This costs three times as much energy as using the "##f"
command to come to a stop but is much more convenient.
.lp     q
Request salvage operation by StarBase on a Nubian freighter
that is within 5 GDU of StarBase.
This is the only officially sanctioned method of disposing of
Nubian freighters.
.lp     $
Print current mission "score"
This includes # of enemy ships shot up, # of salvages, etc.
.lp     !
Generate a sub-shell.  Suspends the action in Star Drek
while you run other programs. When you type control-D to the
sub-shell you will be back in Star Drek where you left off.
.lp     ^
Re-draw the display
The same idea as the [REFRSH] key in the "e"ditor.
.lp     [BACKSPACE]
Ignore partially-typed command.  This is useful for correcting mistakes.
Note that this command is "wired-into" the helm program.

.in 0
Definition of Maneuvers --
.br
The following commands allow creation of "maneuvers":
.lp     ;
Enter "line mode" and wait until a full line of command(s) is typed
before acting.  A full line is indicated by either the
[RETURN] key, a maneuver definition, ('='), or a file execution, ('<'),
.lp     [RETURN]
Terminate "line mode" and carry out the commands typed since the last
';'.  This is an immediate, (non-stored), maneuver.
e.g. ";99s3w" will execute "99s" and "3w" when [RETURN] is struck.
.lp     :
Wait one time unit before doing the following command(s).
e.g. ";512x:x<return>" will turn the ship end-for-end.
.lp     #=
Terminate line mode and store maneuver # for later use.
e.g. ";33p*3=" will define maneuver 3 as being "33p" followed by "*"
.lp     #l
List the commands in maneuver #.
.lp     #m
Execute maneuver #.
.lp     #,
Execute maneuver #, (convenient because it is on many numeric pads).
.lp     file<
Terminate line mode and read commands from specified file.
e.g. ";foo<" will read commands from the file "foo".  When end of file
is encountered control will return to the keyboard.  Note that "<" does
not nest; i.e. the file "foo" may not contain another "<" command.
Also remember that [LINE FEED] is the same code as [down] on many
terminals...
.in 0



THE OBJECT
.br
----------

Your mission is to check out the various ships whose coordinates
are given in your mission vector and dispose of them properly.
.br
"Properly" means blast them to smithereens in the case of Klingons,
Romulans, Vallicians and Rigellians.
In the case of Nubian Freighters, the "proper" thing to do is
to drag them to StarBase with your tractor-pressor and then request a
salvage operation, (you can salvage any ship that's close enough to
StarBase).
.br
Each different type of ship has different characteristics and
vulnerabilities; e.g. the Vallicians understand photon torpedos well
and consequently use very powerful ones and are good at withstanding
torpedo attacks; on the other hand, they don't have very good phasers
and are relatively vulnerable to them.
.br
As you gain experience you will learn how to deal with each
different race.
.br
HINT: The Rigellian Space Fortress was inspired by a very successful
science fiction movie of the late 70s, (which had no Rigellians
in it).


In general a reasonable plan is:
.lp     (1)
Rotate yourself to point toward you mission vector.
If your mission vector is 36, 13, 256 you might do a 12x for
3 time units, (hitting "x" to stop x rotation), then do a 13y for
1 time unit, leaving your mission vector as 0, 0, 256 or so.
.lp     (2)
Warp to within 100-200 units.  A warp factor of 0, "0w", will move you
about 55 distance units; a warp factor of 3, "3w", will move you about
100 units.
.lp     (3)
Sneak up to visible range, (or until attacked), to determine species.
"5f" will start you moving forward at a speed of 5.
.lp     (4)
Either blast it or drag it home.
.lp     (5)
If any other ships remain either go back to step (1) or on to step (6).
.lp     (6)
Return to StarBase and dock.  Accept the cheers of the crowd.
.in 0

Of course, things don't always go according to plan...


Please let me know what areas of Star-Drek need further explanation...
and GOOD LUCK!

.tl '''psl 11/79'
.pl 1
