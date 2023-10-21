.ce
Detailed Descriptions of Each Command
.sp
.PP
What follows is a detailed description of each command.
Each command is referred to by a number from 1 to 31.
After the name of the command is given, a synopsis of the
arguments the command requires is given, if any.
These arguments can be entered on the command line, separated
by whitespace, if you wish.
For instance, to fire phasers 1 through 4 with a spread of 15,
you could type '1 1234 15' on the command line.
.PP
It should be noted that all numbers refer to parameters for
the Heavy Cruiser Class ship, and that other ships have
slightly different characteristics.
.sp
.LP
1.  Fire Phasers.
.PP
[Phasers | all] [Spread]
.PP
Phasers are pure energy units which emit
a beam similar to lasers, but of a pulsating nature which
can be `phased' to interfere with the wave pattern of any
molecular form.
Phasers get their power from phaser banks, which in turn,
derive their power from the ship's engines.
Each phaser bank is capable of holding a charge of 10 units.
When firing, these banks discharge, similar to batteries, to
spread their destructive power through space.
After discharging, these banks are then recharged by the
engines.
Each phaser can be set to automatically track a target or
can be manually rotated.
Unless engineering is jettisoned (code 21), phasers only
fire from 0-125 and 235-360 degrees, relative to the ship's
course.
In other words, each vessel has a 110 degree blind side in
back of it in which phasers cannot fire.
If phasers fired into this blind side, they would destroy
the ship's engineering section.
.sp
.PP
The Captain also designates a wide or narrow phaser beam: a
wide beam to disrupt many targets; a narrow beam to inflict
maximum damage on a single target.
The maximum spread of phasers is 45 degrees, the minimum is
10 degrees.
The total beam width is twice the designated spread.
.sp
.PP
The firing percentage of each bank is preset to 100.
In other words, the bank fully discharges when firing.
This can be changed, however, using code 22.
.sp
.PP
The maximum range of phasers is 1000M; the maximum hit
factor is 45 with a ten degree spread, 10 with a forty-five
degree spread.
Phaser hit factors are calculated by the following formula:
.sp
.ce
hit = (bankunits)(firing%)sqrt(1-range/1000)(45/spread)
.sp
.PP
Phasers fire in .2-second intervals starting with bank one.
Phasers inflict heavy damage and casualties, but do not
destroy shields as much as antimatter explosions do.
.sp
.PP
A phaser is unable to fire if damaged, if firing into your
blind side, or if completely discharged.
.sp
.LP
2.  Fire Photon Torpedos.
.PP
[Tubes | all]
.PP
The Enterprise is equipped with six torpedo tubes, which, as
phasers, can be set to automatically track a target or be
manually rotated.
Unless engineering is jettisoned, tubes only fire from
0-135 and 225-360 degrees.
Each tube fires all its antimatter pods, which are
temporarily held suspended in a magno-photon force field.
Photon torpedos can be fired directly at an enemy, laid out
as a mine field, or scattered in an attacker's path as
depth charges.
.sp
.PP
Tubes must be loaded (code 9) prior to firing.
Normally, torpedos are launched at warp 12 in .2-second
intervals, beginning with tube one.
Photon torpedos have a proximity fuse of 200M.
All of these values can be changed by using code 22.
.sp
.PP
Torpedos must be launched with care since the antimatter
pods which are fired can never be recovered.
It is suggested that you not fire more than four torpedos at
any one time, since a certain number of them do miss, or are
destroyed by the enemy firing phasers at them.
It is also suggested that you fire them at distant targets,
beyond 1100M, to avoid the explosion radii of your own
weapons.
Hit factors resulting from antimatter explosions are
calculated as follows:
.sp
.ce
hit = 5(#podscontained)sqrt(1-range/(50(#podscontained)))
.sp
.PP
The maximum hit factor of an antimatter device is five times
the number of pods contained (in the case of torpedos, 50);
its explosion radius is 50 time the number of pods
contained (in the case of torpedos, 500).
Antimatter explosions heavily weaken shields but do not
damage equipment as much as phasers do.
This formula also applies to vessels, engineering sections,
and antimatter probe explosions.
.sp
.PP
A photon torpedo's proximity fuse will not be activated by a
cloaked ship.
.sp
.PP
Tubes are unable to fire if damaged, if firing into your
blind side, or if unloaded.
.sp
.LP
3.  Lock Phasers.
.PP
[Phasers | all] [Target Name]
.PP
Phasers locked on an enemy vessel will automatically aim
towards it.
Although phasers may track a vessel which is in the firing
blind side, they will not fire unless engineering is
jettisoned.
To fire at vessels in the blind spot, simply change course
at least 55 degrees.
Once a phaser is locked, it is not disengaged until the
target is destroyed (in which case it is then rotated to
zero degrees relative), relocked, manually overridden, or
damaged.
.sp
.PP
Phasers can not be locked onto cloaked enemy ships as they
can not find the target.
Phasers that were previously locked onto a non-cloaked ship
will track the enemy's last known course and position when
that ship cloaks.
.sp
.LP
4.  Lock Tubes.
.PP
[Tubes | all] [Target Name]
.PP
Tubes lock and unlock in the same manner that phasers do.
Tubes suffer the same locking limitations that phasers do in
reference to cloaked ships.
.sp
.LP
5.  Manually Rotate Phasers.
.PP
[Phasers | all] [Bearing]
.PP
Manually rotating phasers disengages any previous locks and
positions them as directed, relative to your course.
For example, if your course is 30, and phasers are rotated
45 degrees, they will hit a target bearing 75 degrees.
Rotating phasers into you blind side is permissible,
however, they will not fire.
.sp
.LP
6.  Manually Rotate Tubes.
.PP
[Tubes | all] [Bearing]
.PP
Manually rotating tubes is similar to rotating phasers.
.sp
.LP
7.  Phaser Status.
.PP
Phaser status reports the control (locks and damages),
deployment, levels, firing percentages (normally 100),
and charge/discharge rates (normally +10) of all phasers.
This command does not use a turn.
Cf. Command 22.
.sp
.LP
8.  Tube Status.
.PP
Tube status reports the control, deployment, tube levels,
launch speeds (normally 12), proximity delays (normally
200), and time delays (normally 10) of all tubes.
This command does not use a turn.
Cf. Command 22.
.sp
.LP
9.  Load/Unload Tubes.
.PP
[l | u] [Tubes | all]
.PP
Each specified tube will be automatically loaded with 10 units or
whatever remains in the engines, whichever is less.
Tubes can also be unloaded if the need arises.
.sp
.LP
10.  Launch Antimatter Probe.
.PP
[Pods] [Time] [Proximity] [Target | [<CR> Course]]
.PP
Probes are slow-moving devices equipped with internal
guidance systems which allow them to chase an enemy vessel.
Probes consist of at least ten antimatter pods which are
launched from an undamaged probe launcher at warp three.
As with torpedos, probes are set with time and proximity
fuses, and use the same hit factor formula as do torpedos.
