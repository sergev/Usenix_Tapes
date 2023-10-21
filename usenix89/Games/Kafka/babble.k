
# RCS Info: $Revision: $ on $Date: $
#           $Source: $
# Copyright (c) 1985 Wayne A. Christopher 
#	Permission is granted to do anything with this code except sell it
#	or remove this message.
#
# The following program is written in 'kafka', a sort of BNF with imbedded
# C code that directs the selection of rules. (CH(x,y) is a macro that
# means basically "accept this rule x out of y times".) Otherwise rules
# are checked from top to bottom.  The program that takes this specification
# and turns it into a C program (which then spits out strings) can be found
# in ~/bin/kafka, and the source and other useful things can be found in
# ~/kafka. (Some on ernie, in ~faustus/kafka.)
# The program should be pretty self-explanatory, though -- it is
# supposed to look a bit like lex and yacc specifications...
# By the way, rules are written <non-terminal> = stuff { C code } ;
# where the stuff can be any mix of non-termnals (surrounded by <>) and
# terminals. The C code should return (1) if the rule is to be used and 0
# if not. If there is no stuff, the C code can return a pointer to a string
# that is taken to be a terminal, but I'm not using this because it isn't
# very neat and is only useful when you are doing non-context free stuff...

%{

#define CH(n, o)	return ((((random() - 1313513) % o) < n) ? 1 : 0)
/* I think there used to be a reason for doing ^^^ this... */

%}

%%

<start>		= <unit> 				;

<unit>		= <s> "." 				{ CH(3, 4); } ;
<unit>		= <rnd> 				;

# We have to be explicit about what kind of nouns go with what verbs.

<s>		= <s> , <Conj> <s> 			{ CH(1, 25); } ;

# verb1 is sort of simple physical stuff (transitive), non-intentional.
<s>		= <thing> <verb1> <thing> 		{ CH(1, 20); } ;

<verb1>		= "fell	on"				{ CH(1, 20); } ;
<verb1>		= was <in-rel-to>			{ CH(1, 20); } ;
<verb1>		= hit					{ CH(1, 20); } ;
<verb1>		= "was in fact"				{ CH(1, 20); } ;
<verb1>		= "was almost as" <adj1> as		{ CH(1, 20); } ;
<verb1>		= "was as" <adj1> as			{ CH(1, 20); } ;
<verb1>		= "was more" <adj1> than		{ CH(1, 20); } ;
<verb1>		= "may have been" <in-rel-to>		{ CH(1, 20); } ;
<verb1>		= "looked like"				{ CH(1, 20); } ;
<verb1>		= reminded <agent> of			{ CH(1, 20); } ;
<verb1>		= "seemed to be"			{ CH(1, 20); } ;
<verb1>		= <verb1>				;

# verb2 is intransitive sorts of things, and complex transitive verbs.
<s>		= <agent> <verb2> 			{ CH(1, 20); } ;

<verb2>		= said "'" <s> "'"			{ CH(1, 20); } ;
<verb2>		= "said that" <s>			{ CH(1, 20); } ;
<verb2>		= ran					{ CH(1, 20); } ;
<verb2>		= died					{ CH(1, 20); } ;
<verb2>		= exploded				{ CH(1, 20); } ;
<verb2>		= ate					{ CH(1, 20); } ;
<verb2>		= became <adj1>				{ CH(1, 20); } ;
<verb2>		= was <adj1>				{ CH(1, 8); } ;
<verb2>		= seemed <adj1>				{ CH(1, 10); } ;
<verb2>		= jumped				{ CH(1, 20); } ;
<verb2>		= walked				{ CH(1, 20); } ;
<verb2>		= fell					{ CH(1, 20); } ;
<verb2>		= disappeared				{ CH(1, 20); } ;
<verb2>		= "hopped around"			{ CH(1, 20); } ;
<verb2>		= flew					{ CH(1, 20); } ;
<verb2>		= used <thing> "as a" <Inan-obj>	{ CH(1, 20); } ;
<verb2>		= gave <thing> to <agent>		{ CH(1, 20); } ;
<verb2>		= "bit the dust"			{ CH(1, 20); } ;
<verb2>		= <verb2>				;

# verb3 is transitive intentional things.
<s>		= <agent> <adv> <verb3> <thing>		{ CH(1, 5); } ;

