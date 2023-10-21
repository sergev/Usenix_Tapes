*	The first three entries in the text section must be the null
*	string, a blank line, and the "Ok." respose - they also
*	exist in the run-time program, for speed's sake.
TEXT	SOK!
TEXT	BLANK
TEXT	OK!
TEXT	LOGON
	Welcome to the *new* UNIX Adventure!  Say "NEWS" to get up-to-date
	game details.
 
TEXT	NEWSDATA
 
	This is the brand-spanking-new version of UNIX Adventure.
	The cave is essentially stable at this point except for bug
	fixes being done as needed.  The cave is almost twice as big as
	before, and has lots of new creatures in it - have fun!
 
	Please contact the local game guru if anything weird happens (that
	is, anything weird that looks like it shouldn't happen).  In
	particular, if you ever see the message "Glitch!", please save
	the printout for error analysis if that's possible. Credit goes
	to David Platt for the design and development of this new version
	of Adventure for the Xerox SIGMA-9 and other Honeywell systems.
	Ken Wellsch designed and wrote the C version for running on UNIX
	systems which uses the original database.
 
TEXT	INTRO
	Somewhere nearby is Colossal Cave, where others have found fortunes in
	treasure and gold, though it is rumored that some who enter are never
	seen again.  Magic is said to work in the cave.  I will be your eyes
	and hands.  Direct me with commands of 1 or 2 words.  I should warn
	you that I look at only the first six letters of each word.
	(Should you get stuck, type "HELP" for some general hints.  For infor-
	mation on how to end your adventure, etc., type "INFO".)
	/			      - - -
	If you have any problems, please contact the local game guru.
 
TEXT	DWARFBLOCK
	A little dwarf with a big knife blocks your way.
TEXT	FIRSTDWARF
 
	A little dwarf just walked around a corner, saw you, threw a little
	axe at you which missed, cursed, and ran away.
TEXT	DWARFHERE
 
	There is a threatening little dwarf in the room with you!
TEXT	DWARVESHERE
 
	There are # threatening little dwarves in the room with you!
TEXT	KNIFETHROWN
	One sharp nasty knife is thrown at you!
TEXT	KNIVESTHROWN
	# nasty sharp knives are thrown at you!
TEXT	KNIVESMISS
	None of them hit you!
TEXT	KNIFEGOTYOU
	One of them gets you!
TEXT	SAYSPLUGH
	A hollow voice says "Plugh".
TEXT	NOCANGO
	There is no way to go that direction.
TEXT	OOF!
	You have run into a wall of rock and can go no further in this
	direction.
TEXT	IAMUNSURE
	I am unsure how you are facing.  Use compass points or nearby objects.
TEXT	INFROMOUT
	I don't know in from out here.  Use compass points or name something
	in the general direction you want to go.
TEXT	CANTAPPLY
	I don't know how to apply that word here.
TEXT	HUH??
	I don't understand that!
TEXT	IAMGAME
	I'm game.  Would you care to explain how?
TEXT	NOCANLOOK
	Sorry, but I am not allowed to give more detail.  I will repeat the
	long description of your location.
TEXT	ITISNOWDARK
	It is now pitch dark.  If you proceed you will likely fall into a pit.
TEXT	WMEANSWEST
	If you prefer, simply type W rather than WEST.
TEXT	BIRDHINT?
	Are you trying to catch the bird?
TEXT
	The bird is frightened right now and you cannot catch it no matter
	what you try.  Perhaps you might try later.
TEXT	GETPASTSNAKE
	Are you trying to somehow deal with the snake?
TEXT
	You can't kill the snake, or drive it away, or avoid it, or anything
	like that.  There is a way to get by, but you don't have the necessary
	resources right now.
TEXT	WANTTOQUIT
	Do you really want to quit now?
TEXT	CRUNCH
	You fell into a pit and broke every bone in your body!
TEXT	YOUHAVEIT
	You are already carrying it!
TEXT	BESERIOUS
	You can't be serious!
TEXT	BIRDISSCARED
	The bird was unafraid when you entered, but as you approach it becomes
	disturbed and you cannot catch it.
TEXT	NEEDCAGE
	You might be able to catch the bird, but you could not carry it.
TEXT	NOTCARRYING
	You aren't carrying it!
TEXT	IDONTSEE
	I see no # here.
TEXT	BIRD..SNAKE
	The little bird attacks the green snake, and in an astounding flurry
	drives the snake away.
TEXT	NEEDKEYS
	You have no keys!
TEXT	NOLOCK
	It has no lock.
TEXT	CANTLOCKIT
	I don't know how to lock or unlock such a thing.
TEXT	ITWASLOCKED
	It was already locked.
TEXT	GRATELOCKED
	The grate is now locked.
TEXT	GRATEUNLOCKED
	The grate is now unlocked.
TEXT	GRATE.STUCK
	The lock seems to be stuck - the key refuses to turn!
TEXT	WASUNLOCKED
	It was already unlocked.
TEXT	NOLIGHTHERE
	You have no source of light.
TEXT	LAMPNOWON
	Your lamp is now on.
TEXT	LAMPNOWOFF
	Your lamp is now off.
TEXT	BEAR..CHAIN
	There is no way to get past the bear to unlock the chain, which is
	probably just as well.
TEXT	NOTHING
	Nothing happens.
TEXT	WHERE?
	Where?
TEXT	PACIFIST
	There is nothing here to attack.
TEXT	BIRDISDEAD
	The little bird is now dead.  Its body disappears.
TEXT	CANTKILLSNAKE
	Attacking the snake both doesn't work and is very dangerous.
TEXT	KILLEDDWARF
	You killed a little dwarf.
TEXT	DWARFDODGES
	You attack a little dwarf, but he dodges out of the way.
TEXT	DWARFSTABS
	You attack a little dwarf, but he dodges out of the way and stabs
	you with his nasty sharp knife!
TEXT	WITHWHAT?
	With your bare hands??
TEXT	MAGICKWORD
	Good try, but that is an old worn-out magic word.
TEXT	HELPDATA
 
	I know of places, actions, and things.  Most of my vocabulary
	describes places and is used to move you there.  To move, try words
	like FOREST, BUILDING, DOWNSTREAM, ENTER, EAST, WEST, NORTH, SOUTH,
	UP, or DOWN.  I know about a few special objects, like a black rod
	hidden in the cave.  These objects can be manipulated using some of
	the action words that I know.  Usually you will need to give both the
	object and action words (in either order), but sometimes I can infer
	the object from the verb alone.  Some objects also imply verbs; in
	particular, "INVENTORY" implies "TAKE INVENTORY", which causes me to
	give you a list of what you're carrying.  The objects have side
	effects; for instance, the rod scares the bird.  Usually people having
	trouble moving just need to try a few more words.  Usually people
	trying unsuccessfully to manipulate an object are attempting something
	beyond their (or my!) capabilities and should try a completely
	different tack.  To speed the game you can sometimes move long
	distances with a single word.  For example, "BUILDING" usually gets
	you to the building from anywhere above ground except when lost in the
	forest.  Also, note that cave passages turn a lot, and that leaving a
	room to the north does not guarantee entering the next from the south.
	Good luck!
 
TEXT	MISSES
	It misses!
TEXT	GETSYOU
	It gets you!
TEXT	UNLOCKKEYS
	You can't unlock the keys.
TEXT	YOUDIDNTMOVE
	You have crawled around in some little holes and wound up back in the
	main passage.
TEXT	WHEREISCAVE
	I don't know where the cave is, but hereabouts no stream can run on
	the surface for long.  I would try the stream.
TEXT	NEEDDETAIL
	I need more detailed instructions to do that.
TEXT	CANTFIND
	I can only tell you what you see as you move about and manipulate
	things.  I cannot tell you where remote things or places are.
TEXT	NOCOMPRENDE
	I don't know the word "#".
TEXT	WHAT?
	Huh??
TEXT	LOOKINGCAVE
	Are you trying to get into the cave?
TEXT	GRATENEEDSKEYS
	The grate is very solid and has a hardened steel lock.  You cannot
	enter without a key, and there are no keys nearby.  I would recommend
	looking elsewhere for the keys.
TEXT	INFOREST
	The trees of the forest are large hardwood oak and maple, with an
	occasional grove of pine or spruce.  There is quite a bit of under-
	growth, largely birch and ash saplings plus nondescript bushes of
	various sorts.  This time of year visibility is quite restricted by
	all the leaves, but travel is quite easy if you detour around the
	spruce and berry bushes.
TEXT	NO.TREES.HERE	{ Bug fix. by Jim@ukc }
	I can't see any trees around here.
TEXT	HITHERE
	Would you like instructions?
TEXT	NEEDSHOVEL
	Digging without a shovel is quite impractical.  Even with a shovel
	progress is unlikely.
TEXT	NEEDDYNAMITE
	Blasting requires dynamite.
TEXT	IMCONFUSED
	I'm as confused as you are.
TEXT	THISISMIST
	Mist is a white vapor, usually water, seen from time to time in
	caverns.  It can be found anywhere but is frequently a sign of a deep
	pit leading down to water.
TEXT	FEETAREWET
	Your feet are now wet.
TEXT	BLEAH
	Yeetttch!  I think I just lost my appetite.
TEXT	URRP
	Thank you, it was delicious!
TEXT	SLURP
	You have taken a drink from the stream.  The water tastes strongly of
	minerals, but is not unpleasant.  It is extremely cold.
TEXT	SALT.H20.BAD
	I'm afraid that all that's available here is salt water, which
	isn't good for anything much... you'de better try elsewhere.
TEXT	WATERGONE
	The bottle of water is now empty.
	You convulse and fall to the ground, screaming and writhing in pain.
TEXT	RUBLAMP
	Rubbing the electric # is not particularly rewarding.  Anyway,
	nothing exciting happens.
TEXT	PECULIAR
	Peculiar.  Nothing unexpected happens.
TEXT	REPULSIVE
	That's a repulsive idea!
TEXT	POURWATER
	Your bottle is empty and the ground is wet.
TEXT	CANTPOUR
	You can't pour that.
TEXT	WATCHIT
	Watch it!
TEXT	WHICHWAY
	Which way should I #?
TEXT	YOUAREDEAD
	Oh dear, you seem to have gotten yourself killed.  I might be able to
	help you out, but I've never really done this before.  Do you want me
	to try to reincarnate you?
TEXT
	All right.  But don't blame me if something goes wr......
	/		   --- poof!! ---
	You are engulfed in a cloud of orange smoke.  Coughing and gasping,
	you emerge from the smoke and find....
TEXT
	Tsk, tsk - you did it again!  Remember - you're only human, and you
	don't have as many lives as a cat!  (at least, I don't think so...)
	That's twice you've ended up dead - want to try for three?
