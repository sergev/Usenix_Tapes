#include "../em1.h"
{
  (c) copyright 1978 by the Vrije Universiteit, Amsterdam, The Netherlands.
  Explicit permission is hereby granted to universities to use or duplicate
  this program for educational or research purposes.  All other use or dup-
  lication  by universities,  and all use or duplication by other organiza-
  tions is expressly prohibited unless written permission has been obtained
  from the Vrije Universiteit. Requests for such permissions may be sent to
       Dr. Andrew S. Tanenbaum
       Wiskundig Seminarium
       Vrije Universiteit
       Postbox 7161
       1007 MC Amsterdam
       The Netherlands
}
{if next line is included the compiler itself is written in standard pascal}
{#define	STANDARD	1}
{temporary kludge for the EM1 cse/csa/csb instruction}
#define	OLDCASE		1

{Author:	Johan Stevenson			Version:	26}
{$l- : no source line numbers}
{$r- : no subrange checking}
{$a- : no assertion checking}
program pem(input,output,em1);
{This Pascal compiler  produces EM1 code as described in
   - A.S.Tanenbaum, J.W.Stevenson & H. van Staveren,
	"Description of a experimental machine architecture for use of
	 block structured languages" Informatika rapport xx.
  A description of Pascal is given in
   - K.Jensen & N.Wirth, "PASCAL user manual and report", Springer-Verlag.
  Several options may be given in the normal pascal way. Moreover,
  a positive number may be used instead of + and -. The options are:
	a:	interpret assertions (+);
	c:	C-type strings allowed (-);
	i:	controls the number of bits in integer sets (16);
	l:	insert code to keep track of source lines (+);
	o:	optimize (+);
	r:	check subranges (+);
	s:	accept only standard pascal programs (-);
	t:	trace procedure entry and exit (-);
	v:	produce code for 24 bit addresses (-);
}
{===================================================================}
#ifdef STANDARD
label 9999
#endif
const
{sizes of standard types}
  {The sizes are set to fit the EM1 machine architecture. Sizes and
    addresses are expressed in bytes (1 byte = 8 bits). All objects
    must have an even size, with the exception of small scalars
    fitting in one byte.
    EM1 requires each object of more than one byte to start on an even
    address. Default the same is true for objects of one byte. However
    the 'packed' qualifier turns this alignment off, both for arrays
    and for records. Strings are packed arrays.
    The size of pointers depends on vopt, indicating
    segmented memory with 24 bits addresses.
    Two words (1 word = 2 bytes) are used for reals.
    The routines involved with alignment are even, address and arraysize.
  }
  charsize=1;
  boolsize=1;
  intsize =2;
  realsize=4;
  maxsetsz=32;		{maximum number of bytes for a set}
  {see handleopts for ptrsize}
{other sizes}
  pdsize=2;		{size of procedure descriptors}
  buffersize=512;	{size of file buffer}
{maximal indices}
  idmax=8;		smax=72;		rmax=72;
{value of nil (2 words) }
  nil1=0;		nil2=0;
{value of real zero (2 words) }
  real1=0;		real2=0;
{opt values}
  off=0;		on=1;
{for push and pop: }
  global=false;		local=true;
{integer bounds}
  minint=-32767;	maxint=32767;
{set bounds}
  minsetint=0;		maxsetint=15;		{default}
  maxsetwd=16;		{maxsetsz div 2}
  maxwbit=15;		{maximal word bit number}
  bytebits=8;		{number of bits in a byte}
{constants describing the compact EM1 code}
  {magic word}
  MAGICLOW	= 172;		MAGICHIGH	= 0;
{miscellaneous}
  maxsg=127;		{maximal segment number}
  maxcharord=127;	{maximal ordinal number of chars}
  maxargc=13;		{maximal index in argv}
  rwlim=34;		{number of reserved words}
#ifdef OLDCASE
  cixmax=256;
#endif
  spaces='        ';

{-------------------------------------------------------------------}
type
{scalar types}
  symbol=	(comma,semicolon,colon1,colon2,notsy,lbrack,ident,intcst,
		 charcst,realcst,stringcst,nilcst,minsy,plussy,lparent,
		 arrow,arraysy,recordsy,setsy,filesy,packedsy,progsy,
		 labelsy,constsy,typesy,varsy,procsy,funcsy,beginsy,
		 gotosy,ifsy,whilesy,repeatsy,forsy,withsy,casesy,
		 becomes,starsy,divsy,modsy,slashsy,andsy,orsy,
		 eqsy,nesy,gtsy,gesy,ltsy,lesy,insy,
		 endsy,elsesy,untilsy,ofsy,dosy,downtosy,tosy,
		 thensy,rbrack,rparent,period
		);			{the order is important}
  chartype=	(undscore,lower,upper,digit,layout,tabch,quotech,dquotech,
		   colonch,periodch,lessch,greaterch,lparentch,lbracech,
						{different entries}
		 rparentch,lbrackch,rbrackch,commach,semich,arrowch,
		   plusch,minch,slash,star,equal,
						{also symbols}
		 others
		);
  standpf=	(pread,preadln,pwrite,pwriteln,pput,pget,
		 preset,prewrite,pnew,pdispose,ppack,punpack,
		 pmark,prelease,ppage,phalt,
						{all procedures}
		 feof,feoln,fabs,fsqr,ford,fchr,fpred,fsucc,fodd,
		 ftrunc,fround,fsin,fcos,fexp,fsqrt,fln,farctan
						{all functions}
		);			{the order is important}
  libmnem=	(OPN ,GETX,RDI ,RDC ,RDR ,RLN ,ELN ,EFL ,CLS ,
						{on inputfiles}
		 CRE ,PUTX,WRI ,WSI ,WRC ,WSC ,WRS ,WSS ,WRB ,
		 WSB ,WRR ,WSR ,WRF ,WRZ ,WSZ ,WLN ,PAG ,
				{on outputfiles, order important}
		 ABR ,RND ,SIN ,COS ,EXPX,SQT ,LOG ,ATN ,
						{floating point}
		 ABI ,BCP ,BTS ,NEWX,SAV ,RST ,INI ,HLT ,
		 ASS ,GTO ,PAC ,UNP, DIS
						{miscellaneous}
		);
  structform=	(scalar,subrange,pointer,power,files,arrays,
		 records,variant,tag);		{order important}
  structflag=	(spack,withfile);
  varflag=	(refer,used,assigned);
  idclass=	(types,konst,vars,field,proc,func);
  kindofpf=	(standard,formal,actual,extrn,forwrd);
  where=	(blck,rec,crec,vrec);
  attrkind=	(cst,direct,indirect,indexed,expr);
  twostruct=	(eq,subeq,ir,ri,es,se,noteq);  {order important}

{subrange types}
  sgrange=	0..maxsg;
  idrange=	1..idmax;
  rwrange=	0..rwlim;
  byte=		0..255;

{pointer types}
  sp=	^structure;
  ip=	^identifier;
  lp=	^labl;
  bp=	^blockinfo;
  np=	^nameinfo;

{set types}
  sos=		set of symbol;
  setofids=	set of idclass;
  formset=	set of structform;
  sflagset=	set of structflag;
  vflagset=	set of varflag;

{array types}
  alpha =packed array[idrange] of char;

{record types}
  position=record		{the addr info of certain variable}
    ad:integer;			{for locals it is the byte offset}
    lv:integer;			{the level of the beast}
    sg:sgrange			{only relevant for globals (lv=0) }
  end;

{records of type attr are used to remember qualities of
  expression parts to delay the loading of them.
  Reasons to delay the loading of one word constants:
	- bound checking
	- set building.
  Reasons to delay the loading of direct accessible objects:
	- efficient handling of read/write
	- efficient handling of the with statement.
}
  attr=record
    asp:sp;				{type of expression}
    case ac:attrkind of			{access method}
      expr:	();
      cst:	(intval:integer);	{one word value of constant}
      direct:	(pos:position);		{sg,lv and ad}
      indirect:	(pk:boolean);		{packed or not}
      indexed:	(arpk:boolean;		{packed or not}
		 arno:integer)		{descriptor number}
  end;

  nameinfo=record		{one for each separate name space}
    nlink:np;			{one deeper}
    fname:ip;			{first name; root of tree}
    case occur:where of
      blck:();
      rec: ();
      crec:(cpos:position);	{for records with known address}
      vrec:(vdspl:integer;	{offset of local pointer to record}
	    rcpk:boolean	{packed or not}
	   )
  end;

  blockinfo=record	{all info of the current procedure}
    nextbp:bp;		{pointer to blockinfo of surrounding proc}
    lc:integer;		{data location counter (from begin of proc) }
    ilbno:integer;	{number of last local label}
    forwcount:integer;	{number of not yet specified forward procs}
    lchain:lp;		{first label; header of chain}
  end;

  structure=record
    size:integer;			{size of structure in bytes}
    sflag:sflagset;			{flag bits}
    case form:structform of
      scalar  :(scalno:integer;		{number of range descriptor}
		fconst:ip		{names of constants}
	       );
      subrange:(min,max:integer;	{lower and upper bound}
		rangetype:sp;		{type of bounds}
		subrno:integer		{number of subr descriptor}
	       );
      pointer :(eltype:sp);		{type of pointed object}
      power   :(elset:sp);		{type of set elements}
      files   :(filtype:sp);		{type of file elements}
      arrays  :(aeltype:sp;		{type of array elements}
		inxtype:sp;		{type of array index}
		arrno:integer		{number of array descriptor}
	       );
      records :(fstfld:ip;		{points to first field}
		tagsp:sp		{points to tag if present}
	       );
      variant :(varval:integer;		{tag value for this variant}
		nxtvar:sp;		{next equilevel variant}
		subtsp:sp		{points to tag for sub-case}
	       );
      tag     :(fstvar:sp;		{first variant of case}
		tfldsp:sp		{type of tag}
	       )
  end;

  identifier=record
    idtype:sp;				{type of identifier}
    name:alpha;				{name of identifier}
    llink,rlink:ip;			{see enterid,searchid}
    next:ip;				{used to make several chains}
    case klass:idclass of
      types    :();
      konst   :(value:integer);		{for integers the value is
		  computed and stored in this field;
		  For strings and reals an assembler constant is
		  defined labeled '.1', '.2', ...  This '.' number is then
		  stored in value. For reals value may be negated to
		  indicate that the opposite of the assembler constant
		  is needed. }
      vars    :(vflag:vflagset;		{flag bits}
		vpos:position		{position of var}
	       );
      field   :(foffset:integer);	{offset to begin of record}
      proc,func:
	(case pfkind:kindofpf of
	   standard:(key:standpf);	{identification}
	   formal,actual,forwrd,extrn:
	     (pflag:vflagset;		{flag bits}
	      pfpos:position;		{lv gives declaration level;
			sg gives instruction segment of this proc and
			ad is relevant for formal pf's and for
			functions (no conflict!!);
			for functions: ad is the result address;
			for formal pf's: ad is the address of the
			descriptor }
	      pfno:integer;		{unique pf number}
	      parhead:ip;		{head of parameter list}
	      headlc:integer		{lc when heading scanned}
	     )
	)
  end;

  labl=record
    nextlp:lp;		{chain of labels}
    seen:boolean;
    labval:integer;	{label number given by the programmer}
    labname:integer;	{label number given by the compiler}
    labdlb:integer	{zero means only locally used;
			  otherwise dlbno of label information}
  end;

{-------------------------------------------------------------------}
var  {the most frequent used externals are declared first}
  sy:symbol;		{last symbol}
  a:attr;		{type,access method,position,value of expr}
{returned by insym}
  ch:char;		{last character}
  chsy:chartype;	{type of ch; used by insym}
  val:integer;		{if last symbol is an constant }
  ix:integer;		{string length}
  zerostring:boolean;	{true for strings in " "}
  id:alpha;		{if last symbol is an identifier}
{blockinfo}
  b:blockinfo;		{all info to be stacked at pfdeclaration}
{some counters}
  linecount:integer;	{line number on input file; (1..n) }
  chcnt:integer;	{char count on current input line; (1..n) }
  lino:integer;		{line number on code file; (1..n) }
  dlbno:integer;	{number of last global number}
  lcmax:integer;	{keeps track of maximum of lc}
  level:integer;	{current static level}
  ptrsize:integer;
  fhsize:integer;	{size of file info}
  argc:integer;		{index in argv}
  lastpfno:integer;	{unique pf number counter}
  copt:integer;		{C-type strings allowed if on}
  iopt:integer;		{number of bits in sets with base integer}
  sopt:integer; 	{standard option}
  vopt:integer;		{two word addresses if on}
{pointers pointing to standard types}
  realptr,intptr,textptr,emptyset,boolptr,charptr,nilptr,stringptr:sp;
{opts}
  giveline:boolean;	{give source line number at next statement}
  eofexpected:boolean;	{quit without error if true (nextch) }
  main:boolean;		{complete programme or a module}
  intypedec:boolean;	{true if nested in typedeclaration}
  fltused:boolean;	{true if floating point instructions are used}
  seconddot:boolean;	{indicates the second dot of '..'}
{pointers}
  fwptr:ip;		{head of chain of forward reference pointers}
  progp:ip;		{program identifier}
  top:np;		{pointer to the most recent name space}
  lastnp:np;		{pointer to nameinfo of last searched ident }
{records}
  fa:attr;		{attr for current file name}
{arrays}
  strbuf:array[1..smax] of char;
  iop:array[boolean] of ip;
			{false:standard input; true:standard output}
  rw:array[rwrange] of alpha;
			{reserved words}
  frw:array[0..idmax] of integer;
			{indices in rw}
  rsy:array[rwrange] of symbol;
			{symbol for reserved words}
  cs:array[char] of chartype;
			{chartype of a character}
  csy:array[rparentch..equal] of symbol;
			{symbol for single character symbols}
  lmn:array[libmnem] of packed array[1..4] of char;
			{mnemonics of pascal library routines}
  opt:array['a'..'z'] of integer;
  forceopt:array['a'..'z'] of boolean;
			{26 different opts}
  undefip:array[idclass] of ip;
			{used in searchid}
  argv:array[0..maxargc] of
	 record name:alpha; ad:integer end;
			{save here the external heading names}
{files}
  em1:file of byte;	{the EM1 code is put there}
{===================================================================}

procedure gen2bytes(b:byte; i:integer);
var b1,b2:byte;
begin
  if i<0 then
    if i<minint then begin b1:=0; b2:=128 end
    else begin i:=-i-1; b1:=255 - i mod 256; b2:=255 - i div 256 end
  else begin b1:=i mod 256; b2:=i div 256 end;
  write(em1,b,b1,b2)
end;

procedure gencst(i:integer);
begin
  if (i>=0) and (i<sp_ncst0) then write(em1,i+sp_fcst0)
  else gen2bytes(sp_cst2,i)
end;

procedure genclb(i:integer);
begin if i<256 then write(em1,sp_ilb1,i) else gen2bytes(sp_ilb2,i) end;

procedure genilb(i:integer);
begin lino:=lino+1;
  if i<sp_nilb0 then write(em1,i+sp_filb0) else genclb(i);
end;

procedure gendlb(i:integer);
begin if i<256 then write(em1,sp_dlb1,i) else gen2bytes(sp_dlb2,i) end;

procedure gen0(b:byte);
begin write(em1,b); lino:=lino+1 end;

procedure gen1(b:byte; i:integer);
begin gen0(b); gencst(i) end;

procedure gend(b:byte; d:integer);
begin gen0(b); gendlb(d) end;

procedure genident(nametype:byte; var a:alpha);
var i,j:integer;
begin i:=idmax;
  while (a[i]=' ') and (i>1) do i:=i-1;
  write(em1,nametype,i);
  for j:=1 to i do write(em1,ord(a[j]))
end;

procedure gensp(m:libmnem);
var i:integer;
begin gen0(op_cal); write(em1,sp_pnam,4);
  for i:=1 to 4 do write(em1,ord(lmn[m][i]))
end;

procedure genpnam(b:byte; fip:ip);
var n:alpha; i,j:integer;
begin
  if fip^.pfpos.lv<=1 then n:=fip^.name else
    begin n:='_       '; j:=1; i:=fip^.pfno;
      while i<>0 do
	begin j:=j+1; n[j]:=chr(i mod 10 + ord('0')); i:=i div 10 end;
    end;
  gen0(b); genident(sp_pnam,n)
end;

procedure genend;
begin write(em1,sp_cend) end;

procedure genlin;
begin giveline:=false;
  if opt['l']<>off then if main then gen1(op_lin,linecount)
end;

{===================================================================}

procedure error(e:integer);
{as you will notice, all error numbers are preceded by '+' and '0' to
  ease their renumbering in case of new errornumbers.
}
begin writeln(e,linecount,chcnt-1); if e>0 then gen1(ps_mes,0) end;

procedure teststandard;
begin if sopt<>off then error(-(+01)) end;

procedure errid(e:integer; var id:alpha);
begin write(id); error(e) end;

procedure errint(e:integer; i:integer);
begin write(i:8); error(e) end;

procedure enterid(fip: ip);
{enter id pointed at by fip into the name-table,
  which on each declaration level is organised as
  an unbalanced binary tree}
var nam:alpha; lip,lip1:ip; lleft:boolean;
begin nam:=fip^.name;
  lip:=top^.fname;
  if lip=nil then top^.fname:=fip else
    begin
      repeat lip1:=lip;
	if lip^.name>nam then
	  begin lip:=lip^.llink; lleft:=true end
	else
	  begin if lip^.name=nam then errid(+02,nam);  {name conflict}
	    lip:=lip^.rlink; lleft:=false;
	  end;
      until lip=nil;
      if lleft then lip1^.llink:=fip else lip1^.rlink:=fip
    end;
  fip^.llink:=nil; fip^.rlink:=nil
end;

procedure initpos(var p:position);
begin p.lv:=level; p.sg:=0; p.ad:=0 end;

function newip(kl:idclass; n:alpha; idt:sp; nxt:ip):ip;
var p:ip;
begin
  case kl of
    types:
      new(p,types);
    konst:
      begin new(p,konst); p^.value:=0 end;
    vars:
      begin new(p,vars); p^.vflag:=[used,assigned]; initpos(p^.vpos) end;
    field:
      begin new(p,field); p^.foffset:=0 end;
    proc,func:
      begin new(p,proc,actual); p^.pfkind:=actual; p^.pflag:=[];
	initpos(p^.pfpos); p^.pfno:=0; p^.parhead:=nil; p^.headlc:=0
      end
  end;
  p^.name:=n; p^.klass:=kl; p^.idtype:=idt; p^.next:=nxt;
  p^.llink:=nil; p^.rlink:=nil; newip:=p
end;

procedure init1;
var c:char;
begin
{initialize the first name space}
  new(top,blck); top^.occur:=blck; top^.nlink:=nil; top^.fname:=nil;
  level:=0;
{reserved words}
  rw[ 0]:='if      ';	rw[ 1]:='do      ';	rw[ 2]:='of      ';
  rw[ 3]:='to      ';	rw[ 4]:='in      ';	rw[ 5]:='or      ';
  rw[ 6]:='end     ';	rw[ 7]:='for     ';	rw[ 8]:='nil     ';
  rw[ 9]:='var     ';	rw[10]:='div     ';	rw[11]:='mod     ';
  rw[12]:='set     ';	rw[13]:='and     ';	rw[14]:='not     ';
  rw[15]:='then    ';	rw[16]:='else    ';	rw[17]:='with    ';
  rw[18]:='case    ';	rw[19]:='type    ';	rw[20]:='goto    ';
  rw[21]:='file    ';	rw[22]:='begin   ';	rw[23]:='until   ';
  rw[24]:='while   ';	rw[25]:='array   ';	rw[26]:='const   ';
  rw[27]:='label   ';	rw[28]:='repeat  ';	rw[29]:='record  ';
  rw[30]:='downto  ';	rw[31]:='packed  ';	rw[32]:='program ';
  rw[33]:='function';	rw[34]:='procedur';
{corresponding symbols}
  rsy[ 0]:=ifsy;	rsy[ 1]:=dosy;		rsy[ 2]:=ofsy;
  rsy[ 3]:=tosy;	rsy[ 4]:=insy;		rsy[ 5]:=orsy;
  rsy[ 6]:=endsy;	rsy[ 7]:=forsy;		rsy[ 8]:=nilcst;
  rsy[ 9]:=varsy;	rsy[10]:=divsy;		rsy[11]:=modsy;
  rsy[12]:=setsy;	rsy[13]:=andsy;		rsy[14]:=notsy;
  rsy[15]:=thensy;	rsy[16]:=elsesy;	rsy[17]:=withsy;
  rsy[18]:=casesy;	rsy[19]:=typesy;	rsy[20]:=gotosy;
  rsy[21]:=filesy;	rsy[22]:=beginsy;	rsy[23]:=untilsy;
  rsy[24]:=whilesy;	rsy[25]:=arraysy;	rsy[26]:=constsy;
  rsy[27]:=labelsy;	rsy[28]:=repeatsy;	rsy[29]:=recordsy;
  rsy[30]:=downtosy;	rsy[31]:=packedsy;	rsy[32]:=progsy;
  rsy[33]:=funcsy;	rsy[34]:=procsy;
{indices into rw to find reserved words fast}
  frw[0]:= 0; frw[1]:= 0; frw[2]:= 6; frw[3]:=15; frw[4]:=22;
  frw[5]:=28; frw[6]:=32; frw[7]:=33; frw[8]:=35;
{char types}
  for c:=chr(0) to chr(maxcharord) do cs[c]:=others;
  for c:='0' to '9' do cs[c]:=digit;
  for c:='A' to 'Z' do cs[c]:=upper;
  for c:='a' to 'z' do cs[c]:=lower;
  for c:=chr(10) to chr(13) do cs[c]:=layout;	{lf,ht,ff,cr}
{characters with corresponding chartype in ASCII order}
  cs['	']:=tabch;	cs[' ']:=layout;	cs['"']:=dquotech;
  cs['''']:=quotech;
  cs['(']:=lparentch;	cs[')']:=rparentch;	cs['*']:=star;
  cs['+']:=plusch;	cs[',']:=commach;	cs['-']:=minch;
  cs['.']:=periodch;	cs['/']:=slash;		cs[':']:=colonch;
  cs[';']:=semich;	cs['<']:=lessch;	cs['=']:=equal;
  cs['>']:=greaterch;	cs['[']:=lbrackch;	cs[']']:=rbrackch;
  cs['^']:=arrowch;	cs['_']:=undscore;	cs['{']:=lbracech;
{single character symbols in chartype order}
  csy[rparentch]:=rparent;	csy[lbrackch]:=lbrack;
  csy[rbrackch]:=rbrack;	csy[commach]:=comma;
  csy[semich]:=semicolon;	csy[arrowch]:=arrow;
  csy[plusch]:=plussy;		csy[minch]:=minsy;
  csy[slash]:=slashsy;		csy[star]:=starsy;
  csy[equal]:=eqsy;
end;

procedure init2;
var s:sp; p,q:ip; k:idclass;
begin
{undefined identifier pointers used by searchid}
  for k:=types to func do
    undefip[k]:=newip(k,spaces,nil,nil);
{standard type pointers}
  new(s,scalar); s^.form:=scalar; s^.sflag:=[];
    s^.size:=intsize; s^.scalno:=0; s^.fconst:=nil; intptr:=s;
  new(s,scalar); s^.form:=scalar; s^.sflag:=[];
    s^.size:=realsize; s^.scalno:=0; s^.fconst:=nil; realptr:=s;
  new(s,scalar); s^.form:=scalar; s^.sflag:=[];
    s^.size:=charsize; s^.scalno:=0; charptr:=s;
  new(s,scalar); s^.form:=scalar; s^.sflag:=[];
    s^.size:=boolsize; s^.scalno:=0; boolptr:=s;
  new(s,pointer); s^.form:=pointer; s^.sflag:=[];
    s^.eltype:=nil; nilptr:=s;  {size set in handleopts}
  new(s,files); s^.form:=files; s^.sflag:=[withfile];
    s^.filtype:=charptr; textptr:=s;  {see handleopts}
  new(s,power); s^.form:=power; emptyset:=s; s^.size:=intsize;
    s^.elset:=nil; s^.sflag:=[];
  new(s,pointer); s^.form:=pointer; s^.sflag:=[];
    s^.eltype:=nil; stringptr:=s;  {see handleopts}
{standard type names}
  enterid(newip(types,'integer ',intptr,nil));
  enterid(newip(types,'real    ',realptr,nil));
  enterid(newip(types,'char    ',charptr,nil));
  enterid(newip(types,'boolean ',boolptr,nil));
  enterid(newip(types,'text    ',textptr,nil));
{standard constant names}
  q:=nil; p:=newip(konst,'false   ',boolptr,q); enterid(p);
  q:=p; p:=newip(konst,'true    ',boolptr,q); p^.value:=1; enterid(p);
  boolptr^.fconst:=p;
  p:=newip(konst,'maxint  ',intptr,nil); p^.value:=maxint; enterid(p);
  p:=newip(konst,spaces,charptr,nil); p^.value:=maxcharord;
  charptr^.fconst:=p;
end;

procedure init3;
var j:standpf; p:ip; q:np;
    pfn:array[standpf] of alpha;
    ftype:array[feof..farctan] of sp;
begin
{names of standard procedures/functions}
  pfn[pread	]:='read    ';	pfn[preadln	]:='readln  ';
  pfn[pwrite	]:='write   ';	pfn[pwriteln	]:='writeln ';
  pfn[pput	]:='put     ';	pfn[pget	]:='get     ';
  pfn[ppage	]:='page    ';	pfn[preset	]:='reset   ';
  pfn[prewrite	]:='rewrite ';	pfn[pnew	]:='new     ';
  pfn[pdispose	]:='dispose ';	pfn[ppack	]:='pack    ';
  pfn[punpack	]:='unpack  ';	pfn[pmark	]:='mark    ';
  pfn[prelease	]:='release ';	pfn[phalt	]:='halt    ';
  pfn[feof	]:='eof     ';	pfn[feoln	]:='eoln    ';
  pfn[fabs	]:='abs     ';	pfn[fsqr	]:='sqr     ';
  pfn[ford	]:='ord     ';	pfn[fchr	]:='chr     ';
  pfn[fpred	]:='pred    ';	pfn[fsucc	]:='succ    ';
  pfn[fodd	]:='odd     ';	pfn[ftrunc	]:='trunc   ';
  pfn[fround	]:='round   ';	pfn[fsin	]:='sin     ';
  pfn[fcos	]:='cos     ';	pfn[fexp	]:='exp     ';
  pfn[fsqrt	]:='sqrt    ';	pfn[fln		]:='ln      ';
  pfn[farctan	]:='arctan  ';
{parameter types of standard functions}
  ftype[feof	]:=nil;		ftype[feoln	]:=nil;
  ftype[fabs	]:=nil;		ftype[fsqr	]:=nil;
  ftype[ford	]:=nil;		ftype[fchr	]:=intptr;
  ftype[fpred	]:=nil;		ftype[fsucc	]:=nil;
  ftype[fodd	]:=intptr;	ftype[ftrunc	]:=realptr;
  ftype[fround	]:=realptr;	ftype[fsin	]:=realptr;
  ftype[fcos	]:=realptr;	ftype[fexp	]:=realptr;
  ftype[fsqrt	]:=realptr;	ftype[fln	]:=realptr;
  ftype[farctan	]:=realptr;	
{standard procedure/function identifiers}
  for j:=pread to phalt do
    begin new(p,proc,standard); p^.klass:=proc;
      p^.name:=pfn[j]; p^.pfkind:=standard; p^.key:=j; enterid(p);
    end;
  for j:=feof to farctan do
    begin new(p,func,standard); p^.klass:=func; p^.idtype:=ftype[j];
      {idtype is used not for result type but for parameter type !! }
      p^.name:=pfn[j]; p^.pfkind:=standard; p^.key:=j; enterid(p);
    end;
{program identifier}
  progp:=newip(proc,'_main   ',nil,nil);
{new name space for user externals}
  new(q,blck); q^.occur:=blck; q^.nlink:=top; q^.fname:=nil; top:=q;
end;

procedure init4;
var c:char;
begin
{pascal library mnemonics}
  lmn[OPN ]:='_opn';	lmn[GETX]:='_get';	lmn[RDI ]:='_rdi';
  lmn[RDC ]:='_rdc';	lmn[RDR ]:='_rdr';	lmn[RLN ]:='_rln';
  lmn[ELN ]:='_eln';	lmn[EFL ]:='_efl';
  lmn[CLS ]:='_cls';
  lmn[CRE ]:='_cre';	lmn[PUTX]:='_put';	lmn[WRI ]:='_wri';
  lmn[WSI ]:='_wsi';	lmn[WRC ]:='_wrc';	lmn[WSC ]:='_wsc';
  lmn[WRS ]:='_wrs';	lmn[WSS ]:='_wss';	lmn[WRB ]:='_wrb';
  lmn[WSB ]:='_wsb';	lmn[WRR ]:='_wrr';	lmn[WSR ]:='_wsr';
  lmn[WRF ]:='_wrf';	lmn[WRZ ]:='_wrz';	lmn[WSZ ]:='_wsz';
  lmn[WLN ]:='_wln';	lmn[PAG ]:='_pag';
  lmn[ABR ]:='_abr';	lmn[RND ]:='_rnd';	lmn[SIN ]:='_sin';
  lmn[COS ]:='_cos';	lmn[EXPX]:='_exp';	lmn[SQT ]:='_sqt';
  lmn[LOG ]:='_log';	lmn[ATN ]:='_atn';	lmn[ABI ]:='_abi';
  lmn[BCP ]:='_bcp';	lmn[BTS ]:='_bts';	lmn[NEWX]:='_new';
  lmn[SAV ]:='_sav';	lmn[RST ]:='_rst';	lmn[INI ]:='_ini';
  lmn[HLT ]:='_hlt';	lmn[ASS ]:='_ass';	lmn[GTO ]:='_gto';
  lmn[PAC ]:='_pac';	lmn[UNP ]:='_unp';	lmn[DIS ]:='_dis';
{opts}
  for c:='a' to 'z' do begin opt[c]:=0; forceopt[c]:=false end;
  opt['a']:=on;
  opt['i']:=maxsetint+1;
  opt['l']:=on;
  opt['o']:=on;
  opt['r']:=on;
  sopt:=off;
{scalar variables}
  b.nextbp:=nil;
  b.lc:=intsize;  {word 0 for line numbers}
  b.ilbno:=0;
  b.forwcount:=0;
  b.lchain:=nil;
  lino:=0;
  dlbno:=0;
  argc:=1;
  lastpfno:=0;
  giveline:=true;
  eofexpected:=false;
  intypedec:=false;
  fltused:=false;
  seconddot:=false;
  iop[false]:=nil;
  iop[true]:=nil;
end;

procedure handleopts;
begin
  copt:=opt['c'];
  iopt:=opt['i'];
  sopt:=opt['s'];
  vopt:=opt['v'];
  ptrsize:=2; if vopt<>off then ptrsize:=ptrsize+2;
  fhsize:=6*intsize + 2*ptrsize;
  nilptr^.size:=ptrsize; textptr^.size:=fhsize+buffersize;
  stringptr^.size:=ptrsize;
  progp^.headlc:=intsize+ptrsize;
  if sopt<>off then begin cs['_']:=others; copt:=off end;
  if copt<>off then enterid(newip(types,'string  ',stringptr,nil));
  if opt['o']=off then gen1(ps_mes,1);
  if vopt<>off then gen1(ps_mes,2);
end;

{===================================================================}

procedure trace(tname:alpha; fip:ip; var namdlb:integer);
var i:integer;
begin
  if opt['t']<>off then
    begin
      if namdlb=0 then
	begin dlbno:=dlbno+1; namdlb:=dlbno; gendlb(dlbno);
	  gen0(ps_con); write(em1,sp_scon,8);
	  for i:=1 to 8 do write(em1,ord(fip^.name[i])); genend;
	end;
      gen1(op_mrk,0); gend(op_lae,namdlb); gen0(op_cal); genident(sp_pnam,tname);
    end;
end;

function formof(fsp:sp; forms:formset):boolean;
begin if fsp=nil then formof:=false else formof:=fsp^.form in forms end;

function sizeof(fsp:sp):integer;
var s:integer;
begin s:=0;
  if fsp<>nil then s:=fsp^.size;
  if s<>1 then if odd(s) then s:=s+1;
  sizeof:=s
end;

function even(i:integer):integer;
begin if odd(i) then i:=i+1; even:=i end;

procedure exchange(l1,l2:integer);
var d1,d2:integer;
begin d1:=l2-l1; d2:=lino-l2;
  if (d1<>0) and (d2<>0) then
    begin gen1(ps_exc,d1); gencst(d2) end
end;

procedure setop(m:byte);
begin gen1(m,even(sizeof(a.asp))) end;

procedure expandemptyset(fsp:sp);
var i:integer;
begin for i:=2 to sizeof(fsp) div 2 do gen1(op_loc,0); a.asp:=fsp end;

procedure push(local:boolean; ad:integer; sz:integer);
begin assert not odd(sz);
  if sz>2 then
    begin if local then gen1(op_lal,ad) else gen1(op_lae,ad);
      gen1(op_loi,sz)
    end
  else
    if local then gen1(op_lol,ad) else gen1(op_loe,ad)
end;

procedure pop(local:boolean; ad:integer; sz:integer);
begin assert not odd(sz);
  if sz>2 then
    begin if local then gen1(op_lal,ad) else gen1(op_lae,ad);
      gen1(op_sti,sz)
    end
  else
    if local then gen1(op_stl,ad) else gen1(op_ste,ad)
end;

procedure lexical(m:byte; lv:integer; ad:integer; sz:integer);
begin gen1(op_lex,level-lv); gen1(op_adi,ad); gen1(m,sz) end;

procedure loadaddr;
begin with a do
  case ac of
    direct: with pos do
      begin
	if lv<=0 then
	  if sg=0 then gen1(op_lae,ad)
	  else begin gen1(op_lsa,sg); gen1(op_adi,ad) end
	else
	  if lv=level then gen1(op_lal,ad)
	  else begin gen1(op_lex,level-lv); gen1(op_adi,ad) end;
	ac:=indirect; pk:=false
      end;
    indirect: ;
    indexed:
      begin gend(op_aar,arno); pk:=arpk; ac:=indirect end;
  end;  {case}
end;

procedure load;
var s1,s2:integer;
begin with a do begin s1:=sizeof(asp); s2:=even(s1);
  if asp<>nil then
    case ac of
      cst: gen1(op_loc,intval);  {only non-real scalars}
      direct:
	with pos do
	  if lv<=0 then
	    if sg=0 then push(global,ad,s2)
	    else begin loadaddr; load end
	  else
	    if lv=level then push(local,ad,s2)
	    else lexical(op_loi,lv,ad,s2);
      indirect:
	if pk then gen1(op_loi,s1)
	else gen1(op_loi,s2);
      indexed: gend(op_lar,arno);
      expr:  {value already on stack}
    end;  {case}
  ac:=expr;
end end;

procedure store;
var s1,s2:integer;
begin with a do begin s1:=sizeof(asp); s2:=even(s1);
  if asp<>nil then
    case ac of
      direct:
	with pos do
	  if lv<=0 then
	    if sg=0 then pop(global,ad,s2)
	    else begin loadaddr; store end
	  else
	    if level=lv then pop(local,ad,s2)
	    else lexical(op_sti,lv,ad,s2);
      indirect:
	if pk then gen1(op_sti,s1) else gen1(op_sti,s2);
      indexed: gend(op_sar,arno);
    end;  {case}
end end;

procedure loadcheap;
begin if formof(a.asp,[arrays,records]) then loadaddr else load end;

{===================================================================}

procedure nextch;
begin
  if eoln(input) then
    begin readln(input);
      if eof(input) then
        begin
          if not eofexpected then error(+03) else
	    begin if fltused then gen1(ps_mes,15); gen0(ps_eof) end;
#ifdef STANDARD
	    goto 9999
#endif
#ifndef STANDARD
	    halt
#endif
	end;
     chcnt:=0; linecount:=linecount+1; giveline:=true;
    end
  else
    get(input);
  ch:=input^; chcnt:=chcnt+1; chsy:=cs[ch];
end;

procedure options(normal:boolean);
var c,ci:char; i:integer;

procedure getc;
var b:byte;
begin
  if normal then
    begin nextch; c:=ch end
  else
    begin read(em1,b); c:=chr(b) end
end;

begin
  repeat getc;
    if (c>='a') and (c<='z') then
      begin ci:=c; getc; i:=0;
	if c='+' then begin i:=1; getc end else
	if c='-' then getc else
	if cs[c]=digit then
	  repeat i:=i*10 + ord(c) - ord('0'); getc;
	  until cs[c]<>digit
	else i:=-1;
	if i>=0 then
	  if not normal then
	    begin forceopt[ci]:=true; opt[ci]:=i end
	  else
	    if not forceopt[ci] then opt[ci]:=i;
      end;
  until c<>',';
end;

procedure putdig;
begin ix:=ix+1; if ix<=rmax then strbuf[ix]:=ch; nextch end;

procedure inident;
label 1;
var i,k:integer;
begin k:=0; id:=spaces;
  repeat
    if chsy=upper then ch:=chr(ord(ch)-ord('A')+ord('a'));
    if k<idmax then begin k:=k+1; id[k]:=ch end;
    nextch
  until chsy>digit;
	{undscore=0,lower=1,upper=2,digit=3; ugly but fast}
  for i:=frw[k-1] to frw[k] - 1 do
    if rw[i]=id then
      begin sy:=rsy[i]; goto 1 end;
  sy:=ident;
1:
end;

procedure innumber;
label 1;
var i:integer;
begin ix:=0; sy:=intcst; val:=0;
  repeat putdig until chsy<>digit;
  if (ch='.') or (ch='e') or (ch='E') then
    begin
      if ch='.' then
        begin putdig;
	  if ch='.' then
	    begin seconddot:=true; ix:=ix-1; goto 1 end;
	  if chsy<>digit then error(+04) else
	    repeat putdig until chsy<>digit;
        end;
      if (ch='e') or (ch='E') then
        begin putdig;
	  if (ch='+') or (ch='-') then putdig;
	  if chsy<>digit then error(+05) else
	    repeat putdig until chsy<>digit;
        end;
      if ix>rmax then begin error(+06); ix:=rmax end;
      sy:=realcst; dlbno:=dlbno+1; val:=dlbno;
      gendlb(dlbno); gen0(ps_con); write(em1,sp_rcon,ix);
      for i:=1 to ix do write(em1,ord(strbuf[i])); genend;
    end;
1:if (chsy=lower) or (chsy=upper) then teststandard;
  if sy=intcst then
    if ix>rmax then error(+07) else
      for i:=1 to ix do
	if val<(maxint div 10) then
	  val:=val*10 - ord('0') + ord(strbuf[i]) else
	if (val = (maxint div 10)) and
	      (strbuf[i] <= chr(maxint mod 10 + ord('0'))) then
	  val:=(maxint div 10)*10 - ord('0') + ord(strbuf[i])
	else begin error(+08); val:=0 end
end;

procedure instring(qc:char);
var i:integer;
begin ix:=0; zerostring:=qc='"';
  repeat
    repeat nextch; ix:=ix+1; if ix<=smax then strbuf[ix]:=ch;
    until eoln(input) or (ch=qc);
    if ch<>qc then error(+09) else nextch;
  until ch<>qc;
  if not zerostring then
    begin ix:=ix-1; if ix=0 then error(+010) end
  else
    begin strbuf[ix]:=chr(0); if copt=off then error(+011) end;
  if (ix=1) and not zerostring then
    begin sy:=charcst; val:=ord(strbuf[1]) end
  else
    begin sy:=stringcst; dlbno:=dlbno+1; val:=dlbno;
      if ix>smax then begin error(+012); ix:=smax end;
      gendlb(dlbno); gen0(ps_con); write(em1,sp_scon,ix);
      for i:=1 to ix do write(em1,ord(strbuf[i])); genend;
    end
end;

procedure insym;
  {read next basic symbol of source program and return its
  description in the global variables sy, op, id, val and ix}
label 1;
var nest:integer;
begin
1:case chsy of
    tabch:
      begin chcnt:=chcnt - chcnt mod 8 + 8; nextch; goto 1 end;
    layout: begin nextch; goto 1 end;
    lower,upper: inident;
    digit: innumber;
    quotech: instring('''');
    dquotech: instring('"');
    colonch:
      begin nextch;
	if ch='=' then begin sy:=becomes; nextch end else sy:=colon1
      end;
    periodch:
      begin nextch;
	if seconddot then begin seconddot:=false; sy:=colon2 end else
	if ch='.' then begin sy:=colon2; nextch end else sy:=period
      end;
    lessch:
      begin nextch;
	if ch='=' then begin sy:=lesy; nextch end else
	if ch='>' then begin sy:=nesy; nextch end else sy:=ltsy
      end;
    greaterch:
      begin nextch;
	if ch='=' then begin sy:=gesy; nextch end else sy:=gtsy
      end;
    lparentch:
      begin nextch;
	if ch<>'*' then sy:=lparent else
	  begin nextch; teststandard; if ch='$' then options(true);
	    repeat
	      while ch<>'*' do nextch; nextch;
	    until ch=')';
	    nextch; goto 1
	  end;
      end;
    lbracech:
      begin nextch; if ch='$' then options(true); nest:=1;
	while nest>0 do
	  begin
	    if (ch='{') and (sopt<>off) then nest:=nest+1;
	    if ch='}' then nest:=nest-1;
	    nextch;
	  end;
	goto 1;
      end;
    rparentch,lbrackch,rbrackch,commach,semich,arrowch,
    plusch,minch,slash,star,equal:
      begin sy:=csy[chsy]; nextch end;
    undscore,others:
      begin error(+013); nextch; goto 1 end;
  end {case}
end;

procedure nextif(fsy:symbol; err:integer);
begin if sy=fsy then insym else error(-err) end;

function find1(sys1,sys2:sos; err:integer):boolean;
{symbol of sys1 expected; return true if sy in sys1}
begin
  if not (sy in sys1) then
    begin error(err); while not (sy in sys1+sys2) do insym end;
  find1:=sy in sys1
end;

function find2(sys1,sys2:sos; err:integer):boolean;
{symbol of sys1+sys2 expected; return true if sy in sys1}
begin
  if not (sy in sys1+sys2) then
    begin error(err); repeat insym until sy in sys1+sys2 end;
  find2:=sy in sys1
end;

function find3(sy1:symbol; sys2:sos; err:integer):boolean;
{symbol sy1 or one of sys2 expected; return true if sy1 found and skip it}
begin find3:=true;
  if not (sy in [sy1]+sys2) then
    begin error(err); repeat insym until sy in [sy1]+sys2 end;
  if sy=sy1 then insym else find3:=false
end;

function endofloop(sys1,sys2:sos; sy3:symbol; err:integer):boolean;
begin endofloop:=false;
  if find2(sys2+[sy3],sys1,err) then nextif(sy3,err+1)
  else endofloop:=true;
end;

function lastsemicolon(sys1,sys2:sos; err:integer):boolean;
begin lastsemicolon:=true;
  if not endofloop(sys1,sys2,semicolon,err) then
    if find2(sys2,sys1,err+2) then lastsemicolon:=false
end;

{===================================================================}

function searchid(fidcls: setofids; prterr:boolean):ip;
{search for current identifier symbol in the name table}
label 1;
var lip:ip; ic:idclass;
begin lastnp:=top;
  while lastnp<>nil do
    begin lip:=lastnp^.fname;
      while lip<>nil do
	if lip^.name=id then
	  if lip^.klass in fidcls then goto 1 else lip:=lip^.rlink
	else
	  if lip^.name< id then lip:=lip^.rlink else lip:=lip^.llink;
      lastnp:=lastnp^.nlink;
    end;
  {search not successfull; suppress error message in case
   of forward referenced type id
   -->procedure varpart
   -->procedure simpletype}
  if prterr then
    begin errid(+014,id);
      if types in fidcls then ic:=types else
      if vars  in fidcls then ic:=vars else
      if konst in fidcls then ic:=konst else
      if proc  in fidcls then ic:=proc else
      if func  in fidcls then ic:=func else ic:=field;
      lip:=undefip[ic];
    end;
1:  searchid:=lip
end;

function searchsection(fip: ip):ip;
{to find record fields and forward declared procedure id's
  -->procedure pfdeclaration
  -->procedure selector}
label 1;
begin
  while fip<>nil do
    if fip^.name=id then goto 1 else
      if fip^.name< id then fip:=fip^.rlink else fip:=fip^.llink;
1:  searchsection:=fip
end;

function searchlab(flp:lp; val:integer):lp;
label 1;
begin
  while flp<>nil do
    if flp^.labval=val then goto 1 else flp:=flp^.nextlp;
1:searchlab:=flp
end;

procedure negatereal(l1:integer);
var l2:integer;
begin l2:=lino; gen1(op_loc,real1); gen1(op_loc,real2);
  exchange(l1,l2); gen0(op_fsb); fltused:=true
end;

function desub(fsp:sp):sp;
begin if formof(fsp,[subrange]) then fsp:=fsp^.rangetype; desub:=fsp end;

function nicescalar(fsp:sp):boolean;
begin
  if fsp=nil then nicescalar:=true else
    nicescalar:=(fsp^.form=scalar) and (fsp<>realptr)
end;

function bounds(fsp:sp; var fmin,fmax:integer):boolean;
{compute bounds if possible, else return false}
begin bounds:=false; fmin:=0; fmax:=0;
  if fsp<>nil then
    if fsp^.form=subrange then
      begin fmin:=fsp^.min; fmax:=fsp^.max; bounds:=true end else
    if fsp^.form=scalar then
      if fsp^.fconst<>nil then
	begin fmin:=0; fmax:=fsp^.fconst^.value; bounds:=true end
end;

procedure checkbnds(fsp:sp);
var sno,min1,max1,min2,max2:integer;
begin
  if (opt['r']<>off) then if bounds(fsp,min1,max1) then
    if (not bounds(a.asp,min2,max2)) or
		(min2<min1) or (max2>max1) then
      begin
	if fsp^.form=scalar then sno:=fsp^.scalno else
	  sno:=fsp^.subrno;
	if sno=0 then
	  begin dlbno:=dlbno+1; sno:=dlbno;
	    gendlb(dlbno); gen1(ps_rom,min1); gencst(max1); genend;
	    if fsp^.form=scalar then fsp^.scalno:=sno else
	      fsp^.subrno:=sno
	  end;
	gend(op_rck,sno);
      end;
  a.asp:=fsp;
end;

function eqstruct(p,q:sp):boolean;
begin eqstruct:=(p=q) or (p=nil) or (q=nil) end;

function string(fsp:sp):boolean;
var lsp:sp; min,max:integer;
begin string:=false;
  if formof(fsp,[arrays]) then
    if eqstruct(fsp^.aeltype,charptr) then
      begin lsp:=fsp^.inxtype; string:=spack in fsp^.sflag;
	if lsp<>nil then if bounds(lsp,min,max) then
	  if min<>1 then string:=false;
      end
end;

function compat(p,q:sp):twostruct;
begin compat:=noteq;
  if eqstruct(p,q) then compat:=eq else
    begin p:=desub(p); q:=desub(q);
      if eqstruct(p,q) then compat:=subeq else
      if p^.form=q^.form then
	case p^.form of
	  scalar:
	    if (p=intptr) and (q=realptr) then compat:=ir else
	    if (p=realptr) and (q=intptr) then compat:=ri;
	  pointer:
	    if (p=nilptr) or (q=nilptr) then compat:=eq;
	  power:
	    if p=emptyset then compat:=es else
	    if q=emptyset then compat:=se else
	    if compat(p^.elset,q^.elset) <= subeq then compat:=eq;
	  arrays:
	    if string(p) and string(q) and (p^.size=q^.size) then compat:=eq;
	  files,records: ;
	end;
    end
end;

procedure checkasp(fsp:sp; err:integer);
begin
  case compat(fsp,a.asp) of
    eq: if fsp<>nil then if withfile in fsp^.sflag then
	  begin error(err); a.asp:=nil end;
    subeq: checkbnds(fsp);
    ri: begin gen0(op_cif); fltused:=true; a.asp:=realptr end;
    se: expandemptyset(fsp);
    noteq,ir,es: begin error(err); a.asp:=nil end;
  end
end;

procedure force(fsp:sp; err:integer);
begin load; checkasp(fsp,err) end;

function stringstruct:sp;
var lsp:sp;
begin {only used when ix and zerostring are still valid}
  if zerostring then lsp:=stringptr else
    begin new(lsp); lsp^.form:=arrays; lsp^.sflag:=[spack];
      lsp^.aeltype:=charptr; lsp^.inxtype:=nil; lsp^.size:=ix*charsize;
    end;
  stringstruct:=lsp;
end;

function address(var lc:integer; sz:integer; pack:boolean):integer;
begin
  if lc >= maxint-sz then begin error(+015); lc:=0 end;
  if (not pack) or (sz>1) then if odd(lc) then lc:=lc+1;
  address:=lc;
  lc:=lc+sz
end;

function reserve(s:integer):integer;
begin reserve:=address(b.lc,s,false);
  if b.lc>lcmax then lcmax:=b.lc
end;

function arraysize(fsp:sp; pack:boolean):integer;
var sz,min,max,tot,n:integer;
begin sz:=sizeof(fsp^.aeltype);
  if not pack then sz:=even(sz);
  if bounds(fsp^.inxtype,min,max) then;  {we checked before}
  dlbno:=dlbno+1; fsp^.arrno:=dlbno;
  gendlb(dlbno); gen1(ps_rom,min); gencst(max-min);
  gencst(sz); genend;
  n:=max-min+1; tot:=sz*n;
  if sz<>0 then if tot div sz <> n then begin error(+016); tot:=0 end;
  arraysize:=tot
end;

procedure treewalk(fip:ip);
var lsp:sp; i:integer;
begin
  if fip<>nil then
    begin treewalk(fip^.llink); treewalk(fip^.rlink);
      if fip^.klass=vars then
	begin if not (used in fip^.vflag) then errid(-(+017),fip^.name);
	  if not (assigned in fip^.vflag) then errid(-(+018),fip^.name);
	  lsp:=fip^.idtype;
	  if lsp<>nil then if withfile in lsp^.sflag then
	    if lsp^.form=files then
	      if level=1 then
		begin
		  for i:=2 to argc do
		    if argv[i].name=fip^.name then
		      argv[i].ad:=fip^.vpos.ad
		end
	      else
		begin gen1(op_mrk,0); gen1(op_lal,fip^.vpos.ad); gensp(CLS) end
	    else
	      if level<>1 then errid(-(+019),fip^.name)
	end
    end
end;

procedure constant(fsys:sos; var fsp:sp; var fval:integer);
var signed,min:boolean; lip:ip;
begin signed:=(sy=plussy) or (sy=minsy);
  if signed then begin min:=sy=minsy; insym end else min:=false;
  if find1([ident..nilcst],fsys,+020) then
    begin fval:=val;
      case sy of
	stringcst: fsp:=stringstruct;
	charcst: fsp:=charptr;
	intcst: fsp:=intptr;
	realcst: fsp:=realptr;
	nilcst: fsp:=nilptr;
	ident:
	  begin lip:=searchid([konst],true);
	    fsp:=lip^.idtype; fval:=lip^.value;
	  end
      end;  {case}
      if signed then
	if (fsp<>intptr) and (fsp<>realptr) then error(+021) else
	  if min then fval:= -fval;
		{note: negating the v-number for reals}
      insym;
    end
  else begin fsp:=nil; fval:=0 end;
end;

function cstinteger(fsys:sos; fsp:sp; err:integer):integer;
var lsp:sp; lval,min,max:integer;
begin constant(fsys,lsp,lval);
  if fsp<>lsp then
    if eqstruct(desub(fsp),lsp) then
      begin
	if bounds(fsp,min,max) then
	  if (lval<min) or (lval>max) then error(+022)
      end
    else
      begin error(err); lval:=0 end;
  cstinteger:=lval
end;

{===================================================================}

function simpletype(fsys:sos):sp;
var lsp,lsp1:sp; lip,hip:ip; min,max:integer; lnp:np;
    newsubrange:boolean;
begin
  if find1([ident..lparent],fsys,+023) then
    if sy=lparent then
      begin insym; lnp:=top;   {decl. consts local to innermost block}
	while top^.occur<>blck do top:=top^.nlink;
	new(lsp,scalar); lsp^.form:=scalar; lsp^.sflag:=[];
	lsp^.scalno:=0; hip:=nil; max:=0;
	repeat
	  if sy<>ident then error(+024) else
	    begin lip:=newip(konst,id,lsp,hip); enterid(lip); insym;
	      hip:=lip; lip^.value:=max; max:=max+1;
	    end;
	until endofloop(fsys+[rparent],[ident],comma,+025);  {+026}
	if max<=256 then lsp^.size:=charsize else lsp^.size:=intsize;
	lsp^.fconst:=hip; top:=lnp; nextif(rparent,+027);
      end
    else
      begin newsubrange:=true;
	if sy=ident then
	  begin lip:=searchid([types,konst],true); insym;
	    if lip^.klass=types then
	      begin lsp:=lip^.idtype; newsubrange:=false end
	    else
	      begin lsp1:=lip^.idtype; min:=lip^.value end
	  end
	else constant(fsys+[colon2,ident..plussy],lsp1,min);
	if newsubrange then
	  begin new(lsp,subrange); lsp^.form:=subrange; lsp^.sflag:=[];
	    lsp^.subrno:=0;
	    if not nicescalar(lsp1) then
	      begin error(+028); lsp1:=nil; min:=0 end;
	    lsp^.rangetype:=lsp1;
	    nextif(colon2,+029); max:=cstinteger(fsys,lsp1,+030);
	    if min>max then begin error(+031); max:=min end;
	    if (min>=0) and (max<256) then lsp^.size:=charsize
	    else lsp^.size:=intsize;
	    lsp^.min:=min; lsp^.max:=max
	  end
      end
  else lsp:=nil;
  simpletype:=lsp
end;

function typ(fsys: sos):sp;
var lsp,hsp,lsp1:sp; lip:ip; oc,sz,min,max,maxbits:integer;
    lflag:sflagset; lnp:np;

function fldlist(fsys:sos):sp;
	{level 2; <<  type}
var fip,hip,lip:ip; lsp:sp;

function varpart(fsys:sos):sp;
	{level 3; <<  fldlist <<  type}
var tip,lip:ip; lsp,headsp,hsp,vsp,tsp,tsp1,tfsp:sp;
    minoc,maxoc,int:integer;
begin insym; tip:=nil; lip:=nil;
  new(tsp,tag); tsp^.form:=tag; tsp^.sflag:=[];
  if sy<>ident then error(+032) else
    begin lip:=searchid([types],false);
      if lip=nil then
	begin tip:=newip(field,id,nil,nil); enterid(tip); insym;
	  nextif(colon1,+033);
	  if sy<>ident then error(+034) else lip:=searchid([types],true)
	end;
    end;
  if lip=nil then tfsp:=nil else
    begin insym; tfsp:=lip^.idtype end;
  tsp^.tfldsp:=tfsp;
  if not nicescalar(desub(tfsp)) then error(+035);
  if tip<>nil then  {explicit tag}
    begin tip^.idtype:=tfsp;
      tip^.foffset:=address(oc,sizeof(tfsp),spack in lflag)
    end;
  nextif(ofsy,+036); minoc:=oc; maxoc:=minoc; headsp:=nil;
  repeat hsp:=nil;  {for each caselabel list}
    repeat
      int:=cstinteger(fsys+[ident..plussy,comma,colon1,lparent,
		    semicolon,casesy,rparent],tfsp,+037);
      lsp:=headsp;  {each label may occur only once}
      while lsp<>nil do
	begin if lsp^.varval=int then error(+038);
	  lsp:=lsp^.nxtvar
	end;
      new(vsp,variant); vsp^.form:=variant; vsp^.varval:=int;
      vsp^.sflag:=[];
      vsp^.nxtvar:=headsp; headsp:=vsp;  {chain of case labels}
      vsp^.subtsp:=hsp; hsp:=vsp;
	    {use this field to link labels with same variant}
    until endofloop(fsys+[colon1,lparent,semicolon,casesy,rparent],
		    [ident..plussy],comma,+039);  {+040}
    nextif(colon1,+041); nextif(lparent,+042);
    tsp1:=fldlist(fsys+[rparent,semicolon,ident..plussy]);
    if oc>maxoc then maxoc:=oc;
    while vsp<>nil do
      begin vsp^.size:=oc; hsp:=vsp^.subtsp;
	vsp^.subtsp:=tsp1; vsp:=hsp
      end;
    nextif(rparent,+043);
    oc:=minoc;
  until lastsemicolon(fsys,[ident..plussy],+044);  {+045 +046}
  tsp^.fstvar:=headsp; tsp^.size:=minoc; oc:=maxoc; varpart:=tsp;
end;

{function fldlist(fsys:sos):sp; }
{var fip,hip,lip:ip; lsp:sp;}
begin
  if find2([ident],fsys+[casesy],+047) then
    repeat lip:=nil; hip:=nil;
      repeat
	if sy<>ident then error(+048) else
	  begin fip:=newip(field,id,nil,nil); enterid(fip); insym;
	    if lip=nil then hip:=fip else lip^.next:=fip; lip:=fip;
	  end;
      until endofloop(fsys+[colon1,ident..packedsy,semicolon,casesy],
			      [ident],comma,+049);  {+050}
      nextif(colon1,+051);
      lsp:=typ(fsys+[casesy,semicolon]);
      if lsp<>nil then if withfile in lsp^.sflag then
	lflag:=lflag+[withfile];
      while hip<>nil do
	begin hip^.idtype:=lsp;
	  hip^.foffset:=address(oc,sizeof(lsp),spack in lflag);
	  hip:=hip^.next
	end;
    until lastsemicolon(fsys+[casesy],[ident],+052);  {+053 +054}
  if sy=casesy then fldlist:=varpart(fsys) else fldlist:=nil;
end;

{function typ(fsys:sos):sp;}
{var lsp,hsp,lsp1:sp; lip:ip; oc,sz,min,max,maxbits:integer;}
{    lflag:sflagset; lnp:np;}
begin lflag:=[];
  if sy=packedsy then begin lflag:=[spack]; insym end;
  if find1([ident..filesy],fsys,+055) then
    if sy in [ident..arrow] then
      begin if spack in lflag then error(+056);
	if sy=arrow then
	  begin new(lsp,pointer); lsp^.form:=pointer; lsp^.sflag:=[];
	    lsp^.size:=ptrsize; lsp^.eltype:=nil; insym;
	    if sy<>ident then error(+057) else
	      begin lip:=searchid([types],false);
		if lip<>nil then lsp^.eltype:=lip^.idtype else
		if not intypedec then error(+058) else
		  fwptr:=newip(types,id,lsp,fwptr);
		insym
	      end
	  end
	else lsp:=simpletype(fsys);
      end
    else case sy of
{<<<<<<<<<<<<}
arraysy:
  begin insym; hsp:=nil; nextif(lbrack,+059);
    repeat new(lsp,arrays); lsp^.form:=arrays; lsp^.sflag:=lflag;
      lsp^.aeltype:=hsp; hsp:=lsp;  {link in reversed order}
      lsp1:=simpletype(fsys+[comma,rbrack,ofsy,ident..packedsy]);
      if not bounds(lsp1,min,max) then
	begin error(+060); lsp1:=nil end;
      lsp^.inxtype:=lsp1;
    until endofloop(fsys+[rbrack,ofsy,ident..packedsy],[ident..lparent],
			comma,+061);  {+062}
    nextif(rbrack,+063); nextif(ofsy,+064);
    lsp:=typ(fsys);
    if lsp<>nil then if withfile in lsp^.sflag then
      lflag:=lflag+[withfile];
    repeat  {reverse pointers and compute size}
      lsp1:=hsp^.aeltype; hsp^.aeltype:=lsp;
      hsp^.size:=arraysize(hsp,spack in lflag); lsp:=hsp; hsp:=lsp1;
    until hsp=nil;  {lsp points to array with highest dimension now}
  end;
recordsy:
  begin insym;
    new(lnp,rec); lnp^.occur:=rec; lnp^.nlink:=top; lnp^.fname:=nil;
    top:=lnp;
    new(lsp,records); lsp^.form:=records; oc:=0;  {offset counter}
    lsp^.tagsp:=fldlist(fsys+[endsy]);
    lsp^.fstfld:=top^.fname; lsp^.size:=oc; lsp^.sflag:=lflag;
    top:=top^.nlink; nextif(endsy,+065)
  end;
setsy:
  begin insym; nextif(ofsy,+066); lsp1:=simpletype(fsys);
    maxbits:=maxsetsz*bytebits;
    if bounds(lsp1,min,max) then lsp1:=desub(lsp1) else
      if lsp1=intptr then
	begin error(-(+067)); max:=iopt-1 end
      else
	begin error(+068); lsp1:=nil end;
    if lsp1=intptr then maxbits:=iopt else
      if bounds(lsp1,min,max) then;  {compute bounds only}
    if (min<0) or (max>=maxbits) then
      begin error(+069); lsp1:=nil; max:=0 end;
    new(lsp,power); lsp^.form:=power; lsp^.elset:=lsp1;
    lsp^.size:=max div bytebits + 1; lsp^.sflag:=[];
  end;
filesy:
  begin insym; nextif(ofsy,+070);
    new(lsp,files); lsp^.form:=files; lsp1:=typ(fsys);
    lsp^.sflag:=[withfile]; lsp^.filtype:=lsp1; sz:=sizeof(lsp1);
    if sz<buffersize then sz:=buffersize; lsp^.size:=sz+fhsize;
  end;
{>>>>>>>>>>>>}
      end  {case}
  else lsp:=nil;
  typ:=lsp;
end;

{===================================================================}

procedure block(fsys:sos; fip:ip); forward;
	{pfdeclaration calls block; with a more obscure lexical
	  structure this forward declaration can be avoided}

procedure labeldeclaration(fsys:sos);
var llp:lp;
begin with b do begin
  repeat
    if sy<>intcst then error(+071) else
      if searchlab(lchain,val)<>nil then errint(+072,val) else
	begin new(llp); llp^.labval:=val;
	  if val>9999 then teststandard;
	  ilbno:=ilbno+1; llp^.labname:=ilbno; llp^.labdlb:=0;
	  llp^.seen:=false; llp^.nextlp:=lchain; lchain:=llp;
	  insym
	end;
  until endofloop(fsys+[semicolon],[intcst],comma,+073);  {+074}
  nextif(semicolon,+075)
end end;

procedure constdeclaration(fsys:sos);
var lip:ip;
begin
  repeat
    if sy<>ident then error(+076) else
      begin lip:=newip(konst,id,nil,nil); insym;
	nextif(eqsy,+077);
	constant(fsys+[semicolon,ident],lip^.idtype,lip^.value);
	nextif(semicolon,+078); enterid(lip);
      end;
  until not find2([ident],fsys,+079);
end;

procedure typedeclaration(fsys:sos);
var lip,hip,lip2:ip;
begin fwptr:=nil; intypedec:=true;
  repeat
    if sy<>ident then error(+080) else
      begin lip:=newip(types,id,nil,nil); insym;
	nextif(eqsy,+081);
	lip^.idtype:=typ(fsys+[semicolon,ident]);
	nextif(semicolon,+082); enterid(lip);
	{has any forward reference been satisfied:}
	hip:=fwptr;
	while hip<>nil do
	  begin
	    if hip^.name=lip^.name then
	      begin hip^.idtype^.eltype:=lip^.idtype;
		if hip=fwptr then fwptr:=hip^.next else
		  lip2^.next:=hip^.next
	      end;
	    lip2:=hip; hip:=hip^.next
	  end;
      end;
  until not find2([ident],fsys,+083);
  while fwptr<>nil do
    begin errid(-(+084),fwptr^.name); fwptr:=fwptr^.next end;
  intypedec:=false;
end;

procedure vardeclaration(fsys:sos);
var lip,hip,vip:ip; lsp:sp;
begin with b do begin
  repeat hip:=nil; lip:=nil;
    repeat
      if sy<>ident then error(+085) else
	begin vip:=newip(vars,id,nil,nil); enterid(vip); insym; vip^.vflag:=[];
	  if lip=nil then hip:=vip else lip^.next:=vip; lip:=vip;
	end;
    until endofloop(fsys+[colon1,ident..packedsy],[ident],comma,+086);  {+087}
    nextif(colon1,+088);
    lsp:=typ(fsys+[semicolon,ident]);
    while hip<>nil do
      begin hip^.idtype:=lsp; hip^.vpos.ad:=address(lc,sizeof(lsp),false);
	hip:=hip^.next
      end;
    nextif(semicolon,+089);
  until not find2([ident],fsys,+090);
end end;

procedure pfhead(fsys:sos;var fip:ip;var again:boolean;param:boolean); forward;

function parlist(fsys:sos; var hlc:integer):ip;
var lastip,hip,lip,pip:ip; lsp:sp; lflag:vflagset; again:boolean;
    sz:integer;
begin parlist:=nil; lastip:=nil;
  repeat  {once for each formal-parameter-section}
    if find1([ident,varsy,procsy,funcsy],fsys+[semicolon],+091) then
      begin
	if (sy=procsy) or (sy=funcsy) then
	  begin
	    pfhead(fsys+[semicolon,ident,varsy,procsy,funcsy],hip,again,true);
	    hip^.pfpos.ad:=address(hlc,pdsize+ptrsize,false);
	    hip^.pfkind:=formal; lip:=hip;
	    top:=top^.nlink; level:=level-1
	  end
	else
	  begin hip:=nil; lip:=nil; lflag:=[assigned];
	    if sy=varsy then
	      begin lflag:=[refer,assigned,used]; insym end;
	    repeat  {once for each identifier}
	      if sy<>ident then error(+092) else
		begin pip:=newip(vars,id,nil,nil); enterid(pip); insym;
		  pip^.vflag:=lflag;
		  if lip=nil then hip:=pip else lip^.next:=pip; lip:=pip;
		end;
	    until endofloop(fsys+[semicolon,colon1],[ident],comma,+093); {+094}
	    nextif(colon1,+095);
	    lsp:=typ(fsys+[semicolon,ident,varsy,procsy,funcsy]);
	    if refer in lflag then sz:=ptrsize else sz:=sizeof(lsp);
	    pip:=hip;
	    while pip<>nil do
	      begin pip^.vpos.ad:=address(hlc,sz,false); pip^.idtype:=lsp;
		pip:=pip^.next
	      end;
	  end;
	if lastip=nil then parlist:=hip else lastip^.next:=hip; lastip:=lip;
      end;
  until endofloop(fsys,[ident,varsy,procsy,funcsy],semicolon,+096);  {+097}
end;

procedure pfhead; {fsys:sos;var fip:ip;var again:boolean;param:boolean}
var lip,lip2:ip; lsp:sp; lnp:np; kl:idclass;
begin lip:=nil; again:=false;
  if sy=procsy then kl:=proc else
    begin kl:=func; fsys:=fsys+[colon1,ident] end;
  insym;
  if sy<>ident then begin error(+098); id:=spaces end;
  if not param then lip:=searchsection(top^.fname);
  if lip<>nil then
    if (lip^.klass<>kl) or (lip^.pfkind<>forwrd) then errid(+099,id) else
      begin b.forwcount:=b.forwcount-1; again:=true end;
  if again then insym else
    begin lip:=newip(kl,id,nil,nil);
      if sy=ident then begin enterid(lip); insym end;
      lastpfno:=lastpfno+1; lip^.pfno:=lastpfno;
    end;
  level:=level+1;
  new(lnp,blck); lnp^.occur:=blck; lnp^.nlink:=top; top:=lnp;
  if again then lnp^.fname:=lip^.parhead else
    begin lnp^.fname:=nil;
      if find3(lparent,fsys,+0100) then
	begin lip^.parhead:=parlist(fsys+[rparent],lip^.headlc);
	  nextif(rparent,+0101)
	end;
    end;
  if (kl=func) and not again then
    begin nextif(colon1,+0102);
      if sy<>ident then error(+0103) else
	begin lip2:=searchid([types],true); insym;
	  lsp:=lip2^.idtype;
	  if formof(lsp,[power..tag]) then
	    begin error(+0104); lsp:=nil end;
	  lip^.idtype:=lsp;
	end
    end;
  fip:=lip;
end;

procedure pfdeclaration(fsys:sos);
var lip:ip; again:boolean; markp:^integer; lbp:bp;
begin with b do begin
  pfhead(fsys+[ident,semicolon,labelsy..beginsy],lip,again,false);
  nextif(semicolon,+0105);
  if find1([ident,labelsy..beginsy],fsys+[semicolon],+0106) then
    if sy=ident then
      if id='forward ' then
	begin insym;
	  if lip^.pfpos.lv>1 then genpnam(ps_fwp,lip);
	  if again then errid(+0107,lip^.name) else
	    begin lip^.pfkind:=forwrd; forwcount:=forwcount+1 end;
	end else
      if id='extern  ' then
	begin lip^.pfkind:=extrn; lip^.pfpos.lv:=1; insym; teststandard end
      else errid(+0108,id)
    else
      begin lip^.pfkind:=actual;
#ifndef STANDARD
	mark(markp);
#endif
	if not again then if lip^.pfpos.lv>1 then genpnam(ps_fwp,lip);
	new(lbp); lbp^:=b; nextbp:=lbp;
	lc:=address(lip^.headlc,0,false);  {align headlc}
	ilbno:=0; forwcount:=0; lchain:=nil;
	if lip^.idtype<>nil then
	  lip^.pfpos.ad:=address(lc,sizeof(lip^.idtype),false);
	block(fsys+[semicolon],lip);
	b:=nextbp^;
#ifndef STANDARD
	release(markp);
#endif
      end;
  if not main then eofexpected:=forwcount=0;
  nextif(semicolon,+0109);
  level:=level-1; top:=top^.nlink;
end end;

{===================================================================}

procedure expression(fsys:sos); forward;
	{this forward declaration cannot be avoided}

procedure selectarrayelement(fsys:sos);
var isp:sp; la:attr;
begin with a do begin
  repeat loadaddr; ac:=indexed;
    if asp<>nil then if asp^.form<>arrays then
      begin error(+0110); asp:=nil end;
    if asp<>nil then
      begin isp:=asp^.inxtype; arpk:=spack in asp^.sflag;
	arno:=asp^.arrno; asp:=asp^.aeltype
      end
    else begin arpk:=true; arno:=0; isp:=nil end;
    la:=a;
    expression(fsys+[comma]); force(desub(isp),+0111);
		{no range check}
    a:=la;
  until endofloop(fsys,[notsy..lparent],comma,+0112);  {+0113}
end end;

procedure selector(fsys: sos; fip:ip; lflag:vflagset);
{selector computes the address of any kind of variable;
  three possibilities:
  1.for direct accessable variables a contains offset and level,
  2.for indirect accessable variables,the address is on the stack.
  3.for array elements, the top of stack gives the index (one word);
    the address of the array is beneath it.
  If a.asp=nil then an error occurred else a.asp gives
  the type of the variable.
}
var lip:ip; lpk:boolean; lsp:sp;
begin with a do begin
  asp:=fip^.idtype; ac:=direct;
  case fip^.klass of
    vars:
      begin pos:=fip^.vpos;
	if refer in fip^.vflag then
	  begin asp:=nilptr; load; asp:=fip^.idtype;
	    ac:=indirect; pk:=false
	  end
      end;
    field:
      case lastnp^.occur of
	vrec:  {indirect or packed}
	  begin push(local,lastnp^.vdspl,ptrsize); ac:=indirect;
	    pk:=lastnp^.rcpk; gen1(op_adi,fip^.foffset)
	  end;
	crec:  {direct and not packed}
	  begin pos:=lastnp^.cpos; pos.ad:=pos.ad+fip^.foffset end;
      end;
    func:
      if fip^.pfkind=standard then
	begin error(+0114); asp:=nil end
      else
	begin pos:=fip^.pfpos; pos.lv:=pos.lv+1;
	  if fip^.pfkind<>actual then error(+0115);
	  if sy=arrow then error(+0116);
	end
  end;  {case}
  while find2([lbrack,period,arrow],fsys,+0117) do
    if sy=lbrack then
      begin insym; selectarrayelement(fsys+[rbrack,lbrack,period,arrow]);
	nextif(rbrack,+0118);
      end else
    if sy=period then
      begin insym;
	if asp<>nil then if asp^.form<>records then
	  begin error(+0119); asp:=nil end;
	if sy<>ident then error(+0120) else
	  begin
	    if asp<>nil then
	      begin lip:=searchsection(asp^.fstfld);
		if lip=nil then begin errid(+0121,id); asp:=nil end else
		  begin lpk:=spack in asp^.sflag; asp:=lip^.idtype;
		    if (ac=direct) and not lpk then
		      pos.ad:=pos.ad+lip^.foffset
		    else
		      begin loadaddr; pk:=lpk;
			gen1(op_adi,lip^.foffset)
		      end
		  end
	      end;
	    insym
	  end
      end
    else
      begin insym; lflag:=[used];
	if asp<>nil then
	  if asp=stringptr then error(+0122) else
	  if asp^.form=pointer then
	    begin load; ac:=indirect; asp:=asp^.eltype;
	      pk:=false;  {pointers always contain even addresses}
	    end else
	  if asp^.form=files then
	    begin lsp:=asp^.filtype; asp:=nilptr; load; asp:=lsp;
	      ac:=indirect; pk:=true;
	    end
	  else error(+0123);
      end;
  if fip^.klass=vars then fip^.vflag:=fip^.vflag+lflag else
  if fip^.klass=func then fip^.pflag:=fip^.pflag+lflag;
end end;

procedure variable(fsys:sos);
var lip: ip;
begin
  if sy=ident then
    begin lip:=searchid([vars,field],true); insym;
      selector(fsys,lip,[used,assigned])
    end
  else begin error(+0124); a.asp:=nil; a.ac:=direct end;
end;

{===================================================================}

function plistequal(p1,p2:ip):boolean;
var ok:boolean;
begin plistequal:=eqstruct(p1^.idtype,p2^.idtype);
  p1:=p1^.parhead; p2:=p2^.parhead;
  while (p1<>nil) and (p2<>nil) do
    begin
      if p1^.klass<>p2^.klass then ok:=false else
	if p1^.klass=vars then
	  ok:=((refer in p1^.vflag) = (refer in p2^.vflag)) and
			eqstruct(p1^.idtype,p2^.idtype)
	else
	  ok:=plistequal(p1,p2);
      if ok=false then plistequal:=false;
      p1:=p1^.next; p2:=p2^.next
    end;
end;

procedure callnonstandard(fsys:sos; lpar:boolean; fip:ip);
var nxt,lip:ip;
begin with a do begin nxt:=fip^.parhead;
  if fip^.pfkind<>formal then gen1(op_mrk,level-fip^.pfpos.lv) else
    begin {op_mrs updates the static link}
      lexical(op_loi,fip^.pfpos.lv,fip^.pfpos.ad+pdsize,ptrsize); gen0(op_mrs)
    end;
  if lpar then
    repeat
      if nxt=nil then
	begin error(+0125); expression(fsys); load end
      else
	begin
	  if nxt^.klass=vars then
	    if refer in nxt^.vflag then  {call by reference}
	      begin variable(fsys); loadaddr;
		if not eqstruct(asp,nxt^.idtype) then error(+0126);
		if pk then error(+0127);
	      end
	    else  {call by value}
	      begin expression(fsys); force(nxt^.idtype,+0128) end
	  else
	    if sy<>ident then error(+0129) else
	      begin lip:=searchid([nxt^.klass],true); insym;
		if not plistequal(nxt,lip) then error(+0130);
		case lip^.pfkind of
		  standard: error(+0131);
		  formal:
		    lexical(op_loi,lip^.pfpos.lv,lip^.pfpos.ad,pdsize+ptrsize);
		  actual,extrn,forwrd:
		    begin genpnam(op_loc,lip);
		      gen1(op_lex,level-lip^.pfpos.lv)
		    end
		end  {case}
	      end;
	  nxt:=nxt^.next
	end
    until not find3(comma,fsys,+0132);
  if fip^.pfkind<>formal then genpnam(op_cal,fip) else
    begin lexical(op_loi,fip^.pfpos.lv,fip^.pfpos.ad,pdsize); gen0(op_cas) end;
  if nxt<>nil then error(+0133); asp:=fip^.idtype;
end end;

procedure fileaddr;
var la:attr;
begin
  if fa.asp=nilptr then
    push(local,fa.pos.ad,ptrsize)	{load pointer to file}
  else				{load address of file}
    begin la:=a; a:=fa; loadaddr; a:=la end
end;

procedure callr(l1,l2:integer);
var la:attr;
begin with a do begin
  la:=a; asp:=desub(asp); ac:=expr; gen1(op_mrk,0); fileaddr;
  if asp=intptr then gensp(RDI) else
  if asp=charptr then gensp(RDC) else
  if asp=realptr then gensp(RDR) else
    if asp<>nil then begin error(+0134); asp:=nil end;
  if asp<>la.asp then checkbnds(la.asp);
  a:=la; exchange(l1,l2); store;
end end;

procedure callw(fsys:sos; l1,l2:integer);
var m:libmnem;
begin with a do begin gen1(op_mrk,0);
  fileaddr; exchange(l1,l2); asp:=desub(asp);
  if string(asp) then
    begin loadaddr; gen1(op_loc,asp^.size); m:=WRS end
  else
    begin m:=WRI; load;
      if asp<>intptr then
      if asp=charptr then m:=WRC else
      if asp=realptr then m:=WRR else
      if asp=boolptr then m:=WRB else
      if asp=stringptr then m:=WRZ else
	if asp<>nil then error(+0135);
    end;
  if find3(colon1,fsys,+0136) then
    begin expression(fsys+[colon1]); force(intptr,+0137); m:=succ(m) end;
  if find3(colon1,fsys,+0138) then
    begin expression(fsys); force(intptr,+0139);
      if m<>WSR then error(+0140) else m:=WRF;
    end;
  gensp(m);
end end;

procedure callrw(fsys:sos; lpar,w,ln:boolean);
var l1,l2,oldlc,errno:integer; ftype,lsp:sp;
begin with b do begin oldlc:=lc; ftype:=textptr;
  with fa,pos do	{default file address}
    begin asp:=textptr; ac:=direct; lv:=0; sg:=0; ad:=argv[ord(w)].ad end;
  if lpar then with a do
    begin l1:=lino; if w then expression(fsys+[colon1]) else variable(fsys);
      l2:=lino;
      if formof(asp,[files]) then
	begin ftype:=asp;
	  if ac=direct then fa:=a else
	    begin fa.asp:=nilptr; fa.pos.ad:=reserve(ptrsize);
	      loadaddr; pop(local,fa.pos.ad,ptrsize);
	    end;
	  if (sy<>comma) and not ln then error(+0141);
	end
      else
	begin if iop[w]=nil then error(+0142);
	  if w then callw(fsys,l1,l2) else callr(l1,l2)
	end;
      while find3(comma,fsys,+0143) do
	begin l1:=lino; if w then expression(fsys+[colon1]) else variable(fsys);
	  l2:=lino;
	  if ftype=textptr then
	    if w then callw(fsys,l1,l2) else callr(l1,l2)
	  else
	    begin errno:=+0144;
	      if w then force(ftype^.filtype,errno) else
		begin store; l2:=lino end;
	      fileaddr; gen1(op_loi,ptrsize); ac:=indirect; pk:=true;
	      if w then store else
		begin lsp:=asp; asp:=ftype^.filtype; force(lsp,errno);
		  exchange(l1,l2)
		end;
	      gen1(op_mrk,0); fileaddr; if w then gensp(PUTX) else gensp(GETX)
	    end
	end;
    end
  else
    if not ln then error(+0145) else
      if iop[w]=nil then error(+0146);
  if ln then
    begin if ftype<>textptr then error(+0147);
      gen1(op_mrk,0); fileaddr;
      if w then gensp(WLN) else gensp(RLN)
    end;
  lc:=oldlc
end end;

procedure callflp(fsys:sos; lpar:boolean; m:libmnem);
begin with a do begin
  if lpar then
    begin variable(fsys); loadaddr;
      if asp<>nil then
	if asp^.form<>files then error(+0148) else
	  if (m<>EFL) and (asp<>textptr) then error(+0149);
    end
  else
    if iop[false]=nil then error(+0150) else gen1(op_lae,argv[0].ad);
  gensp(m);
  if m<>PAG then begin asp:=boolptr; ac:=expr end;
end end;

procedure callnd(fsys:sos; m:libmnem);
label 1;
var lsp:sp; sz,int:integer;
begin with a do begin
  if asp<>nil then
    if asp^.form<>pointer then error(+0151) else
    if asp=stringptr then error(+0152) else
      asp:=asp^.eltype;
  while find3(comma,fsys,+0153) do
    begin
      if asp<>nil then  {asp of form record or variant}
	if asp^.form=records then asp:=asp^.tagsp else
	if asp^.form=variant then asp:=asp^.subtsp else asp:=nil;
      if asp=nil then
	begin error(+0154); constant(fsys,lsp,int) end
      else
	begin assert asp^.form=tag;
	  int:=cstinteger(fsys,asp^.tfldsp,+0155); lsp:=asp^.fstvar;
	  while lsp<>nil do
	    if lsp^.varval<>int then lsp:=lsp^.nxtvar else
	      begin asp:=lsp; goto 1 end;
	end;
1:  end;
  sz:=sizeof(asp); int:=intsize+ptrsize;
  if sz>int then int:=(sz+int-1) div int * int;
  gen1(op_loc,int); gensp(m)
end end;

procedure callpg(m:libmnem);
begin gensp(m); if a.asp<>nil then if a.asp^.form<>files then error(+0156) end;

procedure callrr(m:libmnem);
begin
  if a.asp<>nil then
    if a.asp^.form<>files then error(+0157) else
      if a.asp=textptr then gen1(op_loc,0) else
	gen1(op_loc,sizeof(a.asp^.filtype));
  gensp(m);
end;

procedure callmr(m:libmnem);
begin teststandard; gensp(m);
  if a.asp<>nil then if a.asp^.form<>pointer then error(+0158)
end;

procedure callpu(m:libmnem; zsp,asp,isp:sp);
begin isp:=desub(isp);
  if (zsp<>nil) and (asp<>nil) then
    if (zsp^.form<>arrays) or (asp^.form<>arrays) then error(+0159) else
    if (spack in (zsp^.sflag - asp^.sflag)) and
	eqstruct(zsp^.aeltype,asp^.aeltype) and
	eqstruct(desub(zsp^.inxtype),isp) and
	eqstruct(desub(asp^.inxtype),isp) then
      begin gend(op_lae,zsp^.arrno); gend(op_lae,asp^.arrno); gensp(m) end
    else error(+0160)
end;

procedure call(fsys: sos; fip: ip);
var lkey: standpf; lpar:boolean; lsp,lsp2:sp;
begin with a do begin fsys:=fsys+[comma];
  lpar:=find3(lparent,fsys,+0161); if lpar then fsys:=fsys+[rparent];
  if fip^.pfkind<>standard then callnonstandard(fsys,lpar,fip) else
    begin lkey:=fip^.key;
      if lkey in [pput..phalt,feof..fabs,fround..farctan] then gen1(op_mrk,0);
      if lkey in [pput..prelease,fabs..farctan] then
	begin if not lpar then error(+0162);
	  if lkey <= prelease then
	    begin variable(fsys); loadaddr end
	  else
	    begin expression(fsys); force(fip^.idtype,+0163) end;
	end;
      case lkey of
	pread,preadln,pwrite,pwriteln:	{0,1,2,3 resp}
	  callrw(fsys,lpar,lkey>=pwrite,odd(ord(lkey)));
	pput:
	  callpg(PUTX);
	pget:
	  callpg(GETX);
	ppage:
	  callflp(fsys,lpar,PAG);
	preset:
	  callrr(OPN);
	prewrite:
	  callrr(CRE);
	pnew:
	  callnd(fsys,NEWX);
	pdispose:
	  callnd(fsys,DIS);
	ppack:
	  begin lsp:=asp; nextif(comma,+0164); expression(fsys); load;
	    lsp2:=asp; nextif(comma,+0165); variable(fsys); loadaddr;
	    callpu(PAC,asp,lsp,lsp2)
	  end;
	punpack:
	  begin lsp:=asp; nextif(comma,+0166); variable(fsys); loadaddr;
	    lsp2:=asp; nextif(comma,+0167); expression(fsys); load;
	    callpu(UNP,lsp,lsp2,asp)
	  end;
	pmark:
	  callmr(SAV);
	prelease:
	  callmr(RST);
	phalt:
	  begin teststandard; gensp(HLT) end;
	feof:
	  callflp(fsys,lpar,EFL);
	feoln:
	  callflp(fsys,lpar,ELN);
	fabs:
	  begin asp:=desub(asp);
	    if asp=intptr then gensp(ABI) else
	    if asp=realptr then gensp(ABR) else
	    if asp<>nil then begin error(+0168); asp:=intptr end;
	  end;
	fsqr:
	  begin asp:=desub(asp);
	    if asp=intptr then
	      begin gen1(op_dup,intsize); gen0(op_mul) end else
	    if asp=realptr then
	      begin gen1(op_dup,realsize); gen0(op_fmu); fltused:=true end else
	    if asp<>nil then begin error(+0169); asp:=intptr end;
	  end;
	ford:
	  begin if not nicescalar(desub(asp)) then error(+0170);
	    asp:=intptr
	  end;
	fchr:
	  checkbnds(charptr);
	fpred,fsucc:
	  begin asp:=desub(asp); gen1(op_loc,1);
	    if lkey=fpred then gen0(op_sub) else gen0(op_add);
	    if nicescalar(asp) then checkbnds(asp) else error(+0171)
	  end;
	fodd:
	  begin gen1(op_loc,1); gen1(op_and,intsize); asp:=boolptr end;
	ftrunc:
	  begin gen0(op_cfi); fltused:=true; asp:=intptr end;
	fround:
	  begin gensp(RND); asp:=intptr end;
	fsin:
	  gensp(SIN);
	fcos:
	  gensp(COS);
	fexp:
	  gensp(EXPX);
	fsqrt:
	  gensp(SQT);
	fln:
	  gensp(LOG);
	farctan:
	  gensp(ATN);
      end;
    end;
  if lpar then nextif(rparent,+0172);
end end;

{===================================================================}

procedure convert(fsp:sp; l1:integer);
{convert tries to make the operands of some operator of the same type;
  the operand types are given by fsp and a.asp; the resulting type
  is put in a.asp;
  l1 gives the line# of the first instruction of the right operand.
}
var l2:integer;
begin with a do begin asp:=desub(asp);
  case compat(fsp,asp) of
    eq,subeq: ;
    ir: begin l2:=lino; gen0(op_cif); exchange(l1,l2) end;
    ri: begin gen0(op_cif); asp:=realptr end;
    se: expandemptyset(fsp);
    es: begin l2:=lino; expandemptyset(asp); exchange(l1,l2) end;
    noteq: begin error(+0173); asp:=nil end
  end;
  if asp=realptr then fltused:=true;
end end;

procedure setcheck(var fsp:sp; var sz:integer);
{update fsp and sz}
var min,max:integer; errno:integer;
begin with a do begin asp:=desub(asp);
  if asp<>nil then
    if fsp=nil then
      begin errno:=0;
	if not bounds(asp,min,max) then
	  if asp=intptr then max:=iopt-1 else errno:=+0174;
	if max>=maxsetsz*bytebits then errno:=+0175;
	if errno<>0 then
	  begin asp:=nil; error(errno); max:=0 end;
	sz:=even(max div bytebits + 1); fsp:=asp;
      end
    else {asp<>nil and fsp<>nil}
      if asp<>fsp then
	begin error(+0176); asp:=nil end
end end;

procedure buildset(fsys:sos);
{this is a bad construct in pascal; two objections:
  - expr..expr very difficult to implement on most machines
  - this construct makes it hard to implement sets of different size
}
var cons,varpart:boolean; sz,i,c,c1,c2,l1,l2,cno:integer; lsp:sp;
    cstpart:array[1..maxsetwd] of
	      record case boolean of
		false:(s:set of 0..maxwbit);
		true: (w:integer)
	      end;
begin with a do begin cno:=0; sz:=maxsetsz;
  for i:=1 to maxsetwd do cstpart[i].w:=0;
  varpart:=false; lsp:=nil;
  if find2([notsy..lparent],fsys,+0177) then
    repeat l1:=lino; expression(fsys+[colon2,comma]);
      setcheck(lsp,sz); assert not odd(sz);
#ifdef STANDARD
      cons:=false;
#endif
#ifndef STANDARD
      cons:=ac=cst;
#endif
      if cons then c1:=intval else load;
      if find3(colon2,fsys+[comma,notsy..lparent],+0178) then
	begin expression(fsys+[comma,notsy..lparent]);
	  setcheck(lsp,sz); assert not odd(sz);
	  if not cons then load else
	    if ac=cst then c2:=intval else
	      begin load; l2:=lino; gen1(op_loc,c1);
		exchange(l1,l2); cons:=false
	      end;
	  if not cons then
	    begin l2:=lino; gen1(op_mrk,0); exchange(l1,l2);
	      gen1(op_loc,sz); gensp(BTS)
	    end;
	end
      else
	if not cons then gen1(op_set,sz) else c2:=c1;
      if cons then
	if (c1<0) or (c2>=sz * bytebits) then error(+0179) else
	  for c:=c1 to c2 do
	    begin i:=c div (maxwbit+1) + 1; cno:=cno+1;
	      cstpart[i].s:=cstpart[i].s + [c mod (maxwbit+1)]
	    end
      else
	if varpart then gen1(op_ior,sz) else varpart:=true;
    until endofloop(fsys,[notsy..lparent],comma,+0180);  {+0181}
  ac:=expr;
  if (cno=0) and not varpart then
    begin asp:=emptyset; gen1(op_loc,0) end
  else
    begin new(asp,power); asp^.form:=power;
      asp^.size:=sz; asp^.elset:=lsp; asp^.sflag:=[];
      if cno>0 then for i:=1 to sz div 2 do gen1(op_loc,cstpart[i].w);
      if varpart then if cno>0 then gen1(op_ior,sz);
    end
end end;

procedure factor(fsys: sos);
var lip:ip; l1:integer;
begin with a do
  if find1([notsy..nilcst,lparent],fsys,+0182) then
    case sy of
      ident:
	begin lip:=searchid([konst,vars,field,func],true); insym;
	  case lip^.klass of
	    func: begin call(fsys,lip); ac:=expr end;
			      {result on top stack}
	    konst:
	      begin asp:=lip^.idtype;
		if asp<>nil then
		  if asp^.form=scalar then
		    if asp=realptr then
		      begin ac:=indirect; pk:=false; l1:=lino;
			gend(op_lae,abs(lip^.value)); load;
			if lip^.value<0 then negatereal(l1)
		      end
		    else
		      begin ac:=cst; intval:=lip^.value end
		  else
		    begin gend(op_lae,lip^.value);
		      if asp=stringptr then ac:=expr else
			begin ac:=indirect; pk:=false end
		    end
	      end;
	    field,vars: selector(fsys,lip,[used]);
	  end  {case}
	end;
      intcst:
	begin asp:=intptr; ac:=cst; intval:=val; insym end;
      realcst:
	begin asp:=realptr; ac:=indirect; pk:=false;
	  gend(op_lae,val); insym
	end;
      charcst:
	begin asp:=charptr; ac:=cst; intval:=val; insym end;
      stringcst:
	begin asp:=stringstruct; gend(op_lae,val); insym;
	  if asp=stringptr then ac:=expr else
	    begin ac:=indirect; pk:=false end;
	end;
      nilcst:
	begin insym; ac:=expr; asp:=nilptr;
	  gen1(op_loc,nil1); if vopt<>off then gen1(op_loc,nil2);
	end;
      lparent:
	begin insym; expression(fsys+[rparent]); nextif(rparent,+0183) end;
      notsy:
	begin insym; factor(fsys); load; gen0(op_teq);
	  if (asp<>nil) and (asp<>boolptr) then
	    begin error(+0184); asp:=nil end;
	end;
      lbrack:
	begin insym; buildset(fsys+[rbrack]); nextif(rbrack,+0185) end;
    end
  else begin asp:=nil; ac:=expr end
end;

procedure term(fsys:sos);
var lsy:symbol; lsp:sp; l1,oldlc,llc:integer; first:boolean;
begin with a,b do begin first:=true;
  factor(fsys+[starsy..andsy]);
  while find2([starsy..andsy],fsys,+0186) do
    begin if first then begin load; first:=false end;
      lsy:=sy; insym; l1:=lino; lsp:=asp;
      factor(fsys+[starsy..andsy]); load; convert(lsp,l1);
      if asp<>nil then
	case lsy of
	  starsy:
	    if asp=intptr then gen0(op_mul) else
	    if asp=realptr then gen0(op_fmu) else
	    if asp^.form=power then setop(op_and)
	    else begin error(+0187); asp:=nil end;
	  slashsy:
	    if asp=realptr then gen0(op_fdv) else
	    if asp=intptr then
	      begin oldlc:=lc; llc:=reserve(intsize);
		gen1(op_stl,llc); gen0(op_cif); gen1(op_lol,llc); gen0(op_cif);
		gen0(op_fdv); fltused:=true; asp:=realptr; lc:=oldlc
	      end
	    else begin error(+0188); asp:=nil end;
	  divsy:
	    if asp=intptr then gen0(op_div)
	    else begin error(+0189); asp:=nil end;
	  modsy:
	    if asp=intptr then gen0(op_mod)
	    else begin error(+0190); asp:=nil end;
	  andsy:
	    if asp=boolptr then setop(op_and)
	    else begin error(+0191); asp:=nil end
	end {case}
    end {while}
end end;

procedure simpleexpression(fsys:sos);
var lsy:symbol; lsp:sp; l1:integer; min,first:boolean;
begin with a do begin l1:=lino; first:=true;
  min:=sy=minsy; if min or (sy=plussy) then insym;
  term(fsys + [minsy,plussy,orsy]);
  if min then
    begin load; first:=false; asp:=desub(asp);
      if asp=intptr then gen0(op_neg) else
      if asp=realptr then negatereal(l1) else
      if asp<>nil then begin error(+0192); asp:=nil end;
    end;
  while find2([plussy,minsy,orsy],fsys,+0193) do
    begin if first then begin load; first:=false end;
      lsy:=sy; insym; l1:=lino; lsp:=asp;
      term(fsys+[minsy,plussy,orsy]); load; convert(lsp,l1);
      if asp<>nil then
	case lsy of
	  plussy:
	    if asp=intptr then gen0(op_add) else
	    if asp=realptr then gen0(op_fad) else
	    if asp^.form=power then setop(op_ior)
	    else begin error(+0194); asp:=nil end;
	  minsy:
	    if asp=intptr then gen0(op_sub) else
	    if asp=realptr then gen0(op_fsb) else
	    if asp^.form=power then begin setop(op_com); setop(op_and) end
	    else begin error(+0195); asp:=nil end;
	  orsy:
	    if asp=boolptr then setop(op_ior)
	    else begin error(+0196); asp:=nil end
	end {case}
    end {while}
end end;

procedure expression; { fsys:sos }
var lsy:symbol; lsp:sp; l1,l2,l3:integer;
begin with a do begin l1:=lino;
  simpleexpression(fsys+[eqsy..insy]);
  if find2([eqsy..insy],fsys,+0197) then
    begin lsy:=sy; insym; lsp:=asp; loadcheap; l2:=lino;
      simpleexpression(fsys); loadcheap;
      if lsy=insy then
	begin
	  if asp<>nil then
	    if asp^.form<>power then error(+0198) else
	      if asp=emptyset then setop(op_and)
		{this effectively replaces the word on top of the
		  stack by the result of the 'in' operator: false }
	      else
		if not (compat(lsp,asp^.elset) <= subeq) then error(+0199) else
		  begin exchange(l1,l2); setop(op_inn) end
	end
      else
	begin convert(lsp,l2);
	  if asp<>nil then
	    case asp^.form of
	      scalar:
		if asp=realptr then gen0(op_cmf) else gen0(op_cmi);
	      pointer:
		if lsy in [ltsy,lesy,gtsy,gesy] then error(+0200)
		else gen1(op_cmu,ptrsize);
	      power:
		case lsy of
		  eqsy,nesy: setop(op_cmu);
		  ltsy,gtsy: error(+0201);
		  lesy:
		    begin setop(op_dup); exchange(l1,l2); lsy:=eqsy;
		      setop(op_ior); setop(op_cmu);
		    end;
		  gesy:
		    begin l3:=lino; setop(op_dup); exchange(l2,l3);
		      setop(op_ior); setop(op_cmu); lsy:=eqsy
		    end
		end;  {case}
	      arrays:
		if string(asp) then
		  begin l3:=lino; gen1(op_mrk,0); exchange(l1,l3);
		    gen1(op_loc,asp^.size); gensp(BCP)
		  end
		else error(+0202);
	      records: error(+0203);
	      files: error(+0204)
	    end;  { case }
	  case lsy of
	    ltsy: gen0(op_tlt);
	    lesy: gen0(op_tle);
	    gtsy: gen0(op_tgt);
	    gesy: gen0(op_tge);
	    nesy: gen0(op_tne);
	    eqsy: gen0(op_teq)
	  end
	end;
      asp:=boolptr; ac:=expr
    end;
end end;

{===================================================================}

procedure statement(fsys:sos); forward;
		{this forward declaration can be avoided}

procedure assignment(fsys:sos; fip:ip);
var la:attr; l1,l2:integer;
begin
  l1:=lino; selector(fsys+[becomes],fip,[assigned]); l2:=lino;
  la:=a; nextif(becomes,+0205);
  expression(fsys); loadcheap; checkasp(la.asp,+0206);
  exchange(l1,l2); a:=la;
  if not formof(la.asp,[arrays,records]) then store else
    begin loadaddr; gen1(op_blm,even(sizeof(la.asp))) end;
end;

procedure gotostatement;
{jumps into structured statements can give strange results. }
label 1;
var llp:lp; lbp:bp; diff:integer;
begin
  if sy<>intcst then error(+0207) else
    begin llp:=searchlab(b.lchain,val);
      if llp<>nil then
	if llp^.seen then gen1(op_brb,llp^.labname)
	else gen1(op_brf,llp^.labname)
      else
	begin lbp:=b.nextbp; diff:=1;
	  while lbp<>nil do
	    begin llp:=searchlab(lbp^.lchain,val);
	      if llp<>nil then goto 1;
	      lbp:=lbp^.nextbp; diff:=diff+1;
	    end;
1:	  if llp=nil then errint(+0208,val) else
	    begin
	      if llp^.labdlb=0 then
		begin dlbno:=dlbno+1; llp^.labdlb:=dlbno;
		  gend(ps_imd,dlbno);  {forward data reference}
		end;
	      gen1(op_mrk,diff); gend(op_lae,llp^.labdlb); gensp(GTO);
	    end;
	end;
      insym;
    end
end;

procedure compoundstatement(fsys:sos; err:integer);
begin
  repeat statement(fsys+[semicolon])
  until endofloop(fsys,[beginsy..casesy],semicolon,err)
end;

procedure ifstatement(fsys:sos);
var lb1,lb2:integer;
begin with b do begin
  expression(fsys+[thensy,elsesy]);
  force(boolptr,+0209); ilbno:=ilbno+1; lb1:=ilbno; gen1(op_zeq,lb1);
  nextif(thensy,+0210); statement(fsys+[elsesy]);
  if find3(elsesy,fsys,+0211) then
    begin ilbno:=ilbno+1; lb2:=ilbno; gen1(op_brf,lb2);
      genilb(lb1); statement(fsys); genilb(lb2)
    end
  else genilb(lb1);
end end;

procedure casestatement(fsys:sos);
label 1;
type cip=^caseinfo;
     caseinfo=record
	next: cip;
	csstart: integer;
	cslab: integer
     end;
var lsp:sp; head,p,q,r:cip; csa:boolean;
    l0,l1,l2,i,n,m,min,max,diff:integer;
begin with b do begin
  expression(fsys+[ofsy,semicolon,ident..plussy]); lsp:=a.asp; load;
  if not nicescalar(desub(lsp)) then begin error(+0212); lsp:=nil end;
  ilbno:=ilbno+1; l0:=ilbno; gen1(op_brf,l0);  {jump to CSA/B}
  ilbno:=ilbno+1; l1:=ilbno;
  nextif(ofsy,+0213); head:=nil; max:=-maxint; min:=maxint; n:=0;
  repeat ilbno:=ilbno+1; l2:=ilbno;   {label of current case}
    repeat i:=cstinteger(fsys+[comma,colon1,semicolon],lsp,+0214);
      if i>max then max:=i; if i<min then min:=i; n:=n+1;
      q:=head; r:=nil; new(p);
      while q<>nil do
	begin  {chain all cases in ascending order}
	  if q^.cslab>=i then
	    begin if q^.cslab=i then error(+0215); goto 1 end;
	  r:=q; q:=q^.next
	end;
1:    p^.next:=q; p^.cslab:=i; p^.csstart:=l2;
      if r=nil then head:=p else r^.next:=p;
    until endofloop(fsys+[colon1,semicolon],[ident..plussy],comma,+0216);  {+0217}
    nextif(colon1,+0218); genilb(l2); statement(fsys+[semicolon]);
    gen1(op_brf,l1);
  until lastsemicolon(fsys,[ident..plussy],+0219);  {+0220 +0221}
#ifdef OLDCASE
  csa:=true; diff:=max-min;
  if diff>=cixmax then error(+0222);
#endif
#ifndef OLDCASE
  if n=0 then begin diff:=-1; csa:=true end else
    begin diff:=max-min; csa:=(diff < 8) or (diff div 3 < n) end;
#endif
  dlbno:=dlbno+1; gendlb(dlbno);
  if csa then
    begin gen1(ps_rom,min); gencst(diff); gencst(0);
#ifdef OLDCASE
      m:=op_cse;
#endif
#ifndef OLDCASE
      m:=op_csa;
#endif
      while head<>nil do
	begin
	  while head^.cslab>min do
	    begin gencst(0); min:=min+1 end;
	  genclb(head^.csstart); min:=min+1; head:=head^.next
	end;
#ifndef OLDCASE
    end
  else
    begin gen1(ps_rom,n);
      while head<>nil do
	begin gencst(head^.cslab); genclb(head^.csstart); head:=head^.next end;
      gencst(0); m:=op_csb
#endif
    end;
  genend; genilb(l0); gend(m,dlbno); genilb(l1)
end end;

procedure repeatstatement(fsys:sos);
var lb1: integer;
begin with b do begin
  ilbno:=ilbno+1; lb1:=ilbno; genilb(lb1);
  compoundstatement(fsys+[untilsy],+0223);  {+0224}
  nextif(untilsy,+0225); genlin;
  expression(fsys); force(boolptr,+0226);
  ilbno:=ilbno+1; gen0(op_teq); gen1(op_zeq,ilbno);
  gen1(op_brb,lb1); genilb(ilbno)
end end;

procedure whilestatement(fsys:sos);
var lb1,lb2: integer;
begin with b do begin
  ilbno:=ilbno+2; lb1:=ilbno-1; genilb(lb1); lb2:=ilbno;
  genlin; expression(fsys+[dosy]);
  force(boolptr,+0227); gen1(op_zeq,lb2);
  nextif(dosy,+0228); statement(fsys);
  gen1(op_brb,lb1); genilb(lb2)
end end;

procedure forstatement(fsys:sos);
{the upper bound is evaluated once and stored in a temporary local}
var lip:ip; tosym,cons,local:boolean; cval,endlab,looplab,oldlc,llc:integer;
    lsp:sp; lad:integer;
begin with a,b do begin
  lsp:=nil; lad:=0; tosym:=true; local:=level<>1;
  oldlc:=lc; ilbno:=ilbno+2; endlab:=ilbno; looplab:=endlab-1;
  if sy<>ident then error(+0229) else
    begin lip:=searchid([vars],true); insym;
      if (refer in lip^.vflag) or ((lip^.vpos.lv<>level) and local) then
	error(+0230)
      else
	begin lsp:=lip^.idtype; lad:=lip^.vpos.ad;
	  lip^.vflag:=[used,assigned]
	end;
    end;
  if not nicescalar(desub(lsp)) then begin error(+0231); lsp:=nil end;
  nextif(becomes,+0232);
  expression(fsys+[tosy,downtosy,notsy..lparent,dosy]);
  cons:=ac=cst; if cons then cval:=intval; force(lsp,+0233);
  if not cons then gen1(op_dup,2);
  pop(local,lad,intsize);
  if cons then gen1(op_loc,cval);
  if find1([tosy,downtosy],fsys+[notsy..lparent,dosy],+0234) then
    begin tosym:=sy=tosy; insym end;
  expression(fsys+[dosy]);
  cons:=ac=cst; if cons then cval:=intval; force(desub(lsp),+0235);
  if not cons then
    begin llc:=reserve(intsize);
      gen1(op_dup,2); gen1(op_stl,llc);
    end;
  if tosym then gen1(op_bgt,endlab) else gen1(op_blt,endlab);
  genilb(looplab);
  nextif(dosy,+0236); statement(fsys);
  push(local,lad,intsize);
  if cons then gen1(op_loc,cval) else gen1(op_lol,llc);
  if tosym then gen1(op_bge,endlab) else gen1(op_ble,endlab);
  push(local,lad,intsize); gen1(op_loc,1);
  if tosym then gen0(op_add) else gen0(op_sub);
  a.asp:=desub(lsp); checkbnds(lsp);
  pop(local,lad,intsize);
  gen1(op_brb,looplab); genilb(endlab);
  lc:=oldlc
end end;

procedure withstatement(fsys:sos);
var lnp,oldtop:np; oldlc:integer; lpk:boolean;
begin with a,b do begin
  oldlc:=lc; oldtop:=top;
  repeat variable(fsys+[comma,dosy]);
    if asp<>nil then
      if asp^.form<>records then error(+0237) else
	begin lpk:=spack in asp^.sflag;
	  if (ac=direct) and not lpk then
	    begin new(lnp,crec); lnp^.occur:=crec; lnp^.cpos:=pos end
	  else
	    begin new(lnp,vrec); lnp^.occur:=vrec;
	      lnp^.rcpk:=lpk; lnp^.vdspl:=reserve(ptrsize);
	      loadaddr; pop(local,lnp^.vdspl,ptrsize);
			  {store address of record in temporary}
	    end;
	  lnp^.fname:=asp^.fstfld; lnp^.nlink:=top; top:=lnp;
	end;
  until endofloop(fsys+[dosy],[ident],comma,+0238);  {+0239}
  nextif(dosy,+0240); statement(fsys);
  top:=oldtop; lc:=oldlc;
end end;

procedure assertion(fsys:sos);
begin teststandard;
  if opt['a']=off then
    while not (sy in fsys) do insym
  else
    begin gen1(op_mrk,0); expression(fsys); force(boolptr,+0241);
      gen1(op_loc,linecount); gensp(ASS);
    end
end;

procedure statement; {fsys: sos}
var lip:ip; llp:lp; lsy:symbol;
begin
  assert [labelsy..casesy,endsy] <= fsys;
  assert [ident,intcst] * fsys = [];
  if find2([intcst],fsys+[ident],+0242) then
    begin llp:=searchlab(b.lchain,val);
      if llp=nil then errint(+0243,val) else
	begin if llp^.seen then errint(+0244,val) else llp^.seen:=true;
	  genilb(llp^.labname)
	end;
      insym; nextif(colon1,+0245);
    end;
  if find2([ident,beginsy..casesy],fsys,+0246) then
    begin if giveline then genlin;
      if sy=ident then
	if id='assert  ' then
	  begin insym; assertion(fsys) end
	else
	  begin lip:=searchid([vars,field,func,proc],true); insym;
	    if lip^.klass=proc then call(fsys,lip) else assignment(fsys,lip)
	  end
      else
	begin lsy:=sy; insym;
	  case lsy of
	    beginsy:
	      begin compoundstatement(fsys,+0247);  {+0248}
		nextif(endsy,+0249)
	      end;
	    gotosy:
	      gotostatement;
	    ifsy:
	      ifstatement(fsys);
	    casesy:
	      begin casestatement(fsys); nextif(endsy,+0250) end;
	    whilesy:
	      whilestatement(fsys);
	    repeatsy:
	      repeatstatement(fsys);
	    forsy:
	      forstatement(fsys);
	    withsy:
	      withstatement(fsys);
	  end
	end;
    end
end;

{===================================================================}

procedure body(fsys:sos; fip:ip);
var i,sz,letdlb,namdlb,inidlb:integer; llp:lp;
begin with b do begin namdlb:=0;
{produce PRO}
  genpnam(ps_pro,fip); gencst(fip^.pfpos.sg); gencst(fip^.headlc);
  gencst(ord(fip^.pfpos.lv<=1));
{initialize files}
  if level=1 then  {body for main}
    begin dlbno:=dlbno+1; inidlb:=dlbno; gend(ps_imd,inidlb);
      gen1(op_mrk,0); gend(op_lae,dlbno); gensp(INI);
    end;
  trace('procentr',fip,namdlb);
  dlbno:=dlbno+1; letdlb:=dlbno; gend(ps_imd,letdlb); gend(op_beg,letdlb);
{the body itself}
  lcmax:=lc;
  compoundstatement(fsys,+0251);  {+0252}
  lcmax:=address(lcmax,0,false);  {align lcmax}
  trace('procexit',fip,namdlb);
{undefined or global labels}
  llp:=lchain;
  while llp<>nil do
    begin if not llp^.seen then errint(-(+0253),llp^.labval);
      if llp^.labdlb<>0 then
	begin gendlb(llp^.labdlb); gen1(ps_rom,lcmax);
	  genclb(llp^.labname); genend;
	  {this doesn't work if local generators are around}
	end;
      llp:=llp^.nextlp
    end;
{define BUG size}
  gend(ps_let,letdlb); gencst(lcmax-fip^.headlc);
{finish and close files}
  treewalk(top^.fname);
  if level=1 then
    begin gendlb(inidlb); gen1(ps_con,argc+1);
      for i:=0 to argc do
	begin gencst(argv[i].ad);
	  if (argv[i].ad=0) and (i>1) then errid(+0254,argv[i].name)
	end;
      genend; gen1(op_mrk,0); gensp(HLT)
    end
  else
    begin
      if fip^.klass<>func then sz:=0 else
	begin if not (assigned in fip^.pflag) then errid(-(+0255),fip^.name);
	  sz:=even(sizeof(fip^.idtype)); push(local,fip^.pfpos.ad,sz);
	end;
      gen1(op_ret,sz); gen0(ps_end);
    end
end end;

{===================================================================}

procedure block; { fsys:sos; fip:ip }
var ad:integer;
begin with b do begin
  assert [labelsy..withsy] <= fsys;
  assert [ident,intcst,casesy,endsy,period] * fsys = [];
  if find3(labelsy,fsys,+0256) then labeldeclaration(fsys);
  if find3(constsy,fsys,+0257) then constdeclaration(fsys);
  if find3(typesy,fsys,+0258) then typedeclaration(fsys);
  if find3(varsy,fsys,+0259) then vardeclaration(fsys);
  if fip=progp then
    begin
      if iop[true]<>nil then
	begin ad:=address(lc,fhsize+buffersize,false);
	  argv[1].ad:=ad; iop[true]^.vpos.ad:=ad
	end;
      if iop[false]<>nil then
	begin ad:=address(lc,fhsize+buffersize,false);
	  argv[0].ad:=ad; iop[false]^.vpos.ad:=ad
	end;
      lc:=address(lc,0,false);  {align lc}
      gen1(ps_hol,lc); lc:=progp^.headlc; level:=1
    end;  {externals are also extern for the main body}
  while find2([procsy,funcsy],fsys,+0260) do pfdeclaration(fsys);
  if forwcount<>0 then error(+0261);  {forw proc not specified}
  nextif(beginsy,+0262);
  body(fsys+[casesy,endsy],fip);
  nextif(endsy,+0263);
end end;

{===================================================================}

procedure programme(fsys:sos);
var stdin,stdout:boolean; p:ip;
begin
  nextif(progsy,+0264); nextif(ident,+0265); nextif(lparent,+0266);
  repeat
    if sy<>ident then error(+0267) else
      begin stdin:=id='input   '; stdout:=id='output  ';
	if stdin or stdout then
	  begin p:=newip(vars,id,textptr,nil);
	    enterid(p); iop[stdout]:=p;
	  end
	else
	  if argc<maxargc then
	    begin argc:=argc+1; argv[argc].name:=id; argv[argc].ad:=0 end;
	insym
      end
  until endofloop(fsys+[rparent,semicolon],[ident],comma,+0268);  {+0269}
  if argc>maxargc then
    begin error(+0270); argc:=maxargc end;
  nextif(rparent,+0271); nextif(semicolon,+0272);
  block(fsys,progp);
  if opt['l']<>off then gen1(ps_mes,-linecount);
  eofexpected:=true; nextif(period,+0273);
end;

procedure compile;
var lsys:sos;
begin lsys:=[progsy,labelsy..withsy];
  repeat eofexpected:=false;
    main:=find2([progsy,labelsy,varsy,beginsy..withsy],lsys,+0274);
    if main then programme(lsys) else with b do
      begin lc:=0; level:=1;
	if find3(constsy,lsys,+0275) then constdeclaration(lsys);
	if find3(typesy,lsys,+0276) then typedeclaration(lsys);
	while find2([procsy,funcsy],lsys,+0277) do pfdeclaration(lsys);
      end;
    error(+0278);
  until false;  { the only way out is the halt in nextch on eof }
end;

{===================================================================}

begin  {main body of pcompiler}
  init1; init2; init3; init4;
	{all this initializing must be independent of opts}
  reset(em1); if not eof(em1) then options(false);
  rewrite(em1); write(em1,MAGICLOW,MAGICHIGH);
#ifndef STANDARD
  get(input);
#endif
  if eof(input) then gen0(ps_eof) else
    begin linecount:=1; chcnt:=1; ch:=input^; chsy:=cs[ch]; insym;
      handleopts; {initialize all opt dependent stuff}
      compile;
    end;
end.  {pcompiler}
