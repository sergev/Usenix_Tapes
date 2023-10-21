Relay-Version: version B 2.10.3 4.3bsd-beta 6/6/85; site plus5.UUCP
Posting-Version: version B 2.10.2 9/5/84; site myrias.UUCP
Path: plus5!wuphys!wucs!busch!we53!mgnetp!ihnp4!alberta!myrias!mj
From: mj@myrias.UUCP (Michal Jaegermann)
Newsgroups: net.sources
Subject: Documentation and source for TI-Forth  screen editor(s)
Message-ID: <190@myrias.UUCP>
Date: 13 Nov 85 00:05:28 GMT
Date-Received: 13 Nov 85 17:22:19 GMT
Organization: Myrias Research, Edmonton
Lines: 498

*** REPLACE THIS LINE WITH YOUR MESSAGE ***


     To LCT - the author of an original TI-Forth editor


     You will find here a new screen oriented editor for TI-Forth.
For people with other systems, TI-Forth is a fig-forth with some
computer specific extensions.  The goal was to write a small
editor with a lot of futures, like autorepeating keys, overtype
and insert, some limited form of cut-and-paste, single-stepping
through source, execution of Forth from editor. Results are
below. Compiled code adds below 2K to system stuff.
WARNING: this is a FORTH editor - meaning that it does not force
you to make a stupid thing, but will not protect you if you are
really insistent.

     Porting to other system
     -----------------------
     Should be really not so hard. The editor actually operates
on a virtual 1K buffer with addresses starting at 0. This is
mapped on an actual edit buffer which is displayed on your CRT.
All gory system details are fairly well insulated. As an
evidence - the same editor is displayed on two different ways in
TI enviroment. Using a windowed 40-columns wide text display,
and also is "drawn" in 64-column bit-map mode. Remember - I am
still using old TV for a "monitor" with TI99/4a. Details of 40
columns display are on screens 20 and 21. Same stuff for
64-columns on screens 22 and 23. Screen 24 is also midly system
dependent since it assumes in mapping words that you are editing
1K block organized in 16 lines, every one 64 characters long.
Change apprioprately if necessary. Also a word RKEY, which
implements auto-repeating key, goes to a hardware address to
check if a new key was pressed from the last scan. Implement
your own. A standard KEY will work, but will not autorepeat. All
other stuff should not create problems. Keyboard scan is table
driven, so change it to suit your keyboard, habits and
preferences. Table is called ACTION.
      
      Instalation in TI-Forth
      ------------------------
      (this section for TI-Forth owners only)
      I'm sorry. You need some extra words. Required is CMOVE>
- move up a memory contents.
It requires on stack a starting address,  a target  address 
and count in bytes.  It will not do anything
if a count is not a positive.  For speed  reasons  definition
is in code.
   HEX CODE CMOVE> C0B9 , C079 , C039 , A002 ,
                   A042 , 0600 , 0601 , 0602 ,
                   1102 , D450 , 10FA , 045F ,
     You  will  also need a .S for a stack display.  It loads
as a one of  -DUMP  words  but  you  may  extract  definition
somewhere  else  if  you wish.  Three other are "convenience"
words - if you do not like them - edit the source and  forget
about them.  They are:
: \ IN  C/L + C/L MINUS AND IN ! ; IMMEDIATE
 (for comments - nearly standard)
: 2DUP OVER OVER ;
 (quite obvious)
: AT GOTOXY ;
 (since I am alergic on GOTOXY)
     And  more  thing  about 64 column display.  Since I have
problems with an overscan I moved a 64 column  editor  screen
to the right.
     This  is  done  by modification of SMASH on screen 65 of
the system disk.  In this code you will find (once only!)  an
entry  2000 , .   Replace  it  with 2008 , .  This will cause
end-of-lines to disappear, but I think that this is a smaller
problem.   If  you  are  lucky enough not to have an overscan
problem then leave SMASH alone but remove 2+  from  64-column
.CUR.  By the way:
     CLIST  (  n  --  )  (list contents of a block n ) may be