<verb3>		= hit					{ CH(1, 20); } ;
<verb3>		= used					{ CH(1, 20); } ;
<verb3>		= "acted like"				{ CH(1, 20); } ;
<verb3>		= "thought he was"			{ CH(1, 20); } ;
<verb3>		= "looked like"				{ CH(1, 20); } ;
<verb3>		= ate					{ CH(1, 20); } ;
<verb3>		= "threw out"				{ CH(1, 20); } ;
<verb3>		= "looked at"				{ CH(1, 20); } ;
<verb3>		= disliked				{ CH(1, 20); } ;
<verb3>		= "cooked and ate"			{ CH(1, 20); } ;
<verb3>		= destroyed				{ CH(1, 20); } ;
<verb3>		= lifted				{ CH(1, 20); } ;
<verb3>		= stole					{ CH(1, 20); } ;
<verb3>		= dropped				{ CH(1, 20); } ;
<verb3>		= bought				{ CH(1, 20); } ;
<verb3>		= "heard about"				{ CH(1, 20); } ;
<verb3>		= considered				{ CH(1, 20); } ;
<verb3>		= "interacted with"			{ CH(1, 20); } ;
<verb3>		= painted				{ CH(1, 20); } ;
<verb3>		= <verb3>				;

# verb4 is agent-agent stuff.
<s>		= <agent> <verb4> <agent> 		{ CH(1, 20); } ;

<verb4>		= hated					{ CH(1, 20); } ;
<verb4>		= killed				{ CH(1, 20); } ;
<verb4>		= loved					{ CH(1, 20); } ;
<verb4>		= "disagreed with"			{ CH(1, 20); } ;
<verb4>		= imitated				{ CH(1, 20); } ;
<verb4>		= "talked to"				{ CH(1, 20); } ;
<verb4>		= <verb4>				;

# verb5 is agent-intangible stuff. (I don't know about this...)
<s>		= <agent> <verb5> <intang-obj>		{ CH(1, 20); } ;

<verb5>		= felt					{ CH(1, 20); } ;
<verb5>		= enjoyed				{ CH(1, 20); } ;
<verb5>		= <verb5>				;

# Now for some more things -- intentional statements, (some of these
# may turn out to be ungrammatical, for instance by not using the subjunctive
# mode)...

<s>		= <agent> <int-verb> <s>		{ CH(1, 20); } ;

<int-verb>	= "desired that"			{ CH(1, 20); } ;
<int-verb>	= "believed that"			{ CH(1, 20); } ;
<int-verb>	= "hoped that"				{ CH(1, 20); } ;
<int-verb>	= "thought that"			{ CH(1, 20); } ;
<int-verb>	= "wished that"				{ CH(1, 20); } ;
<int-verb>	= "feared that"				{ CH(1, 20); } ;
<int-verb>	= "wondered whether"			{ CH(1, 20); } ;
<int-verb>	= "was worried that"			{ CH(1, 20); } ;
<int-verb>	= "figured that"			{ CH(1, 20); } ;
<int-verb>	= "predicted that"			{ CH(1, 20); } ;
<int-verb>	= "was ashamed that"			{ CH(1, 20); } ;
<int-verb>	= <int-verb>				;

<s>		= <s>					;

<adv>		= ""					{ CH(4, 5); } ;
<adv>		= <Adv>					;

<in-rel-to>	= <prep>				{ CH(1, 4); } ;
<in-rel-to>	= "similar to"				{ CH(1, 20); } ;
<in-rel-to>	= "in contact with"			{ CH(1, 20); } ;
<in-rel-to>	= "not at all like"			{ CH(1, 20); } ;
<in-rel-to>	= "reminicent of"			{ CH(1, 20); } ;
<in-rel-to>	= "a part of"				{ CH(1, 20); } ;
<in-rel-to>	= "exactly like"			{ CH(1, 20); } ;
<in-rel-to>	= <in-rel-to>				;


# == NOUNS ==
# Here are the noun things -- there are agents, inanimate objects, and
# intangible objects (like concepts). Ignore pronouns for now.
# Intangible things don't have determiners, and don't have "tangible"
# adjectives attatched to them.

<thing>		= <agent>				{ CH(1, 2); } ;
<thing>		= <inan-obj>				;

<agent>		= <agent1> 				;

<agent1>	= <name-p> 				{ CH(1, 3); } ;
<agent1>	= <Det> <agent-adj> <Agent> 		;

