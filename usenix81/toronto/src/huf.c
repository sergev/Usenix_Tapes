/*
/*% cc -s -O -n huf.c
**	Huf - encode and decode Huffman coded text files.
**
**	D. Hugh Redelmeier  1978 June 24
**
**	The arguments to this command are a sequence of file
**	names and flags.
**
**	A flag starts with "-" and continues with any number
**	of options.
**
**	The "w<number>" option sets the upper bound on the
**	number of unique words allowed in a file to be encoded.
**	To reduce hashing collisions, ten percent more words than
**	specified are reserved but not used.  The resulting number
**	is moved up to the nearest prime known to the program.
**
**	The "p" option causes the output to be printed on the
**	standard output (even if this output is encoded).
**	The flag complements the option which is initially off.
**
**	The "n" (for noisy) option causes various statistics
**	to be printed on the diagnostic output.
**	The flag complements the option which is initially off.
**
**	The "u" option causes the input file to be unlinked.
**	If any error is detected(!) during translation, the file
**	is not unlinked.  The flag complements the option which
**	is initially off.
**
**	A file whose name ends in <suffix> is decoded into a file
**	with the same path name, minus the <suffix> (unless the
**	"p" option is used).
**
**	A file whose name does not end in <suffix> is encoded
**	into a file with the same path name, plus <suffix>
**	(unless the "p" option is used).  The encoding process
**	takes two passes, requiring that the file not be a device.
**	The file must not be modified while being encoded.
**
**	The "d" (for dump) option causes huf to abort when an error
**	is detected.
**
**	Structure of encoded file:
**
**	int magic_number;
**	bits(nw_len) num_words;
**	bits(spec_len) num_specs-1;
**	bits(spec_len) specs[num_specs];	/* in frequency order
**	wc words[num_words];	/* in frequency order
**	bit spec_tree_building_decisions[];
**	bit word_tree_building_decisions[];
**	bit starts_with_word?;
**	bit hufman_code[];
*/

#define null		0

#define num_ascii	128
#define ascii_mask	(num_ascii-1)
#define a_flag		0200
#define eof		'9'
#define per_nl		'0'
#define comma_sp	'1'
#define comma_nl	'2'
char *eof_ptr;
#define max_tabs	(('z'-'a')/2)
#define nl_tabs		('a'+max_tabs)
#define sc_nl_tabs	('A'+max_tabs)
int cur_tabs;

#define num_wc		64
#define wc_len		6
#define wc_mask		(num_wc-1)
char wc_to_a[num_wc];
char a_to_wc[num_ascii];

#define wf_bias		num_ascii
#define max_specs	(num_ascii+wf_bias)
#define spec_len	8
#define spec_mask	(max_specs-1)

#define solitary	max_specs
char *sol_ptr;

#define byte_len	8
#define byte_mask	0377
#define long_len	32
#define int_sign	0100000
#define len_len	5
#define len_mask	037

char *mem;
char *mem_roof;
extern char *sbrk();
extern char *brk();

#define max_max_words	(8000-max_specs)
/* nw_len must be less than 16 since nxt_bit must be less than 16 */
#define nw_len		13
#define nw_mask		8191
int max_words		1723;

char *chars;
char *nxt_char;
#define char_roof	mem_roof
#define char_chunk	2048

#define blocksize	512
#define magic_num	0145405

int inf;
char inbuf[blocksize];
char *inpos;
char *inend;
long in_len;
int in_line		0;

int outf;
#define suffix		'H'
#define pn_size		100
#define fn_size		14
char pname[pn_size+1];
char *fname;
char *fn_end;
char outbuf[blocksize];
char *outpos;
long out_len;

int bits;
int nxt_bit;

struct node {
	char *left;
	char *right;
	};
struct node *nodes;

char **cur_parm;
int noisy		0;	/* print statistics */
int print		0;	/* use standard output, not a created file */
int unl			0;	/* unlink input file at end */
int dump		0;	/* dump on error */
/*
 * The structure used by times(II)
 */
struct tbuffer{
	long int proc_user_time;
	long int proc_system_time;
	long int child_user_time;
	long int child_system_time;
};
struct tbuffer tbuffer;

int primes[] {	607,	947,	1109,	1511,
		1913,	2393,	2707,	3121,
		3529,	3911,	4363,	4721,
		5167,	5521,	5939,	6311,
		6719,	7121,	7541,	0 };

