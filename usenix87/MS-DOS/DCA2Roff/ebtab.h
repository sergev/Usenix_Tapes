/* table of EBCDIC characters				*
 * 1st entry is character use type			*
 *	0	forget it				*
 *	1	simple translate to string		*
 *	2	single byte control			*
 *	3	"csp" for multibyte control		*
 *	4	accented character			*
 *	5	troff special character			*
 *	6	non-troff special character		*
 *							*
 * 2nd entry is a string, use depends on type		*
 *      1	string to output			*
 *	2	unused					*
 *	3	unused					*
 *	4	accent <SPACE> character		*
 *	5	string to output			*
 *	6	if string then output			*
 *		if no string ( NULL ) then special	*
 *		character local processing required	*
 *							*/

struct echar {
	int type;
	char *arg;
} ebaray[] = {
		0, "",		/* 0x00 - NULL */
		0, "",		/* 0x01 - xx*/
		0, "",		/* 0x02 - xx*/
		0, "",		/* 0x03 - xx*/
		0, "",		/* 0x04 - xx*/
		2, "\t",	/* 0x05 - horizontal tab */
		2, "\n.br\n",	/* 0x06 - required carrier return */
		0, "",		/* 0x07 - xx*/
		0, "",		/* 0x08 - xx*/
		2, "\u",	/* 0x09 - superscript */
		0, "",		/* 0x0a - xx*/
		0, "",		/* 0x0b - xx*/
		2, "\n",	/* 0x0c - page end */
		2, "\n",	/* 0x0d - zero index carrier return */
		0, "",		/* 0x0e - xx*/
		0, "",		/* 0x0f - xx*/
		0, "",		/* 0x10 - xx*/
		0, "",		/* 0x11 - xx*/
		0, "",		/* 0x12 - xx*/
		0, "",		/* 0x13 - xx*/
		0, "",		/* 0x14 - xx*/
		2, "\n",	/* 0x15 - carrier return*/
		2, "\b",	/* 0x16 - backspace */
		0, "",		/* 0x17 - xx*/
		0, "",		/* 0x18 - xx*/
		0, "",		/* 0x19 - xx*/
		2, "",		/* 0x1a - unit backspace */
		0, "",		/* 0x1b - xx*/
		0, "",		/* 0x1c - xx*/
		0, "",		/* 0x1d - xx*/
		0, "",		/* 0x1e - xx*/
		0, "",		/* 0x1f - xx*/
		0, "",		/* 0x20 - xx*/
		0, "",		/* 0x21 - xx*/
		0, "",		/* 0x22 - xx*/
		2, "",		/* 0x23 - word underscore */
		0, "",		/* 0x24 - xx*/
		2, "",		/* 0x25 - index */
		0, "",		/* 0x26 - xx*/
		0, "",		/* 0x27 - xx*/
		0, "",		/* 0x28 - xx*/
		0, "",		/* 0x29 - xx*/
		0, "",		/* 0x2a - xx*/
		3, "",		/* 0x2b - CSP */
		0, "",		/* 0x2c - xx*/
		0, "",		/* 0x2d - xx*/
		0, "",		/* 0x2e - xx*/
		0, "",		/* 0x2f - xx*/
		0, "",		/* 0x30 - xx*/
		0, "",		/* 0x31 - xx*/
		0, "",		/* 0x32 - xx*/
		2, "",		/* 0x33 - index return */
		0, "",		/* 0x34 - xx*/
		0, "",		/* 0x35 - xx*/
		2, "\\h'-\\w'1'u'",	/* 0x36 - numeric backspace */
		0, "",		/* 0x37 - xx*/
		2, "\d",	/* 0x38 - subscript */
		2, "",		/* 0x39 - indent tab */
		2, "\n.bp\n",	/* 0x3a - required page end */
		0, "",		/* 0x3b - xx*/
		0, "",		/* 0x3c - xx*/
		0, "",		/* 0x3d - xx*/
		0, "",		/* 0x3e - xx*/
		2, "",		/* 0x3f - substitute */
		2, " ",		/* 0x40 - space */
		2, " ",		/* 0x41 - required space */
		4, "^ a",	/* 0x42 - a circumflex */
		4, ".. a",	/* 0x43 - a diaeresis */
		4, "' a",	/* 0x44 - a grave */
		4, "` a",	/* 0x45 - a acute */
		4, "~ a",	/* 0x46 - a tilde */
		4, "de a",	/* 0x47 - a angstrom */
		4, "cd c",	/* 0x48 - c cedilla */
		4, "~ n",	/* 0x49 - n telde */
		1, "[",		/* 0x4a - [ */
		1, ".",		/* 0x4b - . */
		1, "<",		/* 0x4c - < */
		1, "(",		/* 0x4d - ( */
		1, "+",		/* 0x4e - + */
		1, "!",		/* 0x4f - ! */
		1, "&",		/* 0x50 - & */
		4, "` e",	/* 0x51 - e acute */
		4, "^ e",	/* 0x52 - e circumflex */
		4, ".. e",	/* 0x53 - e diaeresis */
		4, "' e",	/* 0x54 - e grave */
		4, "` i",	/* 0x55 - i acute */
		4, "^ i",	/* 0x56 - i circumflex */
		4, ".. i",	/* 0x57 - i diaeresis */
		4, "' i",	/* 0x58 - i grave */
		6, "\\(ss",	/* 0x59 - german sharp s */
		1, "]",		/* 0x5a - ] */
		1, "$",		/* 0x5b - $ */
		1, "*",		/* 0x5c - * */
		1, ")",		/* 0x5d - ) */
		1, ";",		/* 0x5e - ; */
		1, "^",		/* 0x5f - ^ */
		2, "-",		/* 0x60 - required - */
		1, "/",		/* 0x61 - / */
		4, "^ A",	/* 0x62 - A circumflex */
		4, ".. A",	/* 0x63 - A diaeresis */
		4, "' A",	/* 0x64 - A grave */
		4, "` A",	/* 0x65 - A acute */
		4, "~ A",	/* 0x66 - A tilde */
		4, "de A",	/* 0x67 - A angstrom */
		4, "cd C",	/* 0x68 - C cedilla */
		4, "~ N",	/* 0x69 - N tilde */
		1, "|",		/* 0x6a - | */
		1, ",",		/* 0x6b - , */
		1, "%",		/* 0x6c - % */
		1, "_",		/* 0x6d - _ */
		1, ">",		/* 0x6e - > */
		1, "?",		/* 0x6f - ? */
		6, "\\(O/",	/* 0x70 - O slash */
		4, "' E",	/* 0x71 - E acute */
		4, "^ E",	/* 0x72 - E circumflex */
		4, ".. E",	/* 0x73 - E diaresis */
		4, "` E",	/* 0x74 - E grave */
		4, "' I",	/* 0x75 - I acute */
		4, "^ I",	/* 0x76 - I circumflex */
		4, ".. I",	/* 0x77 - I diaresis */
		4, "` I",	/* 0x78 - I grave */
		5, "\(ag",	/* 0x79 - grave */
		1, ":",		/* 0x7a - : */
		1, "#",		/* 0x7b - # */
		1, "@",		/* 0x7c - @ */
		1, "\'",	/* 0x7d - ' */
		1, "=",		/* 0x7e - = */
		1, "\"", 	/* 0x7f - " */
		1, "/",		/* 0x80 - / */
		1, "a",		/* 0x81 - a */
		1, "b",		/* 0x82 - b */
		1, "c",		/* 0x83 - c */
		1, "d",		/* 0x84 - d */
		1, "e",		/* 0x85 - e */
		1, "f",		/* 0x86 - f */
		1, "g",		/* 0x87 - g */
		1, "h",		/* 0x88 - h */
		1, "i",		/* 0x89 - i */
		6, "",		/* 0x8a - European open quote */
		6, "",		/* 0x8b - European close quote */
		6, "",		/* 0x8c - d stroke */
		4, "` y",	/* 0x8d - y acute */
		6, "",		/* 0x8e - small letter thorn */
		5, "\\(+-",	/* 0x8f - +- */
		5, "\\(de",	/* 0x90 - degree */
		1, "j",		/* 0x91 - j */
		1, "k",		/* 0x92 - k */
		1, "l",		/* 0x93 - l */
		1, "m",		/* 0x94 - m */
		1, "n",		/* 0x95 - n */
		1, "o",		/* 0x96 - o */
		1, "p",		/* 0x97 - p */
		1, "q",		/* 0x98 - q */
		1, "r",		/* 0x99 - r */
		4, "_ a",	/* 0x9a - a underscore */
		4, "_ o",	/* 0x9b - o underscore */
		5, "\\(ae",	/* 0x9c - ae ligature */
		4, "\\(cd",	/* 0x9d - Cedilla */
		5, "\\(AE",	/* 0x9e - AE ligature */
		6, "",		/* 0x9f - international currency symbol */
		6, "",		/* 0xa0 - Micro */
		1, "~",		/* 0xa1 - ~ */
		1, "s",		/* 0xa2 - s */
		1, "t",		/* 0xa3 - t */
		1, "u",		/* 0xa4 - u */
		1, "v",		/* 0xa5 - v */
		1, "w",		/* 0xa6 - w */
		1, "x",		/* 0xa7 - x */
		1, "y",		/* 0xa8 - y */
		1, "z",		/* 0xa9 - z */
		6, "\\(!!",	/* 0xaa - inverted ! */
		6, "\\(??",	/* 0xab - inverted ? */
		6, "",		/* 0xac - D stroke */
		4, "` Y",	/* 0xad - Y acute */
		6, "",		/* 0xae - Capital Thorn */
		5, "\\(co",	/* 0xaf - copyright */
		5, "\\(ct",	/* 0xb0 - cent sign */
		6, "",		/* 0xb1 - Pound sign */
		6, "",		/* 0xb2 - Yen */
		6, "",		/* 0xb3 - Peseta */
		6, "",		/* 0xb4 - Floren, Guilder */
		5, "\\(sc",	/* 0xb5 - section sign */
		6, "\\(pa",	/* 0xb6 - paragraph sign */
		5, "\\(14",	/* 0xb7 - one fourth */
		5, "\\(12",	/* 0xb8 - one half */
		5, "\\(34",	/* 0xb9 - three fourths*/
		5, "\\(no",	/* 0xba - logical not */
		5, "\\(or",	/* 0xbb - logical or */
		6, "\\(mc",	/* 0xbc - overbar */
		6, "\\(..",	/* 0xbd - diaeresis */
		5, "\\(aa",	/* 0xbe - acute */
		6, "",		/* 0xbf - double underscore */
		1, "{",		/* 0xc0 - { */
		1, "A",		/* 0xc1 - A */
		1, "B",		/* 0xc2 - B */
		1, "C",		/* 0xc3 - C */
		1, "D",		/* 0xc4 - D */
		1, "E",		/* 0xc5 - E */
		1, "F",		/* 0xc6 - F */
		1, "G",		/* 0xc7 - G */
		1, "H",		/* 0xc8 - H */
		1, "I",		/* 0xc9 - I */
		2, "\\%",	/* 0xca - syllable hyphen */
		4, "^ o",	/* 0xcb - o circumflex */
		4, ".. o",	/* 0xcc - o diaresis */
		4, "' o",	/* 0xcd - o grave */
		4, "` o",	/* 0xce - o acute */
		4, "~ o",	/* 0xcf - o tilde */
		1, "}",		/* 0xd0 - } */
		1, "J",		/* 0xd1 - J */
		1, "K",		/* 0xd2 - K */
		1, "L",		/* 0xd3 - L */
		1, "M",		/* 0xd4 - M */
		1, "N",		/* 0xd5 - N */
		1, "O",		/* 0xd6 - O */
		1, "P",		/* 0xd7 - P */
		1, "Q",		/* 0xd8 - Q */
		1, "R",		/* 0xd9 - R */
		6, "\\(ui",	/* 0xda - dotless i */
		4, "^ u",	/* 0xdb - u circumflex */
		4, ".. u",	/* 0xdc - u diaresis */
		4, "' u",	/* 0xdd - u grave */
		4, "` u",	/* 0xde - u acute */
		4, ".. y",	/* 0xdf - y diaresis */
		1, "\\",	/* 0xe0 - \ */
		2, "\\h'\\w'1'u'",	/* 0xe1 - numeric space */
		1, "S",		/* 0xe2 - S */
		1, "T",		/* 0xe3 - T */
		1, "U",		/* 0xe4 - U */
		1, "V",		/* 0xe5 - V */
		1, "W",		/* 0xe6 - W */
		1, "X",		/* 0xe7 - X */
		1, "Y",		/* 0xe8 - Y */
		1, "Z",		/* 0xe9 - Z */
		6, "\\u\\s-22\\s+2\\d",	/* 0xea - superscript 2 */
		4, "^ O",	/* 0xeb - O circumflex */
		4, ".. O",	/* 0xec - O diaresis */
		4, "' O",	/* 0xed - O grave */
		4, "` O",	/* 0xee - O acute */
		4, "~ O",	/* 0xef - O tilde */
		1, "0",		/* 0xf0 - 0 */
		1, "1",		/* 0xf1 - 1 */
		1, "2",		/* 0xf2 - 2 */
		1, "3",		/* 0xf3 - 3 */
		1, "4",		/* 0xf4 - 4 */
		1, "5",		/* 0xf5 - 5 */
		1, "6",		/* 0xf6 - 6 */
		1, "7",		/* 0xf7 - 7 */
		1, "8",		/* 0xf8 - 8 */
		1, "9",		/* 0xf9 - 9 */
		6, "\\u\\s-23\\s+2\\d",	/* 0xfa - superscript 3 */
		4, "^ U",	/* 0xfb - U circumflex */
		4, ".. U",	/* 0xfc - U diaresis */
		4, "' U",	/* 0xfd - U grave */
		4, "` U",	/* 0xfe - U acute */
		6, "",		/* 0xff - "Eight Ones" */
};
