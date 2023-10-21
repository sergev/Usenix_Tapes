%{

/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Insult -- a program to generate nasty insults. This is a kafka source file.
 *	Compile with etc.c, output.c, and maketext.c
 */

#include <stdio.h>
#include <ctype.h>

#define CH(n, o)	return (((rnum() % o) < n) ? 1 : 0)

rnum()
{
	return (random());
}

char *ones;

#define YOUR	ones = "your"
#define MY	ones = "my"
#define HIS	ones = "his"

/* Instead of defining all these words as rules, keep arrays of them. We
 * can also make the insults more interesting by using alliteration on
 * the words...
 */

char *words1[] = { 	/* Animals. */
	"aardvark",
	"amphibian",
	"animal",
	"ant",
	"apteryx",
	"bird",
	"buzzard",
	"cat",
	"cockroach",
	"cow",
	"dodo",
	"dog",
	"duck",
	"emu",
	"fish",
	"flea",
	"frog",
	"gazelle",
	"gerbil",
	"goat",
	"goose",
	"grep",
	"haddock",
	"hippotomus",
	"hog",
	"horse",
	"insect",
	"jackal",
	"kitten",
	"kobold",
	"leech",
	"louse",
	"milkweed",
	"monkey",
	"moose",
	"mouse",
	"octopus",
	"penguin",
	"pig",
	"pigeon",
	"puppy",
	"racoon",
	"rat",
	"rust monster",
	"sheep",
	"skunk",
	"slime-mold",
	"slug",
	"snail",
	"snake",
	"squid",
	"toad",
	"tuna",
	"turkey",
	"turtle",
	"weed",
	"whale",
	"wombat",
	"worm",
	"yak",
	NULL
} ;

char *words2[] = {	/* Derogatory terms. */
	"airhead",
	"axe murderer",
	"beer", 
	"bug-breath",
	"bum",
	"cookie",
	"creep",
	"cretin",
	"crud",
	"deviate",
	"dink",
	"dipstick",
	"dirt",
	"dork",
	"dude",
	"dumbbell",
	"fairy",
	"fleabag",
	"fruit",
	"garbage",
	"geek",
	"gimp",
	"glob",
	"gloop",
	"goober",
	"goof",
	"grunt",
	"gump",
	"hag",
	"idiot",
	"ignoramus",
	"inanimate object",
	"jerk",
	"lark's vomit",
	"lump",
	"lunkhead",
	"monster",
	"mudface",
	"mudpie",
	"muscle-brain",
	"nasty",
	"numskull",
	"nymphomaniac",
	"pervert",
	"pinhead",
	"plant",
	"plebe",
	"quiche-eater",
	"rat spit",
	"redneck",
	"scary dude",
	"scum that grows on the bottom of the lake",
	"scum",
	"septic tank",
	"slob",
	"super geek",
	"toady",
	"vegetable",
	"weight-lifter",
	"welfare mother",
	"wonderful, sensitive human being",
	NULL
} ;

char *words3[] = {	/* Body parts. */
	"behind",
	"brain",
	"crotch",
	"ear",
	"eye",
	"face",
	"liver",
	"nose",
	"pancreas",
	"private parts",
	"rectum",
	"stomach",
	NULL
} ;

char *words4[] = {	/* Objects. */
	"VAX",
	"baseball bat",
	"big thing",
	"boulder",
	"brick",
	"car",
	"heavy object",
	"knife",
	"manhole cover",
	"needle",
	"object",
	"rock",
	"sword",
	"terminal",
	"tree",
	"truck",
	"two by four",
	NULL
} ;

char *words5[] = {	/* Adjectives. */
	"Lovecraftian",
	"abnormal",
	"absurd",
	"awful",
	"bad",
	"bootlicking",
	"brainless",
	"cowardly",
	"crude",
	"dastardly",
	"dirty",
	"diseased",
	"disgusting",
	"disobedient",
	"dopey",
	"dumb",
	"elitist",
	"evil",
	"fat",
	"flabby",
	"foolish",
	"foul",
	"frightening",
	"funny",
	"furry",
	"green",
	"gruesome",
	"gumpy",
	"hairy",
	"hideous",
	"horrifying",
	"icky",
	"ignorant",
	"imaginary",
	"intolerable",
	"lousy",
	"ludicrous",
	"lumpy",
	"mangy",
	"muddy",
	"nauseating",
	"nauseating",
	"old",
	"pencil-necked",
	"perverted",
	"ratty",
	"religous",
	"ridiculous",
	"rotten",
	"scab-picking",
	"scummy",
	"sick",
	"silly",
	"skinny",
	"slimy",
	"slippery",
	"slobbering",
	"slow",
	"small",
	"smelly",
	"strange",
	"stupid",
	"trashy",
	"ugly",
	"undesirable",
	"unhappy",
	"unpleasant",
	"unspeakable",
	"unstructured",
	"uppity",
	"vile",
	"worthless",
	NULL
} ;

char *words6[] = {	/* Adverbs. */
	"disgustingly",
	"incredibly",
	"mega",
	"microscopically",
	"paradoxically",
	"phenomenally",
	"pseudo",
	"rather",
	"ridiculously",
	"semi",
	"slightly",
	"stupidly",
	"surprisingly",
	"terribly",
	"typically",
	"unbeliveably",
	NULL
} ;

char *words7[] = {	/* Nasty things. */
	"abuse",
	"annihilate",
	"avoid",
	"behead",
	"chop up",
	"damage",
	"destroy",
	"discriminate against",
	"disembowel",
	"dismember",
	"dissect",
	"eat",
	"flush",
	"forget about",
	"frighten",
	"hit",
	"ignore",
	"insult",
	"kick",
	"look at",
	"molest",
	"mush",
	"puree",
	"scramble",
	"shred",
	"spit on",
	"squish",
	"stab",
	"step on",
	NULL
} ;

char *words8[] = {	/* Diseases. */
	"bad breath",
	"gout",
	"leprosy",
	"malaria",
	"plague",
	NULL
} ;

char *lastchoice = NULL;

getword(type)
{
	char **wds;
	int i = 0, j = 0, k;
	int alliterate = lastchoice ? (random() % 4) : 0;

	switch (type) {
		case 1: wds = words1; break;
		case 2: wds = words2; break;
		case 3: wds = words3; break;
		case 4: wds = words4; break;
		case 5: wds = words5; break;
		case 6: wds = words6; break;
		case 7: wds = words7; break;
		case 8: wds = words8; break;
		default: fprintf(stderr, "Bad thing %d\n", type);
	}

	/* See how many words there are that we can use. */
	for (k = 0; wds[k]; k++) {
		if (alliterate && (*wds[k] == *lastchoice))
			j++;
	}
	i = k;
	if (j == 0) {
hoho:
		i = rnum() % i;
		lastchoice = wds[i];
		return ((int) wds[i]);
	} else {
		j = rnum() % j;
		for (k = 0; ; k++)
			if (*wds[k] == (isupper(*lastchoice) ?
					tolower(*lastchoice) : *lastchoice)) {
				if (wds[k] == lastchoice)
					goto hoho;
				if (j == 0) {
					lastchoice = wds[k];
					return ((int) wds[k]);
				} else
					j--;
			}
	}
}

/* Word types. */

#define ANIMAL		1
#define DER_TERM	2
#define BODYPART	3
#define OBJECT		4
#define ADJ		5
#define ADVERB		6
#define NASTY_THING	7
#define DISEASE		8

%}