defined as
     : CLIST (  n  --  )  BLOCK L/SCR 0
               DO I C/L * OVER + C/L I SMASH VMBW LOOP DROP ; 
and you do not really need CLINE and CLOOP.
     The editor will trap all non-printable  characters  with
and exception DEL ( hex 7F ).  If this is going to bother you
add in EDI loop, right after RKEY the following: ` DUP 7F < *
`.  This will remove a problem.
     If  you  will find that a sensitivity of a keyboard does
not suit your taste - play around with a delay loop in  BLINK
and  constants  imbedded  in  RKEY.   They  are  not  exactly
independent but try yourself.
     After you have done all of the above you may  load  your
new editor (you typed it in before - didn't you?) and try how
to use it.
     To make life easier - if you will get ten enclosed screens
into a standard dis/var 80 text file you may use the utility
below to install them on your Forth disk. I have to admit that
this works really well with two drive (RAM drive ok), but it
will work on one drive with files up to 5 screens worth, or
you may extend it adding appriopriate WAIT_AND_PROMPT stuff to
allow for disk swapping. When writing to disk it will not use
more screens then "cnt}, even if a file is bigger. It will ask
you for file name, and when writing, will append old file if you
will respond with <enter>. (Use GETFILE to set up file name
without any other operation).

SCR# 89
 
( SCREENS TO FILE AND BACK MJ 10Jun85 )  0 CLOAD RECORDS
BASE->R HEX  43 CLOAD ~EOF 0 VARIABLE BUF 4E ALLOT
PABS @ 2 + BUF 1400 FILE #CP   #CP
: GETFILE  CR ." Filename: " PAD 40 20 FILL \ <enter> for old nm
  PAD 1+ 30 OVER OVER  EXPECT  -TRAILING 2- -DUP
   IF #CP  SET-PAB  VRBL  50 REC-LEN DUP ROT 1- C!
             1+ PAD PAB@ 9 + ROT VMBW ELSE DROP APPND ENDIF ;
: SCNS>FILE ( scn# cnt -- )  GETFILE OPN OVER + SWAP DO I BLOCK
  400 OVER + SWAP DO I BUF 20 MOVE BUF 40 -TRAILING 1 MAX  WRT
      DROP  40 +LOOP  LOOP CLSE ; \ to append, <enter> for fname
: FILE>SCNS ( scn# cnt -- ) GETFILE OPN       OVER + SWAP
  DO ~EOF IF I BLOCK UPDATE 400 OVER OVER 20 FILL OVER + SWAP
    DO ~EOF IF RD BUF I ROT CMOVE ELSE LEAVE ENDIF 40 +LOOP
           ENDIF LOOP CLSE ;
: RECORDS #CP INPT OPN 0 DO ~EOF IF CR BUF RD TYPE PAUSE
    ELSE 1 ENDIF IF LEAVE ENDIF LOOP CLSE ;              R->BASE
 
 
     How to save youorself more typing
     ---------------------------------
     Send us your disk in  a  self-addressed  mailer  with  a
proper postage included (if not in CANADA you may buy in your
post office a proper amount of things called "Coupon  Reponse
International").   You  will  get  back  a  modified TI-Forth
system disk with this editor an many  other  handy  utilities
included.  It is really worth it.  Our address is
           EDMONTON 99'er COMPUTER USER'S SOCIETY
             P.O. BOX 11983, EDMONTON, ALBERTA
                       CANADA T5J 3L1
 
 ---------------------------------------------------------------------
	   once again a general stuff
 --------------------------------------------------------------------

     Starting and leaving editor
     ---------------------------
     As usual.  20 EDIT will bring on your screen a  contents
of  a  block  20 with an edit cursor in a home position.  ED@
will restart editor on last screen used with cursor at home
position. WHERE brings you to a location of a LOAD error.
     One  extra  -  ER  (EditResume) recals not only the last
screen but also the last cursor position.   So  you  will  be
back  where  you  left the editor the last time.  Once in the
editor ctrl-E will switch to  the  previous  screen  (a  home
positon), and ctrl-X to the next one.  Fctn-9 to get out.
 
     Entering text
     -------------
     Editor  will  come  up in an overtype mode.  So whatever
you are typing replaces a text under cursor.  Fctn-2  toggles
between  overtype and an insert modes.  While inserting a new
text pushes an old text to the right.   Whatever  will  spill
over the right margin is lost.
 
     Marking and unmarking text
     --------------------------
     Think  of  it  that  way.   You  have always exactly two
marks.  If they are invisible then they are  end  of  current
line  and  your  cursor position.  Ctrl-Z puts a visible mark
where your cursor is.  The first one will replace (as mark) a
cursor  position,  the second one - end of line.  If you will
try to put a third mark on a screen - the second one will  be
replaced with a new one.  Visible
marks  are  stored  on stack, so if you have to move the
first one, SWAP them (how to do it  -  later).   Ctrl-U  will
erase all visible marks from your screen.
 
     Deleting and inserting in one line
     ----------------------------------
Fctn-1   will  delete  one  character.   Remember  -  it
autorepeats.
Fctn-3 deletes the whole current line and all subsequent
text  moves  up.  Deleted line is stored in an delete buffer.
Ctrl-8 opens a blank line over the cursor.  Old last line  is
lost.  So everything like one should expext.
Fnct-7  removes  all  text  between  marks  (visible  or
invisible) and replaces it with blanks.  Up to 64  characters
of  removed  text are stored in a delete buffer.  If there is
more - they are lost.
     Action of fctn-8 depends on an editor mode.  In overtype
mode it acts as ctrl-8 but moving a text from a delete buffer
into an opened line.  While in isert mode - it inserts a text
from a delete buffer, WHITHOUT leading and trailing (but one)
blanks in line, on a cursor  position.   Old  text  moves  to
right.   A right margin spillover is lost.  Reread section on
marking and experiment until you will feel comfortable.
     Do not hold fnct-7 too long, since it will clobber  your
delete buffer with freshly created blanks.
     On  the top of it you may Yank text to the delete buffer
with ctrl-Y.  This will  put  away  text  between  marks  for
subsequent  fctn-8 inserting WITOUT removing an original from
a screen.  Same Same limitations as above will apply here.
 
     Moving around
     -------------
     Usual "arrow keys" will work - fnct-S, fctn-X, fctn-E,
fctn-D.
But also you have a "terminal style" controls, i.e.  ctrl-H
left, ctrl-J down, ctrl-K up and ctrl-L right.  They are handy
when singlestepping through a source.  Moreover ctrl-R will move
to the right in one word steps.  No key for a similar movement
backwards.
 
     Singlestep and executing Forth
     ------------------------------
     Ctrl-W  will  execute  (if  possible)  any  word on your
screen which is pointed  by  the  cursor.   The  cursor  will
advance  to the next word and below your edit screen you will
see a display of stack.
     Great for debugging.  Ctrl-Q will do the same  with  two
words  (try  0  CLOAD EDIT).  Do not try to execute compiling
words like DO of IF since you will  get  an  error.   No  big
deal,  ER  will put you where you just have been, but a stack
will be lost.
     You may also hit ctrl-.  to run an internal interpreter.
You  will  be put below an edit screen and you may type there
up to 80 characters of Forth to execute.  Upon enter you will
return  automatically  to the editor and a current stack will
be displayed below.  The editor part of a  screen  is  frozen
and  will  not  scroll  even if you will dump a whole memory.
This means that if you will make an error  then  to  unfreeze
the  screen,  you  have to return to editor (ER is ok) to get
out later with fctn-9.
     This is interpreter is even flexible enough to  start  a
compilation  (say  with  a  colon  definition),  to return to
editor,  to  do  some  editing  and  to  resume  a  suspended
compilation  later.  I am not advising you to use it normally
in that way, but this is a great way to see for yourself  how
a  compiler security is implemented and what DO is putting on
stack to tell LOOP where to branch.
     You will find out that  in  particular  you  may,  using
ctrl-.,  call editor itself - executing, for example, EDIT or
ER.  I would rather advise you not to  do  that.   Reason  is
that  you  are  stroring  on a return stack a return address.
So, once you would like to get out and hit  fctn-9  you  will
return...   back  to editor (previous instance).  If you will
do that many times getting back to FORTH may take a number of
fctn's-9's.  Ctrl-.  and QUIT will always save a day.
 
     How to move big blocks
     ----------------------
     The editor above of course can be extended and made more
powerful.  But a goal was to make it convenient, nice and not
too  big.   For  example,  one  may add big delete buffer and
rewrite deleting and inserting a little bit  to  get  a  full
"cut-and-paste".  But instead of doing this I am rather using
ctrl-.  feature on those unfrequent ocasions when I need more
extensive  capabilities.   For example - how to move block of
five lines from screen number 23 into some other location  on
screen 37.  Type 23 EDIT.  In the editor hit ctrl-.  and once
outside
     EDITOR *CUR.  A word *CUR from vocabulary EDITOR returns
an  address  in an edit buffer which corresponds to a current
cursor position.  You will see it on stack.  Now  ctrl-9  and
37  EDIT.   Once  back  in  the editor put your cursor on the
position when you would like  to  see  your  block  and  type
ctrl-.   *CUR  C/L 5 * CMOVE <enter>.  You will find yourself
back in the editor on  the  very  beginning  of  an  improted
block.   To  mark  the  block  as  an updated just retype one
character on screen.  FLUSH will save all  changes  to  disk.
Other   useful,   for   such  operations,  word  from  EDITOR
vocabulary is ROOM.  See source screens for details.

     To make life easier - here is
     A short refernce
     ----------------
 
fnct-1  delete character
fnct-2  toggle overtype/insert modes
fnct-3  delete line
fnct-5  swap windows in text mode / home in bit-map mode
fnct-6  move right
fnct-7  delete between marks ( one or both may be defaults )
fnct-8  insert text from PAD
        * in an ovetype mode inserts a new line moving other
        text down
        * in insert mode insert contents of PAD on a cursor
        position shifting text on the line to the right
fnct-9  leave editor
fnct-(S,X,E,D) move cursor to the (left, down, up, right)
 
ctrl-8  open blank line
ctrl-(E,X) get the (previous, next) screen
ctrl-W  execute one Word pointed by cursor  - display stack
ctrl-Q  execute two words pointed by cursor - display stack
ctrl-R  move Right one word
ctrl-Y  Yank - store text between marks (64 max) in edit buffer
ctrl-(H,J,K,L) move (left, down, up, right) - terminal style
ctrl-Z  mark cursor positon
ctrl-U  Unmark - replace marks by defaults (cursor, end-of-line)
ctrl-.  escape from editor to execute Forth. All Forth
        available. After <enter> or 80 characters typed returns
        aoutomatically to the editor with a display of a stack.
        If you will get an ERROR type ER to return back to
        the editor. Otherwise an edit screen will be frozen.
***
 
     Source screens
     --------------
     Referenced screen numbers are as on my disk.  Remeber to
change them in LOAD statements if you will put them  in  some
other location.
 
SCR# 20
 
( SCREEN EDITOR - 40 column display 20SEP85*Michal Jaegermann )
( 0 CLOAD EDIT ) BASE @ HEX    21 CLOAD  RANDOMIZE
VOCABULARY EDITOR IMMEDIATE EDITOR DEFINITIONS      3 WIDTH !
0 VARIABLE S_H           0 VARIABLE CLE          0 VARIABLE INS
: BLINK CURPOS @ DUP VSBR SWAP 1E OVER VSBW C0 0 DO LOOP VSBW ;
     18 LOAD   \ load cursor positions and auto-repeating key
: HLIST  0 0 AT SCR @ ."  SCR # " . CR CR CR
    L/SCR 0 DO I 3 .R CR LOOP ;
: LINE. DUP SCR @ (LINE) DROP S_H @ 1D * +
    SWAP 28 * 7C +  23 VMBW ;
: UP#  DUP 3 + SWAP DO I 0A .R LOOP ."  " 4 2 AT ;
: +.0  3 0 DO ." +....0...." LOOP ;
: CLIST L/SCR 0 DO I LINE. LOOP ;
: LLIST  4 1 AT       1 UP# ." ...."  +.0 ." +" CLIST ;
: RLIST  4 1 AT ." 3" 4 UP# ." 0...." +.0       CLIST ;
-->
 
SCR# 21
 
( SCREEN EDITOR - 40 column display 20SEP85*Michal Jaegermann )
: ELIST  0 SCRN_START ! BASE->R DECIMAL HLIST S_H @
           IF RLIST ELSE LLIST ENDIF    R->BASE ;
 
: .CUR    COL S_H @ 2DUP 6 * 22 - MINUS >
    IF 0= 19 ELSE -4 ENDIF SWAP
    IF 1 S_H @ - S_H ! ELIST ENDIF - CUR C/L / 3 + AT ;
: ED>   2F8 SCRN_START ! PAGE 0 IN ! ;
: >ED   0   SCRN_START !  ELIST .CUR ;
: /INS/ 8F1 6 INS @ 1 OVER - INS ! 9C * 30 + VFILL ;
: FLIP  S_H @ -3A * 1D + +CUR .CUR ;
: BCK   NOP 2F8 DUP CURPOS ! C8 20 VFILL 8F1 6 84 VFILL
            R> DROP R> DROP ;  \ NOP is forward reference holder
 
    19 LOAD     \ load a generic (TI-Forth) editor
BASE ! ;S
 
SCR# 22
 
( SCREEN EDITOR - 64 column support 20SEP85*Michal Jaegermann )
0  CLOAD EDIT    BASE @ HEX      39 CLOAD LINE   33 CLOAD TEXT
             36 CLOAD GRAPHICS2  37 CLOAD SPLIT  41 CLOAD CLIST
VOCABULARY EDITOR IMMEDIATE  EDITOR DEFINITIONS       3 WIDTH !
2 VARIABLE S_H              \ value for conditional compilation
: CINIT 3800 DUP ' SATR !  DUP ' SPDTAB ! 800 / 6 VWTR
  D000 SP@ SATR 2 VMBW DROP
  0000 0000 0000 0000 1 SPCHAR  0 1 F 5 0 SPRITE
  0000 0000 0000 00E0 2 SPCHAR
  E040 4040 4040 E000 3 SPCHAR
  0 1000 10 VFILL   0D 7 VWTR ; \ colors of fg (10) and bg (0D)
0 VARIABLE RATE      0 VARIABLE CLE           0 VARIABLE INS
: BLINK RATE @ DUP 1 RATE +!  4 = IF -4 RATE ! ENDIF
    0< IF 1 0 SPRPAT ELSE 2 INS @ + 0 SPRPAT ENDIF
                          180 0 DO LOOP ;
  18 LOAD  -->  \ load cursor positions and auto-repeating key
 
SCR# 23
 
( SCREEN EDITOR - 64 column support 20SEP85*Michal Jaegermann )
: LINE. DUP SCR @ (LINE) ROT SMASH VMBW ;                  HEX
: HLIST BASE->R DECIMAL 0 0 AT ."  SCR # " . CR R->BASE ;
: .CUR CUR C/L /MOD 3 SLA 1+ SWAP 2+ 2 SLA 8 MAX SWAP
     0 SPRPUT ;
 
: /INS/  1 INS @ - INS ! ;
: ELIST  SCR @ DUP HLIST CLIST 30F0 8 0 VFILL ;
: FLIP   0 !CUR .CUR ; \ this is HOME - name for compatibility
: EOUT   1 0 SPRPAT 30F0 8 ROT VFILL CLS SCR @ HLIST 0 IN ! ;
: ED>    66 EOUT ;
: >ED       .CUR ;
: BCK    NOP FF EOUT QUIT ;  \ NOP is a forward reference holder
 
   19 LOAD     \ load a generic (TI-Forth) editor
BASE !  ;S
 
SCR# 24
 
( SCREEN EDITOR - cursor & auto/rpt  6OCT85*Michal Jaegermann )
HEX     0 CONSTANT CUR
: MAP  SCR @ BLOCK + ;
: *CUR CUR MAP       ;
: !CUR 0 MAX B/BUF 1- MIN ' CUR ! ;
: +CUR CUR + !CUR ;
: &ROW  CUR C/L MINUS AND ;  : NXT   &ROW C/L + MAP ;
: COL  CUR C/L 1- AND ;
 
: RKEY  BLINK ?KEY -DUP  IF CLE @ 1 CLE +!  \ waits long enough?
   0< IF 837C C@                            \ new key pressed  ?
        IF -18 CLE ! ELSE DROP [ LATEST PFA ]  LITERAL >R ENDIF
      ELSE -1 CLE ! ENDIF                   \ time out - accept
  ELSE -18 CLE ! [ S_H @ IN +! (       \ conditional compilation
  LATEST PFA ]  LITERAL >R ENDIF ;     ;S
  ) ]   KEY ENDIF ;             \ wait here for some key pressed
 
SCR# 25
 
( SCREEN EDITOR - marking & deleting 9OCT85*Michal Jaegermann )
: REDRAW ELIST .CUR UPDATE ;                              HEX
: RELINE 0D EMIT  CUR C/L / LINE. UPDATE ;
    1 VARIABLE UNMARKED
: MARKOUT DUP C@ 7F AND SWAP C! ;
: MARK *CUR DUP DUP C@ 80 OR SWAP C! UNMARKED @
   IF NXT 1- 0 UNMARKED ! ELSE SWAP MARKOUT ENDIF ELIST .CUR ;
: ~MARK UNMARKED @ 0=
     IF 2 0 DO MARKOUT LOOP 1 UNMARKED ! ELIST .CUR ENDIF ;
: MYMARKS UNMARKED @ IF *CUR NXT ELSE 2DUP ~MARK
         2DUP > IF SWAP ENDIF 1+ ENDIF ;   \ unsigned comparison
: RIP PAD C/L BLANKS OVER - C/L MIN PAD SWAP CMOVE> ;
: YANK MYMARKS RIP ;
: ~TAIL  MYMARKS 2DUP RIP OVER - BLANKS REDRAW ;
: ~LINE  ~MARK &ROW !CUR MYMARKS 2DUP RIP SWAP OVER
    B/BUF MAP - MINUS CMOVE 3C0 MAP C/L BLANKS REDRAW ;     -->
 
SCR# 26
 
( SCREEN EDITOR - moving text  17SEP85*Michal Jaegermann ) HEX
: ROOM   >R OVER + R> OVER - CMOVE> ;     ( from cnt below -- )
: MDOWN   &ROW DUP !CUR MAP DUP C/L B/BUF MAP ROOM ;
: OLINE   MDOWN C/L BLANKS  REDRAW ;
: RWORD *CUR DUP BL ENCLOSE DROP SWAP DROP + BL ENCLOSE
   DROP DROP + - MINUS +CUR ; \ move cursor to the next word
: RMOVE RWORD .CUR ;
 
: ILINE  INS @
    IF PAD BL ENCLOSE DROP DROP C/L OVER - >R + R>
    -TRAILING 1+ *CUR SWAP 2DUP
       NXT DUP >R ROOM OVER R> - MINUS MIN CMOVE
    ELSE  MDOWN PAD SWAP C/L CMOVE  ENDIF         REDRAW ;
: DEL BL NXT DUP *CUR DUP 1+ DUP >R SWAP ROT R> - CMOVE
    1- C! RELINE .CUR ;
-->
 
SCR# 27
 
( SCREEN EDITOR - cursor control    17SEP85*Michal Jaegermann )
: +.CUR     +CUR .CUR ;
: NWL C/L COL - +.CUR ;
: LEFT       -1 +.CUR ;   : RIGHT       1 +.CUR ;
: UP C/L MINUS  +.CUR ;   : DOWN      C/L +.CUR ;
 
: PUTCHAR *CUR INS @ IF DUP 1 NXT ROOM ENDIF C!
                RELINE 1 +.CUR ;
 
: NEWSCR CLS COL 22 > S_H ! ELIST .CUR ;
: +SCR SCR @ 1+ DISK_HI @ < SCR +! 0 !CUR NEWSCR ;
: -SCR SCR @ 1- 0 MAX       SCR  ! 0 !CUR NEWSCR ;
 
: ED.XE            1 ED> R> DROP ; IMMEDIATE
: ONEDO *CUR       0 ED> R> DROP ;
: TWODO *CUR RWORD 0 ED> R> DROP ;                      -->
 
SCR# 28
 
( SCREEN EDITOR - execution table   30SEP85*Michal Jaegermann )
: DOIT  IF  QUERY ELSE ( adr -- ) RWORD *CUR OVER - 1-
         C/L MIN TIB @ 2DUP + 0 SWAP C! SWAP CMOVE> ENDIF
    INTERPRET .S >ED ; IMMEDIATE
 
: WAIL   7 EMIT ;
 
' WAIL  CFA  DUP VARIABLE ACTION             ' ~TAIL CFA , \ 01
        DUP ,  ' DEL   CFA ,  ' /INS/ CFA ,  ' -SCR  CFA , \ 05
' ILINE CFA ,  ' ~LINE CFA ,  ' LEFT  CFA ,  ' RIGHT CFA , \ 09
' DOWN  CFA ,  ' UP    CFA ,  ' RIGHT CFA ,  ' NWL   CFA , \ 0D
' FLIP  CFA ,  ' BCK   CFA ,          DUP ,  ' TWODO CFA , \ 11
' RMOVE CFA ,          DUP ,          DUP ,  ' ~MARK CFA , \ 15
        DUP ,  ' ONEDO CFA ,  ' +SCR  CFA ,  ' YANK  CFA , \ 19
' MARK  CFA ,  ' ED.XE CFA ,          DUP ,          DUP , \ 1D
' OLINE CFA ,              ,                 -->           \ 1F
 
SCR# 29
 
( SCREEN EDITOR - main loop & entry 30SEP85*Michal Jaegermann )
                                                          HEX
: EDI   BEGIN RKEY DUP BL <
   IF 1 SLA ACTION + @ EXECUTE ELSE PUTCHAR ENDIF AGAIN ;
 
' ~MARK CFA ' BCK !  1F WIDTH !  FORTH DEFINITIONS
 
: ER EDITOR      [ S_H @ IN +! ( ]     \ conditional compilation
    VDPMDE @ 5 = 0= IF SPLIT CINIT ENDIF          [ 2 IN +! ) ]
 1 INS ! /INS/ NEWSCR BEGIN EDI [COMPILE] DOIT AGAIN ;
 
: WHERE EDITOR SCR ! 2- !CUR ER ;
: ED@   EDITOR       0  !CUR ER ;
: EDIT         SCR !        ED@ ;
 
;S
 
 --------------------- that's all -----------------------------
 Michal Jaegermann
 ...ihnp4!alberta!myrias!mj