main(argc,argv)
		int argc;
		char *argv[];
		{
	register int i;
	register char *p;
	register int c;
	int *ip;
	int putime, pstime;
	cur_parm=argv;
	for (c=i=0; i!=num_ascii; i++) {
		if ('a'<=i && i<='z' ||
			'A'<=i && i<='Z' ||
			'0'<=i && i<='9' ||
			i=='_') {
			c++;
			if (c>=num_wc)
				abort("Bad wc");
			wc_to_a[c]=i;
			a_to_wc[i]=c;
			}
		else
			a_to_wc[i]=0;
		}
	mem_roof=mem=sbrk(0);
	times(&tbuffer);
	while ((p = *++cur_parm) != 0) {
		if (*p=='-') {
			while (*++p) {
				switch (*p) {
				case 'd':
					dump = !dump;
					break;
				case 'n':
					noisy = !noisy;
					break;
				case 'p':
					print = !print;
					break;
				case 'u':
					unl = !unl;
					break;
				case 'w':
					for (i=0; '0'<= p[1] && p[1]<='9';)
						i=i*10+(*++p-'0');
					for (ip=primes; *ip && *ip<i; ip++)
						;
					if (*ip==0)
						error("Number must be less than %d",
						(*-ip/11)*10);
					max_words = *ip;
					break;
				default:
					error("Bad flag: \"%c\"",*p);
					}
				}
			}
		else {
			inf=open(p,0);
			if (inf<0)
				error("Can't open");
			fn_end=fname=pname;
			while (*p!=0) {
				if (*p == '/') {
					while (p[1] == '/')
						p++;
					if (p[1]==0)
						break;
					fname=fn_end+1;
					}
				if (fn_end>=fname+fn_size || fn_end>=pname+pn_size)
					error("File name too long");
				*fn_end++ = *p++;
				}
			if (noisy)
				printf("%s: ",*cur_parm);
			outpos=outbuf;
			out_len=0;
			bits=nxt_bit=0;
			if (fn_end!=fname && fn_end[-1]==suffix)
				decode();
			else
				encode();
			in_line=0;
			close(inf);
			putbuf();
			if (outf!=1)
				close(outf);
			if (unl) {
				if (unlink(*cur_parm) == -1)
					error("Can't unlink");
				}
			if (noisy) {
				printf("%ld bytes in; %ld bytes out.\n",
					in_len,out_len);
				putime=tbuffer.proc_user_time;
				pstime=tbuffer.proc_system_time;
				times(&tbuffer);
				printf("%.2fs user; %.2fs system.\n",
					(tbuffer.proc_user_time-putime)/60.,
					(tbuffer.proc_system_time-pstime)/60.);
				}
			}
		}
	}

/*
**	Input/Output Routines
*/

error(m,n1,n2) char *m; int n1, n2; {
	printf("huf(%s)",*cur_parm);
	if (in_line)
		printf(" in line %l",in_line);
	printf(": ");
	printf(m,n1,n2);
	printf(".\n");
	if (dump)
		abort("Error.");
	exit(1);
	}

char inchsub() {
	register int len;
	if (inpos==inend) {
		if (inend==inbuf)
			error("Unexpected EOF");
		inpos=inend=inbuf;
		len=read(inf,inbuf,blocksize);
		if (len<=0) {
			if (len<0)
				error("Input I/O error");
			return(0);
			}
		in_len =+ len;
		inend =+ len;
		}
	return(*inpos++);
	}

#define inch()	(inpos==inend? inchsub():*inpos++)

need_byte() {
	if (bits>>nxt_bit)
		abort("Bad bits");
	bits =| (inch()&byte_mask)<<nxt_bit;
	nxt_bit =+ byte_len;
	}

#define need_bits(n)	{ if(nxt_bit<(n)) need_byte(); nxt_bit =- (n); }

#include <sys/types.h>
#include <sys/stat.h>

croutf() {
	struct stat statbuf;
	struct { int int_field; };

	if (stat(pname,&statbuf) != -1)
		error("\"%s\" already exists; I will not overwrite",pname);
	if (fstat(inf,&statbuf) == -1)
		error("Can't fstat input file");
	outf=creat(pname,statbuf.st_mode&0666);
	if (outf == -1)
		error("Can't create \"%s\"",pname);
	}

