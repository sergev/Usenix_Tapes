/*
 * TREK73: mission.c
 *
 * Mission Assignment
 *
 */

#include "structs.h"
#include "defines.h"

extern int terse;
extern char title[];
extern char foerace[];
extern char foename[];
extern char foestype[];
extern char captain[];
extern char science[];
extern char com[];
extern char helmsman[];
extern struct ship *shiplist[];
extern int shipnum;

mission()
{
	int onef;
	extern char *plural(), *vowelstr();

	if (terse)
		return;
	onef = (shipnum == 1);
	printf("\n\n\nSpace, the final frontier.\n");
	printf("These are the voyages of the starship %s.\n", shiplist[0]->name);
	printf("Its five year mission: to explore strange new worlds,\n");
	printf("to seek our new life and new civilizations,\n");
	printf("to boldly go where no man has gone before!\n");
	printf("\n");
	printf("                    S T A R    T R E K\n");
	printf("\n");
	missionlog();
	printf("%s: %s, I'm picking up %d vessel%s on interception\n", helmsman, title, shipnum, plural(shipnum));
	printf("   course with the %s.\n", shiplist[0]->name);
	printf("%s: Sensors identify %s as ", science, onef ? "it" : "them");
	if (onef)
		printf("a%s ", vowelstr(foerace));
	printf("%s %s%s,\n", foerace, foestype, plural);
	printf("   probably under the command of Captain %s.\n", foename);
	printf("%s: Sound general quarters, Lieutenant!\n", captain);
	printf("%s: Aye, %s!\n", com,  title);
}

warning()
{
	register int i;

	printf("Computer: The %ss are attacking the %s with the ", foerace, shiplist[0]->name);
	if (shipnum == 1) {
		printf("%s", shiplist[1]->name);
	} else {
		for (i = 1; i <= shipnum; i++) {
			if (i == shipnum)
				printf("and the ");
			printf("%s", shiplist[i]->name);
			if (i == shipnum)
				break;
			printf(", ");
			/*
			if ((shipnum == 2 && i == 1) || i == 2 || i == 7)
			*/
			if (i == 1 || i == 7)
				printf("\n   ");
		}
	}
	printf(".\n");
}

missionlog()
{
	static char *missiontab[] = {
	"   We are acting in response to a Priority 1 distress call from",
	"space station K7.",
	"   We are orbiting Gamma 2 to make a routine check of automatic",
	"communications and astrogation stations.",
	"   We are on course for Epsilon Canares 3 to treat Commissioner",
	"Headford for Sukaro's disease.",
	"   We have been assigned to transport ambassadors to a diplomatic",
	"conference on the planet code named Babel.",
	"   Our mission is to investigate a find of tritanium on Beta 7.",
	0,
	"   We are orbiting Rigel 4 for therapeutic shore leave.",
	0,
	"   We are orbiting Sigma Iota 2 to study the effects of",
	"contamination upon a devoloping culture.",
	"   We have altered course for a resue mission on the Gamma 7A",
	"system.",
	"   We are presently on course for Altair 6 to attend inauguration",
	"cermonies on the planet.",
	"   We are on a cartographic mission to Polex 9.",
	0,
	"   We are headed for Malurian in response to a distress call",
	"from that system.",
	"   We are to negotiate a treaty to mine dilithium crystals from",
	"the Halkans.",
	"   We are to investigate strange sensor readings reported by a",
	"scoutship investigating Gamma Triangula 6.",
	"   We are headed for planets L370 and L374 to investigate the",
	"disappearance of the starship Constellation in that vincinity.",
	"   We are ordered, with a skeleton crew, to proceed to Space",
	"Station K2 to test Dr. Richard Daystrom's computer M5.",
	"   We have encountered debris from the SS Beagle and are",
	"proceeding to investigate.",
	"   We are on course for Ekos to locate John Gill.",
	0,
	"   We are to divert an asteroid from destroying an inhabited",
	"planet.",
	"   We are responding to a distresss call form the scientific",
	"expedition on Triacus.",
	"   We have been assigned to transport the Medusan Ambassador to",
	"to his home planet."
	};
	int t1, t2;

	t2 = randm(100) - 1;
	t1 = randm(100) - 1;
	printf("%s:  Captain's log, stardate %d.%d.\n", captain, t1, t2);
	t1 = (randm(20) - 1) * 2;
	printf("%s\n", missiontab[t1]);
	t1++;
	if (!missiontab[t1])
		return;
	printf("   %s\n", missiontab[t1]);
}
