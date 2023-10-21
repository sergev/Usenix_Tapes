.sp
.LP
11.  Probe Control.
.PP
[y | [n [Probe] [y | [n [Target | [<CR> Course]]]]]]
.PP
Probe control allows you to detonate or redirect probes
which may have missed.
.sp
.LP
12.  Position Report.
.PP
Position reports are vital since valuable information on
courses, bearings and ranges are given to aid the formation
of good strategy.
.PP
Each ship is listed along with its current speed, course,
and bearing.
Also listed is your relative bearing to that ship.
A relative bearing of 0 means you are pointed directly at
the ship, whereas a relative bearing of 180 means you are
pointed directly away from the ship.
Next is the reverse relative bearing, which gives the relative
bearing of you with respect to the ship listed.
.PP
Cloaked ships show up with an asterisk (*) before the name,
and the information displayed is the last available information
on those ships.
If no position report has been performed prior to the enemy ship
engages a cloaking device, no information will be available on 
that ship.
This order does not use a turn.
.sp
.LP
13.  Position Display.
.PP
[Radius of scan]
.PP
Position displays, similar to radar scans, show objects
which surround your vessel.
The Enterprise is indicated by a `+', jettisoned engineering
sections by a `#', probes by a `*', torpedos by a `:', and
enemy vessels by the first letter of their names.
Enemy vessels that are cloaked appear as lower case letters
and remain in their last noted absolute position.
.sp
.LP
14.  Pursue An Enemy Vessel.
.PP
[Target Name] [Warp Factor]
.PP
This order instructs the ship's navigation to face an enemy
vessel whenever possible.
Obviously it is impossible to pursue a cloaked vessel.
.sp
.LP
15.  Run From An Enemy Vessel.
.PP
[Target Name] [Warp Factor]
.PP
This order, just the opposite of order #14, instructs the
navigation to keep the stern of the Enterprise towards an
enemy vessel whenever possible.
Running from a cloaked vessel is not very useful.
.sp
.LP
16.  Manually Change Course and Speed.
.PP
[Course] [Warp Factor]
.PP
This order instructs navigation to maintain a fixed course
and speed.
The following information applies to the above three orders:
.sp
.PP
Your maximum rotation rate when turning is:
.ce
degrees per sec = 5 * (11 - current warp speed)
.sp
.PP
Accordingly, you can turn 55 degrees at warp one, 50 at
warp two, and so on down to 10 degrees at warp nine.
In other words, the faster your speed, the less
maneuverable you are.
You are also less maneuverable if your warp drive is damaged
or destroyed.
Your maximum speed is warp nine, the enemy's is warp eleven.
.sp
.LP
17.  Damage Report.
.PP
This report informs you of certain equipment status.
A destroyed computer make orders 3 (lock
phasers), 4 (lock torpedos), 14 (pursue), 15 (run), 27
(initiate self-destruct), and 28 (abort self-destruct)
impossible to execute.
You will be required to manually rotate phasers and
torpedos, and manually change course and speed.
Destroyed sensors makes orders 13 (position display) and
18 (scan) impossible.
A destroyed probe launcher prevents you from
launching probes.
A destroyed warp drive slows your maximum speed to warp 0.99 and
severly limits your maneuverability.
See order 20 about jettisoned engineering section.
When your crew of 450 dies, your vessel is as good as dead.
There are 350 men aboard each enemy vessel.
.sp
.PP
All of the above systems can be partially damaged.
A damaged warp drive (common) lowers your maximum speed
and maneuverability.
A damaged probe launcher (sometimes) may refuse to launch.
Damaged sensors (rare) may not be able to return position
displays or be able to scan an enemy.
A damaged computer (very rare) will sometimes refuse to lock onto
targets, and in addition, when damaged, may lose some of the
locks held by the weapons or by the helm.
.sp
.PP
Shield percentage is calculated by its energy drain times
its operating efficiency.
Efficiency starts at 100 and declines with each hit.
No damages of any kind are incurred when a shield absorbs
its first hit, no matter how great the hit.
Shield one is 1.5 times as strong as the other three shields.
.sp
.PP
`Efficiency' indicates the number of energy units being
burned per warp-second.
This number is initially one (.75 for enemy) and increases
per hit.
.sp
.PP
`Regeneration' indicates the number of energy units being
gained per second.
Initially set at 10, this number decreases per hit.
.sp
.PP
`Fuel capacity' indicates the number of matter-antimatter
pods a vessel has aboard.
This number rapidly decreases with each torpedo or probe
fired.
.sp
.PP
`Fuel' indicates the number of matter-antimatter pods which
are filled with energy.
This number rapidly decreases when maintaining high warp
speeds or firing phasers.
.sp
.LP
18.  Scan Enemy (Damage Report of Enemy).
.PP
[Ship Name | Probe id | #Ship Name]
.PP
An enemy damage report is essentially the same as the
Enterprise's.
Sensor reports can not be had for cloaked vessels.
.PP
By giving the id number of a probe, information about it can
be gathered.
The same information can be gathered about a ship's (jettisoned)
engineering by prepending a '#' before the ship's name.
.sp
.LP
19.  Alter Power Distribution.
.PP
[Shld 1 drain [* | ... Sh 4]] [Phsr 1 drain [* | ... Ph 4]]
.PP
The synopsis of this command can be confusing.
The first set of numbers gives the drains for each shield.
All four shield drains can be specified, but
if a star is used immediately after a shield drain
(eg, 19 0.5 1*),
then the remaining shields will all be given a drain equal to the
number preceding the '*'.
(Thus, in the above example, shield 1's drain is 0.5, whereas
shields 2, 3, and 4 have a drain of 1).
The same applies to the phaser drains.
.sp
.PP
The power circuits of all vessels are illustrated in
Appendix 2.
Dilithium crystals produce energy much like generators.
This power is then used to maintain warp speeds, recharge
antimatter pods in the engine reserve, recharge phaser
banks, or maintain shield power.
Your initial regeneration is ten, however, the shields normally
drain four units and the engines require one unit per each
warp-second.
.sp
.PP
Shields can be thought of as electromagnets.
The more energy put into them, the stronger their force field
becomes.
Therefore, a shield's overall percentage is calculated by
the following formula:
.sp
.ce
shield percentage = (energy in)(effective %)
.sp
.PP
Notice that dropping power to a shield has the same effect
as having it hit.
Notice also that if your regeneration drops below four,
you may have to discharge your phaser banks to maintain full
shield power.
.sp
.PP
Phaser banks, similar to batteries, not only discharge (when
firing), but also recharge.
Initially, they are set to recharge fully in one second
(+10) so that you can continually use them.
However, they can discharge fully (-10) in one second to
provide extra power to shields, warp engines, or engine
reserve.
.sp
.PP
Under most conditions, you need not concern yourself with
power distribution unless some special need arises.
Distribution, for the most part, is automatic.
Regeneration is calculated first; that power is placed in
reserve, along with any discharged phaser units.
Shield drain is calculated next, then the cloaking device,
then phaser and engine drains.
.sp
.PP
Be concerned with wasting power by indiscriminately firing
phasers and torpedos, maintaining speeds over warp three, or
dumping scores of units onto antimatter probes.
Huge power losses cannot be made up in battle.
.sp
.LP
20.  Jettison Engineering.
.PP
Although this order was never executed in the television
series, it is quite possible according to its producer.
Jettisoning engineering has serious consequences, but it may
be your only course of action.
.sp
.PP
One would jettison engineering if being pursed by vessels,
probes or torpedos, or as a suicidal gesture.
.sp
.PP
The following things happen when engineering is jettisoned:
A: You lose all your fuel and reserve capacity; B: you lose
your regeneration; C: you lose your warp drive; D: your lose
your probe launcher; E: you lose your shields until you
designate phasers to discharge; F: the engineering section
itself decelerates to a stop; G: a ten second time delay on
it is set (hopefully, when it does explode, you are far
from its effects); H: you lose your cloaking device;
I: your phasers and torpedos are now free to fire in any direction.
