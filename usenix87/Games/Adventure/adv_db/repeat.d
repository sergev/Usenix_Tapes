REPEAT
	BIT   ADMIN,TICKER
	   CALL  TICK
	FIN
	BIT   STATUS,MOVED
	ELSE
	   PROCEED
	FIN
	BIT   THERE,ONE.EXIT
	   AND
	IFLOC DWARF,THERE
	   GOTO  THERE
	   BIC   STATUS,MOVED
	   SAY   DWARFBLOCK
	   PROCEED
	FIN
	IFNEAR   FOG {must be done before clearing MOVED flag}
	   CALL  PHOG
	FIN
	BIC   STATUS,MOVED
	ADD   MOVES,1
	BIT   ADMIN,DEMO
	   SET   I,MAX.DEMO
	ELSE
	   SET   I,MAX.GAME
	FIN
	IFEQ  MOVES,I
	   SAY   WIZARD.ENDS
	   CALL  FINIS
	FIN
	IFNEAR   LAMP
	   IFEQ  LAMP,1
	      SUB   LAMPLIFE,1
	      IFEQ  LAMPLIFE,40
		 OR
	      IFEQ  LAMPLIFE,0
		 CALL  LAMPREY
	      FIN
	   FIN
	FIN
	IFLOC GOBLINS,LIMBO {resting place}
	ELSE
	   APPORT   GOBLINS,HERE
	   IFGT  GOBLINS,-1
	      SAY   GOBLIN.CHASE
	   FIN
	   ADD   GOBLINS,1
	FIN
	SET   K,0
	BIT   HERE,LIT
	   SET   K,1
	ELSE
	   IFNEAR   LAMP
	      IFEQ  LAMP,1
		 SET   K,1
	      FIN
	   FIN
	FIN
	BIT   HERE,BEENHERE
	   AND
	BIT   STATUS,QUICKIE
	   OR
	BIT   STATUS,FASTMODE
	   SET   J,0   {list objects immediately after place description}
	ELSE
	   SET   J,1   {stick blank line after place description}
	FIN
	IFEQ  K,1
	   SAY   HERE
	   BIT   HERE,BEENHERE
	      SET   K,0
	   FIN
	   BIS   HERE,BEENHERE
	   ITLIST I
	      IFNEAR   I
		 AND
		 NOT
	      IFHAVE   I
		 BIS   I,SEEN
		 IFEQ  J,1
		    AND
		    NOT
		 BIT   I,INVISIBLE
		    SAY   BLANK
		    SET   J,0
		 FIN
		 SAY   I
	      FIN
	   EOI
	   IFHAVE   BEAR
	      SAY   I.C.A.BEAR
	   FIN
	ELSE
	   BIT   THERE,LIT
	      OR
	   CHANCE   75
	      OR
	   BIT   ADMIN,RANOUT   {don't fall if lamp just died}
	      SAY   ITISNOWDARK
	   ELSE
	      SAY   CRUNCH
	      CALL  CORONER
	   FIN
	FIN
	BIC   ADMIN,RANOUT      {clear "lamp just died"}
	IFAT  Y2
	   CHANCE   35
	      SAY   SAYSPLUGH
	   FIN
	FIN
	IFNEAR   GOBLINS
	   ADD   GOBLINS,1
	   IFGT  GOBLINS,6  {highest existing state}
	      CALL  CORONER
	   FIN
	FIN
	IFLOC DWARF,LIMBO
	ELSE
	   BIT   HERE,NOTINCAVE
	      OR
	   BIT   HERE,NODWARF
	   ELSE
	      APPORT   DWARF,HERE
	   FIN
	FIN
	BIT   HERE,NOTINCAVE
	ELSE
	   SUB   CLOCK,2
	   SUB   CLOCK,K     {compensating factor for new rooms}
	   IFLT  CLOCK,1
	      CALL  CLOCK4
	   FIN
	FIN
	IFNEAR   DWARF
	   BIC   PIRATE,SPECIAL1
	   IFGT  DWARFCOUNT,0
	      IFEQ  DWARROWS,1
		 SAY   DWARFHERE
	      ELSE
		 VALUE DWARVESHERE,DWARROWS
	      FIN
	      SET   J,DWARROWS
	      ADD   J,4
	      RANDOM   J,J
	      SUB   J,3
	      IFGT  J,0
		 IFEQ  J,1
		    SAY   KNIFETHROWN
		 ELSE
		    VALUE KNIVESTHROWN,J
		 FIN
		 SET   I,INVCT
		 MULT  I,-5
		 ADD   I,95     {it's hard to hit a moving target}
		 BIT   DWARF,SPECIAL2  { (is he mad?) }
		    SUB   I,20
		 FIN
		 IFEQ  MUSHROOM,2
		    ADD   I,25
		 FIN
		 DIVIDE I,J     {slim chance if more than 1 knife!}
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
		    IFEQ  J,1
			SAY   GETSYOU
		    ELSE
			SAY   KNIFEGOTYOU
		    FIN
		    CALL  CORONER
		 FIN
	      FIN
	   FIN
	FIN
	IFLOC LAMP,YLEM
	   AND
	IFAT  ROAD
	   AND
	IFLT  CLOSURE,4      {lamp vanishes during end-game}
	   SAY   LAMP.DEAD!
	   SET   QUITTING,1
	   CALL  FINIS
	FIN
REPEAT
	BIT   HERE,HINTABLE
	   ADD   HINT.TIME,1
	   IFGT  HINT.TIME,30
	      AND
	      NOT
	   BIT   HERE,INMAZE
	      OR
	   IFGT  HINT.TIME,50
	      CALL  HINT.LOGIC
	   FIN
	ELSE
	   SET   HINT.TIME,0
	FIN
	IFEQ  BLOB,16
	   APPORT   BLOB,HERE
	FIN
	SET   INVCT,0
	ITOBJ I
	   IFHAVE   I
	      AND
	      NOT
	   BIT   I,FREEBIE
	      ADD   INVCT,1
	   FIN
	EOI
	INPUT
	SUB   FOOBAR,1
	ADD   TURNS,1
	IFEQ  STATUS,0
	   QUIT
	FIN
	BIT   ARG1,BADWORD
	   SAY   WHAT?
	   QUIT
	FIN
	IFKEY SAY
	   CALL  PRESAY
	FIN
	IFGT  STATUS,1
	   BIT   ARG2,BADWORD
	      LDA   I,RESTORE   {last special command}
	      IFGT  ARG1,I
		 NAME  NOCOMPRENDE,ARG2
		 QUIT
	      FIN
	   FIN
	FIN
	CALL  HERE
	BIT   ARG1,PLACE
	   IFAT  ARG1
	      SAY   YOU.ARE.THERE
	   ELSE
	      SAY   NO.CAN.GO
	   FIN
	ELSE
	   CALL ARG1
	   BIT   ARG1,OBJECT
	      IFNEAR   ARG1
		 NAME  WHAT.DO,ARG1
	      ELSE
		 NAME  IDONTSEE,ARG1
	      FIN
	   ELSE
	      CALL  BAILOUT
	   FIN
	FIN