putbuf() {
	if (write(outf,outbuf,outpos-outbuf) == -1)
		error("Output I/O error");
	out_len =+ outpos-outbuf;
	outpos=outbuf;
	}

#define outch(c)	{ *outpos++ = c; if(outpos==outbuf+blocksize) putbuf(); }

got_byte() {
	outch(bits);
	bits =>> byte_len;
	nxt_bit =- byte_len;
	}

#define got_bits(n)	{ nxt_bit =+ (n); if(nxt_bit>=byte_len) got_byte(); }

/*
**	Encoder dynamic memory structures:
**
**	struct tree forest[max_syms];
**		forest, eof_ptr, sol_ptr, forest_roof.
**	struct node nodes[max_words];
**		nodes.
**	char *words[max_words] overlaying start of nodes;
**		words.
**	char chars[] overlaying end of nodes and extending beyond;
**		chars, nxt_char, char_roof (synonym for mem_roof).
*/

struct tree {
	unsigned count;	/* can be changed to int type */
	char *ptr;
	};
struct { /* overlay for above */
	long code;
	};

/*  magic type converters  */
struct { char *char_ptr; };
struct { struct tree *tree_ptr; };
struct { struct node *node_ptr; };

char **words;
struct tree *forest;
struct tree *forest_roof;
int word_limit;
int words_left;
int max_syms;
int max_code_len;
long tab_len;
long sym_lookups,hash_probes;
long bytes_specials,bytes_words;
char saved;
char *char_ceil;	/* char_roof-1 */
struct tree *lex();	/* declare forward */

encode() {
	register int i;
	char *mem_need;
	register struct tree *symbol;
	long lbits;

	word_limit=max_words-max_words/11;
	max_syms=max_specs+1+max_words;
	forest=mem;
	sol_ptr=forest+solitary;
	eof_ptr=forest+eof;
	forest_roof=forest+max_syms;
	nodes=forest_roof;
	mem_need=nodes+max_words;
	words=forest_roof;
	chars=words+max_words;
	if (mem_need<chars)
		mem_need=chars;
	if (mem_roof<mem_need) {
		if (brk(mem_need) == -1)
			error("Can't get memory needed");
		mem_roof=mem_need;
		}
	char_ceil=char_roof-1;
	max_code_len=0;
	hash_probes=0;
	sym_lookups=0;
	census_pass();
	init_lex();
	bits =| (a_to_wc[saved&ascii_mask]!=0)<<nxt_bit;
	got_bits(1);
	do {
		symbol=lex();
		lbits=symbol->code;
		if (lbits == -1L)
			lbits=sol_ptr->code;
		i=lbits&len_mask;
		lbits = ((lbits>>len_len)<<nxt_bit)|bits;
		nxt_bit =+ i;
		while (nxt_bit>=byte_len) {
			outch( (char) lbits );
			lbits =>> byte_len;
			nxt_bit =- byte_len;
			}
		bits=lbits;
		if (symbol->code == -1L)
			enc_word(symbol);
		} while (symbol!=eof_ptr);
	if (bits!=0) {
		outch(bits);
		if (bits<0)
			abort("Signed bits");
		}
	if (noisy) {
		printf("%d words containing %d characters.\n",
			word_limit-words_left,nxt_char-chars);
		printf("%ld symbol lookups;  %ld hash probes.\n",
			sym_lookups,hash_probes);
		printf("%ld bytes of tables; %d bits in longest code.\n",
			tab_len,max_code_len);
		printf("%ld bytes of coded specials; %ld bytes of coded words.\n",
			bytes_specials,bytes_words);
		}
	}

