*
*	Define control bits
*
*	First: bits set in ARG1 and ARG2 to indicate word type
*
SYNON    15,OBJECT
SYNON    14,VERB
SYNON    13,PLACE
SYNON    12,BADWORD
*
*	Next, bits set in STATUS variable
*
SYNON    0,MOVED       * We moved last turn
SYNON    1,QUICKIE      * BRIEF output mode in effect
SYNON    2,FASTMODE     * FAST output mode in effect
SYNON    3,FULLDISP     * Display full place description
*
*	Next, bits related to places
*
SYNON    0,LIT          * Place is self-illuminated
SYNON    1,BEENHERE     * We've been here at least once
SYNON    2,NODWARF      * Dwarves can't go here
SYNON    3,NOBACK       * Can't use BACK to go to/from this place
SYNON    4,NOTINCAVE    * This place is not in the cave
SYNON    5,HINTABLE     * There may be a hint for this place
SYNON    6,H20HERE      * Water is available here
SYNON    7,INMAZE       * This room is in one of the mazes
SYNON    8,ONE.EXIT     * Only one exit out of room - dwarves can block
*			* your way and force you to fight them.
SYNON    9,THROWER      * Throwing objects here will send them elsewhere,
*			* unless you're throwing them at something
*			* special (like a troll or dwarf).
*
*	Next, bits related to objects
*
SYNON    0,PORTABLE     * Object can be carried
SYNON    1,SEEN         * We've seen the object
SYNON    2,VALUED       * It's a treasure - must be left in the building
SYNON    3,SCHIZOID     * It's schizoid - in 2 places at once
SYNON    4,UNSTABLE     * Unstable objects get kicked from state 0
*			into state 1 the first time they're picked
*			up - applies only to objects that aren't
*			special-cased anywhere in the code.
SYNON    5,MORTAL       * This is a killable entity
SYNON    6,OPENABLE     * Object can be opened/unlocked
SYNON    7,INVISIBLE    * Object can't be seen.
SYNON    8,EDIBLE       * Object can be eaten (food/mushroom)
SYNON    9,FREEBIE      * Object is effectively weightless
SYNON    10,SPECIAL1    * Special control bit 1 - depends on context
SYNON    11,SPECIAL2    * Special control bit 2 - depends on context
*
*	Define bits for the ADMIN variable
*
SYNON    0,DEMO              * Demonstration game
SYNON    1,TICKER             * Call TICK label after each move, if set.
SYNON    3,NOMAGIC           * Magic words are being suppressed
SYNON    4,PANICED           * He paniced during closure
SYNON    5,OLORIN             * He's a wizard
SYNON    6,RANOUT             * His lamp just died.
*
*	Define administrative control parameters
*
SYNON    30,MINTIME     {# of minutes you must wait after SAVE}
SYNON    30,MAX.DEMO    {maximum # of moves in demo games}
SYNON    600,MAX.GAME   {maximum # of moves in real games}
SYNON    20,HINT.COST   {what each hint costs you}
*
*	Define worth of each CLOSURE phase
*
SYNON    20,CLOSE.CREDIT   {each phase of CLOSURE is worth 20 points}
*
*	The phases of cave closure are as follows:
*
*	0 - cave open and operating normally
*	1 - ditto - all treasures are in well-house
*	2 - "Cave closing soon"
*	3 - "Cave closed" - in cylindrical room
*	4 - escaped from cylindrical room, back outside cave
*	5 - located the treasure room, wins game!
*
