/*
 * Text pattern matcher.
 * Mkpatt returns pointer to alloc'd string which is an encoding
 * of the regular expression given as its argument.  (The pattern
 * is a string if the expression is; if the expression contains
 * nulls then so will the pattern).
 * Match returns true or false depending on whether the text
 * string matches the encoded pattern or not.
 * Pattern syntax is the usual:
 *      ^ means anchor to start of line if its the first char
 *      . means any char
 *      * means indefinite repeat
 *      $ means anchor to end of line if its the last char
 *      \ escapes any special char
 *      [] enclose alternates: if ^ is first it negates the set
 *              w.r.t. printable chars.  All alternates must be
 *              listed, i.e., no '-' shorthand.  If ~ is first
 *              it negates the set w.r.t. all chars.
 *      any other character matches itself.
 * The global match_m is a struct { int len; char *ptr; } giving the
 * subset of the string which satisfies the match.
 */

#define EOF     0
#define BOL     0136
#define ANY     056
#define STAR    1
#define EOS     044
#define CHR     0142
#define CCL     050

char *mkpatt(s,l) register char *s;{
	register char c, *p;
	char *se, *patt, *lastp;

	if(l==0) l= strlen(s);
	se= s+l;
	if((p= alloc(2*l +2))==0177777) return 0;
	for(patt= p, lastp= 0; s<se; ){
		c= *s++;
		if(c!='*') lastp= p;
		switch(c){
			case '^':
				if(p!=patt) goto regchar;
				*p++= BOL;
				break;
			case '.':
				*p++= ANY;
				break;
			case '*':
				if(lastp==0) goto regchar;
				*lastp=| STAR;
				lastp= 0;
				break;
			case '$':
				if(s<se) goto regchar;
				*p++= EOS;
				break;
			case '\\':
				if(s<se) c= *s++;
			default:
			regchar:
				*p++= CHR;
				*p++= c;
				break;
			case '[':
				*p++= CCL;
				for(p++; s<se && (c= *s++)!=']'; *p++= c)
					if(c=='\\' && s<se) c= *s++;
				lastp[1]= p - lastp -1;
				break;
		}       } 
	*p= EOF;
	return patt; }

struct { int len; char *ptr; } match_m;

match(p,s,l) register char *p, *s;{
	register char *se;

	if(l==0) l= strlen(s);
	se= s+l;
	if(*p==BOL){
		match_m.ptr= s;
		return amatch(p+1,s,se); }
	if(*p==CHR){            /* fast check if char first */
		for(; s<se; s++)
			if(*s==p[1]){
				match_m.ptr= s;
				if(amatch(p,s,se)) return 1; }
		return 0; }
	for(; s<se; s++){       /* regular algorithm */
		match_m.ptr= s;
		if(amatch(p,s,se)) return 1; }
	return 0; }

static amatch(p,s,se) register char *p, *s, *se;{
	char *ssave;

	for(;;) switch(*p++){
			case EOF:
				match_m.len= s- match_m.ptr;
				return 1;
			case CHR:
				if(s>=se || *s++!=*p++) return 0;
				break;
			case ANY:
				if(s>=se) return 0;
				s++;
				break;
			case EOS:
				if(s!=se) return 0;
				break;
			case CCL:
				if(s>=se || !inset(*s++, p+1, p+ *p))
					return 0;
				p=+ *p;
				break;
			case ANY|STAR: 
				ssave= s;
				s= se;
				goto starloop;
			case CHR|STAR:
				ssave= s;
				while(s<se && *s==*p) s++;
				p++;
				goto starloop;
			default: return -1;
			case CCL|STAR:
				ssave= s;
				while(s<se && inset(*s, p+1, p+ *p)) s++;
				p=+ *p;
			starloop:
				for(; s>=ssave; s--)
					if(amatch(p,s,se)) return 1;
				return 0;
	}               } 

static inset(c,s,se) register char c, *s, *se;{
	if(s<se && (*s=='^'|| *s=='~')){
		if(c<' ' && *s=='^') return 0;
		for(s++; s<se; ) if(c==*s++) return 0;
		return 1; }
	while(s<se) if(c==*s++) return 1;
	return 0; }
