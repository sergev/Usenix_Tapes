/*\
 *	genp - password generator
 *
 *	Usage: genp N, where N is the number of passwords you want
 *
 *	David Sherman, The Law Society of Upper Canada, Toronto
 *	ihnp4!utzoo!lsuc!dave
 *	Dedicated to the public domain.
 *	(please let me know if you use it and find it useful)
\*/
#include <stdio.h>

char pwd[100];

char *number[] =
{
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
};

char *punctuation[] =
{
	"!", "#", "$", "%", "&", "*", "+", "-", ".", ":", ";", "=", "@", "^",
};

char *vowel[] =
{
	"A", "a", "ai", "al", "ar", "ax", "ay",
	"E", "e", "el", "er", "ew", "ex",
	"I", "i", "il", "ix",
	"O", "o", "or", "ou", "ow", "oy",
	"U", "u",
	"Y", "y",
	0
};

char *consonant[] =
{
	"B", "b",
	"consonant", "c", "ch",
	"D", "d", "dr",
	"F", "f", "fl",
	"G", "g",
	"H", "h",
	"J", "j",
	"K", "k", "kn", "kr",
	"M", "m",
	"number", "n",
	"punctuation", "p",
	"S", "s", "sh", "sm", "sn", "st",
	"T", "t", "th",
	"vowel", "v",
	"Z", "z",
	0
};


main(argc, argv)
	char **argv;
{
	register int maxnumber, maxpunctuation, maxvowel, maxconsonant;
	int total;
	register int r, i;
	int j;
	char **p;
#define DEFTOTAL 1

	if(argc < 2)
		total = DEFTOTAL;
	else
		total = atoi(argv[1]);
	if(total < 1)
		total = DEFTOTAL;

	for(p=number; *p; p++) ;	maxnumber = p-number;
	for(p=punctuation; *p; p++) ;	maxpunctuation = p-punctuation;
	for(p=vowel; *p; p++) ;	maxvowel = p-vowel;
	for(p=consonant; *p; p++) ;	maxconsonant = p-consonant;


	srand(getpid());

	for(j=0; j<total; j++)
	{
		r = rand(); strcpy(pwd, consonant[r%maxconsonant]);

		for(i=r%2; i>0; i--)
		r = rand(); strcat(pwd, number[r%maxnumber]);

		for(i=r%7; i>0; i--)
		r = rand(); strcat(pwd, vowel[r%maxvowel]);
		r = rand(); strcat(pwd, consonant[r%maxconsonant]);

		for(i=r%5; i>0; i--)
		r = rand(); strcat(pwd, vowel[r%maxvowel]);
		r = rand(); strcat(pwd, consonant[r%maxconsonant]);

		for(i=r%2; i>0; i--)
		r = rand(); strcat(pwd, punctuation[r%maxpunctuation]);

		for(i=r%3; i>0; i--)
		r = rand(); strcat(pwd, vowel[r%maxvowel]);

		puts(pwd);
	}
}