TEXT
	As you wish.  Hang on for just a second while I fire up my thurible...
	>foof!<    An immense cloud of orange smoke fills the air.  As it
	clears, you find that once again....
TEXT
	You clumsy oaf, you've done it again!  I don't know how long I can
	keep this up.  Do you want me to try reincarnating you again?
TEXT
	Okay, now where did I put my orange smoke?....  >poof!<
	Everything disappears in a dense cloud of orange smoke.
TEXT
	Now you've really done it!  I'm out of orange smoke!  You don't expect
	me to do a decent reincarnation without any orange smoke, do you?
TEXT
	Yes....  well, that's the kind of blinkered, Philistine pig-ignorance
	that I've come to expect from you mortals.  You sit there on your
	loathsome, spotty behind, squeezing blackheads and not caring a
	thinker's damn about the struggling cave-sprite, you simpering,
	whining, mouldy-faced heap of parrot droppings!  If you're so
	smart, then you can just reincarnate yourself, because quite
	frankly I'm as mad as hell and I'm not going to take this
	anymore - I'm leaving!!!!
 
TEXT	SNIDELEY
TEXT	CANTGOBACK
	Sorry, but I no longer seem to remember how it was you got here.
TEXT	ARMSAREFULL
	You can't carry anything more.  You'll have to drop something first.
TEXT	CANTPASSLOCK
	You can't go through a locked steel grate!
TEXT	ITISHERENOW
	I believe what you want is right here with you.
TEXT	DONTFITSLIT
	You don't fit through a two-inch slit!
TEXT	TRYTHEBRIDGE
	I respectfully suggest you go across the bridge instead of jumping.
TEXT	NOWAYACROSS
	There is no way across the fissure.
TEXT	ARMSAREEMPTY
	You're not carrying anything.
TEXT	YOUNOWHAVE
	You are currently holding the following:
TEXT	BIRDSEED
	It's not hungry (it's merely pinin' for the fjords).  Besides, you
	have no bird seed.
TEXT	SNAKE..BIRD
	The snake has now devoured your bird.
TEXT	SNAKEWONTEAT
	There's nothing here it wants to eat (except perhaps you).
TEXT	FED..DWARF
	You fool, dwarves eat only coal!  Now you've made him *really* mad!!
TEXT	NOWAYTOCARRY
	You have nothing in which to carry it.
TEXT	BOTTLEWASFULL
	Your bottle is already full.
TEXT	NOTHING2FILL
	There is nothing here with which to fill the bottle.
TEXT	BOTTLE..H20
	Your bottle is now full of water.
TEXT	BOTTLE..OIL
	Your bottle is now full of oil.
TEXT	CANTFILLTHAT
	You can't fill that.
TEXT	HAH!
	Don't be ridiculous!
TEXT	DOORNEEDSOIL
	The door is extremely rusty and refuses to open.
TEXT	OIL..PLANT
	The plant indignantly shakes the oil off its leaves and asks, "Water?"
TEXT	HINGES..RUST
	The hinges are quite thoroughly rusted now and won't budge.
TEXT	OIL..DOOR
	The oil has freed up the hinges so that the door will now move,
	although it requires some effort.
TEXT	GET..PLANT
	The plant has exceptionally deep roots and cannot be pulled free.
TEXT	NO..KNIVES
	The dwarves' knives vanish as they strike the walls of the cave.
TEXT	WONT..FIT
	Something you're carrying won't fit through the tunnel with you.
	You'd best take inventory and drop something.
TEXT	CLAM.2.BIG
	You can't fit this five-foot clam through that little passage!
TEXT	OYSTER.2.BIG
	You can't fit this five-foot oyster through that little passage!
TEXT	DROP.THE.CLAM
	I advise you to put down the clam before opening it.  >strain!<
TEXT	DROP.THE.OYSTER
	I advise you to put down the oyster before opening it.  >wrench!<
TEXT	NEED.TRIDENT
	You don't have anything strong enough to open the clam.
TEXT	NEED.TRIDNT2
	You don't have anything strong enough to open the oyster.
TEXT	CLAM..OPENED
	A glistening pearl falls out of the clam and rolls away.  Goodness,
	this must really be an oyster.  (I never was very good at identifying
	bivalves.)  Whatever it is, it has now snapped shut again.
TEXT	OYSTER..OPENED
	The oyster creaks open, revealing nothing but oyster inside.  It
	promptly snaps shut again.
TEXT	CRAWL..CAVEIN
	You have crawled around in some little holes and found your way
	blocked by a recent cave-in.  You are now back in the main passage.
TEXT	RUSTLING
	There are faint rustling noises from the darkness behind you.
TEXT	PIRATE..ZOTZ
 
	Out from the shadows behind you pounces a bearded pirate!  "Har, har,"
	he chortles, "I'll just take all this booty and hide it away with me
	chest deep in the maze!"  He snatches your treasure and vanishes into
	the gloom.