%%

<start>		= <invective> ;

<invective>	= you <der-term> <ep> 	{ CH(1, 20); } ;
<invective>	= <action> <ep> 	{ YOUR; CH(1, 20); } ;
<invective>	= "why you" <der-term> , <invective> 	{ YOUR; CH(1, 20); } ;
<invective>	= "you just make me" <adj> <pp> 	{ YOUR; CH(1, 20); } ;
<invective>	= "you're a perfect example of a" <der-term> <pp> 	{ YOUR; CH(1, 20); } ;
<invective>	= "you make me want to" <action> <pp> 	{ YOUR; CH(1, 25); } ;
<invective>	= <exclam> <ep> 	{ MY; CH(1, 20); } ;
<invective>	= "you make me want to" <action> <pp> 	{ MY; CH(1, 30); } ;
<invective>	= "why don't you" <action> <qp> 	{ YOUR; CH(1, 25); } ;
<invective>	= "I'd like to" <nasty-thing> you <ep> 	{ CH(1, 30); } ;
<invective>	= "you should" <action> <pp> 	{ YOUR; CH(1, 25); } ;
<invective>	= "I wish you'd" <action> <pp> 	{ YOUR; CH(1, 25); } ;
<invective>	= "why don't you" <action> <qp> 	{ YOUR; CH(1, 25); } ;
<invective>	= "you could make a" <animalp> <action> <pp> 	{ HIS; CH(1, 40); } ;
<invective>	= go <action> <pp> 	{ YOUR; CH(1, 20); } ;
<invective>	= "do me a favor," <der-term> ", and" <action> <pp> { YOUR; CH(1, 35); } ;
<invective>	= "you resemble a" <animalp> <pp> { CH(1,25); } ;
<invective>	= "make the world a better place and" <action> <pp> { YOUR; CH(1,40); } ;
<invective>	= "why you" <der-term> , <invective>  { CH(1,30); } ;
<invective>	= "I used to know a" <der-term> "just like you" <pp> { CH(1,40); } ;
<invective>	= <der-term> <ep> { CH(1,25); } ;
<invective>	= "are you a" <der-term> ", or just a" <der-term> <qp> { CH(1, 40); } ;
<invective>	= <exclam> ", you're a" <der-term> <ep> { CH(1, 40); } ;
<invective>	= "you're just such a" <der-term> <pp> { CH(1,20); } ;
<invective>	= "you should" <nasty-thing> yourself <pp> { CH(1,35); } ;
<invective>	= "you're so" <adj> <pp> { CH(1,20); } ;
<invective>	= "oh, go" <action> <ep> { YOUR; CH(1,20) ; } ;
<invective>	= hey, <der-term> <pp> "Yeah, you!" { CH(1,20) ; } ;
<invective>	= get <disease> <pp> { CH(1, 20); } ;
<invective>	= "you look like you would" <action> <pp> { YOUR; CH(1, 20); } ;
<invective>	= "you look like you would" <action> <pp> { MY; CH(1, 20); } ;
<invective>	= "what a" <der-term> "you are" <ep> { CH(1, 20); } ;
<invective>	= "you must really be a" <animalp> <pp> { CH(1, 20); } ;
<invective>	= "even a" <animalp> "would want you to" <action> <pp> { HIS; CH(1, 20); } ;
<invective>	= "you're even more" <adj> "than a" <der-term> <ep> { CH(1, 20); } ;
<invective>	= "if I were you I'd" <action> <pp> { MY; CH(1, 30); } ;
<invective>	= "oh, I get it, you're a" <der-term> <pp> { CH(1, 30); } ;
<invective>	= "why don't you just" <action> "and get it over with" <qp> { YOUR; CH(1, 20); } ;
<invective>	= "you look like a" <thing> <pp> { CH(1, 20); } ;
<invective>	= "I don't know about you" <pp> { CH(1, 20); } ;
<invective>	= "hey," <exclam> "," <action> <ep> { YOUR; CH(1, 20); } ;
<invective>	= "whenever I" <action> "it gives me" <disease> <pp> { YOUR; CH(1, 20); } ;
<invective>	= "I wonder why" <invective> { CH(1, 20); } ;
<invective>	= "you make me want to become" <adj> ", you know" <qp> { CH(1, 20); } ;
<invective>	= <exclam> "," <invective> { CH(1, 20); } ;
<invective>	= "if only you were a" <der-term> <pp> { CH(1, 20); } ;
<invective>	= <invective> ;

