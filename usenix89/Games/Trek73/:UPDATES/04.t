.ce
Options
.sp
.PP
In TREK73, all the names of crewmembers are taken from the
Star Trek series.
Through the use of options, the names, as well as other
aspects of the game, can be changed to whatever you want.
.sp
.PP
To use the options, you must add the variable TREK73OPTS to
your environment.
A sample would be (using the C-shell):
.br
.sp
setenv TREK73OPTS 'name=Jerk, ship=Boobyprize, terse'
.sp
.PP
The option string is a list of comma-separated options.
Options are designated as either boolean or string options.
Boolean options are turned on by typing their name and turned
off by prepending 'no' to them.
String options are set equal to the string which follows the "=".
.sp
.PP
There follows a list of all the options, what type it is,
and an explanation of what they mean.
The default for the option is in square brackets following
the option.
.sp
.IP "terse BOOLEAN [noterse]"
This option, when set, turns off the information regarding the
ship's purpose in the area.
It thus reduces the amount of drek on the screen.
If you are on a slow terminal, this is a nice option to have set.
.sp
.IP "shipname STRING [Enterprise]"
This option names your ship.
.sp
.IP "name STRING"
This option names the captain of the ship.
If this option is not set, then the program will ask for a name.
The captain is the one who must make all the decisions of strategy
and tactics for the ship.
.sp
.IP "sex STRING"
This option gives the captain a gender.
If this option is not set, the program will ask for it's value.
If you respond with something that starts with other than "m" or "f",
beware!
.sp
.IP "science STRING [Spock]"
This option names the science officer, who is responsible
for checking the parameters of the captain's commands.
It is also this officer's duty to report damage to the ship
as well as scan for enemy damage.
.sp
.IP "engineer STRING [Scott]"
This option names the chief engineer of the ship.
It is this officer's duty to report on the status of the ship,
especially its energy supply and distribution of the same.
The officer also controls the launching of anti-matter probes.
.sp
.IP "helmsman STRING [Sulu]"
This option names the ship's helmsman.
This officer's duty is to control the speed of the ship
and also controls the firing of the ship's weapons.
.sp
.IP "nav STRING [Chekov]"
This option names the ship's navigator, who is responsible
for the navigation of the ship.
This officer makes changes to the ship's course as directed
by the captain.
This officer also controls any anti-matter probes after they
have been launched.
.sp
.IP "com STRING [Uhura]"
This option names the ship's communications officer.
It is the duty of this officer to handle all communications between
the ship and the rest of the universe.
.sp
.IP "enemy STRING [random]"
If this option is set, it tells the program which race you wish
to fight.
The available races are:
Klingon, Romulan, Kzinti, Gorn, Orion, Hydran, Lyran, or Tholian.
If the option is not set, the race you will fight is chosen at random.
.sp
.IP "foename STRING [random]"
If this option is set, it specifies the name of the commander
of the enemy ship(s).
If this option is not specified, the name is chosen at random.
.sp
.IP "class STRING [CA]"
This option specifies the kind of ship you are commanding.
Allowable classes are: DN, CA, CL, and DD, standing for
dreadnought, heavy cruiser, light cruiser, and destroyer.
In general, the larger the ship, the more weapons and stronger
shields you have, at the cost of less speed and maneuverability.
.sp
.IP "foeclass STRING [CA]"
This option specifies the kind of ship that you are fighting.
The different classes are explained above.
.sp
.IP "silly BOOLEAN [nosilly]"
If this option is set, an additional race is added to the list of
possible races to fight.
This race is the Monty Pythons.
Note that if you wish to always fight the Monty Python's, you merely
have to set the enemy option above.
.sp
.IP "time STRING [30]"
Time is used to specify the time between commands.
The longer this value, the more time may be used in issuing a command.
.sp
.IP "teletype BOOLEAN [noteletype]"
The teletype option causes some of the output to come out as it did
in the original teletype version.
.sp
.IP "savefile STRING [$HOME/trek73.save]"
The savefile option specifies where the data image is to be stored if the
game is saved during play.  ``~'' is not expanded, so the path should be
explicit and fully expanded.
.sp 3
.ce
Command Line Options
.sp
.PP
In all cases, the arguments you place on the command line will supersede
options in the environment.
.sp
.PP
The following is a description of the command line options:
.sp
.IP \-t
Turns on terse mode.
No initial scenario description is given.
This is useful for terminals running at low baud rates.
This option is normally off.
.sp
.IP \-c
Allows the specification of the Federation captain's name.
.sp
.IP \-s
Specify the sex of the captain of the Federation vessel.
.sp
.IP \-S
Specify the name of the Science Officer of the Federation vessel.
.sp
.IP \-E
Specify the name of the Chief Engineer of the Federation vessel.
.sp
.IP \-C
Specify the name of the Communications Officer of the Federation vessel.
.sp
.IP \-N
Specify the name of the Navigator of the Federation vessel.
.sp
.IP \-H
Specify the name of the Helmsman of the Federation vessel.
.sp
.IP \-f
Specify the name of the enemy commanding officer.
.sp
.IP \-r
Specify the race of the enemy.
The race should be one of the following:
Klingon, Romulan, Kzinti, Gorn, Hydran, Lyran, Tholian, Orion,
or Monty Python.
.sp
.IP \-d
Set the delay time for command entry.
Higher times can be useful for novices or for playing on very slow
terminals.
.sp
.IP \-y
Silly option.
Adds the Monty Pythons as a possible enemy race.
This option is normally off.
.sp
.IP \-T
Teletype option.
Causes certain parts of the output to come out as they did on the
original teletype implementation.
Doesn't do much for the game on crts.
This option is normally off.
.sp
.IP \-n
Specify the name of the Federation vessel.
The default name for the Federation vessel is randomly chosen from a
set of names.
.sp
.IP \-F
Specify the class of the enemy vessel(s).
Allowable classes are Destroyer (DD), Light Cruiser (CL),
Heavy Cruiser (CA), and Dreadnought (DN).
If the argument is none of the above, the program assumes that this is
the name of a file where a player-designed ship is stored.
.sp
.IP \-l
Specify the class of the Federation vessel.
Available classes are the same as the enemy's.
.sp
.IP \-R
Restore the game from the savefile.  It is assumed that the TREK73OPTS
contains the name of the savefile, otherwise it is not possible to restart
the game with the -R option.  In case the savefile name is not in TREK73OPTS,
the game may be restored by issuing the command with the path to the savefile
as the first argument.
.bp