census_pass() {
	register struct tree *symbol;
	struct tree *wf;		/* word forest */
	register struct tree *wfn;	/* word forest next */
	register struct tree *sfn;	/* spec forest next */
	extern int compar();
	extern long build_tree();

	init_lex();
	for (symbol=forest; symbol!=forest_roof; symbol++)
		symbol->count=0;
	do {
		symbol=lex();
/*		printf("Symbol %d\n",symbol-forest); /*DEBUG*/
		if (++(symbol->count) <= 0)
			error("Counter overflow. Symbol 0%o",symbol-forest);
		} while (symbol!=eof_ptr);
	in_line=0;
	for (symbol=sfn=forest; symbol!=sol_ptr; symbol++) {
		if (symbol->count!=0) {
			sfn->ptr=symbol;
			sfn->count=symbol->count;
			sfn++;
			}
		}
	for (wf = wfn = symbol =+ 1; symbol!=forest_roof; symbol++) {
		if (symbol->count != 0) {
			if (symbol->count==1)
				sol_ptr->count++;
			else {
				wfn->ptr=symbol;
				wfn->count=symbol->count;
				wfn++;
				}
			}
		}
	if (sol_ptr->count!=0)
		sol_ptr->ptr = wf = sol_ptr;
	outf=1;
	if (!print) {
		if (fn_end>=fname+fn_size)
			error("File name too long");
		*fn_end++ = suffix;
		*fn_end = 0;
		croutf();
		}
	outch(magic_num&byte_mask);
	outch(magic_num>>byte_len);
	bits=wfn-wf;	/* nxt_bit must be zero or else! */
	got_bits(nw_len);
	bits =| (sfn-forest-1)<<nxt_bit;
	got_bits(spec_len);
	qsort(forest,sfn-forest,sizeof *forest,&compar);
	for (symbol=forest; symbol!=sfn; symbol++) {
		bits =| (symbol->ptr.tree_ptr - forest) << nxt_bit;
		got_bits(spec_len);
		}
	if (wfn!=wf) {
		qsort(wf,wfn-wf,sizeof *forest,&compar);
		for (symbol=wf; symbol!=wfn; symbol++)
			enc_word(symbol->ptr.tree_ptr);
		}
	bytes_specials=build_tree(forest,sfn,sol_ptr);
	bytes_words=0L;
	if (wfn!=wf)
		bytes_words=build_tree(wf,wfn,forest_roof);
	tab_len = out_len + (outpos-outbuf);
	}

long build_tree(f,ofn,fr)
		struct tree *f;		/* forest */
		struct tree *ofn;	/* old forest next */
		struct tree *fr;	/* forest roof */
		{
	register struct node *nxt_node;
	struct tree *of;	/* old forest */
	struct tree *mf;	/* merge forest */
	register struct tree *mfn;	/* merge forest next */
	register struct tree *p;
	int tree_count;
	int decision;
	struct node *top_node;
	long bits_code;

	if (f[0].count>ofn[-1].count)
		abort("Sort failed");
	nxt_node=nodes;
	bits_code=0L;
	mf=mfn=of=f;
	for (tree_count=ofn-f; --tree_count; ) {
		decision = of==ofn || mf==mfn;
		if (of==ofn ||
		    mf!=mfn && (mf->count)<(of->count)) {
			p=mf++;
			decision =+ 2;
			}
		else
			p=of++;
		nxt_node->left = p->ptr;
		mfn->count = p->count;
		if (of==ofn ||
		    mf!=mfn && (mf->count)<(of->count)) {
			p=mf++;
			decision =+ 4;
			}
		else
			p=of++;
		nxt_node->right = p->ptr;
		mfn->ptr = nxt_node;
		mfn->count =+ p->count;
		if (mfn->count < p->count) {
			if (tree_count != 1)
				error("Counter overflow. %d trees left",
					tree_count);
			bits_code =+ 0200000L;
			}
		bits_code =+ mfn->count;
		if ((decision&1)==0) {
			if (decision == 2) {
				/* turn into decision=4 */
				nxt_node->right = nxt_node->left;
				nxt_node->left = p->ptr;
				decision=4;
				}
			if (decision==4) {
				/* bits =| 0<<nxt_bit; */
				got_bits(1);
				}
			else /* decision==0 || decision==6 */ {
				bits =| (decision&2|1)<<nxt_bit;
				got_bits(2);
				}
			}
		nxt_node++;
		mfn++;
		}
	top_node = (of!=ofn? of:mf)->ptr;
	for (p=f; p!=fr; p++)
		p->code = -1L;
	assign_codes(top_node,0L,0);
	return(bits_code>>3);
	}

int compar(p,q)
		struct forest *p;
		struct forest *q;
		{
	return (p->count - q->count);
	}

enc_word(w) struct tree *w; {
	register char *p;

	if (w!=sol_ptr) {
		p=words[(w-forest)-(solitary+1)];
		do {
			bits =| a_to_wc[*p&ascii_mask]<<nxt_bit;
			got_bits(wc_len);
			} while ((*p++ & a_flag) == 0);
		}
	/* bits =| 0<<nxt_bit; */
	got_bits(wc_len);
	}

