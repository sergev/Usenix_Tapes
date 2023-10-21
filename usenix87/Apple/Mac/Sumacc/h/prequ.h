|-------------------------------------------------------------------------------
|       these are the definitions for the print code
|-------------------------------------------------------------------------------
fprdbgok        =       1               |debug enable
iprdeapshit     =       29              |printcode deapshit
iprabort        =       128             |abort
iprrelease      =       2               |current version number of the code.
__getresource	= /a9a0			| this belongs somewhere else!
__closeresfile	= /a99a			| this belongs somewhere else!
__openresfile	= /a997			| this belongs somewhere else!

|-------------------------------------------------------------------------------
| the private print globals; 16 bytes located at [sysequ] printvars
|-------------------------------------------------------------------------------
iprerr          =       0               |current print error.  set to iprabort to abort printing.
bdocloop        =       iprerr+2        |the doc style: draft, spool, .., and ..
                                        |currently use low 2 bits; the upper 6 are for flags.
buser1          =       bdocloop+1
luser1          =       buser1+1
luser2          =       luser1+4
luser3          =       luser2+4
iprvarsize      =       luser3+4        |the prvar's size.[16]

bdraftloop      =       0               |the docloop types
bspoolloop      =       1
buser1loop      =       2
buser2loop      =       3
|-------------------------------------------------------------------------------
| these are the prdrvr constants.
|-------------------------------------------------------------------------------
iprdrvrid       =               2               |driver's resid
iprdrvrref      =               /fffd           |driver's refnum = not resid
iprdrvrdev      =               /fd00           |driver's qd dev num = refnum in hi byte, variant in lo
iprbitsctl      =               4               |the bitmap print proc's ctl number
lscreenbits     =               /00000000       |the bitmap print proc's screen bitmap param
lpaintbits      =               /00000001       |the bitmap print proc's paint [sq pix] param
iprioctl        =               5               |the raw byte io proc's ctl number
iprevtctl       =               6               |the prevent proc's ctl number
lprevtall       =               /00fffffd       |the prevent proc's cparam for the entire screen
lprevttop       =               /00fefffd       |the prevent proc's cparam for the top folder
iprdevctl       =               7               |the prdevctl proc's ctl number
iprreset        =               1               |the prdevctl proc's cparam for reset
iprpageend      =               2               |the prdevctl proc's cparam for end page
ifmgrctl        =               8               |the fmgr's tail-hook ctl call
ifmgrstat       =               8               |the fmgr's pre-hook status call

|-------------------------------------------------------------------------------
| various resource types & id's
|-------------------------------------------------------------------------------
lpstrtype       =      /53545220        |"str ": res type for the pr rsrc file name
ipstrrfil       =      /e000            |str -8192 is in sysres & names the current printer

lprinttype      =      /50524543        |"prec": res type for the hprint records
iprintdef       =      0                |default hprint
iprintlst       =      1                |last used hprint
iprintdrvr      =      2                |.print's parameter record; not a print rec

lpftype         =      /5046494c        |"pfil"
lpfsig          =      /50535953        |"psys"
ipficon         =      140              |

lprtype         =      /4150504c        |"appl"
lprsig          =      /50535953        |"psys"
ipricon         =      138              |

|-------------------------------------------------------------------------------
| the printing data structures
|-------------------------------------------------------------------------------
|print info record: the parameters needed for page composition.
|-------------------------------------------------------------------------------
idev            =       0         |font mgr/quickdraw device code
ivres           =       idev+2    |resolution of device, in device coordinates.
ihres           =       ivres+2   |   ..note: v before h => compatable with point.
rpage           =       ihres+2   |the page (printable) rectangle in device coordinates.
iprinfosize     =       rpage+8   |the prinfo size.[14]

|-------------------------------------------------------------------------------
|printer style: the printer configuration and usage information.
|-------------------------------------------------------------------------------
wdev            =       0         |the drvr number.  hi byte=refnum, lo byte=variant.
ipagev          =       wdev+2    |paper size in units of 1/iprpgfract
ipageh          =       ipagev+2  |   ..note: v before h => compatable with point.
bport           =       ipageh+2  |the io port number.
feed            =       bport+1   |paper feeder type.
iprstlsize      =       feed+1    |the prstl size.[8]

|-------------------------------------------------------------------------------
|print extra info: the print time extra information.
|-------------------------------------------------------------------------------
irowbytes       =       0               |the band's rowbytes.
ibandv          =       irowbytes+2     |size of band, in device coordinates
ibandh          =       ibandv+2        |   ..note: v before h => compatable with point.
idevbytes       =       ibandh+2        |size for allocation.  may be more than rbounds size!
ibands          =       idevbytes+2     |number of bands per page.
bpatscale       =       ibands+2        |pattern scaling
bulthick        =       bpatscale+1     |3 underscoring parameters
buloffset       =       bulthick+1
bulshadow       =       buloffset+1
scan            =       bulshadow+1     |band scan direction
bxinfox         =       scan+1          |an extra byte.
iprxinfosize    =       bxinfox+1       |the prxinfo size.[16]