<inan-obj>	= <A-Det> <thing-adj> <Inan-obj> 	;

<intang-obj>	= <intang-adj> <Intang-obj>		;

<name-p>	= <Name> 				;

<prep>		= "along with"   			{ CH(1, 10); } ;
<prep>		= "on top of"  				{ CH(1, 10); } ;
<prep>		= "next to"   				{ CH(1, 10); } ;
<prep>		= near   				{ CH(1, 10); } ;
<prep>		= like   				{ CH(1, 10); } ;
<prep>		= "apart from"   			{ CH(1, 10); } ;
<prep>		= "in addition to"			{ CH(1, 10); } ;
<prep>		= <prep>				;

# == ADJECTIVES == 
# Intangible adjectives -- good, bad, etc. Tangible adjectives -- red, cold,
# etc. Agent agectives -- mean, selfish, etc. Agents can have any one, things
# can have tangible and intangible, but intangibles can have only intangibles.

<intang-adj>	= ""					{ CH(4, 5); } ;
<intang-adj>	= <Intang-adj> <intang-adj>		;

<agent-adj>	= ""					{ CH(4, 5); } ;
<agent-adj>	= <adj1> <agent-adj>			;

<thing-adj>	= ""					{ CH(4, 5); } ;
<thing-adj>	= <adj2> <thing-adj> 			;

<adj1>		= <Intang-adj>				{ CH(1, 3); } ;
<adj1>		= <Agent-adj>				{ CH(1, 3); } ;
<adj1>		= <Tang-adj>				;

<adj2>		= <Intang-adj>				{ CH(1, 2); } ;
<adj2>		= <Tang-adj>				;

# Some generic words.

<Conj>		= but 					{ CH(1, 20); } ;
<Conj>		= and 					{ CH(1, 20); } ;
<Conj>		= or 					{ CH(1, 20); } ;
<Conj>		= so 					{ CH(1, 20); } ;
<Conj>		= however 				{ CH(1, 20); } ;
<Conj>		= nevertheless 				{ CH(1, 20); } ;
<Conj>		= therefore 				{ CH(1, 20); } ;
<Conj>		= <Conj> 				;

# P-Det is a posession determiner -- can't be used with an agent.

<A-Det>		= <Det>					{ CH(1, 2); } ;
<A-Det>		= <P-Det>				;

<Det>		= a 					{ CH(1, 10); } ;
<Det>		= the 					{ CH(1, 7); } ;
<Det>		= that 					{ CH(1, 15); } ;
<Det>		= "some kind of" 			{ CH(1, 20); } ;	
<Det>		= this 					{ CH(1, 15); } ;
<Det>		= some	 				{ CH(1, 20); } ;
<Det>		= <Det> 				;

<P-Det>		= my 					{ CH(1, 20); } ;
<P-Det>		= your 					{ CH(1, 20); } ;
# The # mark here ---------v means don't put a space in between...
<P-Det>		= <agent1> "#'s"			{ CH(1, 20); } ;
<P-Det>		= his 					{ CH(1, 20); } ;
<P-Det>		= her 					{ CH(1, 20); } ;
<P-Det>		= their 				{ CH(1, 20); } ;
<P-Det>		= <P-Det> 				;

<Agent>		= person				{ CH(1, 20); } ;
<Agent>		= dog 					{ CH(1, 20); } ;
<Agent>		= cat 					{ CH(1, 20); } ;
<Agent>		= duck 					{ CH(1, 20); } ;
<Agent>		= sheriff				{ CH(1, 20); } ;
<Agent>		= professor				{ CH(1, 20); } ;
<Agent>		= student				{ CH(1, 20); } ;
<Agent>		= "CS grad student"			{ CH(1, 20); } ;
<Agent>		= "disk jockey"				{ CH(1, 20); } ;
<Agent>		= hacker				{ CH(1, 20); } ;
<Agent>		= horse					{ CH(1, 20); } ;
<Agent>		= pig					{ CH(1, 20); } ;
<Agent>		= dude					{ CH(1, 20); } ;
<Agent>		= communist				{ CH(1, 20); } ;
<Agent>		= goose					{ CH(1, 20); } ;
<Agent>		= scumbag				{ CH(1, 20); } ;
<Agent>		= "scuba diver"				{ CH(1, 20); } ;
<Agent>		= "disembodied spirit"			{ CH(1, 20); } ;
<Agent>		= guy					{ CH(1, 20); } ;
<Agent>		= Australian				{ CH(1, 20); } ;
<Agent>		= aardvark				{ CH(1, 20); } ;
<Agent>		= "space alien"				{ CH(1, 20); } ;
<Agent>		= <Agent>				;

