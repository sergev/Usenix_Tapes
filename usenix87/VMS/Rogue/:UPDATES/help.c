help(what)
int what;{

	switch(what){

	case '\003'  : message(" quit",1);
			break;
	case '.'     : message(" rest a turn",1);
			break;
	case 'i'     : message(" inventory rogue pack",1);
			break;
	case 'f'     : message(" fight",1);
			break;
	case 'F'     : message(" fight harder",1);
			break;
	case 'h'     : message(" move left",1);
			break;
	case 'j'     : message(" move down",1);
			break;
	case 'k'     : message(" move up",1);
			break;
	case 'l'     : message(" move right",1);
			break;
	case 'y'     : message(" move up and to the left",1);
			break;
	case 'u'     : message(" move up and to the right",1);
			break;
	case 'n'     : message(" move down and to the left",1);
			break;
	case 'b'     : message(" move down and to the right",1);
			break;
	case 'H'     : message(" run left",1);
			break;
	case 'J'     : message(" run down",1);
			break;
	case 'K'     : message(" run up",1);
			break;
	case 'L'     : message(" run right",1);
			break;
	case 'Y'     : message(" run up and to the left",1);
			break;
	case 'U'     : message(" run up and to the right",1);
			break;
	case 'N'     : message(" run down and to the left",1);
			break;
	case 'B'     : message(" run down and to the right",1);
			break;
	case '\010'  : message(" move left, until adjacent",1);
			break;
	case '\012'  : message(" move down, until adjacent",1);
			break;
	case '\013'  : message(" move up, until adjacent",1);
			break;
	case '\014'  : message(" move right, until adjacent",1);
			break;
	case '\031'  : message(" move up and to the left,until adjacent",1);
			break;
	case '\025'  : message(" move up and to the right,until adjacent",1);
			break;
	case '\006'  : message(" move down and to the left, until adjacent",1);
			break;
	case '\002'  : message(" move down and to the right,until adjacent",1);
			break;
	case 'e'     : message(" eat",1);
			break;
	case 'q'     : message(" quaff potion",1);
			break;
	case 'r'     : message(" read scroll",1);
			break;
	case 'm'     : message(" move onto",1);
			break;
	case 'd'     : message(" drop",1);
			break;
	case '\020'  : message(" recall last message",1);
			break;
	case '>'     : message(" go down stairs",1);
			break;
	case '<'     : message(" go up stairs",1);
			break;
	case 'I'     : message(" inventory a single item",1);
			break;
	case 'R'     : message(" redaw screen",1);
			break;
	case 'T'     : message(" take off armor",1);
			break;
	case 'W'     : message(" put on armor",1);
			break;
	case 'P'     : message(" put on armor",1);
			break;
	case 'w'     : message(" wield",1);
			break;
	case 'c'     : message(" call an item",1);
			break;
	case 'z'     : message(" zap a wand",1);
			break;
	case 't'     : message(" throw a weapon",1);
			break;
	case '\032'  : message(" suspend process ",1);
			break;
	case '!'     : message(" spawn a command",1);
			break;
	case 'v'     : message(" version number",1);
			break;
	case 'Q'     : message(" quit",1);
			break;
	case '/'     : message(" identify monster",1);
			break;
	case '?'     : message(" help on a command",1);
			break;
	default      : message(" Unknown command",1);
	}
}