assign_codes(node,c,cl)
		register struct tree *node;	/* also struct node *node */
		long c;
		register int cl;
		{

/*	printf("Assign %d %O %d\n",node,c,cl); /*DEBUG*/
	if (node<nodes) {
		if (cl>max_code_len) {
			max_code_len=cl;
			if (cl>long_len-1-len_len)
				error("A code would be too long");
			}
		node->code = c<<len_len | cl;
		}
	else {
		assign_codes(node->left,c,cl+1);
		assign_codes(node->right,c|(1L<<cl),cl+1);
		}
	}

init_lex() {
	register char **p;
	register int i;

	nxt_char=chars;
	p=words;
	i=max_words;
	do {
		*p++ = null;
		} while (--i);
	if (lseek(inf,0L,0)<0)
		error("Seek failed");
	words_left=word_limit;
	inpos=inend=inbuf+1;
	in_len=0;
	in_line=1;
	cur_tabs=0;
	saved=inch();
	}

struct tree *lex() {
	register char c;
	register char *p;
	register int i;
	char *end_word;

	c=saved;
	if ((c&~ascii_mask)!=0)
		error("Non-ASCII character: \\%o",c&byte_mask);
	if (a_to_wc[c]!=0) {
		i=0;
		p=nxt_char;
		do {
			i = i*011111+a_to_wc[c];
			if (p>=char_ceil) {
				mem_roof =+ char_chunk;
				if (brk(mem_roof) == -1)
					error("Out of memory");
				char_ceil=char_roof-1;
				}
			*p++ = c;
			c=inch();
			} while ((c&~ascii_mask)==0 && a_to_wc[c]!=0);
		saved=c;
		p[-1] =| a_flag;
		p[0] = 0;
		end_word=p;
		sym_lookups++;
		i = (i&~int_sign) % max_words;
		for (;;) {
			hash_probes++;
			if ((p=words[i]) == null) {
				words[i]=nxt_char;
				nxt_char=end_word;
				if (--words_left == 0)
					error("Out of %d words",word_limit);
				break;
				}
			c.char_ptr=nxt_char;
			do; while(*p++ == *c.char_ptr++);
			if (c.char_ptr>end_word)
				break;
			i++;
			if (i==max_words)
				i=0;
			}
		i =+ max_specs+1;
		}
	else {
		if (inend==inbuf)
			c=eof;
		else {
			saved=inch();
			switch (c) {
			case ';':
				if (saved!='\n')
			break;
				saved=inch();
				/* fall through */
			case '\n':
				in_line++;
				for (i=0; saved=='\t' && i<=max_tabs; i++)
					saved=inch();
				c = (c==';'? sc_nl_tabs : nl_tabs)+i-cur_tabs;
				cur_tabs=i;
				break;
			case ',':
				if (saved==' ') {
					saved=inch();
					c=comma_sp;
					}
				else if (saved=='\n') {
					in_line++;
					saved=inch();
					c=comma_nl;
					}
				break;
			case '.':
				if (saved=='\n') {
					in_line++;
					saved=inch();
					c=per_nl;
					}
				}
			}
		i=a_to_wc[saved&ascii_mask]!=0? c+wf_bias : c;
		}
	return (forest+i);
	}

/*
**	Decoder dynamic memory structures:
**
**	struct node nodes[num_specs+num_words];
**		nodes, nxt_node.
**	char chars[num_chars];
**		chars, eof_ptr, sol_ptr, nxt_char, char_roof (synonym of mem_roof).
*/

char *spec_tree;
char *word_tree;
char *dec_word();	/* declare forward */
char *get_tree();	/* declare forward */