<Inan-obj>	= house 				{ CH(1, 20); } ;
<Inan-obj>	= fruit 				{ CH(1, 20); } ;
<Inan-obj>	= slime-mold 				{ CH(1, 20); } ;
<Inan-obj>	= "lisp manual" 			{ CH(1, 20); } ;
<Inan-obj>	= rock	 				{ CH(1, 20); } ;
<Inan-obj>	= terminal 				{ CH(1, 20); } ;
<Inan-obj>	= textbook 				{ CH(1, 20); } ;
<Inan-obj>	= pen	 				{ CH(1, 20); } ;
<Inan-obj>	= box 					{ CH(1, 20); } ;
<Inan-obj>	= steak 				{ CH(1, 20); } ;
<Inan-obj>	= vax	 				{ CH(1, 20); } ;
<Inan-obj>	= "salad fork" 				{ CH(1, 20); } ;
<Inan-obj>	= thingamajig 				{ CH(1, 20); } ;
<Inan-obj>	= doorknob 				{ CH(1, 20); } ;
<Inan-obj>	= axe	 				{ CH(1, 20); } ;
<Inan-obj>	= "357 magnum" 				{ CH(1, 20); } ;
<Inan-obj>	= adm3a 				{ CH(1, 20); } ;
<Inan-obj>	= saxaphone 				{ CH(1, 20); } ;
<Inan-obj>	= pineapple 				{ CH(1, 20); } ;
<Inan-obj>	= keyboard 				{ CH(1, 20); } ;
<Inan-obj>	= "baked potato"			{ CH(1, 20); } ;
<Inan-obj>	= "carpet tack"				{ CH(1, 20); } ;
<Inan-obj>	= "bottle of bee"			{ CH(1, 20); } ;
<Inan-obj>	= coathanger 				{ CH(1, 20); } ;
<Inan-obj>	= "bayobab tree"			{ CH(1, 20); } ;
<Inan-obj>	= <Inan-obj>				;

# These are all mental conditions.

<Intang-obj>	= fear					{ CH(1, 20); } ;
<Intang-obj>	= loathing				{ CH(1, 20); } ;
<Intang-obj>	= sympathy				{ CH(1, 20); } ;
<Intang-obj>	= hope					{ CH(1, 20); } ;
<Intang-obj>	= disgust				{ CH(1, 20); } ;
<Intang-obj>	= envy					{ CH(1, 20); } ;
<Intang-obj>	= distaste				{ CH(1, 20); } ;
<Intang-obj>	= excitement				{ CH(1, 20); } ;
<Intang-obj>	= <Intang-obj>				;

<Name>		= "Generic Joe" 			{ CH(1, 20); } ;
<Name>		= John 					{ CH(1, 20); } ;
<Name>		= Mary 					{ CH(1, 20); } ;
<Name>		= "Ronald Reagan" 			{ CH(1, 20); } ;
<Name>		= "Fritz Mondale" 			{ CH(1, 20); } ;
<Name>		= "Marvin Minsky" 			{ CH(1, 20); } ;
<Name>		= "Howard the Duck" 			{ CH(1, 20); } ;
<Name>		= "Doug Cooper" 			{ CH(1, 20); } ;
<Name>		= Chris 				{ CH(1, 20); } ;
<Name>		= Batman 				{ CH(1, 20); } ;
<Name>		= "Wyatt Earp" 				{ CH(1, 20); } ;
<Name>		= "Richard Nixon" 			{ CH(1, 20); } ;
<Name>		= "The Spanish Inquisition"		{ CH(1, 20); } ;
<Name>		= <Name> 				;

