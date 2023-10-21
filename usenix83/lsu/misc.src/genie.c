#
#include <stdio.h>
/* Genie.c  when all else fails, consult the oracle
	S. Klyce - LSU/NO		*/
main()
{
	char quest[200];
	int tbuf[2]; char *c;
	time(tbuf);
	c = ctime(tbuf);
	printf("Yes? ");
	scanf("%s",&quest);
	switch(*(c+18)) {
	case '0':
		printf("Does a duck have lips?\n");
		break;
	case '1':
		printf("Does a bear go in the woods?\n");
		break;
	case '2':
		printf("Is the sky blue?\n");
		break;
	case '3':
		printf("Does it rain in New Orleans?\n");
		break;
	case '4':
		printf("Is a duck's bottom waterproof?\n");
		break;
	case '5':
		printf("Most assuredly.\n");
		break;
	case '6':
		printf("No way!\n");
		break;
	case '7':
		printf("Sure.\n");
		break;
	case '8':
		printf("Let me think...");
		sleep(1);
		printf("cannot answer at this time\n");
		break;
	case '9':
		printf("That is one of the mysteries of life.\n");
		break;
	default:
		system("echo 'system clock error' > /dev/tty8");
		break;
	}
}
