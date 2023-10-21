LABEL	GETSCORE
	IFEQ  QUITTING,0
	   OR
	IFGT  CLOSURE,2
	   SET   SCOREX,9
	ELSE
	   SET   SCOREX,0
	FIN
	SET   MAXSCORE,9
	ITOBJ I
	   BIT   I,VALUED
	      IFLOC I,BUILDING
		 OR
	      IFGT  CLOSURE,2
		 ADD   SCOREX,15
	      ELSE
		 BIT   I,SEEN
		    ADD   SCOREX,2
		 FIN
	      FIN
	      ADD   MAXSCORE,15
	   FIN
	EOI
*
	IFLOC MAGAZINES,WITTSEND
	   ADD   SCOREX,1
	FIN
	ADD   MAXSCORE,1
*
	BIT   DEBRIS,BEENHERE
	   OR
	BIT   Y2,BEENHERE
	   ADD   SCOREX,20
	   BIT   LAIR,SEEN
	      ADD   SCOREX,10
	   FIN
	   BIT   BEACH,SEEN
	      ADD   SCOREX,10
	   FIN
	   BIT   FACES,SEEN
	      ADD   SCOREX,10
	   FIN
	FIN
	ADD   MAXSCORE,50
*
	SET   I,CLOSURE
	MULT  I,20
	ADD   SCOREX,I
	ADD   MAXSCORE,100
*
	SET   L,DEATHS
	MULT  L,10
	SUB   SCOREX,L
*
	SUB   SCOREX,PENALTIES
*
	IFLT  SCOREX,0
	   SET   SCOREX,0
	FIN
*
LABEL	FINIS
	CALL  GETSCORE
	VALUE YOUSCORED,SCOREX
	VALUE TOPSCORE,MAXSCORE
	VALUE NMOVES,MOVES
	IFLT  SCOREX,20
	   LDA   I,FISH
	   SUB   SCOREX,20
	ELSE
	   IFLT  SCOREX,130
	      LDA   I,NOVICE
	      SUB   SCOREX,130
	   ELSE
	      IFLT  SCOREX,240
		 LDA   I,EXPERIENCED
		 SUB   SCOREX,240
	      ELSE
		 IFLT  SCOREX,350
		    LDA   I,SEASONED
		    SUB   SCOREX,350
		 ELSE
		    IFLT  SCOREX,470
			LDA   I,JUNIORMASTER
			SUB   SCOREX,470
		    ELSE
			IFLT  SCOREX,510
			  LDA   I,MASTER.C
			  SUB   SCOREX,510
			ELSE
			  IFLT  SCOREX,530
			     LDA   I,MASTER.B
			     SUB   SCOREX,530
			  ELSE
			     IFLT  SCOREX,550
				LDA   I,MASTER.A
				SUB   SCOREX,550
			     ELSE
				LDA   I,GRAND
				LDA   SCOREX,0
			     FIN
			  FIN
			FIN
		    FIN
		 FIN
	      FIN
	   FIN
	FIN
	SAY   I
	SAY   BLANK
	MULT  SCOREX,-1
	IFGT  SCOREX,0
	   IFEQ  SCOREX,1
	      SAY   NEED1
	   ELSE
	      VALUE NEED,SCOREX
	   FIN
	FIN
	SAY   BLANK
	STOP
LABEL	PHOG     {control fog and glow in fog}
	IFLT  FOG,8    {state 8 = invisible fog}
	   RANDOM   FOG,8    {change color of fog}
	FIN
	RANDOM   GLOW,8   {move the glow around}
	IFNEAR   LAMP
	   AND
	IFEQ  LAMP,1      {lamp here and glowing?}
	   APPORT   GLOW,LIMBO  {if so, masks out faint glow}
	ELSE
	   APPORT   GLOW,PLAIN.2   {if not, move glow into place}
	   IFAT  PLAIN.2        {is that where we are?}
	      AND
	      NOT
	   BIT   STATUS,MOVED
	      SAY   GLOW        {announce faint glimmer of light}
	   FIN
	FIN