decode() {
	register char *node;
	register int c;
	int num_specs;
	int num_words;

	inpos=inend=inbuf+1;
	in_len=0;
	cur_tabs=0;
	c=inch()&byte_mask;
	c =| inch()<<byte_len;
	num_words=inch()&byte_mask;
	need_bits(nw_len-byte_len);
	num_words =| (bits<<byte_len)&nw_mask;
	bits =>> nw_len-byte_len;
	need_bits(spec_len);
	num_specs=(bits&spec_mask)+1;
	bits =>> spec_len;
	if (c!=magic_num ||
	    num_specs>max_specs ||
	    num_words>max_max_words+1)
		error("Input is not Hufman encoded");
	nodes=mem;
	chars=nodes+num_specs+num_words;
	eof_ptr=chars+eof;
	sol_ptr=chars+solitary;
	nxt_char=sol_ptr+1;
	if (mem_roof<nxt_char) {
		mem_roof=nxt_char+char_chunk;
		if (brk(mem_roof) == -1)
			error("Can't get needed memory");
		}
	outf=1;
	if (!print) {
		*--fn_end=0;
		if (fn_end<=fname)
			error("File name too short");
		croutf();
		}
	node=nodes;
	for (c=num_specs; c!=0; c--) {
		need_bits(spec_len);
		node->left = chars+(bits&spec_mask);
		bits =>> spec_len;
		node.node_ptr++;
		}
	for (c=num_words; c!=0; c--) {
		node->left = nxt_char;
		nxt_char=dec_word();
		if (nxt_char==node->left)
			node->left = sol_ptr;
		node.node_ptr++;
		}
	word_tree=spec_tree=get_tree(nodes,nodes+num_specs);
	if (num_words!=0)
		word_tree = get_tree(nodes+num_specs,node);
	need_bits(1);
	node=bits&1?word_tree:spec_tree;
	bits=>>1;
	for (;;) {
/*		printf("Node = 0%o\n",node);	/*DEBUG*/
		if (node<chars) {
			if (--nxt_bit<0) {
				bits=inch()&byte_mask;
				nxt_bit=byte_len-1;
				}
			node=bits&1? node->right : node->left;
			bits=>>1;
			}
		else if (node>=sol_ptr) {
			if (node==sol_ptr) {
				node=dec_word();
				node=nxt_char;
				}
			do {
				outch(*node.char_ptr&ascii_mask);
				} while ((*node.char_ptr++&a_flag) == 0);
			node=spec_tree;
			}
		else {
			if (node==eof_ptr)
				break;
			c=(node.char_ptr-chars)&~wf_bias;
			if (a_to_wc[c]) {
				if (c==per_nl) {
					outch('.');
					outch('\n');
					}
				else if (c==comma_sp) {
					outch(',');
					outch(' ');
					}
				else if (c==comma_nl) {
					outch(',');
					outch('\n');
					}
				else {
					if (nl_tabs-max_tabs<=c && c<=nl_tabs+max_tabs)
						c =- nl_tabs;
					else {
						c =- sc_nl_tabs;
						outch(';');
						}
					outch('\n');
					cur_tabs = c =+ cur_tabs;
					while (c--)
						outch('\t');
					}
				}
			else
				outch(c);
			node=node<chars+wf_bias?spec_tree:word_tree;
			}
		}
	}

char *get_tree(of,ofn)
		register struct node *of;	/* used as a copy */
		struct node *ofn;
		{
	struct node *mf;
	register struct node *mfn;
	register struct node *p;
	int tree_count;
	int decision;

	mf=mfn=of;
	for (tree_count=ofn-of; --tree_count; ) {
		if (mfn>of)
			abort("Tree builder");
		if (of!=ofn && mf!=mfn) {
			/* both lists are non-empty, so read "decision" */
			need_bits(1);
			if ((bits&1)==0)
				decision=4;
			else {
				bits =>> 1;
				need_bits(1);
				decision = (bits&1)==0? 0 : 6;
				}
			bits =>> 1;
			}
		if (of==ofn ||
		    mf!=mfn && (decision&2)!=0)
			p=mf++;
		else
			p = of++ -> left;
		mfn->left = p;
		if (of==ofn ||
		    mf!=mfn && (decision&4)!=0)
			p=mf++;
		else
			p = of++ -> left;
		mfn->right = p;
		mfn++;
		}
	return (of!=ofn? of->left:mf.char_ptr);
	}

char *dec_word() {
	register char *p;
	register char c;

	p=nxt_char;
	for (;;) {
		need_bits(wc_len);
		c=bits&wc_mask;
		bits=>>wc_len;
		if (c==0) {
			if (p!=nxt_char)
				p[-1] =| a_flag;
			return(p);
			}
		if (p>=char_roof) {
			mem_roof =+ char_chunk;
			if (brk(mem_roof) == -1)
				error("Out of memory for characters");
			}
		*p++ = wc_to_a[c];
		}
	}
