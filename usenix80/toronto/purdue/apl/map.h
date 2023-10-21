
/*
 *      Mapping arrays
 *
 *
 * The following arrays perform character mapping between the APL
 * character set (LSI modified by Peter Hallenbeck) and the ASCII
 * character set.
 *
 */


	/* Map to the APL character set from ASCII */

char map_apl[]{

	' ',		/* space maps to itself */
	'!',            /* does not map */
	' ',            /* does not map */
	'(',            /* mapped pound sign (not equals) */
	'$',		/* dollar sign maps to itself (.le.) */
	'=',            /* mapped percent (division) */
	'&',            /* & becomes .ge. automatically */
	'K',            /* mapped single quote mark */
	'+',            /* mapped opening parenthesis */
	'`',            /* mapped closing parenthesis */
	'P',            /* mapped asterisk */
	':',            /* mapped plus */
	',',            /* , maps to itself */
	'*',            /* mapped minus */
	'.',            /* . maps to itself */
	'/',            /* / maps to itself */
	'0','1','2','3','4','5','6','7','8','9',        /* all digits ok */
	'>',            /* mapped colon */
	'<',            /* mapped semi-colon */
	'#',            /* mapped less-than */
	'%',            /* mapped equals */
	047,            /* mapped greater-than */
	'Q',            /* mapped question-mark */
	' ',            /* @ does not map */
	'{',            /* capital A = and */
	'B',            /* captial B equals decode */
	'C',            /* captial C equals inverted U */
	'D',            /* captial d equals floot */
	'E',            /* capital e equals epsilon */
	' ',            /* captial f does not map */
	'G',            /* captial g equals del */
	'H',            /* captial h equals interted del */
	'I',            /* captial i equals iota */
	'J',            /* captial j equals circle */
	' ',            /* captial k does not map */
	'L',            /* captial l equals quad */
	'-',            /* mapped multiply */
	'N',            /* capital n equals encode */
	'O',            /* captial o equals big circle */
	'=',            /* mapped divide */
	')',            /* mapped "or" */
	'R',            /* captial r equals rho */
	'S',            /* captial s equals ceiling */
	' ',            /* captial t does not map */
	'U',            /* capital u equals drop */
	')',            /* mapped "or" */
	'W',            /* captial w equals omega */
	'-',            /* captial x equals times sign */
	'Y',            /* captial y equals take */
	'Z',            /* captial z equals inverted subset symbol */
	';',            /* mapped open brace */
	'?',            /* mapped backslash */
	'@',            /* mapped closing brace */
	'{',		/* mapped caret */
	'\\',           /* mapped underscore */
	'"',            /* raised minus */
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v','w','x','y','z',
	'\\',            /* mapped open brace */
	'M',            /* mapped mod (abs) */
	'|',            /* mapped closing brace */
	'T',            /* mapped "not" */
	0177            /* delete maps to itself */
};


	/* Map to ASCII from the APL character set */

char map_ascii[]{

	' ',
	'!',
	'`',
	'<',
	'$',
	'=',
	'&',
	'>',
	'#',
	'V',
	'-',
	'(',
	',',
	'X',
	'.',
	'/',
	'0','1','2','3','4','5','6','7','8','9',
	'+',
	'[',
	';',
	'%',
	':',
	'\\',
	']',
	' ',
	'B',
	'C',
	'D',
	'E',
	'_',
	'G',
	'H',
	'I',
	'J',
	047,
	'L',
	'|',
	'N',
	'O',
	'*',
	'?',
	'R',
	'S',
	'~',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'$',
	'{',
	']',
	010,
	'_',
	')',
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v','w','x','y','z',
	'^',
	'}',
	'}',
	' ',
	0177
};
