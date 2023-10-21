main()
{
	register char c;

    while((c = getchar()) != '\0'){
	if((c >= 'A')&&(c <= 'Z'))c =+ 32;
	switch(c){
		case 'a':
			printf("._ ");
			break;
		case 'b':
			printf("_... ");
			break;
		case 'c':
			printf("_._. ");
			break;
		case 'd':
			printf("_.. ");
			break;
		case 'e':
			printf(". ");
			break;
		case 'f':
			printf(".._. ");
			break;
		case 'g':
			printf("__. ");
			break;
		case 'h':
			printf(".... ");
			break;
		case 'i':
			printf(".. ");
			break;
		case 'j':
			printf(".___ ");
			break;
		case 'k':
			printf("_._ ");
			break;
		case 'l':
			printf("._.. ");
			break;
		case 'm':
			printf("__ ");
			break;
		case 'n':
			printf("_. ");
			break;
		case 'o':
			printf("___ ");
			break;
		case 'p':
			printf(".__. ");
			break;
		case 'q':
			printf("__._ ");
			break;
		case 'r':
			printf("._. ");
			break;
		case 's':
			printf("... ");
			break;
		case 't':
			printf("_ ");
			break;
		case 'u':
			printf(".._ ");
			break;
		case 'v':
			printf("..._ ");
			break;
		case 'w':
			printf(".__ ");
			break;
		case 'x':
			printf("_.._ ");
			break;
		case 'y':
			printf("_.__ ");
			break;
		case 'z':
			printf("__.. ");
			break;
		case '1':
			printf(".____ ");
			break;
		case '2':
			printf("..___ ");
			break;
		case '3':
			printf("...__ ");
			break;
		case '4':
			printf("...._ ");
			break;
		case '5':
			printf("..... ");
			break;
		case '6':
			printf("_.... ");
			break;
		case '7':
			printf("__...");
			break;
		case '8':
			printf("___.. ");
			break;
		case '9':
			printf("____. ");
			break;
		case '0':
			printf("_____ ");
			break;
		case '.':
			printf("._._._ ");
			break;
		case '?':
			printf("..__.. ");
			break;
		case ',':
			printf("__..__ ");
			break;
		case '"':
			printf("._.._. ");
			break;
		case '/':
			printf("_.._. ");
			break;
		case '(':
			printf("_.__. ");
			break;
		case ')':
			printf("_.__._ ");
			break;
		case '$':
			printf("..._.._ ");
			break;
		case '!':
			printf("___. ");
			break;
		case '\'':
			printf(".____. ");
			break;
		case '-':
			printf("_...._ ");
			break;
		case ';':
			printf("_._._. ");
			break;
		case ':':
			printf("___... ");
			break;
		default:
			putchar(c);
		}
	}
	putchar('\0');
}
