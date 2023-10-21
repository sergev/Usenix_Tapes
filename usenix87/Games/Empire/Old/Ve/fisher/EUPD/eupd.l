/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

RADDES  [-.^cudg*ihamwtfr+#)=!bxs? BCDFMPST/]
SECTDES [-.^cudg*ihamwtfr+#)=!bxs]
%{
#define	SMAXNO	24
char	*Sect[] = { "se", "mo", "sa", "wi", "ca", "ur", "de",
		    "sh", "mi", "go", "ha", "wa", "ai",
		    "ag", "te", "fo", "re", "hi", "ra",
		    "we", "br", "br", "ba", "ex"
		  };
char	Sectchar[] = { '.', '^', 's', '-', 'c', 'u', 'd',
		       'i', 'm', 'g', 'h', 'w', '*',
		       'a', 't', 'f', 'r', '+', ')',
		       '!', '#', '=', 'b', 'x'
		     };
char	Cenflags[16];
extern	int	Maphdr, Maptens, Maplcol, Mapx;
extern	int	Colno, Line;
extern	int	Nrows, Radflag;
extern	char	Stype[];
int	j;
%}
%p 3000
%Start	RAD CEN MAP
%%
cmsgpob							{
			Colno += yyleng;
			BEGIN CEN;
			return(CMSGPOB);
							}
<CEN>("*"|" ")[$.0-7]+("%"|" ")				{
			if( Colno != 7 ) REJECT;
			if( yyleng != 9 ) REJECT;
			strncpy(Cenflags, yytext, 9);
			Colno += yyleng;
			return(CENFLGS);
							}
<CEN>" "{SECTDES}" "					{
			if( Colno != 16 ) REJECT;
			Colno += yyleng;
			yylval = (int)yytext[1];
			return(SECT);
							}
<CEN>" sector"s?					{
			Colno += yyleng;
			BEGIN 0;
			return(SECTORS);
							}
<RAD>^({RADDES}[ /])+{RADDES}				{
			strncpy(Rscan[Nrows++], yytext, MAXSCN*2);
			Colno += yyleng;
							}
<RAD>.							{
			Radflag = 0;
			if( Nrows ) dorad();
			BEGIN 0;
			REJECT;
							}
<MAP>^"     "[-0-9](" "[-0-9])+				{
			if( yyleng < 8 ) REJECT;
			Maphdr++;
			if( Maphdr == 4 ) {
				Maphdr = 0;
				BEGIN 0;
				return(MAPHDR);
			}
			if( Maphdr == 2 ) {
				Mapx = yytext[5] - '0'; /* units part first */
				j = Mapx - yytext[7] + '0'; /* check sign */
				Mapx += Maptens * 10; /* now add in tens */
				if( j == 1 || j == -9 ) Mapx = -Mapx;
				Maplcol = yyleng;
			}
			return(MAPHDR);
							}
"     "[-0-9](" "[-0-9])+				{
			if( Maphdr != 0 || yyleng < 8 ) REJECT;
			if( yytext[5] == '-' ) {
				Maptens = 0;
			} else {
				Maptens = yytext[5] - '0';
			}
			Maphdr = 1;
			BEGIN MAP;
			return(MAPHDR);
							}
"rad"a?r?						{
			Colno += yyleng;
			return(RADAR);
							}
"Efficiency"						{
			Colno += yyleng;
			return(EFF);
							}
"barometer"						{
			Colno += yyleng;
			return(BAROM);
							}
"max range"						{
			Colno += yyleng;
			return(RANGE);
							}
"Command"						{
			Colno += 7;
			BEGIN 0;
			return(CMD);
							}
"Execute"						{
			Colno += 7;
			BEGIN 0;
			return(EX);
							}
"Spy"							{
			Colno += yyleng;
			return(SPY);
							}
"deported"						{
			Colno += yyleng;
			return(DPRTD);
							}
(pt" "b|mines|destr|subma|freig|tende|battl|carri)[^ ]*	{
			Colno += yyleng;
			for( j=0; i<=TMAXNO; j++ ) {
				if( yytext[0] == Stype[j] ) break;
			}
			yylval = j;
			return(SHIP);
							}
"stopped"						{
			Colno += yyleng;
			return(STPD);
							}
"Snorkel"						{
			Colno += yyleng;
			return(SUB);
							}
Your							{
			Colno += yyleng;
			return(YOUR);
							}
Unowned							{
			Colno += yyleng;
			return(UNOWN);
							}
eff							{
			Colno += yyleng;
			return(EFF);
							}
with							{
			Colno += yyleng;
			return(WITH);
							}
approx"."						{
			Colno += yyleng;
			return(APPRX);
							}
Deliver							{
			Colno += yyleng;
			return(DLVR);
							}
"from "							{
			Colno += yyleng;
			return(FROM);
							}
" to "							{
			Colno += yyleng;
			return(TO);
							}
thresh							{
			Colno += yyleng;
			return(THRSH);
							}
"Now over"						{
			Colno += yyleng;
			return(NWOVR);
							}
"Flying over"						{
			Colno += yyleng;
			return(FLYOVR);
							}
(completely|partially|minimally)					{
			Colno += yyleng;
			return(CMPRTLY);
							}
constructed						{
			Colno += yyleng;
			return(CNSTRCTD);
							}
civilians						{
			Colno += yyleng;
			yylval = CIV;
			return(CIVTKN);
							}
(troops|military)					{
			Colno += yyleng;
			yylval = MIL;
			return(MILTKN);
							}
shells							{
			Colno += yyleng;
			yylval = SHL;
			return(SHLTKN);
							}
guns							{
			Colno += yyleng;
			yylval = GUN;
			return(GUNTKN);
							}
planes							{
			Colno += yyleng;
			yylval = PLN;
			return(PLNTKN);
							}
" ore "							{
			Colno += yyleng;
			yylval = ORE;
			return(ORETKN);
							}
"bars of gold"						{
			Colno += yyleng;
			yylval = BAR;
			return(BARTKN);
							}
(sea|mount|sanct|wilde|capit|urban" "a|defense" "p)[a-z]*		{
			Colno += yyleng;
			yylval = schar(yytext);
			return(SECT);
							}
(shell" "i|mine|gold" "m|harb|ware|airf|agri)[a-z]*	{
			Colno += yyleng;
			yylval = schar(yytext);
			return(SECT);
							}
(technical" "c|fortr|research" "l|high|radar" "i)[a-z]*	{
			Colno += yyleng;
			yylval = schar(yytext);
			return(SECT);
							}
(weather" "s|bridge" "h|bridge" "s|bank|exch)[a-z]*	{
			Colno += yyleng;
			yylval = schar(yytext);
			return(SECT);
							}
-?[0-9]+						{
			Colno += yyleng;
			yylval = atoi(yytext);
			return(NUM);
							}
" at"							{
			Colno += yyleng;
			return(AT);
							}
[ \t]			Colno++;
\n							{
			Colno = 0;
			Line++;
			if( Radflag ) {
				BEGIN RAD;
			}
			return(EOL);
							}
([:,%#.@<>()&]|"["|"]")				{
			Colno++;
			yylval = Colno;
			return((int)*yytext);
							}
[abcdfghimrstuwx]					{
			Colno++;
			yylval = (int)*yytext;
			return(SECTLET);
							}
[-!=^*+?]						{
			Colno++;
			yylval = (int)*yytext;
			return(SECTNLET);
							}
.							{
			Colno++;
			yylval = Colno;
			return(ANYCHAR);
							}
%%
schar(s)
char	*s;
{
	extern	char	*Sect[], Sectchar[];
	int	i;

	for( i=0; i<SMAXNO; i++ ) {
		if( strncmp( s, Sect[i], 2) == 0 ) break;
	}
/*
	Adjust for bridge span
*/
	if( Sectchar[i] == '#' && s[7] == 's' ) i++;
	return( (int)Sectchar[i] );
}