<Intang-adj>	= lousy 				{ CH(1, 20); } ;
<Intang-adj>	= undesirable 				{ CH(1, 20); } ;
<Intang-adj>	= bad	 				{ CH(1, 20); } ;
<Intang-adj>	= strange 				{ CH(1, 20); } ;
<Intang-adj>	= unexpected 				{ CH(1, 20); } ;
<Intang-adj>	= fascinating 				{ CH(1, 20); } ;
<Intang-adj>	= useful 				{ CH(1, 20); } ;
<Intang-adj>	= appropriate 				{ CH(1, 20); } ;
<Intang-adj>	= nice 					{ CH(1, 20); } ;
<Intang-adj>	= incredible 				{ CH(1, 20); } ;
<Intang-adj>	= bizarre 				{ CH(1, 20); } ;
<Intang-adj>	= awful	 				{ CH(1, 20); } ;
<Intang-adj>	= unnatural 				{ CH(1, 20); } ;
<Intang-adj>	= "out of place"			{ CH(1, 20); } ;
<Intang-adj>	= terrible 				{ CH(1, 20); } ;
<Intang-adj>	= <Intang-adj>				;

<Tang-adj>	= red 					{ CH(1, 20); } ;
<Tang-adj>	= green					{ CH(1, 20); } ;
<Tang-adj>	= big 					{ CH(1, 20); } ;
<Tang-adj>	= hot 					{ CH(1, 20); } ;
<Tang-adj>	= heavy					{ CH(1, 20); } ;
<Tang-adj>	= fast 					{ CH(1, 20); } ;
<Tang-adj>	= slow 					{ CH(1, 20); } ;
<Tang-adj>	= huge 					{ CH(1, 20); } ;
<Tang-adj>	= lumpy					{ CH(1, 20); } ;
<Tang-adj>	= <Agent> "#-like" 			{ CH(1, 20); } ;
<Tang-adj>	= <Inan-obj> "#-like" 			{ CH(1, 20); } ;
<Tang-adj>	= shabby 				{ CH(1, 20); } ;
<Tang-adj>	= <Tang-adj>				;

<Agent-adj>	= benevolent				{ CH(1, 20); } ;
<Agent-adj>	= nasty					{ CH(1, 20); } ;
<Agent-adj>	= misanthropic				{ CH(1, 20); } ;
<Agent-adj>	= conceited				{ CH(1, 20); } ;
<Agent-adj>	= arrogant				{ CH(1, 20); } ;
<Agent-adj>	= misguided				{ CH(1, 20); } ;
<Agent-adj>	= nervous				{ CH(1, 20); } ;
<Agent-adj>	= clever				{ CH(1, 20); } ;
<Agent-adj>	= stupid				{ CH(1, 20); } ;
<Agent-adj>	= <Agent-adj>				;

<Adv>		= quickly 				{ CH(1, 20); } ;
<Adv>		= badly 				{ CH(1, 20); } ;
<Adv>		= slowly 				{ CH(1, 20); } ;
<Adv>		= stupidly 				{ CH(1, 20); } ;
<Adv>		= selfishly 				{ CH(1, 20); } ;
<Adv>		= properly				{ CH(1, 20); } ;
<Adv>		= <Adv> 				;

<spunct>	= "."					;

# Now, just for the heck of it, some random things...

<rnd>		= "what a" <Inan-obj> "!"		{ CH(1, 20); } ;
<rnd>		= "oh no, it's" <thing>	"!"		{ CH(1, 20); } ;
<rnd>		= "NOBODY expects" <thing> "!"		{ CH(1, 20); } ;
<rnd>		= "look at" <thing> "!"			{ CH(1, 20); } ;
<rnd>		= gosh!					{ CH(1, 20); } ;
<rnd>		= yeah, <s> .				{ CH(1, 10); } ;
<rnd>		= "wouldn't ya know, " <s> "!"		{ CH(1, 20); } ;
<rnd>		= "well, shiver me timbers!"		{ CH(1, 20); } ;
<rnd>		= "lemme tell ya -" <s> "!"		{ CH(1, 20); } ;
<rnd>		= "why, you" <Inan-obj> "!"		{ CH(1, 20); } ;
<rnd>		= "look at me when I'm talking to you!"	{ CH(1, 20); } ;
<rnd>		= <thing> ... uh...			{ CH(1, 10); } ;
<rnd>		= uh...					{ CH(1, 20); } ;
<rnd>		= "hic! (excuse me...)"			{ CH(1, 20); } ;
<rnd>		= "you know," <s> .			{ CH(1, 20); } ;
<rnd>		= <agent> "said that" <s> . 		{ CH(1, 5); } ;
<rnd>		= <rnd>				;

%%