LABEL	CORONER
	SET   QUITTING,0
	SAY   BLANK
	BIC   ADMIN,NOMAGIC     {reset magic-inhibit mode}
	BIC   ADMIN,TICKER      {clear once-per-move}
	SET   BLOB,0            {reset blob}
	APPORT   BLOB,LIMBO     {and get rid of him}
	APPORT   GOBLINS,LIMBO  {get rid of goblins, if around}
	IFEQ  BASILISK,1        {adjust the basilisk so that}
	   SET   BASILISK,0     {he gets viewed from the south}
	ELSE                    {as he should - whether he's}
	   IFEQ  BASILISK,3     {petrified or not...}
	      SET   BASILISK,2
	   FIN
	FIN
	APPORT   FOG,PLAIN.1    {move the fog back to its initial}
	SET   FOG,8             {position, and make it semi-visible}
	ADD   DEATHS,1
	IFGT  CLOSURE,1
	   IFEQ  CLOSURE,2
	      SAY   DEAD&CLOSED
	   ELSE
	      SUB   DEATHS,1 {don't charge for dying in repository}
	   FIN
	   CALL  FINIS
	FIN
	LDA   I,YOUAREDEAD-2
	ADD   I,DEATHS
	ADD   I,DEATHS
	SAY   I
	QUERY  SOK!
	   ADD   I,1
	   SAY   I
	   SAY   BLANK
	   LDA   J,SNIDELEY-1
	   IFLT  I,J
	      IFHAVE   VASE
		 APPORT   VASE,LIMBO
		 GET   POTTERY
	      FIN
	      ITOBJ I
		 IFHAVE I
		    DROP  I
		 FIN
	      EOI
	      APPORT   WATER,LIMBO
	      APPORT   OIL,LIMBO
	      SET   INVCT,0
	      SET   LAMP,0
	      APPORT   FOG,PLAIN.1
	      SET   FOG,8
	      CALL  PHOG     {chase glow into place}
	      GOTO  BUILDING
	      SET   THERE,0
	      APPORT   LAMP,ROAD
	      IFEQ  LAMPLIFE,0
		 BIT   LAMP,SPECIAL1
		    OR
		    NOT
		 BIT   LAIR,BEENHERE
		    LOCATE   I,BATTERIES
		    IFEQ  BATTERIES,1
			OR
			NOT
		    BIT   I,NOTINCAVE
			APPORT   LAMP,YLEM
		    FIN
		 FIN
	      FIN
	      APPORT   DWARF,LIMBO
	      SET   DWARROWS,0
	      BIC   PIRATE,SPECIAL1   {clear chasing mode}
	      QUIT
	   ELSE
	      CALL  FINIS
	   FIN
	ELSE
	   CALL  FINIS
	FIN
	STOP
LABEL	GETBIRD
	NEAR  BIRD
	IFHAVE   BIRD
	   SAY   YOUHAVEIT
	   QUIT
	FIN
	IFEQ  BIRD,1
	   GET   CAGE
	   GET   BIRD
	   SAY   OK
	ELSE
	   IFHAVE   CAGE
	      IFHAVE   ROD
		 SAY   BIRDISSCARED
	      ELSE
		 BIC   BIRDCHAMBER,HINTABLE
		 GET   BIRD
		 SET   BIRD,1
		 SAY   OK
	      FIN
	   ELSE
	      SAY   NEEDCAGE
	   FIN
	FIN
	QUIT
LABEL	GETCAGE
	NEAR  CAGE
	IFHAVE   CAGE
	   SAY   YOUHAVEIT
	   QUIT
	FIN
	GET   CAGE
	IFNEAR   BIRD
	   IFEQ  BIRD,1
	      GET   BIRD
	   FIN
	FIN
	SAY   OK
	QUIT
LABEL	GETKNIFE
	BIT   AXE,SEEN
	   SAY   NO..KNIVES
	   QUIT
	FIN
LABEL	DROPBIRD
	HAVE  BIRD
	DROP  BIRD
	SET   BIRD,0
	IFNEAR   SNAKE
	   SAY   BIRD..SNAKE
	   APPORT   SNAKE,LIMBO
	   BIC   MTKING,HINTABLE
	   QUIT
	ELSE
	   IFNEAR   DRAGON
	      IFEQ  DRAGON,0
		 APPORT   BIRD,LIMBO  {incinerate bird}
		 SAY   BIRD..DRAGON
		 QUIT
	      FIN
	   FIN
	FIN
	SAY   OK
	QUIT
LABEL	DROPCAGE
	HAVE  CAGE
	DROP  CAGE
	IFHAVE   BIRD
	   DROP  BIRD
	FIN
	SAY   OK
	QUIT
LABEL	DROPVASE
	HAVE  VASE
	DROP  VASE
	IFAT  SOFT
	   SAY   OK
	ELSE
	   IFHAVE   PILLOW
	      OR
	      NOT
	   IFNEAR   PILLOW
	      SET   VASE,2
	      SAY   VASE
	      APPORT   VASE,LIMBO
	      APPORT   POTTERY,HERE
	   ELSE
	      SET   VASE,1
	      SAY   VASE
	      SET   VASE,0
	   FIN
	FIN
	QUIT
LABEL	DROPLIQUID
	HAVE  ARG2
	APPORT   ARG2,LIMBO
	SET   BOTTLE,1
	IFNEAR   DWARF
	   AND
	IFKEY THROW    {throwing oil or water at dwarf - dangerous!}
	   IFEQ  DWARROWS,1
	      NAME  DOUSED.DWARF,ARG2
	   ELSE
	      NAME  DOUSED.DWARVES,ARG2
	   FIN
	   BIS   DWARF,SPECIAL2    {dwarf gets angry}
	ELSE
	   SAY   POURWATER
	FIN
	QUIT
LABEL	DROPBOTTLE
	HAVE  BOTTLE
	DROP  BOTTLE
	APPORT   OIL,LIMBO
	APPORT   WATER,LIMBO
	SAY   OK
	QUIT
LABEL	GETBOTTLE
	NEAR  BOTTLE
	IFHAVE   BOTTLE
	   SAY   YOUHAVEIT
	   QUIT
	FIN
	IFLT  INVCT,STRENGTH
	   GET   BOTTLE
	   IFEQ  BOTTLE,0
	      GET   WATER
	   ELSE
	      IFEQ  BOTTLE,2
		 GET OIL
	      FIN
	   FIN
	   SAY   OK
	ELSE
	   SAY   ARMSAREFULL
	FIN
	QUIT
LABEL	GETOIL
	AT    EASTPIT
	IFHAVE   BOTTLE
	   IFEQ  BOTTLE,1
	      SET   BOTTLE,2
	      GET   OIL
	      SAY   BOTTLE..OIL
	   ELSE
	      SAY   BOTTLEWASFULL
	   FIN
	ELSE
	   SAY   NOWAYTOCARRY
	FIN
	QUIT
LABEL	GETWATER
	BIT   HERE,H20HERE
	ELSE
	   PROCEED
	FIN
	IFHAVE   BOTTLE
	   IFEQ  BOTTLE,1
	      SET   BOTTLE,0
	      GET   WATER
	      SAY   BOTTLE..H20
	   ELSE
	      SAY   BOTTLEWASFULL
	   FIN
	ELSE
	   SAY   NOWAYTOCARRY
	FIN
	QUIT
LABEL	KILLTROLL
	SAY   TROLL.DATA
	QUIT
LABEL	KILLBEAR
	IFEQ  BEAR,0
	   SAY   KILL..BEAR
	ELSE
	   SAY   BEAR.PUZZLED
	FIN
	QUIT
LABEL	KILLSNAKE
	SAY   CANTKILLSNAKE
	QUIT
LABEL	KILLDRAGON
	IFGT  DRAGON,0
	   SAY   IT.IS.DEAD
	   QUIT
	FIN
	QUERY WITHWHAT?
	   SET   DRAGON,1
	   SAY   DRAGON
	   SET   DRAGON,2
	   APPORT   RUG,SECRETCYNNE1
	   APPORT   TEETH,SECRETCYNNE1
	   BIC   DRAGON,SCHIZOID
	   ITOBJ I
	      IFNEAR   I
		 IFHAVE   I
		 ELSE
		    APPORT   I,SECRETCYNNE1
		 FIN
	      FIN
	   EOI
	   GOTO  SECRETCYNNE1
	ELSE
	   SAY   OK
	FIN
	QUIT
LABEL	KILLBIRD
	IFLT  CLOSURE,3
	   APPORT   BIRD,LIMBO
	   SAY   BIRDISDEAD
	ELSE
	   SAY   LEAVE.BIRD
	FIN
	QUIT
LABEL	KILLBIVALVE
	SAY   KILL.OYSTER
	QUIT
LABEL	KILLDWARF
	QUERY WITHWHAT?
	   SET   I,STRENGTH
	   SUB   I,INVCT
	   ADD   I,2
	   MULT  I,10
	   CHANCE   I
	      SAY   KILLEDDWARF
	      SUB   DWARROWS,1
	      IFEQ  DWARROWS,0
		 APPORT   DWARF,LIMBO
	      FIN
	      SUB   DWARFCOUNT,1
	   ELSE
	      CHANCE   I
		 SAY   DWARFDODGES
	      ELSE
		 SAY   DWARFSTABS
		 CALL CORONER
	      FIN
	   FIN
	ELSE
	   SAY   OK
	FIN
	QUIT
LABEL	KILLOGRE
	QUERY WITHWHAT?
	   CHANCE   50
	      SAY   OGRE.TOO.TOUGH
	   ELSE
	      SAY   OGRE.RIPS.HEAD.OFF
	      CALL  CORONER
	   FIN
	ELSE
	   SAY   OK
	FIN
	QUIT
LABEL	KILLBLOB
	SAY   BOUNCE.BLOB
	QUIT
LABEL	KILLDJINN
	SAY   TOUGH.DJINN
	QUIT
LABEL	KILLGOBLINS
	SAY   KILL.GOBLINS
	CALL  CORONER
LABEL	KILLBASILISK
	IFLT  BASILISK,2
	   SAY   HIT.BASILISK
	   CALL  CORONER
	ELSE
	   SAY   IT.IS.DEAD
	   QUIT
	FIN
LABEL	HITGONG
	IFNEAR   TURTLE
	   SAY   GONG.RINGS
	ELSE
	   SAY   GONG.FETCH
	   APPORT   TURTLE,HERE
	FIN
	QUIT
LABEL	GETBEAR
	NEAR  BEAR
	IFHAVE   BEAR
	   SAY   I.C.A.BEAR
	ELSE
	   IFAT  BEARHERE
	      IFEQ  BEAR,2
		 SAY   OK
		 GET BEAR
	      ELSE
		 SAY   BEAR.IS.CHAINED
	      FIN
	   ELSE
	      SAY   OK
	      GET   BEAR
	   FIN
	FIN
	QUIT
LABEL	DROPBEAR
	HAVE  BEAR
	DROP  BEAR
	SAY   OK
	QUIT
LABEL	FREEDJINN
	NEAR  DJINN
	SAY   DJINN.ADVICE
	APPORT   DJINN,LIMBO
	BIS   DJINN,SPECIAL1
	QUIT
LABEL	GETCHAIN
	AT BEARHERE
	NEAR  CHAIN
	IFEQ  CHAIN,0
	   PROCEED
	ELSE
	   SAY   CHAIN.LOCKED
	FIN
	QUIT
LABEL	GETSWORD
	NEAR  SWORD
	IFEQ  SWORD,0
	   AND
	IFLT  INVCT,STRENGTH
	   AND
	   NOT
	IFHAVE   SWORD
	   IFEQ  MUSHROOM,2
	      SAY   GOT.THE.SWORD
	      SET   SWORD,1
	      GET   SWORD
	   ELSE
	      SAY   SWORD.IS.STUCK
	   FIN
	   QUIT
	FIN
LABEL	GETSCEPTRE
	NEAR  SCEPTRE
	IFEQ  SCEPTRE,0
	   AND
	IFLT  INVCT,STRENGTH
	   GET   SCEPTRE
	   SET   SCEPTRE,1
	   RANDOM   PASSWORD,5
	   IFEQ  PASSWORD,0
	      LDA   PASSWORD,BLERBI
	   ELSE
	      IFEQ  PASSWORD,1
		 LDA   PASSWORD,KLAETU
	      ELSE
		 IFEQ  PASSWORD,2
		    LDA   PASSWORD,KNERL
		 ELSE
		    IFEQ  PASSWORD,3
			LDA   PASSWORD,SNOEZE
		    ELSE
			LDA   PASSWORD,ZORTON
		    FIN
		 FIN
	      FIN
	   FIN
	   IFEQ  SAFE,0
	      NAME  WHISPER,PASSWORD
	   ELSE
	      SAY   BLEW.SAFE
	   FIN
	   APPORT   SKELETON,LIMBO
	   QUIT
	FIN
LABEL	SPLATTER
*
*        Label SPLATTER should be called any time it is desirable to have
*        the adventurer fall to a painful death.  One of a series of
*        text messages will be output, depending on how many times he
*        has been killed so far during this game.  Before calling SPLATTER,
*        the calling routine should GOTO the place that is at the bottom of
*        whatever the adventurer has jumped into - if that place isn't
*        well-defined (bottom of the chasm, volcanic gorge, etc.), then
*        go to Ylem.
*
	LDA   I,PLUMMET
	ADD   I,DEATHS
	SAY   I
	CALL  CORONER
LABEL	DO.CAMEO          {generate strange cameo appearances if possible}
	BIT   HERE,NOTINCAVE {cameos work only in the cave}
	   OR
	BIT   HERE,NODWARF   {and only in dwarf-accessable areas}
	   OR
	BIT   HERE,LIT       {and only in un-lit rooms}
	   OR
	BIT   HERE,ONE.EXIT  {and only where there are several exits}
	   OR
	   NOT
	IFHAVE   LAMP        {and only if you have your lamp}
	   OR
	IFEQ  LAMP,0         { (which must be turned on) }
	   OR
	BIT   PIRATE,SPECIAL1 {and not if the pirate is chasing you}
	   OR
	IFNEAR   DWARF       {and not if you're being plagued by dwarves}
	   OR
	IFNEAR   DRAGON      {or near the dragon (living or dead) }
	   OR
	IFNEAR   TROLL       {or argueing with the troll}
	   OR
	IFNEAR   SNAKE       {or trying to get past the snake}
	   OR
	IFNEAR   QUICKSAND   {or trying to cross the quicksand}
	   PROCEED
	FIN
	SET   CAMEO.TIME,0   {only one cameo per game, max}
	LDA   I,CAMEO
	RANDOM   J,LAST.CAMEO-CAMEO+1
	ADD   I,J
	SAY   BLANK
	SAY   I
	SAY   BLANK
LABEL	CLOSE.THE.CAVE
	SAY   GO.REPOSIT
	BIC   STATUS,FASTMODE
	BIC   STATUS,QUICKIE
	ITOBJ I
	   LOCATE   J,I
	   BIT   J,NOTINCAVE
	      OR
	   IFHAVE   I
	      AND
	   BIT   I,PORTABLE
	      APPORT   I,YLEM
	   FIN
	EOI
	ITPLACE  I
	   BIT   I,NOTINCAVE
	      BIC   I,BEENHERE
	   FIN
	EOI
	SET   CLOSURE,3   {in cylindrical chamber}
	SET   CLOCK,999
	GOTO  CYLINDRICAL
	SET   ESCAPE,0
	QUIT
LABEL	CLOCK4           { (Administrative clock has ticked) }
	IFEQ  CLOSURE,0
	   SET   CLOSURE,1
	   ITOBJ I
	      BIT   I,VALUED
		 AND
		 NOT
	      IFLOC I,BUILDING
		 SET   CLOSURE,0
	      FIN
	   EOI
	   IFEQ  CLOSURE,1
	      SET   CLOCK,35
	   ELSE
	      RANDOM   CLOCK,10
	      ADD   CLOCK,30
	   FIN
	   IFGT  SCULPTURE,0
	      RANDOM   SCULPTURE,@SCULPTURE-1    {skip the "on shelf" state}
	      ADD   SCULPTURE,1
	   FIN
	   IFGT  SWORD,0
	      RANDOM   SWORD,@SWORD-1      {skip the "in stone" state}
	      ADD   SWORD,1
	   FIN
	   IFEQ  DRAGON,2
	      SUB   DRAGTIME,LASTCLOCK
	      IFLT  DRAGTIME,0
		 SET   DRAGON,3
	      FIN
	   FIN
	   BIT   DJINN,SPECIAL1
	      AND
	      NOT
	   BIT   DJINN,SPECIAL2
	      AND
	      NOT
	   IFNEAR   DWARF
	      BIS   DJINN,SPECIAL2
	      SAY   PHUGGG.DATA
	      SET   CLOCK,5
	      PROCEED
	   FIN
	   IFGT  MUSHROOM,1
	      SUB   MUSHTIME,LASTCLOCK
	      IFLT  MUSHTIME,0
		 IFEQ  MUSHROOM,2
		    SET   MUSHROOM,3
		    SET   MUSHTIME,40
		    SAY   MUSHROOM
		    SET   STRENGTH,7
		    SET   CLOCK,8
		    PROCEED
		 ELSE
		    SET   MUSHROOM,0
		    APPORT   MUSHROOM,CUBICLE
		 FIN
	      FIN
	   FIN
	   IFGT  CAMEO.TIME,0
	      AND
	   IFLT  CAMEO.TIME,MOVES
	      RANDOM   CLOCK,10
	      ADD   CLOCK,10
	      CALL  DO.CAMEO
	      PROCEED
	   FIN
	   BIT   MISTS,BEENHERE
	      OR
	   BIT   Y2,BEENHERE
	      IFGT  MOVES,150
		 AND
		 NOT
	      BIT   CHEST,SEEN
		 BIS   PIRATE,SPECIAL1
	      FIN
	      BIT   HERE,NODWARF
		 OR
	      BIT   HERE,NOTINCAVE
		 BIC   PIRATE,SPECIAL1   {clear "chasing"}
		 RANDOM   CLOCK,10    {set short clock interval}
		 ADD   CLOCK,8
	      ELSE
		 SET   I,DWARFCOUNT
		 ADD   I,4
		 MULT  I,10
		 CHANCE   I
		    OR
		 BIT   PIRATE,SPECIAL1
		    RANDOM   I,10
		    ADD   I,DWARROWS
		    IFEQ  I,0
			OR
		    BIT   PIRATE,SPECIAL1
			BIT   CHEST,SEEN  {found treasure chest yet?}
			  OR
			BIT   HERE,LIT {don't pounce in lit rooms}
			  OR
			  NOT
			IFHAVE   LAMP  {don't pounce if lamp elsewhere}
			  OR
			IFEQ  LAMP,0   {or if it's dead}
*                             {do nothing in this case}
			ELSE
			  BIC   PIRATE,SPECIAL1      {clear "chasing"}
			  SET   J,0
			  BIC   RING,VALUED {so it doesn't get stolen}
			  ITOBJ I
			     BIT   I,VALUED
				AND
			     IFNEAR   I
				APPORT   I,MAZEA.114
				SET   J,1
			     FIN
			  EOI
			  BIS   RING,VALUED
			  IFEQ  J,0
			     IFLOC CHEST,LIMBO {first time thru?}
				SAY   PIRATE.RUNS
			     ELSE
				SAY   RUSTLING
				BIS   PIRATE,SPECIAL1    {set "following"}
				RANDOM   CLOCK,10
				ADD      CLOCK,4
			     FIN
			  ELSE
			     SAY   PIRATE..ZOTZ
			  FIN
			  IFLOC CHEST,LIMBO    {first time through?}
			     APPORT   CHEST,MAZEA.114
			     APPORT   MESSAGE,MAZED.140
			  FIN
			FIN
		    ELSE
			IFGT  DWARFCOUNT,0
			  BIT   AXE,SEEN {have we seen a dwarf yet?}
			     APPORT   DWARF,HERE
			     ADD   DWARROWS,1
			     IFEQ  DWARROWS,1
				BIS   DWARF,SPECIAL1 {knife not thrown yet}
				BIC   DWARF,SPECIAL2 {not angry}
			     FIN
			  ELSE  {nope - fetch axe, invoke scared dwarf}
			     APPORT   AXE,HERE
			     BIS   AXE,SEEN {special case}
			     SAY   FIRSTDWARF
			  FIN
			FIN
		    FIN
		 FIN
	      FIN
	   FIN
	ELSE
	   IFEQ  CLOSURE,1   {is it near closing time?}
	      SET   CLOSURE,2   {set "Closing soon"}
	      SET   GRATE,0     {lock the grate}
	      SAY   CLOSING.NOW {Sepulchral voice}
	      IFNEAR   DWARF
		 SAY   DWARF.QUITS {fades into the gloom}
	      FIN
	      APPORT   DWARF,LIMBO {get rid of him/them}
	      SET   DWARROWS,0  {zilch all present dwarves}
	      SET   DWARFCOUNT,0   {don't let him reappear}
	      SET   FISSURE,0      {destroy bridge}
	      SET   GORGE,0        {destroy wheatstone bridge}
	      APPORT   TROLL,LIMBO {remove troll}
	      APPORT   DRAGON,LIMBO {remove dragon}
	      SET   TROLL,5        {scared - inhibit return}
	      APPORT   TROLL2,SWOFCHASM {fetch fake troll}
	      BIS   FISSURE,INVISIBLE
	      BIS   GORGE,INVISIBLE
	      SET   CLOCK,25       {time to try to leave}
	   ELSE        {must be closing time!}
	      BIT   ADMIN,PANICED  {did he try to get out?}
		 BIC   ADMIN,PANICED {reset panic flag}
		 SET   CLOCK,15    {let him get frantic}
	      ELSE                 {if he was calm,}
		 CALL  CLOSE.THE.CAVE
	      FIN
	   FIN
	FIN
	SET   LASTCLOCK,CLOCK
LABEL	BAILOUT
	IFEQ  STATUS,1
	   NAME  CLARIFY,ARG1
	ELSE
	   IFEQ  STATUS,2
	      AND
	   BIT   ARG2,OBJECT
	      IFNEAR   ARG2
		 NAME  DUNNO.HAO,ARG1
	      ELSE
		 NAME  IDONTSEE,ARG2
	      FIN
	   ELSE
	      NAME  DUNNO.HAO,ARG1
	   FIN
	FIN
	QUIT
LABEL	LAMPREY           * Lamp getting dim or has gone out
	IFGT  LAMPLIFE,0
	   IFEQ  BATTERIES,1
	      SAY   LAMP.NOFUEL
	   ELSE
	      IFNEAR   BATTERIES
		 SAY   LAMP.REFUEL
		 SET   BATTERIES,1
		 ADD   LAMPLIFE,300
		 BIC   LAMP,SPECIAL1     {clear "recharged" flag}
	      ELSE
		 BIT   BATTERIES,SEEN
		    SAY   LAMP.BATTERIES
		 ELSE
		    SAY   LAMP.IS.DIM
		 FIN
	      FIN
	   FIN
	ELSE
	   IFEQ  CLOSURE,2
	      CALL  CLOSE.THE.CAVE
	   ELSE
	      IFNEAR   BATTERIES
		 AND
	      IFEQ  BATTERIES,0
		 SAY   LAMP.REFUEL
		 SET   BATTERIES,1
		 ADD   LAMPLIFE,300
	      ELSE
		 SAY   LAMP.IS.DEAD
		 SET   LAMP,0
		 BIS   ADMIN,RANOUT      {don't fall in a pit this move}
		 CALL  PHOG     {chase glow into place}
	      FIN
	   FIN
	FIN
LABEL	READ.MAGAZINES
	SAY   MAG.DWARVISH
	QUIT
LABEL	READ.TABLET
	SAY   DARK.ROOM
	QUIT
LABEL	READ.MESSAGE
	SAY   CHEST.ELSEWHERE
LABEL	READ.MACHINE
	SAY   MACHINE.SIGN
	QUIT
LABEL	HINT.LOGIC
	SET   HINT.TIME,0
	SET   I,0
	IFAT  DEPRESSION
	   IFEQ  GRATE,0
	      NOT
	      IFHAVE   KEYS
		 LDA   I,LOOKINGCAVE
	      FIN
	   FIN
	FIN
	IFAT  BIRDCHAMBER
	   IFNEAR   BIRD
	      IFEQ  BIRD,0
		 IFHAVE   ROD
		    LDA   I,BIRDHINT?
		 FIN
	      FIN
	   FIN
	FIN
	IFAT  MTKING
	   IFNEAR   SNAKE
	      NOT
	      IFNEAR   BIRD
		 LDA   I,GETPASTSNAKE
	      FIN
	   FIN
	FIN
	IFAT  WITTSEND
	   LDA   I,HINT.WITTS?
	FIN
	IFAT  PLOVER
	   OR
	IFAT  ALCOVE
	   OR
	IFAT  DARK
	   NOT
	   BIT   DARK,BEENHERE
	      LDA   I,HINT.PLOVER?
	   FIN
	FIN
	IFAT  PLAIN.2
	   LDA   I,PLAIN.HINT
	FIN
	BIT   HERE,INMAZE
	   LDA   I,HINT.MAZE?
	FIN
	LDA   J,SLIDE-1
	LDA   K,ICECAVE.30+1
	IFGT  HERE,J
	   AND
	IFLT  HERE,K
	   LDA   I,ICE.HINT?
	FIN
	IFGT  I,0
	   QUERY I
	      VALUE ILL.GIVE.HINT,HINT.COST
	      QUERY WANT.HINT?
		 ADD   I,1
		 SAY   I
		 ADD   PENALTIES,HINT.COST
		 BIC   HERE,HINTABLE
		 IFAT  PLOVER
		    OR
		 IFAT  ALCOVE
		    OR
		 IFAT  DARK
		    BIC   PLOVER,HINTABLE
		    BIC   DARK,HINTABLE
		    BIC   ALCOVE,HINTABLE
		 ELSE
		    BIT   HERE,INMAZE
			ITPLACE  I
			  BIT   I,INMAZE
			     BIC   I,HINTABLE
			  FIN
			EOI
		    FIN
		 FIN
	      FIN
	   FIN
	FIN
LABEL	NO.MOVE.POSSIBLE
	IFNEAR   LAMP
	   AND
	IFEQ     LAMP,1
	   OR
	BIT   HERE,LIT
	   SAY   NOCANGO
	ELSE
	   CHANCE   25
	      SAY   CRUNCH
	      CALL  CORONER
	   ELSE
	      SAY   OOF!
	   FIN
	FIN
	IFNEAR   LAMP
	   AND
	IFEQ  LAMP,1
	   SUB   LAMPLIFE,1
	   IFEQ  LAMPLIFE,0
	      OR
	   IFEQ  LAMPLIFE,40
	      CALL  LAMPREY
	   FIN
	FIN
	QUIT
LABEL	BREAK.VIAL
	APPORT   VIAL,LIMBO
	SAY   VIAL.BANG
	RANDOM   I,LAST.FUME-FIRST.FUME+1
	LDA   J,FIRST.FUME
	ADD   I,J
	SAY   I
	SAY   BLANK
	IFNEAR   DWARF
	   IFEQ  DWARROWS,1
	      SET   VIAL,1
	   ELSE
	      SET   VIAL,2
	   FIN
	   SAY   VIAL
	   APPORT   DWARF,LIMBO
	   SUB   DWARFCOUNT,DWARROWS
	   SET   DWARROWS,0
	FIN
	IFNEAR   TROLL
	   SET   VIAL,3
	   SAY   VIAL
	FIN
	IFNEAR   BEAR
	   SET   VIAL,4
	   IFGT  BEAR,0
	      SET   VIAL,5
	   FIN
	   SAY   VIAL
	FIN
	IFNEAR   SNAKE
	   SET   VIAL,6
	   SAY   VIAL
	FIN
	IFNEAR   BIRD
	   SET   VIAL,7
	   SAY   VIAL
	FIN
	IFNEAR   SLIME
	   SET   VIAL,8
	   SAY   VIAL
	   APPORT   SLIME,LIMBO
	FIN
	IFNEAR   DRAGON
	   AND
	IFEQ  DRAGON,0
	   SET   VIAL,9
	   SAY   VIAL
	FIN
	IFNEAR   DJINN
	   SET   VIAL,10
	   SAY   VIAL
	FIN
	IFNEAR   BASILISK
	   AND
	IFLT  BASILISK,2
	   SET   VIAL,11
	   SAY   VIAL
	FIN
	IFNEAR   GOBLINS
	   SET   VIAL,12
	   SAY   VIAL
	   APPORT   GOBLINS,LIMBO
	FIN
	QUIT
LABEL	DROPVIAL
	IFHAVE   VIAL
	   AND
	CHANCE   10
	   SAY   VIAL.EXPLODES
	   APPORT   VIAL,LIMBO
	   CALL  CORONER
	FIN
LABEL	WEAPONRY.2     {handle more attack stuff}
	IFNEAR   BASILISK
	   IFGT  BASILISK,1
	      SAY   IT.IS.DEAD
	      IFKEY THROW
		 GET   ARG2
	      FIN
	   ELSE
	      NAME  AXE.BASILISK,ARG2
	      CALL  CORONER
	   FIN
	ELSE
	   IFNEAR   DJINN
	      NAME  REBOUND,ARG2
	   ELSE
	      IFNEAR   GOBLINS
		 NAME   KILL.A.FEW,ARG2
		 CALL  CORONER
	      ELSE
		 IFKEY THROW {throwing weapon with no target}
		    GET   ARG2  {will cause us to wade into the}
		    PROCEED  {cause us to throw it to another place}
		 ELSE
		    SAY   OK
		 FIN
	      FIN
	   FIN
	FIN
	QUIT
LABEL	WEAPONRY    {Handle attacks with weapons}
	IFHAVE   ARG2
	   IFKEY THROW
	      DROP  ARG2
	   FIN
	   IFNEAR   DWARF
	      SET   I,STRENGTH
	      SUB   I,INVCT
	      MULT  I,5
	      SET   J,DWARROWS
	      ADD   J,2
	      MULT  J,15
	      ADD   I,J
	      IFKEY AXE
		 EOR
	      IFKEY SWING
		 ADD   I,15
	      FIN
	      CHANCE   I
		 SAY   DWARF.POOF
		 SUB   DWARROWS,1
		 IFEQ  DWARROWS,0
		    APPORT   DWARF,LIMBO
		 FIN
		 SUB   DWARFCOUNT,1
	      ELSE
		 SAY   DWARFDODGES
		 SET   J,DWARROWS
		 ADD   J,1
		 RANDOM   J,J   {how many knives thrown?}
		 IFGT  J,0      {at least 1?}
		    IFEQ  J,1
			SAY   KNIFETHROWN
		    ELSE
			VALUE KNIVESTHROWN,J
		    FIN
		    BIT   DWARF,SPECIAL2 { (is he mad?) }
			SUB   I,20
		    FIN
		    CHANCE   I
			OR
		    BIT   DWARF,SPECIAL1
			IFEQ  J,1
			  SAY   MISSES
			ELSE
			  SAY   KNIVESMISS
			FIN
			BIC   DWARF,SPECIAL1
		    ELSE
			SAY   GETSYOU
			CALL  CORONER
		    FIN
		 FIN
	      FIN
	   ELSE
	      IFNEAR   SNAKE
		 SAY   CANTKILLSNAKE
		 IFKEY THROW
		    GET   ARG2
		 FIN
	      ELSE
		 IFNEAR   DRAGON
		    NAME   KILL.DRAGON,ARG2
		 ELSE
		    IFNEAR   BEAR
			IFEQ  BEAR,0
			  IFKEY THROW
			     IFKEY AXE
				SAY   AXE..BEAR
				SET   AXE,1
			     ELSE
				SAY   SWORD.MISSES
			     FIN
			  ELSE
			     CHANCE   50
				SAY   BEAR.MISSES
			     ELSE
				SAY   BEAR.GETS.YOU
				CALL  CORONER
			     FIN
			  FIN
			ELSE
			  SAY   BEAR.PUZZLED
			FIN
			QUIT
		    ELSE
			IFNEAR   TROLL
			  SAY   TROLL.DATA
			ELSE
			  IFNEAR   OGRE
			     IFKEY SWING
				NAME  OGRE.REBUFF,ARG2
				CALL  CORONER
			     ELSE
				IFKEY AXE
				   NAME  OGRE.CATCH,ARG2
				   CALL  CORONER
				ELSE
				   SAY   OGRE.KILLED
				   APPORT   SWORD,LIMBO
				   APPORT   OGRE,LIMBO
				   APPORT   RING,HERE
				   QUIT
				FIN
			     FIN
			  ELSE
			     IFNEAR   BLOB
				NAME  SLICE.BLOB,ARG2
				QUIT
			     ELSE
				CALL WEAPONRY.2
				PROCEED  {if no attack, use normal}
			     FIN   {"THROW" logic (see UPCHUCK)}
			  FIN
			FIN
		    FIN
		 FIN
	      FIN
	   FIN
	ELSE
	   NAME  YOUDONTHAVE,ARG2
	FIN
	QUIT
LABEL	PASSPHRASE
	IFNEAR   SAFE
	   IFEQ  SAFE,0
	      IFEQ  STATUS,2
		 AND
	      IFEQ  ARG2,PASSWORD
		 OR
	      IFEQ  ARG1,PASSWORD
		 SAY   SAFE.OPENS
		 SET   SAFE,1
		 BIS   SAFE,SPECIAL1
		 QUIT
	      ELSE
		 NOT
		 BIT   SAFE,SPECIAL1
		    SAY   SAFE.FUSES
		    SET   SAFE,2            {melt the safe's door shut}
		    SET   BLOB,1            {wake up the blob}
		    BIS   ADMIN,TICKER      {BLOB is chasing us - quickly!}
		    BIS   ADMIN,NOMAGIC     {inhibit PLUGH etc.}
		    SET   GRATE,0           {lock him in the cave}
		    QUIT
		 FIN
	      FIN
	   FIN
	FIN
	SAY   NOTHING
	QUIT
LABEL	TICK     {once-per-input routine}
	IFGT  BLOB,0
	   IFEQ  BLOB,16     {special case for blob}
	      SET   BLOB,17
	      SAY   BLOB
	      CALL  CORONER
	   FIN
	   SAY   BLOB
	   ADD   BLOB,1
	FIN
LABEL	PRESAY
	IFGT  STATUS,1
	   NAME  SAID,ARG2
	FIN
LABEL	PLUNGE   {for plunging into a bottomless pit}
	GOTO YLEM
	IFEQ  LAMP,1
	   SET   LAMPLIFE,0
	   IFHAVE   LAMP
	      SAY   FALL&STARVE
	   ELSE
	      SAY   FALL&STARVED
	   FIN
	ELSE
	   SAY   FALL&STARVED
	FIN
	CALL  CORONER
LABEL	UPCHUCK
	SET   I,0
	IFAT PIT
	   LDA   I,THROW.PIT
	   LDA   J,MISTS
	FIN
	IFAT EASTOFFISSURE
	   LDA   I,THROW.FISSURE
	   LDA   J,CAVERN
	FIN
	IFAT WESTOFFISSURE
	   LDA   I,THROW.FISSURE
	   LDA   J,CAVERN
	FIN
	IFAT WEND2PIT
	   LDA   I,THROW.PIT
	   LDA   J,WESTPIT
	FIN
	IFAT EEND2PIT
	   LDA   I,THROW.PIT
	   LDA   J,EASTPIT
	FIN
	IFAT LOWNSPASSAGE
	   LDA   I,THROW.HOLE
	   LDA   J,DIRTY
	FIN
	IFAT WINDOW
	   LDA   I,THROW.PIT
	   LDA   J,MIRRORCNYN
	FIN
	IFAT WINDOW2
	   LDA   I,THROW.PIT
	   LDA   J,MIRRORCNYN
	FIN
	IFAT BRINK
	   LDA   I,THROW.PIT
	   LDA   J,STREAMPIT
	FIN
	IFAT DUSTY
	   LDA   I,THROW.HOLE
	   LDA   J,COMPLEX
	FIN
	IFAT MAZEA.57.PIT
	   LDA   I,THROW.PIT
	   LDA   J,BIRDCHAMBER
	FIN
	IFAT SECRETNSCYN
	   LDA   I,THROW.ROOM
	   LDA   J,SLAB
	FIN
	IFAT SECRETNSCPAS
	   LDA   I,THROW.ROOM
	   LDA   J,BEDQUILT
	FIN
	IFAT SECRETEW.TITE
	   LDA   I,THROW.CANYON
	   LDA   J,NSCANYONWIDE
	FIN
	IFAT INCLINE
	   LDA   I,THROW.ROOM
	   LDA   J,LOW
	FIN
	IFAT CAVERN
	   LDA   I,THROW.WHIRLPOOL
	   LDA   J,YLEM
	FIN
	IFAT MISTY
	   LDA   I,THROW.CAVERN
	   LDA   J,CAVERN
	FIN
	IFAT STALACT
	   LDA   I,THROW.ROOM
	   LDA   J,MAZEA.53
	FIN
	IFAT  RESERVOIR
	   OR
	IFAT  RESERVOIR.N
	   LDA   I,THROW.RESERVOIR
	   LDA   J,YLEM
	FIN
	IFAT BALCONY
	   LDA   I,THROW.ROOM
	   LDA   J,YLEM
	FIN
	IFAT SWOFCHASM
	   LDA   I,THROW.CHASM
	   LDA   J,YLEM
	FIN
	IFAT NEOFCHASM
	   LDA   I,THROW.CHASM
	   LDA   J,YLEM
	FIN
	IFAT BREATHTAKER
	   LDA   I,THROW.GORGE
	   LDA   J,YLEM
	FIN
	IFAT FACES
	   LDA   I,THROW.GORGE
	   LDA   J,YLEM
	FIN
	IFAT TUBE
	   LDA   I,THROW.CHIMNEY
	   LDA   J,CHIMNEY
	FIN
	IFAT TUBE.SLIDE
	   LDA   I,THROW.TUBE
	   LDA   J,PLAIN.1
	FIN
	IFAT BASQUE.FORK
	   LDA   I,THROW.STEPS
	   LDA   J,ON.STEPS
	FIN
	IFAT ON.STEPS
	   LDA   I,THROW.STEPS
	   LDA   J,STEPS.EXIT
	FIN
	IFAT STEPS.EXIT
	   LDA   I,THROW.STEPS
	   LDA   J,STORAGE
	FIN
	IFAT BRINK.1
	   LDA   I,THROW.PIT
	   LDA   J,YLEM
	FIN
	IFAT BRINK.2
	   LDA   I,THROW.PIT
	   LDA   J,YLEM
	FIN
	IFAT ICE
	   LDA   I,THROW.SLIDE
	   LDA   J,SLIDE
	FIN
	IFAT BRINK.3
	   LDA   I,THROW.PIT
	   LDA   J,YLEM
	FIN
	IFAT SHELF
	   LDA   I,THROW.BEACH
	   LDA   J,BEACH
	FIN
	IFAT PLATFORM
	   LDA   I,THROW.GORGE
	   LDA   J,YLEM
	FIN
	IFEQ  I,0   {"THROWER" bit set but can't find target}
	   OR
	IFKEY BEAR     {you can't throw bear into chasm!}
	   PROCEED
	FIN
	NAME  I,ARG2
	APPORT   ARG2,J
	IFKEY VASE
	   APPORT   VASE,YLEM
	   APPORT   SHARDS,J
	   LDA   I,YLEM
	   IFEQ  I,J
	   ELSE
	      SAY   SHATTERED.IT
	   FIN
	FIN
	IFKEY BOTTLE
	   APPORT   OIL,LIMBO
	   APPORT   WATER,LIMBO
	FIN
	IFKEY OIL
	   OR
	IFKEY WATER
	   SET   BOTTLE,1    {threw liquid away - drain bottle}
	   APPORT   ARG2,LIMBO
	FIN
	IFKEY CAGE
	   AND
	IFHAVE   BIRD
	   APPORT   BIRD,J
	FIN
	IFKEY LAMP
	   AND
	IFEQ  LAMP,1
	   AND
	   NOT
	BIT   HERE,LIT
	   SAY   ITISNOWDARK
	FIN
	IFKEY BIRD
	   SET   BIRD,0
	FIN
	QUIT
LABEL	GROPE.FOR.IT   {grope around in the dark for objects}
	IFEQ  STATUS,1 {did jhe say what to get?}
	   SAY   CANT.SEE.ANYTHING
	   QUIT
	FIN
	BIT   ARG2,OBJECT
	   IFHAVE   ARG2
	      OR
	      NOT
	   BIT   ARG2,PORTABLE
	      PROCEED
	   FIN
	   SET   I,INVCT
	   SUB   I,STRENGTH
	   MULT  I,5
	   ADD   I,60
	   CHANCE   I
	      NAME  GROPE.FALL,ARG2
	      CALL  CORONER
	   ELSE
	      CHANCE   50
		 OR
		 NOT
	      IFNEAR   ARG2
		 NAME  GROPE.MISS,ARG2
		 QUIT
	      ELSE
		 NAME  GROPE.FIND,ARG2
		 PROCEED
	      FIN
	   FIN
	FIN
