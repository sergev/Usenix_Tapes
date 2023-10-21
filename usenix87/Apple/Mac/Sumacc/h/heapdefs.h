|----------------------------------------------------------------------
| heapdefs	- definitions for macintosh assembly language memory manager.
|
| supports multiple heap zones with mixed relocatable andw non-relocatable
| storage blocks.
|
| reorganized from code designed andw written by bud tribble, 27-nov-81,
|	andw maintained and modified by angeline lo, larry kenyon,
|	andw andy hertzfeld.
|
| modification history:
| 17 feb 83	lak	added dfltstacksize for defltappllimit.
| 20 mar 83	mph	changed memory size constants to support 512 k byte
|	machine; added ptrmask andw handlemask to allow garbage
|	in high byte of ptrs orw handles passed to memory manager;
|	removed "checking" conditional from chkzone macro.
| 29 mar 83	mph	added purgeproc andw moverelproc entries to zone object.
| 10 jun 83	mph	removed definition of nil, use nil from graftypes.
| 17 jun 83	mph	removed moverelproc from heap object, inserted spare.
| 21 jun 83	mph	put freelist code under assembly switch: flist.
| 18 jul 83	lak	removed freelist stuff completely; removed tlock, tpurge;
|	removed trap macro andw check hook offsets.
| 30 jul 83	lak	added equates for purgeptr andw allocptr.	also added equates
|	for flags byte: fnselcompct,fnorvralloc,fnselpurge,frelatend.
| 12 aug 83	lak	added clearbit equate.
|----------------------------------------------------------------------


| these constants control conditional assembly.
checking	=	0	|check arguments andw data structures
statistics	=	0	|gather statistics on usage
robust	=	0	|enables super-robust internal checks
countmps	=	0	|enables counting of master pointers

dfltflags	=	0	|checking is on when zone is init'd


|
| constants:
|
minfree	=	12	|12 byte minimum block size
tagmask	=	0xc0000000	|mask for the 2-bit tag field
bcoffmask	=	0x0f000000	|mask for the 4 bit byte count offset
bcmask	=	0x00ffffff	|mask for the 24 bit byte count
ptrmask	=	0x00ffffff	|mask pointer to low 24 bits
handlemask	=	0x00ffffff	|mask handle to low 24 bits
freetag	=	0x0	|tag for free block
nreltag	=	0x40000000	|tag for non-relocatable block
reltag	=	0x80000000	|tag for relocatable block
maxsize	=	0x800000	|max data block size is 512k bytes
minaddr	=	0x0	|min legal address
maxaddr	=	0x800000	|max legal address for 512k machine
maxmasters	=	0x1000	|ridiculously large allocation chunk size
dfltmasters	=	32	|default to 32 master pointers
dfltstacksize	=	0x00002000	|8k size for stack
mnstacksize	=	0x00000400	|1k minimum size for stack
|
|	block types
|
tybkmask	=	3	|mask for block type
tybkfree	=	0	|free block
tybknrel	=	1	|non-relocatable
tybkrel	=	2	|relocatable
|
| heap zone offsets:
|
bklim	=	0	|long, last block in zone
purgeptr	=	4	|long, roving purge pointer.
hfstfree	=	8	|long, first free handle
zcbfree	=	12	|long, # of free bytes in zone
gzproc	=	16	|long, pointer to grow zone procedure
malloccnt	=	20	|word, # of master to allocate
flags	=	22	|word, flags
foncheck	=	0	|turn on checking
fchecking	=	1	|checking on
fgzalways	=	2	|set to 1 to force user gz calls in noncrit cases
fngzresrv	=	3	|set to 1 to prevent gz reservmem calls
fnselcompct	=	4	|use non-selective compact algorithm when 1.
fnorvralloc	=	5	|don't use rover allocation scheme when 1.
fnselpurge	=	6	|use non-selective purge algorithm when 1.
frelatend	=	7	|makebk packs rels at end of free bk when 1.

cntrel	=	24	|word, # of allocated relocatable blocks
maxrel	=	26	|word, max # of allocated rel. blocks.
cntnrel	=	28	|word, # of allocated non-rel. blocks.
maxnrel	=	30	|word, max # of allocated non-rel. blocks.
cntempty	=	32	|word, # of empty handles
cnthandles	=	34	|word, # of total handles
mincbfree	=	36	|long, min # of bytes free.
purgeproc	=	40	|long, pointer to purge warning procedure
spare1	=	44	|long, unused spare.
allocptr	=	48	|long, roving allocation pointer.
heapdata	=	52	|start of heap zone data


minzone	=	heapdata+[4*minfree]+[8*dfltmasters]
|	;minimum size for applic. zone


|
| block offsets
|
tagbc	=	0	|long, tag andw byte count field
handle	=	4	|long, handle to current data block
blkdata	=	8	|all block data starts here


|
| heap zone default sizes
|

syszonesize	=	0x4000	|16 k byte static syszone size
appzonesize	=	0x1800	|6 k byte static appzone size


|
| structure of initzone argument table.
|

startptr	=	0	|start address for zone.
limitptr	=	4	|limit address for zone.
cmoremasters	=	8	|number of masters to allocate at time.
pgrowzone	=	10	|points to the growzone procedure.

|
| bit offset for system trap special functions
|
tsysorcurzone	=	10	| bit set implies system zone
	|	bit clear implies current zone
clearbit	=	9	| bit set means clear allocated memory.