|-------------------------------------------------------------------------------
|print job: print "form" for a single print request.
|-------------------------------------------------------------------------------
ifstpage        =       0               |page range.
ilstpage        =       ifstpage+2
icopies         =       ilstpage+2      |no. copies.
bjdocloop       =       icopies+2       |draft quality print flag
ffromapp        =       bjdocloop+1     |printing from an app (not prapp) flag
pidleproc       =       ffromapp+1      |the proc called while waiting on io etc.
pfilename       =       pidleproc+4     |spool file name: nil for default.
ifilevol        =       pfilename+4     |spool file vol, set to 0 initially.
bfilevers       =       ifilevol+2      |spool file version, set to 0 initially.
bjobx           =       bfilevers+1     |an extra byte.
iprjobsize      =       bjobx+1         |the prjob size.[20]

|-------------------------------------------------------------------------------
|the universal 120 byte printing record
|-------------------------------------------------------------------------------
iprversion      =       0                       |printing software version
prinfo          =       iprversion+2            |the prinfo data associated with the current style.
rpaper          =       prinfo  + iprinfosize   |the paper rectangle [offset from rpage].
prstl           =       rpaper  + 8             |this print request's style.
prinfopt        =       prstl   + iprstlsize    |print time imaging metrics
prxinfo         =       prinfopt+ iprinfosize   |print-time (expanded) print info record.
prjob           =       prxinfo + iprxinfosize  |the print job request
iprintsize      =       120                     |the print record size.[120]

|-------------------------------------------------------------------------------
|print port: a graf port, its procs, plus some extra.
|-------------------------------------------------------------------------------
gport           =       0               |the printer's graf port.
gprocs          =       108             |..and its procs
lgparam1        =       gprocs+13*4     |some params: our relocatable stuff etc.
lgparam2        =       lgparam1+4
lgparam3        =       lgparam2+4
lgparam4        =       lgparam3+4

fourptr         =       lgparam4+4      |whether the prport allocation was done by us.
fourbits        =       fourptr+1       |whether the bitmap allocation was done by us.
iprportsize     =       fourbits+1      |the prport size.[178]

|-------------------------------------------------------------------------------
|print status: runtime status for prpicfile & prpic procs.
|-------------------------------------------------------------------------------
itotpages       =       0               |total pages in print file.
icurpage        =       itotpages+2     |current page number
itotcopies      =       icurpage+2      |total copies requested
icurcopy        =       itotcopies+2    |current copy number
itotbands       =       icurcopy+2      |total bands per page.
icurband        =       itotbands+2     |current band number
fpgdirty        =       icurband+2      |true if current page has been written to.
fimaging        =       fpgdirty+1      |set while in band's drawpic call.
hprint          =       fimaging+1      |handle to the active printer record
pprport         =       hprint+4        |ptr to the active prport
hpic            =       pprport+4       |handle to the active picture
iprstatsize     =       hpic+4          |the prstatus size.[26]

|-------------------------------------------------------------------------------
| these are the constants for using resources to swap in the non-driver
| print code.  three numbers are needed:
|       restype
|       resid
|       offset into the seg's jump table
|
| the offset is really a formatted long that contains three fields:
|       frame size; unlock flag; offset into the seg's jump table
| we could use the topmost byte for further stuff: a stack adjust for
| storing the registers needed by the link code so that it would be re-entrant.
|-------------------------------------------------------------------------------

lpdeftype        =      /50444546       |pr resource type: "pdef"

iprdraftid       =      0               |pr draft resource id
iprspoolid       =      1               |pr spool resource id
ipruser1id       =      2               |pr spare1 resource id
ipruser2id       =      3               |pr spare2 resource id
lopendoc         =      /000c0000           |propendoc jumptable offset
lclosedoc        =      /00048004           |prclosedoc jumptable offset
lopenpage        =      /00080008           |propenpage jumptable offset
lclosepage       =      /0004000c           |prclosepage jumptable offset

iprdlgsid        =      4               |pr dialogs resource id
ldefault         =      /00048000           |printdefault jumptable offset
lstldialog       =      /00048004           |prstldialog jumptable offset
ljobdialog       =      /00048008           |prjobdialog jumptable offset
lstlinit         =      /0004000c           |prstlinit jumptable offset
ljobinit         =      /00040010           |prjobinit jumptable offset
ldlgmain         =      /00088014           |prdlgmain jumptable offset
lprvalidate      =      /00048018           |printvalidate jumptable offset
lprjobmerge      =      /0008801c           |printvalidate jumptable offset

iprpicid         =      5               |pic printing resource id
lprpicfile       =      /00148000           |prpicfile jumptable offset
lprpic           =      /00148004           |prpic jumptable offset

icfgdlgid        =      6               |configuration proc resource id
lcfgdialog       =      /00008000           |prcfgdialog jumptable offset

iprhackid        =      7               |the "oops, i forgot" resource id
lprhack          =      /000c8000           |prhack jumptable offset

|-------------------------------------------------------------------------------