<thing>		= <noun> ;

<animalp>	= <adjp> <animal> { CH(1, 3); } ;
<animalp>	= <animal> ;

<animal>	= { return (getword(ANIMAL)); } ;

<der-term>	= <adjp> <der-term-1>	{ CH(1, 3); } ;
<der-term>	= <der-term-1> ;

<der-term-1>	= <animalp> { CH(1, 4) ; } ;
<der-term-1>	= <animalp> "#'s " <bodypart> { CH(1, 10); } ;
<der-term-1>	= <adj> "#ness" { CH(1, 10) ; } ;
<der-term-1>	= <der-term> squad { CH(1, 25) ; } ;
<der-term-1>	= "son of a" <der-term> { CH(1, 25) ; } ;
<der-term-1>	= <bodypart> { CH(1, 25) ; } ;
<der-term-1>	= <der-term> "#y" <der-term> { CH(1, 25) ; } ;
<der-term-1>	= <bodypart> { CH(1, 25) ; } ;
<der-term-1>	= <adj> "#ness" { CH(1, 25) ; } ;
<der-term-1>	= <der-term-x> ;

<der-term-x>	= { return (getword(DER_TERM)); } ;

<bodypart>	= <adjp> <bodypart> { CH(1, 10) ; } ;
<bodypart>	= <bodypart-x> ;

<bodypart-x>	= { return (getword(BODYPART)); } ;

<object>	= <animalp> { CH(1, 5); } ;
<object>	= "pile of" <thing> "#s" { CH(1, 15); } ;
<object>	= "bunch of" <thing> "#s" { CH(1, 15); } ;
<object>	= <object-x> ;

<object-x>	= { return (getword(OBJECT)); } ;

<adjp>		= <adverb> <adj>	{ CH(1, 10); } ;
<adjp>		= <adj> <adj> { CH(1, 10) ; } ;
<adjp>		= <adj>	;

<adj>		= <animal> "#-infested" { CH(1, 40) ; } ;
<adj>		= <animal> "#ish" { CH(1, 40) ; } ;
<adj>		= <noun> "#-loving" { CH(1, 40) ; } ;
<adj>		= <noun> "#-eating" { CH(1, 40) ; } ;
<adj>		= <noun> "#like" { CH(1, 40) ; } ;
<adj>		= <disease> "#-infested" { CH(1, 40) ; } ;
<adj>		= <animal> <bodypart> "#ed" { CH(1, 40) ; } ;
<adj>		= <adj-x> ;

