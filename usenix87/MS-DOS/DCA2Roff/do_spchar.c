
/* special characters, not normally in troff */
/* should be read in from a file for each device/font */
/* we will output the 3rd field in the struct as a troff comment line */
/* if the 2nd field is null */

struct spc {
	int spcode;			/* dca code for character */
	char *spstr;			/* titroff code for char on imagen */
	char *explan;			/* what is the char ? */
} spchar[] = {
		0x59, "\\(ss",		"german sharp s",
		0x70, "\\(O/",		"O slash",
		0x8a, "",		"European open quote",
		0x8b, "",		"European close quote",
		0x8c, "",		"d stroke",
		0x8e, "",		"small letter thorn",
		0x9f, "",		"international currency sym",
		0xa0, "",		"Micro",
		0xaa, "\\(!!",		"inverted !",
		0xab, "\\(??",		"inverted ?",
		0xac, "",		"D stroke",
		0xae, "",		"Capital Thorn",
		0xb1, "",		"Pound sign",
		0xb2, "",		"Yen",
		0xb3, "",		"Peseta",
		0xb4, "",		"Floren, Guilder",
		0xb6, "\\(pa",		"paragraph sign",
		0xbc, "\\(mc",		"overbar",
		0xbd, "\\(..",		"diaeresis",
		0xbf, "",		"double underscore",
		0xda, "\\(ui",		"dotless i",
		0xea, "\\u\\s-22\\s+2\\d",	"superscript 2",
		0xfa, "\\u\\s-23\\s+2\\d",	"superscript 3",
		0xff, "",		"Eight Ones",
		0x00, "",		""
};

#include "dca2troff.h"

do_spchar(ch)
char ch;
{
	int i;
	for(i=0; spchar[i].spcode != ch; i++)
		{
		if (spchar[i].spcode == 0)
			{
			fprintf(stderr, "error: unknown special char 0X%02x\n", ch);
			return(0);
			}
		}
	if(strcmp(spchar[i].spstr, "") == 0)
		{
		sprintf(tline, "\\\" %s\n", spchar[i].explan);
		outstr(tline);
		}
	else
		outstr(spchar[i].spstr);
	return(0);
}