TEXT	CLOSING.NOW
 
	A sepulchral voice reverberating through the cave, says, "Cave closing
	soon.  All adventurers please report to the treasure room via the
	alternate entrance to claim your treasure."
 
TEXT	GRATE.CLOSED
 
	A mysterious recorded voice groans into life and announces:
	/   "This exit is closed.  Please report to the treasure room via
	/   the alternate entrance to claim your treasure."
 
TEXT	DEAD&CLOSED
 
	It looks as though you're dead.  Well, seeing as how it's so close to
	closing time anyway, I think we'll just call it a day.
TEXT	GO.REPOSIT
 
	The sepulchral voice entones, "The cave is now closed."  As the echoes
	fade, there is a blinding flash of light (and a small puff of orange
	smoke). . . .    As your eyes refocus, you look around and find...
 
TEXT	LEAVE.BIRD
	Oh, leave the poor unhappy bird alone.
TEXT	HERESOMEWHERE
	I daresay whatever you want is around here somewhere.
TEXT	NO.CAN.GO
	I don't know how to get there from here.
TEXT	YOU.ARE.THERE
	That's where you are now!
TEXT	I.C.A.BEAR
	You are being followed by a very large, tame bear.
TEXT	INFO.2
 
	If you want to end your adventure early, say "QUIT".
	To suspend your adventure such that you can continue later, say
	"SUSPEND" (or "PAUSE" or "SAVE").  To re-start your game at a later
	time, start up a new adventure and after I say "You are standing...",
	you must say "RESTORE".  You can also name your game by saying
	"SUSPEND mine" (and "RESTORE mine") where "mine" is the name that
	you wish your suspended game to be known by (1-4 characters).
	To see what hours the cave is normally open, say "HOURS".
	To see how well you're doing, say "SCORE".  To get full credit for a
	treasure, you must have left it safely in the building, though you get
	partial credit just for locating it.  You lose points for getting
	killed, or for quitting, though the former costs you more.  There are
	also points based on how much (if any) of the cave you've managed to
	explore; in particular, there is a large bonus just for getting in (to
	distinguish the beginners from the rest of the pack), and there are
	other ways to determine whether you've been through some of the more
	harrowing sections.  If you think you've found all the treasures, just
	keep exploring for a while.  If nothing interesting happens, you
	haven't found them all yet.  If something interesting *does* happen,
	it means you're getting a bonus and have an opportunity to garner many
	more points in the master's section.  I may occasionally offer hints
	if you seem to be having trouble.  If I do, I'll warn you in advance
	how much it will affect your score.  To save paper, you may specify
	"BRIEF", which tells me never to repeat the full description of a
	place unless you explicitly ask me to by saying "LOOK".  If you
	are an experienced adventurer, you may wish to specify "FAST", which
	is like "BRIEF" but more so;  in "FAST" mode I will *never* under
	any circumstances give the full description of a place.  Finally, if
	you are in "BRIEF" or "FAST" modes, you may return to the normal mode
	of operation by saying "FULL".
 
TEXT	QUIT.NOW?
	Do you indeed wish to quit now?
TEXT	NOTHING.VASE
	There is nothing here with which to fill the vase.
TEXT	SHATTER.VASE
	The sudden change in temperature has delicately shattered the vase.
TEXT	NO.CAN.FIX
	It is beyond your power to do that.
TEXT	DUNNO.HOW
	I don't know how.
TEXT	TOO.FAR.UP
	It is too far up for you to reach.
TEXT	DWARF.POOF
	You killed a little dwarf.  The body vanishes in a cloud of greasy
	black smoke.
TEXT	KILL.OYSTER
	The shell is very strong and is impervious to attack.
TEXT	START.OVER
	What's the matter, can't you read?  Now you'd best start over.
TEXT	KILL.DRAGON
	The # bounces harmlessly off the dragon's thick scales.
TEXT	PAST.DRAGON
	The dragon looks rather nasty.  You'd best not try to get by.
TEXT	BIRD..DRAGON
	The little bird attacks the green dragon, and in an astounding flurry
	gets burnt to a cinder.  The ashes blow away.
TEXT	ON.WHAT?
	On what?
TEXT	BRIEF.OK
	Okay, from now on I'll only describe a place in full the first time
	you come to it.  To get the full description, say "LOOK".
TEXT	TROLL.DATA
	Trolls are close relatives with the rocks and have skin as tough as
	that of a rhinoceros.  The troll fends off your blows effortlessly.
TEXT	EL.CHEAPO
	The troll deftly catches the #, examines it carefully, and tosses it
	back, declaring, "Good workmanship, but it's not valuable enough."
TEXT	BOUGHTHIMOFF
	The troll catches the # and scurries away out of sight.
TEXT	TROLL.SEZ.NO
	The troll refuses to let you cross.
TEXT	NO.BRIDGE
	There is no longer any way across the chasm.
TEXT	BEAR..BRIDGE
	Just as you reach the other side, the bridge buckles beneath the
	weight of the bear, which was still following you around.  You
	scrabble desperately for support, but as the bridge collapses you
	stumble back and fall into the chasm.
TEXT	BEAR..TROLL
	The bear lumbers toward the troll, who lets out a startled shriek and
	scurries away.  The bear soon gives up the pursuit and wanders back.
TEXT	AXE..BEAR
	The axe misses and lands near the bear where you can't get at it.
TEXT	KILL..BEAR
	With what?  Your bare hands??  Against *his* bear hands???  Don't be
	ridiculous - he'd tear you to shreds!
TEXT	BEAR.PUZZLED
	The bear is confused; he only wants to be your friend.
TEXT	IT.IS.DEAD
	For crying out loud, the poor thing is already dead!
TEXT	BEAR..URRP
	The bear eagerly wolfs down your food, after which he seems to calm
	down considerably and even becomes rather friendly.
TEXT	BEAR.IS.CHAINED
	The bear is still chained to the wall.
TEXT	CHAIN.LOCKED
	The chain is still locked.
TEXT	CHAIN.UNLOCKED
	The chain is now unlocked.
TEXT	LOCK..CHAIN
	The chain is now locked.
TEXT	LOCK..CHAIN?
	There is nothing here to which the chain can be locked.
TEXT	NO.FOOD
	There is nothing here to eat.
TEXT	ILL.GIVE.HINT
	I can give you some advice that might help you solve your problem,
	but I'll have to charge you # points for it.  TANSTAAFL, y'know!
TEXT	WANT.HINT?
	Do you want the hint?
TEXT	HINT.MAZE?
	Do you need help getting out of here?
TEXT
	You can make the passages look less alike by dropping things.  You
	could then make a map that would let you find your way around.
TEXT	HINT.PLOVER?
	Are you trying to explore beyond the Plover room?
TEXT
	There is a way to explore that region without having to worry about
	falling into a pit.  None of the objects available is immediately
	useful in discovering the secret.
TEXT	HINT.WITTS?
	Do you need help getting out of here?
TEXT
	Don't go west.
TEXT	FEED..TROLL
	Gluttony is not one of the troll's vices.  Avarice, however, is.
TEXT	LAMP.IS.DIM
	Your lamp is getting dim.  You'd best start wrapping this up, unless
	you can find some fresh batteries.  I seem to recall there's a vending
	machine in the maze.  Bring some coins with you.
TEXT	LAMP.IS.DEAD
	Your lamp has run out of power.
TEXT	LAMP.DEAD!
 
	There's not much point in wandering around out here, and you can't
	explore the cave without a lamp.  So let's just call it a day.
