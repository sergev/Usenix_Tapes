.de sh
.ne 6
.sp
.PP
\s12\fB\\$1\fR\s10
.sp
.PP
..
.TH PHANTASIA 6 GAMES
.UC 4
.SH NAME
phantasia \- an interterminal fantasy game
.SH SYNOPSIS
\fBphantasia\fR [\fB\-s\fR] [\fB\-m\fR] [\fB\-a\fR] [\fB\-x\fR] [\fB\-h\fR]
[\fB\-H\fR] [\fB\-p\fR] [\fB\-b\fR]
.SH DESCRIPTION
.I Phantasia
is a role playing game which allows players to roll up characters of
various types to fight monsters and other players.
Progression of characters is based upon gaining experience from fighting
monsters (and other players).
.PP
Most of the game is menu driven and self-explanatory (more or less).
The screen is cursor updated, so be sure to set up the \fB\s8TERM\s10\fR
variable in your environment.
.PP
The options provide for a variety of functions to support the game.
They are:
.PP
.TP .5i
.B \-s
Invokes \fIphantasia\fR without header information.
.TP .5i
.B \-m
Get a monster listing.
.TP .5i
.B \-a
Get a listing of all character names on file.
.TP .5i
.B \-x
Examine/change a particular character on file.
.TP .5i
.B \-h
Get a help listing (this message).
.TP .5i
.B \-H
Get header listing only.
.TP .5i
.B \-p
Purge old characters.
.TP .5i
.B \-b
Show scoreboard of top characters per login.
.PP
The characters are saved on a common file, in order to make the game
interactive between players.  The characters are given a password
in order to retrieve them later.  Only characters above
\fRlevel\fR zero are saved.
Characters unused for more than three weeks will be purged.
Characters are only placed on the scoreboard when they die.
.SH AUTHOR
Edward Estes, AT&T Teletype Corporation.
.br
Modified by Chris Robertson, September 1985 ({linus, decvax, ihnp4} !
utzoo! ! nonh).
.SH PARTICULARS
.sp
.sh "Playing in General"
Certain of the player's more important statistics are almost always
displayed on the screen, with maximum values (where applicable) in
parentheses.
.PP
The character is placed randomly near the center of a cartesian system.
Most commands are selected with a single letter or digit.
For example, one may move by hitting \fBE, W, N,\fR or \fBS\fR
(lower case may also be used;  at no time is the game case-dependent).
The \fBH, J, K,\fR and \fBL\fR keys may also be used to move
(equivalent to W, S, N, and E, respectively).
To move to a particular (x,y) coordinate, use the
\fBmove\fR (`1') command.  The distance a character can move is calculated by
1 plus 1.5 per \fBlevel\fR.
Moving in a particular compass direction will move the player the maximum
allowed distance in that direction.
.PP
A player may at any time see who else is playing with a \fBplayers\fR
(`2') option.  One may see only those who are the same distance or closer
to the origin as he/she.
A \fBpalantir\fR removes these retrictions.
\fBKings\fR and \fBcouncil of the wise\fR can see and can be seen by everyone.
.PP
One can talk to other players with the \fBtalk\fR
(`3') option.  In general, this is a line or so of text.  To remove a current
message, just press \s8RETURN\s10 when prompted for a message.
.PP
The \fBstats\fR (`4') option shows additional characteristics of a player.
.PP
One may leave the game either with the \fBquit\fR
(`5') option, or by hitting interrupt.  Quitting during battle results
in death, for obvious reasons.
.PP
One may rest by default (any key except the menu ones).
Resting lets one regain maximum \fBenergy level\fR, and also lets one find
\fBmana\fR (more is found for larger levels and further distances from
the origin).
.PP
One may call a monster by hitting `9' or `C'.
.PP
Several other options become available as the player progresses in \fBlevel\fR
and \fBmagic\fR, or to other stations in the game (\fBvalar, council of the
wise, king\fR).
These are described elsewhere.
In general, a control-L will force the redrawing of the screen.
.PP
On most versions, `!' gives a shell escape;  if followed by a command, just
that command is executed.
The environment variable \s8SHELL\s10 is used for a subshell if it is set;  the
default is \fI/bin/sh\fR.
.PP
Other things which may happen are more or less self-explanatory.
.sh "Fighting Monsters"
A player has several options while fighting monsters.  They are as follows:
.TP 1.5i
.I melee
(`1' or `\ ')  Inflicts damage on the monster, based upon \fBstrength\fR.
Also decreases the monster's \fBstrength\fR some.
.TP 1.5i
.I skirmish
(`2')  Inflicts a little less damage than \fBmelee\fR,
but decreases the monster's \fBquickness\fR instead.
.TP 1.5i
.I evade
(`3')  Attempt to run away.  Success is based upon both the player's and
the monster's \fBbrains\fR and \fBquickness\fR.
.TP 1.5i
.I spell
(`4' or `M')  Several options for throwing spells (described elsewhere).
.TP 1.5i
.I nick
(`5')  Hits the monster one plus the player's \fBsword\fR,
and gives the player 10% of the monster's \fBexperience\fR.
Decreases the monster's \fBexperience\fR
an amount proportional to the amount granted.
This also increases the monster's quickness.
Paralyzed monsters wake up very fast when nicked.
.TP 1.5i
.I luckout
(`6' or `B')  This is essentially a battle of wits with the monster.  Success
is based upon the player's and the monster's \fBbrains\fR.
The player gets credit for slaying the monster if he/she succeeds.
Otherwise, nothing happens, and the chance to \fBluckout\fR is lost.
.sh "Character Statistics"
.TP 1.5i
.I strength
determines how much damage a character can inflict.
.TP 1.5i
.I quickness
determines how many chances a character gets to make decisions while
fighting.
.TP 1.5i
.I energy level
specifies how much damage a character may endure before dying.
.TP 1.5i
.I magic level
determines which spells a character may throw, and how effective those
spells will be.
.TP 1.5i
.I brains
are basically the character's intelligence;  used for various
fighting options and spells.
.TP 1.5i
.I mana
is used as a power source for throwing spells.
.TP 1.5i
.I experience
is gained by fighting monsters and other characters.
.TP 1.5i
.I level
indicates how much experience a character has accumulated;  it progresses
geometrically as \fBexperience\fR increases.
.TP 1.5i
.I poison
is sickness which degrades a character's performance (affects
\fBenergy level\fR and \fBstrength\fR).
.TP 1.5i
.I sin
is accumulated as a character does certain nasty things;  used only rarely
in normal play of the game.
.TP 1.5i
.I age
number of seconds of playing time for the character.
As \fBage\fR increases, many personal statistics degenerate.
.sh "Character Types"
Character statistics are rolled randomly from the above list, according
to character type.  The types are as follows:
.TP 1.5i
.I magic user
strong in \fBmagic level\fR and \fBbrains\fR, weak in other areas.
Must rely on wits and magic to survive.
.TP 1.5i
.I fighter
good in \fBstrength\fR and \fBenergy level\fR, fairly good in other areas.
This adds up to a well-equipped fighter.
.TP 1.5i
.I elf
very high \fBquickness\fR and above average \fBmagic level\fR are
\fBelves'\fR selling points.
.TP 1.5i
.I dwarf
very high \fBstrength\fR and \fBenergy level\fR, but with a tendency to
be rather slow and not too bright.
.TP 1.5i
.I halfling
rather quick and smart, with high \fBenergy level\fR, but poor in
\fBmagic\fR and \fBstrength\fR.
Born with some \fBexperience\fR.
.TP 1.5i
.I experimento
very mediocre in all areas.  However, the \fBexperimento\fR
may be placed almost anywhere within the playing grid.
.PP
The possible ranges for starting statistics are summarized in
the following table.
.sp
.PP
.TS
center, box;
l | c c c c c c
l | c c c c c c.
\fBType	Strength	Quickness	Mana	Energy	Brains	Magic\fR
_
\fBMagic User\fR	20-25	30-35	50-100	30-45	60-85	5-9
\fBFighter\fR	40-55	30-35	30-50	45-70	25-45	3-6
\fBElf\fR	35-45	28-38	45-90	30-50	40-65	4-7
\fBDwarf\fR	50-70	25-30	25-45	60-100	20-40	2-5
\fBHalfling\fR	20-25	34	25-45	55-90	40-75	1-4
\fBExperimento\fR	25	27	100	35	25	2
.TE
.sp
.PP
Not only are the starting characteristics different for the different
character types, the characteristics progress at different rates for the
different types as the character goes up in \fBlevel\fR.  Experimentoes'
characteristics progress randomly as one of the other types.
The progression as characters increase in
\fBlevel\fR is summarized in the following table.
.PP
.TS
center, box;
l | c c c c c
l | n n n n n.
\fBType	Strength	Mana	Energy	Brains	Magic\fR
_
\fBMagic User\fR	2.0	75	20	6	2.75
\fBFighter\fR	3.0	40	30	3.0	1.5
\fBElf\fR	2.5	65	25	4.0	2.0
\fBDwarf\fR	5	30	35	2.5	1
\fBHalfling\fR	2.0	30	30	4.5	1
.TE
.sp
.PP
Character types are identified by numeric values as follows:
.sp
1:  \fBMagic User\fR    2:  \fBFighter\fR    3:  \fBElf\fR    4:  \fBDwarf\fR\ \ \ \ 
5:  \fBHalfling\fR    6:  \fBExperimento\fR
.PP
Characters with one or more \fBcrowns\fR
are designated as higher types.
\fBKings\fR have ten added to their type;  members of the
\fBcouncil of the wise\fR have twenty added to their type.
\fBValar\fR are type 99, and \fBex-valar\fR are type 90.
.sh "Spells"
During the course of the game, the player may exercise his/her particular
magic powers.  These cases are described below.
.TP 1.5i
.I cloak
.I Magic level necessary:
20 (plus level 7)
.br
.I Mana used:
35 plus 3 per rest period
.br
Used during normal play.  Prevents monsters from finding the character,
as well as hiding the player from other players.  His/her coordinates
show up as `?' in the \fBplayers\fR option.  Players cannot collect
\fBmana\fR, find trading posts, or discover the \fBgrail\fR while cloaked.
Calling a monster uncloaks, as well as choosing this option while cloaked.
.br
.TP 1.5i
.I teleport
.I Magic level necessary:
40 (plus level 12)
.br
.I Mana used:
30 per 75 moved
.br
Used during normal play.  Allows the player too move with much more freedom
than with the \fBmove\fR option, at the price of expending mana.
The maximum distance possible to move is based upon \fBlevel\fR
and \fBmagic level\fR.
.TP 1.5i
.I power blast
.I Magic level necessary:
none
.br
.I Mana used:
5 times \fBlevel\fR
.br
Used during inter-terminal battle.  Damage is based upon \fBmagic level\fR
and \fBstrength\fR.
Hits much harder than a normal hit.
.TP 1.5i
.I all or nothing
.I Magic level necessary:
none
.br
.I Mana used:
1
.br
Used while combatting monsters.
Has a 25% chance of working.  If it works it hits the monster just enough
to kill it.  If it fails, it doesn't hit the monster, and doubles the
monster's \fBquickness\fR and \fBstrength\fR.
Paralyzed monsters wake up much quicker as a result of this spell.
.TP 1.5i
.I magic bolt
.I Magic level necessary:
5
.br
.I Mana used:
variable
.br
Used while combatting monsters.  Hits the monster based upon the amount
of \fBmana\fR expended and \fBmagic level\fR.
Guaranteed to hit at least 10 per \fBmana\fR.
.TP 1.5i
.I force field
.I Magic level necessary:
15
.br
.I Mana used:
30
.br
Used during monster combat.  Throws up a shield to protect from damage.
The shield is added to actual energy level, and is a fixed number, based
upon maximum energy.  Normally, damage occurs first to the shield, and
then to the players actual \fBenergy level\fR.
.TP 1.5i
.I transform
.I Magic level necessary:
25
.br
.I Mana used:
50
.br
Used during monster combat.  Transforms the monster randomly into one
of the other 100 monsters from the monster file.
.TP 1.5i
.I increase might
.I Magic level necessary:
35
.br
.I Mana used:
75
.br
Used during combat with monsters.  Increases strength up to a certain maximum.
.TP 1.5i
.I invisibility
.I Magic level necessary:
45
.br
.I Mana used:
90
.br
Used while fighting monsters.  Makes it harder for the monster to hit,
by temporarily increasing the player's \fBquickness\fR.
This spell may be thrown several times, but a maximum level will be reached.
.TP 1.5i
.I transport
.I Magic level necessary:
60
.br
.I Mana used:
125
.br
Used during monster combat.  Transports the monster away from the
player.  Success is based upon player's \fBmagic\fR and \fBbrains\fR,
and the monster's \fBexperience\fR.
If it fails the player is transported instead.  60% of the time, the monster
will drop any treasure it was carrying.
.TP 1.5i
.I paralyze
.I Magic level necessary:
75
.br
.I Mana used:
150
.br
Used during monster combat.  ``Freezes'' the monster by putting its
\fBquickness\fR slightly negative.  The monster will slowly wake up.
Success is based upon player's \fBmagic\fR and the monster's \fBexperience\fR.
If it fails, nothing happens.
.TP 1.5i
.I specify
.I Magic level necessary:
none
.br
.I Mana used:
1000
.br
Used during monster combat only by \fBvalar\fR or \fBcouncil of the wise\fR.
Allows the player to pick which monster to fight.
.sh "Monsters"
Monsters get bigger as one moves farther from the origin (0,0).  Rings of
distance 125 from the origin determine the size.  A monster's
\fBexperience, energy level\fR, and \fBbrains\fR are multiplied by the size.
\fBStrength\fR is increased 50% per size over one, and \fBquickness\fR
remains the same, regardless of size.
.PP
Also, meaner monsters are to be found as one progress farther out
from the origin.  Monsters also may flock.  The percent chance of that
happening is designated as \fIflock%\fR in the monster listing.
Monsters outside the first ring
may carry treasure, as determined by their treasure type.
Flocking monsters and bigger monsters increase the chances of treasure.
.PP
Certain monsters have special abilities;  they are as follows:
.TP 1.5i
.I Unicorn
can only be subdued if the player is in possession of a \fBvirgin\fR.
.TP 1.5i
.I Modnar
has random characteristics, including treasure type.
.TP 1.5i
.I Mimic
will pick another name from the list of monsters in order to confuse.
.TP 1.5i
.I Dark Lord
very nasty person.  Does not like to be hit (especially nicked),
and many spells do not work well against him.
One can always \fBevade\fR from the \fBDark Lord\fR.
.TP 1.5i
.I Leanan-Sidhe
also a very nasty person.  She will permanently sap \fBstrength\fR from someone.
.TP 1.5i
.I Saruman
wanders around with \fBWormtongue\fR, who can steal a \fBpalantir\fR.
Also, \fBSaruman\fR may turn a player's gems into gold pieces,
or scramble her/his stats.
.TP 1.5i
.I Thaumaturgist
can transport a player.
.TP 1.5i
.I Balrog
inflicts damage by taking away \fBexperience\fR, not \fBenergy\fR.
.TP 1.5i
.I Vortex
may take some \fBmana\fR.
.TP 1.5i
.I Nazgul
may try to steal a \fBring\fR or neutralize part of one's \fBbrains\fR.
.TP 1.5i
.I Tiamat
may take half a players \fBgold\fR and \fBgems\fR and escape.
.TP 1.5i
.I Kobold
may get nasty and steal one gold piece and run away.
.TP 1.5i
.I Shelob
may bite, inflicting the equivalent of one \fBpoison\fR.
.TP 1.5i
.I Assorted Faeries
These are killed if attacking someone carrying \fBholy water\fR.
These are \fBCluricaun, Fir Darrig, Fachan, Ghille Dhu, Bogle, Killmoulis\fR,
and \fBBwca\fR.
.TP 1.5i
.I Lamprey
may bite, inflicting 1/2 of a \fBpoison\fR.
.TP 1.5i
.I Shrieker
will call one of its (much bigger) buddies if picked upon.
.TP 1.5i
.I Bonnacon
will become bored with battle, fart devastatingly, and run off.
.TP 1.5i
.I Smeagol
will try to steal a \fBring\fR from a player, if given the chance.
.TP 1.5i
.I Succubus
may inflict damage through a \fBforce field\fR.
This subtracts from \fBenergy level\fR instead of any shield the player
may have thrown up.
This is a very easy way to die.
.TP 1.5i
.I Cerberus
loves metal and will steal all the metal treasures from a player if able.
.TP 1.5i
.I Ungoliant
can bite and poison.  This inflicts five \fBpoisons\fR, and also takes
one from the player's \fBquickness\fR.
.TP 1.5i
.I Jabberwock
may tire of battle, and leave after calling one of his bigger friends
(\fBJubjub Bird\fR or \fBBandersnatch\fR).
.TP 1.5i
.I Morgoth
actually \fBModnar\fR, but reserved for \fBcouncil of the wise, valar\fR,
and \fBex-valar\fR.
Fights with \fBMorgoth\fR end when either he or the player dies.
His characteristics are calculated based upon the player's.
The player is given the chance to ally with him.  No magic, except
\fBforce field\fR works when battling \fBMorgoth\fR.
.TP 1.5i
.I Troll
may regenerate its \fBenergy\fR and \fBstrength\fR while in battle.
.TP 1.5i
.I Wraith
may make a player blind.
.sh "Treasures"
Various treasure types are as follows:
.TP 1.5i
\fBType zero\fR
.I none
.TP 1.5i
\fBType one\fR
.I power booster
\- adds mana.
.br
.I druid
\- adds experience.
.br
.I holy orb
\- subtracts 0.25 sin.
.TP 1.5i
\fBType two\fR
.I amulet
\- protects from cursed treasure.
.br
.I holy water
\- kills \fBassorted faeries\fR.
.br
.I hermit
\- reduces \fBsin\fR by 25% and adds some \fBmana\fR.
.TP 1.5i
\fBType three\fR
.I shield
\- adds to maximum \fBenergy level\fR
.br
.I virgin
\- used to subdue a \fBunicorn\fR, or to give much \fBexperience\fR
(and some \fBsin\fR).
.br
.I athelas
\- subtracts one \fBpoison\fR.
.TP 1.5i
.B Type four (scrolls)
.I shield
* \- throws a bigger than normal \fBforce field\fR.
.br
.I invisible
* \- puts the finder's \fBquickness\fR to one million.
.br
.I ten fold strength
* \- multiplies finder's strength by ten.
.br
.I pick monster
\- allows finder to pick next monster to battle.
.br
.I general knowledge
\- adds to finder's \fBbrains\fR and \fBmagic level\fR.
.PP
All the scrolls except \fBgeneral knowledge\fR
automatically call a monster.  Those that are marked with a * preserve any
spells that were already in effect.  Those that call monsters are only in
effect while in battle.
.TP 1.5i
.B Type five
.I dagger
\- adds to \fBstrength\fR.
.br
.I armour
\- same as a \fBshield\fR, but bigger.
.br
.I tablet
\- adds brains.
.TP 1.5i
.B Type six
.I priest
\- rests to maximum; adds 
\fBmana, brains\fR;  and halves \fBsin\fR.
.br
.I Robin Hood
\- increases \fBshield\fR and adds permanently to \fBstrength\fR.
.br
.I axe
\- like \fBdagger\fR, but bigger.
.TP 1.5i
.B Type seven
.I charm
\- protects from cursed treasure (used before \fBamulet\fR);  used in
conjunction with \fBblessing\fR to battle \fBDark Lord\fR.
.br
.I Merlyn
\- adds \fBbrains, magic\fR, and \fBmana\fR.
.br
.I war hammer
\- like an \fBaxe\fR, but bigger.
.TP 1.5i
.B Type eight
.I healing potion
\- sets \fBpoison\fR to -2, or subtracts two from \fBpoison\fR,
whichever is better.
.br
.I transporter
\- allows finder to move anywhere.
.br
.I sword
\- like a \fBwar hammer\fR, but bigger.
.TP 1.5i
.B Type nine
.I golden crown
\- allows the player to become \fBking\fR, by going to (0,0).
.br
.I blessing
\- cuts \fBsin\fR to 1/3, adds \fBmana\fR, rests to maximum, kills
\fBDark Lord\fR with a \fBcharm\fR, and gives bearer first hit on all monsters.
.br
.I quicksilver
\- adds to \fBquickness\fR.
.TP 1.5i
.B Type ten
.I elven boots
\- adds permanently to \fBquickness\fR.
.TP 1.5i
.B Type eleven
.I palantir
\- allows one to see all the other players;  used by
\fBcouncil of the wise\fR to seek the \fBgrail\fR.
.TP 1.5i
.B Type twelve/thirteen
.I ring
\- allows one to hit much harder in battle, etc.
.PP
Any treasure type 10-13 monsters may carry a type nine treasure instead.
.PP
A monster may also be carrying \fBgold\fR or \fBgems\fR.
These are used at \fBtrading posts\fR to buy things.  A \fBgem\fR
is worth 1000 gold pieces.  Too much \fBgold\fR
will slow a player down.  One may carry 1000 plus 200 per
\fBlevel\fR of \fBgold\fR.
A \fBgem\fR weighs one half a gold piece.
Monsters of treasure type 7 or higher may carry \fBgems\fR.
.PP
The chance of a cursed treasure is based upon treasure type.
The more valuable treasures have a greater chance of being cursed.
A cursed treasure knocks \fBenergy level\fR very low, and adds 0.25
\fBpoison\fR.
.sh "Rings"
\fBRings\fR are only carried by \fBnazguls\fR and \fBDark Lord\fR.
They come in four different flavors.
All \fBrings\fR rest the player to maximum and cause him/her to hit much harder
in battle with monsters (assuming one has chosen to use the
\fBring\fR for battle.)
.PP
Two types of \fBrings\fR are cursed and come either from \fBnazguls\fR
or \fBDark Lord\fR.
After a few times of using these types, the player falls
under the control of the \fBring\fR, and strange, random things will occur.
Eventually, the player dies, and gives his/her name to a monster
on the file.
Dying before the \fBring\fR is used up also renames the monster.
.PP
The two remaining types of \fBrings\fR are much more benign.
The one from a \fBnazgul\fR
is good for a limited number of battle rounds, and will save
the player from death if it was being used when he/she died.
The one from \fBDark Lord\fR is the same, except that it never is used up.
\fBRings\fR disappear after saving someone from death.
In general, cursed \fBrings\fR occur much more often than normal ones.
It is usually not a good idea to pick one up.
The only way to get rid of a \fBring\fR is to have a monster steal it.
.sh "King"
A player may become \fBking\fR by finding a \fIcrown\fR
and going to (0,0).  Players must have a \fBlevel\fR
in the range of 10 to 1000 to be able to find a \fIcrown\fR.
.PP
Once a player is king, he/she may do certain things while in 
the Lord's Chamber (0,0).  These fall under the \fBdecree\fR (`0') option.
.TP 1.5i
.I census
Lists all players on file, whether playing or not.
.TP 1.5i
.I transport
This is done to another player.  It randomly moves the affected
player about.  A \fBcharm\fR protects from transports.
.TP 1.5i
.I curse
This is done to another player.  It is analogous to cursed treasure,
but worse.  It inflicts two \fBpoison\fR, knocks \fBenergy level\fR
very low, and degrades the maximum energy.  It also removes a \fBcloak\fR.
A \fBblessing\fR protects from king's curses.
.TP 1.5i
.I energy void
The king may put as many of these (within reason) scattered about
his/her kingdom as he/she pleases.
If a player hits one, he/she loses \fBmana, energy\fR, and \fBgold\fR.
The energy void disappears after being hit.
.TP 1.5i
.I bestow
This is also done to another player.  The king may
wish to reward one or more loyal subjects by sharing his/her
riches (\fBgold\fR).  Or it is a convenient way to dispose of some unwanted
deadweight.
.TP 1.5i
.I collect taxes
Everyone pays 10% tax on all \fBgold\fR collected, regardless of the
existence of a \fBking\fR.
The king may collect this amount with this option.
.PP
The \fBking\fR may also \fBteleport\fR
anywhere for free by using the origin as a starting place.
.sh "Special Places"
Certain regions of the playing grid have different names.
In general, this is only to give the player some idea of
his/her present location.  Some special places do exist.
.TP 1.5i
.I Trading Posts
These are located at |x| == |y| == n*n*100 for n = 1, 2...1000.
Trading posts farther out have more things for sale.
Be careful about cheating merchants there, as they have short
tempers.
Merchants are dishonest about 5% of the time.
.TP 1.5i
.I Lord's Chamber
This is located at (0,0).  Only players with
\fIcrowns\fR may enter.
.TP 1.5i
.I Point of No Return
This is located beyond 1.2e+6 in any direction.
The only way to return from here is a \fItransporter\fR or to have a
\fBvalar\fR relocate the player.
.TP 1.5i
.I Dead Marshes
This is a band located fairly distant from the origin.  The first
fourteen monsters (water monsters) can normally only be found here.
.TP 1.5i
.I Valhala
This place is where the \fBvalar\fR resides.
It is associated with no particular coordinate on the playing grid.
.TP 1.5i
.I Wormholes
At fixed locations on the grid are several holes to underground
defects in the playing area.
Sixty-four chambers exist;  sixteen of which open to the outside world.
While in the wormholes, one move by going forward, backward,
left, or right.
One can always undo a move by going in the opposite direction.
The wormholes are mappable.
For example, to move from the first wormhole (at (-400,0)) to
the second wormhole, type `LL'.
.sh "Miscellaneous"
There are several bits of trivial knowledge which fall under this category.
.PP
Once a player reaches \fBlevel\fR 5, the game will start to time out
waiting for input.
This is to try to keep the game a bit faster paced.
The game may be temporarily suspended (when not fighting) by typing
Control-S;  any normal command re-starts it.
.PP
A \fIguru\fR will never be disgusted with your \fBsins\fR
if they are less than one.
.PP
A \fIphysician\fR wants half of a player's \fBgold\fR
to be happy.  Offering more than one has, or a negative amount
will anger the \fIphysician\fR, who will make the player worse (add one
\fBpoison\fR).
.PP
The \fBHoly Grail\fR does little for those who are not ready to behold it.
Whenever anyone finds it, it moves.
It is always located within 1e+6 in any compass direction of the origin.
.PP
There is a maximum amount of \fBmana\fR and \fBcharms\fR
a player may posses, based upon \fBlevel\fR.
.I Quicksilver
is always limited to to a maximum of 99.
.PP
.I Books
bought at a \fBtrading post\fR increase \fBbrains\fR,
based upon the number bought.
It is unwise, however to buy more than 1/10 of one's \fBlevel\fR
in books at a time.
.PP
Players over level 10000 are automatically retired.
.PP
A \fIblindness\fR goes away in random time.
.sh "Inter-terminal Battle"
When two player's coordinates correspond, they may engage in battle.
In general, the player with the highest \fBquickness\fR
gets the first hit.
If the two players are severely mis-matched, the stronger player
is drastically handicapped for the battle.
In order to protect from being stuck in an infinite loop,
the player waiting for response may time out.  Options for battle are:
.TP 1.5i
.I fight
Inflicts damage upon other person.
.TP 1.5i
.I run away
Escape from battle.  Has a 75% chance of working.
.TP 1.5i
.I power blast
Battle spell.
.TP 1.5i
.I luckout
One-time chance to try to win against the foe.  Has a 10% chance of working.
.PP
Sometimes waits for the other player may be excessive, because
he/she may be battling a monster.  Upon slaying a player in battle
the winner gets the other's \fBexperience\fR and treasures.
\fBRings\fR do not work for inter-terminal battle.
.sh "Council of the Wise, Valar"
A player automatically becomes a member of the \fBcouncil of the wise\fR
upon reaching level 3000.  Members of the council cannot have \fBrings\fR.
Members of the council have a few extra options which they can exercise.
These fall under the \fBintervention\fR (`8') option.
One is to \fIheal\fR another player.  This is just a quick way for
that player to be rested to maximum and lose a little \fBpoison\fR.
The main purpose in life for members of the council is to seek the
\fBHoly Grail\fR.
This is done with a \fBpalantir\fR under the
.I seek grail
option.  The distance cited by the seek is accurate within 10%, in order
not to make it too easy to find the grail.  Seeking costs 1000 \fBmana\fR.
A player must have infintesimally small
\fBsin\fR, or else it's all over upon finding the grail.
In order to help members of the council on their quest, they may
.I teleport
with greater ease.
.PP
Upon finding the grail, the player advance to position of \fBvalar\fR.
He/she may then exercise more and niftier options under
.I intervention.
These include all of the council members' options plus the
ability to move other players about, bless them, and throw monsters at them.
\fBValar\fR are essentially immortal, but are actually given five lives.
If these are used up, the player is left to die, and becomes an
\fBex-valar\fR.
\fBValar\fR cannot
.I move, teleport,
or call monsters.
Any monsters which a \fBvalar\fR encounters are based upon his/her size.
Only one valar may exists at a time.
A player replaces the exiting valar upon finding the grail.
The valar is then bumped back to the council of the wise.
.SH BUGS
The handling of incorrectly exited characters is a kludge.
The screen is set up assuming a 24 by 80
character screen;  no attempt was made to provide otherwise.
If any of the data items get too big for the allotted space on the screen,
no guarantees are made about what will happen.