<adj-x>		= { return (getword(ADJ)); } ;

<adverb>	= <adj> "#ly" { CH(1, 50) ; } ;
<adverb>	= <adverb-x> ;

<adverb-x>	= { return (getword(ADVERB)); } ;

<nasty-thing>	= <nasty-thing-x> ;

<nasty-thing-x>	= { return (getword(NASTY_THING)); } ;

<disease>	= { return (getword(DISEASE)); } ;

<noun>		= <animal>	{ CH(2, 3); } ;
<noun>		= <object> ;

<action>	= eat a <thing>  { CH(1, 30) ; } ;
<action>	= lick a <animalp> { CH(1, 30) ; } ;
<action>	= lick a <animalp> "#'s" <bodypart> { CH(1, 30) ; } ;
<action>	= <nasty-thing> a <animalp> "#'s" <bodypart> { CH(1, 30) ; } ;
<action>	= "have sex with a" <animalp> { CH(1, 30) ; } ;
<action>	= die { CH(1, 30) ; } ;
<action>	= become a <der-term>  { CH(1, 30) ; } ;
<action>	= <nasty-thing> <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= "go out and meet a nice" <der-term> { CH(1, 30) ; } ;
<action>	= "adopt a" <animalp> { CH(1, 30) ; } ;
<action>	= become <adj> { CH(1, 30) ; } ;
<action>	= vomit { CH(1, 30) ; } ;
<action>	= "gag on a" <object> { CH(1, 30) ; } ;
<action>	= stick a <object> in <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= stick a <object> up <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= "cut off" <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= "make somebody happy and" <action> { CH(1, 30); } ;
<action>	= "live with a" <thing> { CH(1, 30) ; } ;
<action>	= put a <thing> in <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= die  { CH(1, 30) ; } ;
<action>	= "admit that you're" <adj>  { CH(1, 30) ; } ;
<action>	= "blow up" <ones> house  { CH(1, 30) ; } ;
<action>	= remove <ones> files  { CH(1, 30) ; } ;
<action>	= "sleep on the freeway"  { CH(1, 30) ; } ;
<action>	= take <adj> "lessons from a" <thing> { CH(1, 30) ; } ;
<action>	= "realize how" <adj> "you are"  { CH(1, 30) ; } ;
<action>	= relocate <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= "crawl under a" <object> { CH(1, 30) ; } ;
<action>	= pick <ones> <bodypart> { CH(1, 30) ; } ;
<action>	= "go blind" { CH(1, 30) ; } ;
<action>	= make <ones> <bodypart> <adj> { CH(1, 30) ; } ;
<action>	= "go away" { CH(1, 30) ; } ;
<action>	= "get a" <animalp> to <action> { CH(1, 30) ; } ;
<action>	= get <disease> { CH(1, 30) ; } ;
<action>	= "make a" <animalp> get <disease> { CH(1, 30) ; } ;
<action>	= stick <ones> <bodypart> "in a" <thing> { CH(1, 30) ; } ;
<action>	= <action> and <action> { CH(1, 10); } ;
<action>	= <action> ;

<exclam>	= "oh god"	{ CH(1, 10); } ;
<exclam>	= "wow, man"	{ CH(1, 10); } ;
<exclam>	= goodness	{ CH(1, 10); } ;
<exclam>	= "ah say"	{ CH(1, 10); } ;
<exclam>	= "ain't it the truth"	{ CH(1, 10); } ;
<exclam>	= <adj> <thing> "#s"	{ CH(1, 10); } ;
<exclam>	= gosh	{ CH(1, 10); } ;
<exclam>	= "land 'o goshens"	{ CH(1, 10); } ;
<exclam>	= well, <nasty-thing> <ones> <bodypart>	{ CH(1, 10); } ;
<exclam>	= "lemme tell ya" { CH(1, 10); } ;
<exclam>	= "sure as day"	{ CH(1, 10); } ;
<exclam>	= shoot	{ CH(1, 10); } ;
<exclam>	= sheeeit { CH(1, 10); } ;
<exclam>	= "hot puppies" { CH(1, 10); } ;
<exclam>	= well, <exclam> { CH(1, 10); } ;
<exclam>	= <exclam> ;

<pp>		= . { lastchoice = NULL; return (1); } ;
<ep>		= ! { lastchoice = NULL; return (1); } ;
<qp>		= ? { lastchoice = NULL; return (1); } ;

<ones>		= { return ((int) ones); } ;

