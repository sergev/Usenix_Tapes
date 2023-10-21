.ce
Options
.sp
.PP
In TREK73, all the names are taken from the series Star Trek.
Through the use of options, the names can be changed to whatever
you want.
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
.IP "ships STRING"
This option, if set, tells the program how many ships you wish to fight.
If it is not set, then the program will ask.
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
.IP "silly BOOLEAN [nosilly]"
If this option is set, an additional race is added to the list of
possible races to fight.
This race is the Monty Pythons.
Note that if you wish to always fight the Monty Python's, you merely
have to set the enemy option above.
.bp