TEXT	PIRATE.RUNS
 
	There are faint rustling noises from the darkness behind you.  As you
	turn toward them, the beam of your lamp falls across a bearded pirate.
	He is carrying a large chest.  "Shiver me timbers!" he cries, "I've
	been spotted!  I'd best hie meself off to the maze to hide me chest!"
	With that, he vanishes into the gloom.
TEXT	LAMP.BATTERIES
	Your lamp is getting dim.  You'd best go back for those batteries.
TEXT	LAMP.REFUEL
	Your lamp is getting dim.  I'm taking the liberty of replacing the
	batteries.
TEXT	LAMP.NOFUEL
	Your lamp is getting dim, and you're out of spare batteries.  You'd
	best start wrapping this up.
TEXT	MAG.DWARVISH
	I'm afraid the magazine is written in Dwarvish.
TEXT	CHEST.ELSEWHERE
	"This is not the maze where the pirate leaves his treasure chest."
TEXT	THIS.IS.A.CLUE
	Hmmm, this looks like a clue, which means it'll cost you 10 points to
	read it.  Should I go ahead and read it anyway?
TEXT	NEW.EFFECT
	It says, "There is something strange about this place, such that one
	of the words I've always known now has a new effect."
TEXT	UNCHANGED
	It says the same thing it did before.
TEXT	MACHINE.SIGN
	It reads, "Drop coins here to receive fresh batteries."
TEXT	I.DONT.UNDERSTAND
	I'm afraid I don't understand.
TEXT	DARK.ROOM
	It says, "Congratulations on bringing light into the Dark-room!"
TEXT	SHATTER.MIRROR
	You strike the mirror a resounding blow, whereupon it shatters into a
	myriad tiny fragments.
TEXT	THROW.VASE
	You have taken the vase and hurled it delicately to the ground.
TEXT	IS.THIS.OK?
	Is this acceptable?
TEXT	SUSP.DEMO
	There's no point in suspending a demonstration game.
TEXT	BROKENECK
	You are at the bottom of the pit with a broken neck.
TEXT	UNCLIMBABLE
	The dome is unclimbable.
TEXT	SNAKEBLOCKS
	You can't get past the snake.
TEXT	MISTCRAWL
	You have crawled around in a little passage north of and parallel
	to the Hall of Mists.
TEXT	PLUMMET
*
*        The following statements define the text to be used when
*        the person tries to jump over the chasm, jumps down the
*        pit, etc.  There should be one text message for every
*        reincarnation that is allowed, plus 1 (i.e., one of these
*        messages for each of the message pairs printed
*        by CORONER.
	Aaaaaaaaaiiiiiiiiiiieeeeeeeeee........               >splat<
	Hmmmm - I never saw a red flapjack before!
TEXT	PLUMMET1
	Aaaaaaaaaaaaaaaaaaahhhhhhhhhhhhhhhhhh........        >squish<
	Yetch - what a mess!
TEXT	PLUMMET2
	Gaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhh............         >crunch<
TEXT	PLUMMET3
	Haaaaaaaaaaaaaalllllllllllllllllpppppp...........      >smash<
TEXT	NOCANCLIMB
	There's nothing climbable here.
TEXT	WHAT.DO
	What do you want me to do with the #?
TEXT	MISTCRACK
	The crack is far too small for you to enter.
TEXT	PIPEFIT
	The stream flows out through a pair of 1 foot diameter sewer pipes.
	It would be advisable to leave via the exit.
TEXT	YOUDONTHAVE
	You have no #!
TEXT	NOCLIMBUP
	There's nothing to climb here.  Say "UP" or "OUT" to leave the pit.
TEXT	SHORTPLANT
	You have climbed up the plant and out of the pit.
TEXT	LONGPLANT
	You scurry up the plant and into the hole in the wall.
TEXT	CANTGETAXE
	As you approach the bear, it snarls threateningly;  you are forced
	to retreat without the #.
TEXT	DRAGON.RUG
	You can't get by the dragon to get at the rug.
TEXT	YOUSCORED
 
 
	You have scored a total of # points, out of a possible maximum of
TEXT	TOPSCORE
	# points.  During this game of Adventure, you have taken a total of
TEXT	NMOVES
	# turns.
 
 
TEXT	NOJUMPING
	There is nowhere for me to jump to.
TEXT	CLARIFY
	I'm not sure what you want me to # - please be more specific.
TEXT	DWARF.QUITS
	The dwarf quickly scuttles off into the gloom.
TEXT	SAID
	Ok - "#".
TEXT	IFYOUQUIT
	If you were to quit now, you would score a total of # points, out
TEXT	IFYOUQUIT2
	of a possible maximum of # points.
TEXT	FISH
	You are obviously a rank amateur.  Better luck next time.
TEXT	NOVICE
	Your score qualifies you as a novice-class adventurer.
TEXT	EXPERIENCED
	You have achieved the rating; "Experienced Adventurer".
TEXT	SEASONED
	You may now consider yourself a "Seasoned Adventurer".
TEXT	JUNIORMASTER
	You have reached "Junior Master" status.
TEXT	MASTER.C
	Your score puts you in Master Adventurer class C.
TEXT	MASTER.B
	Your score puts you in Master Adventurer class B.
TEXT	MASTER.A
	Your score puts you in Master Adventurer class A.
TEXT	GRAND
	All of Adventuredom gives tribute to you, Adventurer Grandmaster!
TEXT	NEED1
	You only need one more point to reach the next level of expertise!
TEXT	NEED
	To reach the next qualification level you need # more points.
TEXT	NEW.BATTERIES
	The old batteries in your lamp were pretty well shot - I've taken the
	the liberty of putting in the new ones.
TEXT	DUNNO.HAO
	I don't know how to # such a thing.
TEXT	MUSTWAIT
	I can save your Adventure for you, but if I do you'll have to wait
	at least # minutes before continuing.
TEXT	CANT.SAVE
	I'm sorry, but I can't save your game at the moment;  something
	seems to be wrong with the save file.
TEXT	NO.IMAGE
	I can't find any saved game for you to restore.
TEXT	EXPLOSION
	An explosion has destroyed the well-house during the time that your
	game was suspended.  You're going to have to start over.
TEXT	SAVE.THE.IMAGE
	Do you want me to keep the save-image?
TEXT	CONTINUE.NOW?
	Do you wish to continue with the game immediately?
TEXT	TOO.SOON
	I'm sorry - only a wizard can restart a game in less than # minutes.
TEXT	DOUSED.DWARF
	The # flies through the air and thoroughly drenches the dwarf.  He
	shakes himself off and curses violently; he *REALLY* looks angry!
TEXT	DOUSED.DWARVES
	The # flies through the air and thoroughly drenches the dwarves. They
	shake themselves off and curse violently; they *REALLY* look angry!
TEXT	CANTDRINK
	There is nothing here that I can drink!
TEXT	CANT.READ.OYSTER
	There's nothing written on the oyster.
TEXT	OYSTER.IS.BARE
	There is nothing written on the top of the oyster.
TEXT	CAVE.NOT.OPEN
	I'm terrible sorry - Colossal Cave is closed.  Our hours are:
 
TEXT	ETMF.TOO.HIGH
	I'm afraid that I can't start up a real Adventure for you at the
	moment; the system's ETMF is too high.
TEXT	TOO.MANY.USERS
	I'm afraid that there are too many people on the system at the moment;
	I can't start up a real Adventure for you.
TEXT	CAN.DEMO
	We do allow visitors to make short explorations during our off hours.
 
TEXT	WANT.DEMO
	Would you like to do that?
TEXT	TA.TA
	Ok, then - I suggest that you try again later.
TEXT	WIZARD.ENDS
 
	A large cloud of green smoke appears in front of you.  It clears away
	to reveal a tall wizard, clothed in grey.  He fixes you with a steely
	glare and declares, "This adventure has lasted too long."  With that
	he makes a single pass over you with his hands, and everything around
	you fades away into a grey nothingness.
TEXT	RESTORE.PRIME
	I'm afraid that I can't restore your saved game - it's prime time
	at the moment, and only demonstration Adventures are permitted.
TEXT	RESTORE.ETMF
	I'm sorry, but I can't restore your Adventure - the ETMF is too
	high.  You'll have to try again later.
TEXT	RESTORE.USERS
	I'm sorry, but there are too many users on the system at the moment;
	please try again later when the system is less heavily in use, and
	I'll restore your Adventure then.
TEXT	HOURS.ARE
 
	Colossal Cave is open during the following hours:
 
TEXT	R.U.A.WIZARD?
	Are you actually a wizard?
TEXT	PROVE.IT
	Prove it - say the magic word!
TEXT	OH.POOH
	Oh, pooh - you are nothing but a charlatan!  That little piece of
	deception is going to cost you 10 points!!
TEXT	SO.YOU.ARE
	Oh, dear, you really *are* a wizard!  Sorry to have bothered you....
TEXT	NOTHING.OBVIOUS
	Nothing obvious happens.
TEXT	SCHLURP
	Hmmmm..  This sand is rather soft, and you're sinking in a little...
	In fact you're sinking in a lot!   Oh, no - it's QUICKSAND!!  HELP!!
	HELP!!! HELP!! >glub<   >glub<    >glub<     >blurp<  
TEXT	SCHLURP2
	You know, I've heard of people who really fell in for the soft sell,
	but  >glub<  this  >glub<  is  >glub<  ridiculous!         >blop!<
TEXT	SLIMED
	As you enter into the passage, you are forced to brush up against
	some of the green slime.  Instantly it flows down and covers your
	body, and rapidly digests away all of your flesh.
TEXT	FZAP
	With a sharp sizzling sound, a large spark of electricity jumps
	out of thin air and strikes your lamp.  The immense electrical charge
	flows to ground through your body and fries you to a crisp.
TEXT	LAMP.GOES.POOF
	With a loud "zap" a bolt of lightning springs out of midair and strikes
	your lamp, which immediately and violently explodes.  You narrowly
	miss being torn to shreds by the flying metal.
 
TEXT	LAMP.EXPLODES
	In a loud crackle of electricity, a bolt of lightning jumps out of
	nowhere and strikes your lamp.  The lamp instantly explodes like a
	grenade, and you are mown down by a cloud of shrapnel.
TEXT	LAMP.RECHARGED
	The air fills with tension, and there is a subdued crackling sound.
	A blue aura forms about your lantern, and small sparks jump from the
	lantern to the ground.  The aura fades away after several seconds,
	and your lamp is once again shining brightly.
TEXT	GOT.THE.SWORD
	The singing sword slides easily out of the rock.
TEXT	SWORD.IS.STUCK
	The sword is firmly embedded in the stone, and you aren't strong
	enough to pull it out.
TEXT	VIAL.EXPLODES
	The vial strikes the ground and explodes with a violent >foom<,
	neatly severing your foot.  You bleed to death quickly and messily.
TEXT	VIAL.BANG
	The vial explodes into splinters and disintegrates, releasing an
	oily liquid which rapidly sublimes into a large mushroom-shaped cloud
TEXT	FIRST.FUME
	of pale puce vapor smelling like sequoia sap and ozone.
TEXT
	of bright green vapor smelling like pine needles and sea water.
TEXT
	of thick yellow vapor smelling like cheddar cheese and bananas.
TEXT
	of choking green vapor smelling like chlorine and apples.
TEXT
	of misty white vapor with no scent.
TEXT
	of nearly-invisible vapor with a strong odor of walnuts and onions.
TEXT	LAST.FUME
	of ominously glowing vapor smelling of hot iron.
TEXT	REVENGE
	As you reach the middle of the bridge, the troll appears from out
	of the tunnel behind you, wearing a large backpack.  "So, Mister
	Magician," he shouts, "you like to use magic to steal back my hard-
	earned toll?  Let's see how you like a little of MY magic!!"  With
	that, he aims a tube running from the backpack directly at the bear
	and pulls a trigger.  A spout of magical fire roars out and singes the
	bear's fur;  the bear bellows in pain and dashes onto the bridge to
	escape.  The bridge shudders, groans, and collapses under the weight,
	and you and the bear plunge down into the chasm.
TEXT	REVENGE.1
	As you reach the middle of the bridge, the troll appears from out
	of the tunnel behind you, wearing a large backpack.  "So, Mister
	Magician," he shouts, "you like to use magic to steal back my hard-
	earned toll?  Let's see how you like a little of MY magic!!"  With
	that, he aims a tube running from the backpack directly at the bridge
	and pulls a trigger.  A spout of magical fire roars out and incinerates
	the bridge supports, causing the bridge to sway giddily and collapse
	into the chasm.  You plunge down to your death.
TEXT	SWORD.MISSES
	The sword misses the bear, bounces off of the wall, and lands at
	your feet.
TEXT	BEAR.MISSES
	The bear dodges away from your attack, growls, and swipes at you
	with his claws.  Fortunately, he misses.
TEXT	BEAR.GETS.YOU
	The bear snarls, ducks away from your attack and slashes you to death
	with his claws.
TEXT	FOOF
 
	>>Foof!<<
 
TEXT	OOF
	Wheeeeeeeeeeeeeeee.......     >oof<
 
TEXT	SLIDE.SLIPPERY
	The icy slide is far too steep and slippery to climb.
TEXT	OGRE.BLOCKS
	The ogre growls at you and refuses to let you pass.
TEXT	OGRE.REBUFF
	The ogre contemptously catches the # in mid-swing, rips it out
	of your hands, and uses it to chop off your head.
TEXT	OGRE.CATCH
	The ogre casually catches the # in mid-air, braces his feet, winds
	up and throws the # straight back at you with incredible force.  You
	are unable to dodge it and it chops you in half.
TEXT	OGRE.KILLED
	The sword halts in mid-air, twirls like a dervish, and chants several
	bars of "Dies Ire" in a rough tenor voice.  It then begins to spin
	like a rip-saw blade and flies directly at the ogre, who attempts to
	catch it without success;  it strikes him full on the chest.  There is
	a brilliant flash of light, a deafening roar and a cloud of oily grey
	smoke;  when the smoke clears (and your eyes begin working properly
	again) you see that the ogre has vanished.  The sword is lying on the
	ground, sparking and flaming.  Before your eyes it softens and melts,
	writhes as if in pain, and shrinks rapidly until all that is left is
	a small silvery ring which cools rapidly.
TEXT	OGRE.TOO.TOUGH
	You attack the ogre, but he fends off your attack easily and comes
	very close to crushing your skull with *his* bare (but extremely
	strong) hands.  You are forced to retreat in disgrace.
TEXT	OGRE.RIPS.HEAD.OFF
	You attack the ogre - a brave but foolish action.  He quickly grabs
	you and with a heave of his mighty arms rips your body limb from limb.
TEXT	NO.ARCH
	I'm afraid I can't go that way - walking on red-hot lava is contrary
	to union regulations (and is bad for your health anyhow).
TEXT	LEAVE.BEAR
	That archway looks pretty fragile - you'd better leave the bear here.
TEXT	FUMES.BURN
	As you approach the center of the archway, hot vapors saturated with
	brimstone drift up from the lava in the gorge beneath your feet.  You
	are swiftly overcome by the foul gasses and, with your lungs burned
	out, fall off of the bridge and into the gorge.
TEXT	FUMES.MISS
	As you approach the center of the archway, hot vapors saturated with
	brimstone drift up from the lava in the gorge beneath your feet.  The
	mithril ring in your hand quivers and glows, and the fumes eddy away
	from the bridge without harming you.
 
TEXT	GHOST.BANG
	As you reach the center of the bridge, a ghostly figure appears in
	front of you.  He (?) stands at least eight feet tall, and has the
	lower body of an enormous snake, six arms, and an angry expression on
	his face.  "You'll not have my sceptre that easily!" he cries, and
	makes a complex magical gesture with his lower right arm.  There is a
	brilliant flash of light and a vicious >crack<, and the bridge cracks
	and plummets into the gorge.
TEXT	NO.KEYHOLE
	The door to the safe has no keyhole, dial, or handle - I can't figure
	out how to open it!
TEXT	ALREADY.OPEN
	The # is already open!
TEXT	IT.IS.MELTED
	The door to the safe seems to be fused shut - I can't open it.
TEXT	ALREADY.SHUT
	The # is already closed!
TEXT	SAFE.SHUTS
	>Creeeeeeeeeeeeeeeeeeeeeeeeeeeek<     >ker-CHUNK!<
 
	The safe is now closed.
TEXT	SAFE.OPENS
	>ker-THUNK<
	/            >screeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeech<
 
	The (somewhat rusty) safe is now open.
TEXT	WHISPER
	You pluck the sceptre from the skeleton's bony hand.  As you do, the
	skeleton raises its head and whispers "Remember - #!" in a
	forboding tone; it then sags to the ground and crumbles into dust which
	drifts away into the still air of the cave.
TEXT	BLEW.SAFE
	As you pluck the sceptre from the skeleton's grasp, it raises its head
	and whispers, "You blew it!".  It then shivers and collapses into a
	pile of fine dust which quickly vanishes.
TEXT	SAFE.FUSES
 
	>bong<                 The very air quivers with sound as though
	/   >bong<               someone, somewhere in the distance, has struck
	/      >bong<             three powerful blows on an immense brass gong.
 
	Smoke trickles out from around the edges of the safe's door, and the
	door itself glows red with heat for a moment.
 
	A hollow voice says,  "This is a Class 1 security alarm.  All cave
	security forces go to Orange Alert.  I repeat - Orange Alert."
 
TEXT	SAFE.BLOCKS
	The safe's door is blocking the exit passage - you'll have to close
	the safe to get out of here.
TEXT	SIZZLE
	EEEEEEEEEAAAAAAAAAAAaaaaaahhhhhhhhhhhhh..........
 
	/                                                      >sizzle<
TEXT	SLICE.BLOB
	The # cuts through the blob's body (?) without harming it.
TEXT	BOUNCE.BLOB
	You attack the strange blob, but bounce harmlessly off of its strong
	but very rubbery skin.
TEXT	CRUMBLE
	Rock silently crumbles off of the wall in front of you, revealing
	dark passages leading northwest, north, and northeast.
TEXT	PLAIN.HINT
	Having problems?
TEXT
	Ok - what you need to do is apply a little philosophy.  To wit:  there
	is a question that you need to ask whenever you explore a new room
	in this cave.  In most places, the answer to the question is "yes".
	In some other places, it's "no" for an obvious reason.  Right here,
	the answer is "no" but the reason isn't so obvious.  If you can figure
	out what the question is, you can get out of here easily.  I can tell
	you this - it's always a vital question if you wish to survive.
TEXT	BAS.GRUMBLE
	The basilisk stirs restlessly and grumbles in its sleep as you pass,
	but it does not awaken.
 
TEXT	PETRIFY.SELF
	The basilisk stirs grumpily and awakens, peering sleepily about.  It
	sees its reflection in the metal plate that you are carrying,
	shudders, and turns into solid granite.
 
TEXT	PETRIFY.ME
	The basilisk stirs grumpily and awakens, peering sleepily about.  It
	spies you, growls, and stares you straight in the eye.  Your body
	is instantly petrified.
TEXT	HIT.BASILISK
	You attack the basilisk mightily.  It instantly awakens and looks you
	dead in the eye, and your body turns to solid rock.
TEXT	AXE.BASILISK
	The # rebounds harmlessly from the basilisk's tough scales.  The
	basilisk awakens, grunting in shock, and glares at you.  You are
	instantly turned into a solid rock statue (and not a particularly
	impressive one, at that).
TEXT	REBOUND
	The # rebounds harmlessly from the pentagram's magic force
	field.  It's just as well - the djinn doesn't seem dangerous.
TEXT	KILL.A.FEW
	You kill several of the gooseberry goblins with your #, but
	the others swarm at you, force you to the ground, and rip out
	your throat.
TEXT	TOUGH.DJINN
	That's not a wise thing to try - djinni are essentially immortal
	and thus are pretty hard to hurt.  Besides, this one seems rather
	friendly - why don't you just try releasing him?
TEXT	GOBL.EAT.YOU
	Goblins live exclusively on human flesh, and you can't spare
	any of your own to placate them.  On the other hand, I suspect
	that they're going to eat you pretty soon whether you like it
	or not - you'd better find some way of killing them or driving
	them away!
TEXT	WARRIORS
 
	As each of the dragon's teeth strikes the ground, a fully-armed human
	skeleton springs up from where it struck and leaps to your defense!
	The skeletal warriors attack the vicious gooseberry goblins and drive
	them away in screaming panic;  they then salute you with their ancient
	and rusty swords, and fade silently into nothingness.
 
TEXT	POLITE.DJINN
	The wax seal breaks away easily.  A cloud of dark smoke pours up from
	the mouth of the flask and condenses into the form of a twelve-foot
	Djinn standing in the pentagram.  He pushes experimentally at the
	magical wall of the pentagram (which holds), and nods politely to
	you.  "MY THANKS, OH MORTAL," he says in an incredibly deep bass
	voice.  "IT HAS BEEN THREE THOUSAND YEARS SINCE SOLOMON SEALED ME
	INTO THAT BOTTLE, AND I AM GRATEFUL THAT YOU HAVE RELEASED ME.  IF
	YOU WILL OPEN THIS PENTAGRAM AND LET ME GO FREE, I WILL GIVE YOU SOME
	ADVICE THAT YOU MAY ONE DAY WISH TO POSSESS."
TEXT	RUDE.DJINN
	The flask's wax seal crumbles at your touch.  A large cloud of black
	smoke pours out, solidifying into the form of a twelve-foot Djinn.
	"AT LAST!" he says in an earth-shaking voice, "I KNEW THAT SOMEDAY
	SOMEONE WOULD RELEASE ME!  I WOULD REWARD YOU FOR THIS, MORTAL, BUT
	IT HAS BEEN THREE THOUSAND YEARS SINCE I HAD A SOLID MEAL, AND I'M
	NOT GOING TO STAND HERE CHATTERING WHEN I COULD BE OUT EATING A SIX-
	INCH SIRLOIN STEAK.  FAREWELL."  With that, he somewhat rudely explodes
	back into smoke and drifts quickly out of sight.
TEXT	DJINN.ADVICE
	The pentagram's magical barrier sparks fitfully and goes down.  The
	Djinn stretches gratefully and smiles at you.  "AGAIN, MY THANKS," he
	says.  "MY ADVICE TO YOU WILL TAKE THE FORM OF A HISTORY LESSON.
	WHEN RALPH WITT, THE ARCHITECT AND CONSTRUCTOR OF THIS CAVE, WAS VERY
	YOUNG, HE BECAME VERY INCENSED THAT HIS NAME WAS AT THE END OF THE
	ALPHABET.  HE FELT (FOR SOME REASON) THAT THE LETTER W BELONGED NEAR
	THE BEGINNING OF THE ALPHABET, AND THAT ALL OF THOSE "UPSTART LETTERS
	WHICH UNFAIRLY USURPED THE BEST PLACES" SHOULD BE FORCED INTO EXILE
	AT THE END OF THE ALPHABET.  HIS INSTINCT FOR MATTERS MAGICAL AND
	MYSTICAL LED HIM TO APPLY THIS STRANGE BELIEF INTO THE CAVE'S
	STRUCTURE WHEN HE EXCAVATED IT.  YOU HAVEN'T YET BEEN AFFECTED BY HIS
	STRANGE HABITS, BUT YOU SHOULD REMEMBER THIS.  FAREWELL, AND GOOD
	LUCK."  With that, the Djinn evaporates into a cloud of smoke and
	drifts rapidly away.
TEXT	KILL.GOBLINS
	You attack the goblins and manage to squash a few, but the others
	overwhelm you, forcing you to the ground and ripping out your throat.
TEXT	EMPTY.PENTA
	The pentagram is empty - there's nothing to let out!
TEXT	GOBLIN.CHASE
	You are being pursued by a vicious horde of little gooseberry goblins!
 
TEXT	FALL&STARVE
	You have jumped into a bottomless pit.  You continue to fall for
	a very long time.  First, your lamp runs out of power and goes
	dead. Later, you die of hunger and thirst.
TEXT	FALL&STARVED
	You have jumped into a bottomless pit.  Eventually, you die of thirst.
TEXT	CANTENTERSAFE
	The safe's door is closed, and you can't get in!
TEXT	NO.HANDLE
	There is no handle on the inside of the safe door, nor any other way
	to get a grip on it.  You'll have to leave the safe before shutting it.
TEXT	CANT.SWIM
	I can't swim, or walk on water.  You'll have to find some other way
	to get across, or get someone to assist you.
TEXT	TURTLE.BACK
	You step gently on Darwin the Tortoise's back, and he carries you
	smoothly over to the southern side of the reservoir.  He then blows
	a couple of bubbles at you and sinks back out of sight.
 
TEXT	GONG.RINGS
 
	>BONNNNGGGGGGGGGG<
 
	Darwin the Tortoise blinks in surprise at the noise, but does nothing.
TEXT	GONG.FETCH
 
	>BONNNNNGGGGGGGGG<
 
	A hollow voice says, "The GallopingGhost Tortoise Express is now at
	your service!"
 
	With a swoosh and a swirl of water, a large tortoise rises to the
	surface of the reservoir and paddles over to the shore near you.
	The message, "I'm Darwin - ride me!" is inscribed on his back in
	ornate letters.
 
TEXT	WHIRLPOOL?
	Into the whirlpool??
TEXT	FLOW.DOWN
	You plunge into the water and are sucked down by the whirlpool.
TEXT	FLOW.RIP
	You plunge into the water and are sucked down by the whirlpool.  The
	current is incredibly strong, and you barely manage to hold onto
	your lamp;  everything else is pulled from your grasp and is lost in
	the swirling waters.
TEXT	FLOW.DARK
	You plunge into the water and are sucked down by the whirlpool into
	pitch darkness.
TEXT	FLOW.D.RIP
	You plunge into the water and are sucked down by the whirlpool into
	pitch darkness.  The current is incredibly strong, and everything that
	you are carrying is ripped from your grasp and is lost in the swirling
	waters.
TEXT	WHIRL.LAND
 
	The swirling waters deposit you, not ungently, on solid ground.
 
TEXT	PHUGGG.DATA
 
	A large phosphorescent cloud of smoke drifts into view, and a large
	mouth and two dark eyes take shape on the side.  One of the eyes winks
	at you, and the djinn's deep voice says "GREETINGS AGAIN, MORTAL.  I
	HAVE REMEMBERED A PIECE OF ANCIENT LORE THAT I LEARNED FROM MY AUNT,
	AN AFREET OF GREAT KNOWLEDGE.  THERE IS ANOTHER MAGIC WORD THAT YOU
	MIGHT FIND OF USE IF YOU SHOULD EVER FIND YOURSELF BEING ATTACKED BY
	THOSE PESTIFEROUS DWARVES.  YOU SHOULD USE IT ONLY AS A LAST RESORT,
	THOUGH, SINCE IT IS A MOST POTENT WORD AND IS PRONE TO BACKFIRE FOR
	NO OBVIOUS REASON;  ALSO, IT SHOULD NEVER BE USED NEAR WATER OR NEAR
	ANY SHARP WEAPON OR THE RESULTS MAY BE MOST UNFORTUNATE.  THE WORD
	IS 'phuggg'", whispers the djinn, "AND IT MUST BE PRONOUNCED CAREFULLY
	IF IT IS TO HAVE THE PROPER EFFECT.  FAREWELL AGAIN, AND GOOD LUCK!"
	With that, the djinn-cloud drifts away out of sight.
 
TEXT	JELLYFISH
 
	>splurch!<
 
	Oh, no!  You've turned yourself into a jellyfish, and fallen to the
	ground and been splattered far and wide!  Well, that certainly wasn't
	very smart!!!  You were warned not to use that work near water!
TEXT	CAVE.DESTROYED
 
	The ground begins to shudder ominously, and the very cave walls around
	you begin to creak and groan!  A sulphurious stench fills the air!
 
	With an incredible lurch, the ground begins to dance and ripple as
	though it were liquid!  You are thrown off of your feet and tossed
	violently up and down!  The cave walls begin to crumble and split from
	the stress!
 
	There is a terrible ROAR of rending rock!!  The cave ceiling splits,
	and rocks plunge down and smash your lower body to a gooey paste!!
 
	There is a violent blast in the distance!  Steam and smoke drift into
	view through the rents in the walls, and furiously-bubbling red-hot
	lava flows in and surrounds you.  The cave ceiling disintegrates in
	an incredible orgy of grinding destruction, and the cave walls fall
	and are pounded into fine dust.
 
 
 
	You are lying, badly mangled, on a small rock island in a sea of
	molten lava.  Above you, the sky is faintly visible through a thick
	pall of smoke and steam.  A short distance to the north, the remains
	of a well-house are sinking slowly into the bubbling ooze.
 
 
	There is a distant, sourceless screech of incredible anguish!  With
	a sharp >poof< and a small puff of orange smoke, a bent and bearded
	elf appears.  He is dressed in working clothes, and has a name-tag
	marked "Ralph" on his shirt.  "You blithering idiot!" he storms.
	"You were warned quite clearly not to use that work near water!!  I
	hadn't gotten all of the bugs out of it yet, and now your incredible
	incompetance has totally destroyed Colossal Cave!!  Do you have the
	faintest scintilla of an iota of an understanding of how much work
	I'm going to have to do to get the cave rebuilt?!?  I'll have to go
	all the way to Peking for another dragon, and I'll have to convince
	the Goblin's Union to send me another team of gooseberry goblins;
	I'll have to sub-contract the building of the volcano out to the
	local totrugs, and worst of all I'll have to go through eight months
	of paperwork and red tape to file a new Environmental Impact
	statement!!  All because you couldn't follow directions, you
	purblind and meatbrained moron!  I'm rescinding all of your game
	points and throwing you out!  Out!   OUT!   GET OUT!$!%#&'@%!!%%!"
 
TEXT	ZOT.AXE
	Your axe glows bright orange and fades into nothingness.
TEXT	ZOT.SWORD
	Your sword jumps into the air, chants several bars of the "Volga
	Boatman", shoots off several fitful blue sparks, and disintegrates.
TEXT	IT.WORKED
	A clear, liquid chime sounds in midair.  A large, four-clawed hand
	reaches out of the ground, grabs the dwarf, and pulls it down into
	nothingness.
TEXT
	A clear, liquid chime sounds in midair.  A long green tentacle
	covered with sucker disks reaches out from nowhere, grabs the
	dwarves, and pulls them back to wherever it came from.
TEXT
	There is a sharp sizzling sound.  The dwarf explodes into flame
	and vanishes.
TEXT
	There is a sharp sizzling sound.  The dwarves are engulfed in
	a wave of fire that appears from nowhere, and are completely
	incinerated;  the flames then vanish into nothingness again.
TEXT
	There is a sharp whistling sound from nowhere.  The dwarf shudders
	and turns into a moth, which then flies away.
TEXT
	There is a sharp whistling sound from nowhere.  The dwarves stiffen,
	shudder, and melt down into a large puddle of soggy goo that quickly
	soaks into the ground and vanishes.
TEXT	IT.DIDNT.WORK
	A clear, liquid chime sounds in midair.  A large, four-clawed foot
	appears in midair and stomps violently downward, missing the dwarf
	but thoroughly squashing you.
TEXT
	A clear, liquid chime sounds in midair.  A large and very toothy
	mouth appears in midair and chomps ferociously.  The dwarves manage
	to evade it, but it bites you in half.
TEXT
	There is a sharp sizzling sound.  A ball of fire roars out of nowhere,
	misses the dwarf, bounces off of a wall, and incinerates you.
TEXT
	There is a sharp sizzling sound.  A ball of fire appears from nowhere,
	bounces off of the ground, and explodes violently, incinerating both
	you and the dwarves.
TEXT
	There is a sharp crackling sound from the air above you.  The dwarf
	shudders and turns into a sabre-toothed tiger, which attacks and
	kills you in short order.
TEXT
	There is a sharp crackling sound from the air above you.  The dwarves
	stiffen, fall to the ground, and melt into a large puddle of soggy
	goo.  The goo twitches a few times and then flows at you with
	incredible speed;  it attacks and strangles you with little
	difficulty.
TEXT	SET.FLASK.DOWN
	You have set the flask down in the center of the pentagram.
TEXT	CAMEO
	From somewhere in the distance, there comes a musical skirl of
	light, elvish laughter and the sounds of merriment.
TEXT
	From somewhere nearby, there suddenly comes a sound of something
	mechanical in motion.  As you turn towards it, an incredible
	figure rolls into the light of your lamp.  It stands about five
	feet high on a wheeled metal pedestal, and has a globular light-
	filled head, accordion-pleated metal arms, and a cylindrical body
	the size of an oil drum with a plastic panel on the front.  It rolls
	past without taking any notice of you, all the while waving its
	arms, flashing a light behind its front panel and bellowing "WARNING!
	WARNING!  DANGER!" at the top of its not inconsiderable voice.  It
	rolls on out of sight, and moments later there is an immense flash
	of light and a tremendous blast of sparks and smoke.  When the air
	clears, you find that no trace remains of the strange apparition.
TEXT
	With a sudden gust of air, a large cave bat flutters into view,
	flies around your head several times, squeaks with disgust, and
	flutters on out of sight.
TEXT
	Suddenly, the ground quivers underfoot;  a dull rumbling sound
	resounds from the rock around you, and in the distance you can
	hear the crash of falling rock.  The earth tremor subsides after
	a few seconds without causing any major damage.
TEXT
	From the darkness nearby comes the sound of shuffling feet.  As you
	turn towards the sound, a nine-foot cyclops ambles into the light of
	your lamp.  The cyclops is dressed in a three-piece suit of worsted
	wool, and is wearing a black silk top-hat and cowboy boots and is
	carrying an ebony walking-stick.  It catches sight of you and stops,
	seeming frozen in its tracks, with its bloodshot eye bulging in
	amazement and its fang-filled jaw drooping with shock.  After staring
	at you in incredulous disbelief for a few moments, it reaches into
	the pocket of its vest and pulls out a small plastic bag filled with
	a leafy green substance, and examines it carefully.  "It must be
	worth eighty pazools an ounce after all" mumbles the cyclops, who
	casts one final look at you, shudders, and staggers away out of
	sight.
TEXT
	From somewhere in the distance comes a heart-wrenching scream of
	mortal terror!  "NO!  DON'T!  NO!  NO!  HELP!!!!" cries the voice,
	and then lets out a wail of agony that is cut off abruptly.  Subdued
	crunching and slurping sounds can be heard for a minute or so, and
	then silence falls.
TEXT	LAST.CAMEO
	From somewhere nearby come the sounds of sliding, stumbling feet.
	As you turn towards them, the beam of your lamp falls upon a tall,
	shambling figure approaching you out of the darkness.  Standing no
	more than five feet tall, it cannot possibly weigh more than fifty
	pounds including the shroud and bandages in which it is wrapped;
	a musty reek like the scent of old, dead earth seeps from it and
	fills the air.  As you cower back in disgust and horror, the figure
	halts, examines you through eyes resembling wet pebbles, and
	whispers "Peace, man!" in a voice like wind rustling through dead
	trees.  It then turns and shambles away into the darkness.
TEXT	THROW.PIT
	You have tossed the # down into the pit.
TEXT	THROW.FISSURE
	You have thrown the # down into the fissure.
TEXT	THROW.HOLE
	You have tossed the # down into the hole in the floor.
TEXT	THROW.ROOM
	You have tossed the # down into the room below you.
TEXT	THROW.CANYON
	You have tossed the # down into the canyon beneath your feet.
TEXT	THROW.WHIRLPOOL
	You have hurled the # into the middle of the whirlpool.
TEXT	THROW.CAVERN
	You have thrown the # down into the mist filling the cavern.
TEXT	THROW.CHASM
	You have hurled the # down into the misty bottom of the chasm.
TEXT	THROW.GORGE
	You have hurled the # down into the boiling lava at the bottom
	of the volcanic gorge.
TEXT	THROW.CHIMNEY
	You have tossed the # down the chimney.
TEXT	THROW.TUBE
	You have tossed the # down into the lava tube and out of sight.
TEXT	THROW.STEPS
	You have thrown the # down the steps and out of view.
TEXT	THROW.SLIDE
	You have tossed the # down the icy slide and out of sight.
TEXT	THROW.BEACH
	You have thrown the # down onto the beach.
TEXT	THROW.RESERVOIR
	You have negligently tossed the # into the center of the reservoir.
TEXT	SHATTERED.IT
	A delicate crash sounds from below.
TEXT	ICE.HINT?
	Are you having problems getting out of the ice tunnels?
TEXT
	To get out of here, you'll first have to get your bearings so that
	you know where you are.  I suggest that you draw a careful, accurate
	map of the tunnel system;  for clarity's sake, keep your lines as
	straight as is feasible and draw in all of the dead ends and such.
	Once you've got a complete and accurate map, examine it carefully; if
	your thoughts refuse to clarify, you might try using the old Yoga
	trick of standing on your head, and see if that helps.
TEXT	CANT.SEE.ANYTHING
	It's pitch dark in here - I can't tell whether there's anything here
	that I can pick up!
TEXT	GROPE.FALL
	Hmmph - you're not asking for much, are you - it's pitch dark in
	here!  Well, I'll grope around and try to find the #.....
	{hunt}    {hunt}    {rummage}    {trip}   Aiiieeeee...   >SPLAT!<
 
	You stumbled into a pit and broke your back!
TEXT	GROPE.MISS
	Hmmph - you're not asking for much, are you - it's pitch dark
	in here, and I'll have to grope around to try to find the #.
	Well, if I must, I must......   {hunt}   {search}    {hunt}
	{hunt}    {peer-blindly-into-darkness}   {touch}    {hunt}
	{stumble}    {search}     {scrape}   {swear}    {hunt}
 
	No luck - I can't find the #!.  Get me some light and maybe I'll
	be able to do better!
TEXT	GROPE.FIND
	Hmmph - you're not asking for much, are you - it's pitch dark in
	here, and I'll have to grope around to find the #.  Oh,
	well, I suppose that that's part of my job...    {hunt}  {search}
	{seek}   Could this be the #?   No.     {search}       {hunt}
	/    {trip}   {curse}   {catch self}   {nurse scraped hand}  {seek}
 
	Aha!    I have located the #!.
 
