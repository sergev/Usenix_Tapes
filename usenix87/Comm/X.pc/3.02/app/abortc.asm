	  title   ABORTC.ASM  Initiate execution of a Lattice C program
	  subttl  As adapted by Curt Truman for ^C/^BREAK handling.
	  name	  ABORTC
	  page	  60,132
; ------------------------------------------------------------------------
;	  filename = abortc.asm     (rewrite of 'c.asm')
;
; This file contains entry and exit routines for the standard in/out programs.
; A special 'C.OBJ' was required to handle control-C and control-Break input.
; The file can be LINKed instead of 'C.OBJ'.  Check the integer 'abort' to
; see if the control-C or control-Break keys have been hit (abort = TRUE).
; ------------------------------------------------------------------------
;
; XCMAIN  This is the main module for a C program on the MS-DOS implementation.
;	  It initializes the segment registers, sets up the stack, and calls
;	  the C main function _main with a pointer to the remainder of the
;	  DOS command line.
;
; XCEXIT  Is called by _main when the user function 'main' returns. This
;	  routine restores the original control-C and control-Break address,
;	  sets the stack back to original size, and does far return to caller.
;
; CNTRL_BRK  is called by DOS when the user hits a control-break or control-C.
;	  The routine just sets the word 'abort' to TRUE (1), and returns.
; ----------------------------------------------------------------------------
;    date  chg	   by	 Reason
;  8/24/84  01	   curt  Initial conversion of normal 'C' startup file.
; 10/16/84  02	   curt  CNTRL_BRK needs to use proper DS value!!
; ----------------------------------------------------------------------------
CTRBRK_VEC  EQU  23h		;control-break vector
DRIVER_VEC  EQU  7Ah		;XPC driver vector
DOS	    equ  21h
DISP_STRG   equ  09h		;DOS	  function
DOS_VERSION equ  30h		;DOS	  function
SET_VECT    equ  25h		;DOS	  function
GET_VECT    equ  35h		;DOS	  function
DOS_EXIT    equ  4Ch		;DOS	  function
	  subttl  DOS MACRO supplied by Lattice C
;	  page+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  include DOS.MAC
	  subttl  Segment grouping statements
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; CODE/DATA segment grouping declarations, depending upon memory model chosen.
; ----------------------------------------------------------------------------
	  IF	  S8086
pgroup	  group   base,prog,tail
base	  segment word public 'prog'
	  dw	  0
_ABORT_DS dw	  0		  ; data segment of _ABORT variable.
base	  ends
	  ENDIF

	  IF	  P8086
base	  segment word
	  dw	  1
base	  ends
	  ENDIF

	  IF	  D8086
cgroup	  group   base,code
base	  segment word public 'code'
	  dw	  2
base	  ends
	  ENDIF

	  IF	  L8086
base	  segment word
	  dw	  3
base	  ends
	  ENDIF

	  IF	  LPROG
	  extrn   _MAIN:far
	  extrn   RBRK:far
ECODE	  equ	  4		  ; offset of error code for XCEXIT
	  ELSE
ECODE	  equ	  2		  ; offset of error code for XCEXIT
	  ENDIF

TAB	  equ	  09H			  ; tab character

dgroup	  group   data,stack
	  subttl  data segment declaration
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The data segment defines locations which contain the offsets of the stack
; base and top.  It also contains the variable '_ABORT', which is set to TRUE
; (1) if the ^C or ^BREAK key is hit by the PAT.
; ----------------------------------------------------------------------------
data	  segment para public 'DATA'
	  extrn  _STACK:word
	  public  _VER,_TOP,_BASE,_INAME,_ONAME,_PSP,_MBASE,_MNEXT,_MSIZE
	  public  _ENV,_DOS,_TSIZE,_ESIZE,_SS,_SP,_ABORT,_OSERR
NULL	  dw	  0
_VER	  db	  "Lattice C 2.1"
_DOS	  db	  0		  ; DOS major version number
	  db	  0		  ; DOS minor version number
_SS	  dw	  0		  ; stack segment number
_SP	  dw	  0		  ; SP reset value
_TOP	  dw	  0		  ; top of stack (relative to SS)
_BASE	  dw	  offset dgroup:sbase	  ; base of stack (relative to DS)
_INAME	  db	  32 DUP(0)	  ; input file name
_ONAME	  db	  32 DUP(0)	  ; output file name
_PSP	  dd	  0		  ; program segment prefix pointer
_MBASE	  dd	  0		  ; base of memory pool
_MNEXT	  dd	  0		  ; next available memory location
_MSIZE	  dd	  0		  ; number of bytes left in pool
_TSIZE	  dw	  0		  ; total size in paragraphs
_ENV	  dd	  0		  ; pointer to environment
_ESIZE	  dw	  0		  ; environment size in bytes
_ABORT	  dw	  0		  ; this word set to TRUE when ^c has been hit.
_OSERR	  dw	  0		  ; Operating system error code (v 2.15 Lattice)
CTRL_ADDR dd	  0		  ; startup stores original ^C address here.

STKERR	  db	  "Invalid stack size",0DH,0AH,"$"
NAMERR	  db	  "Invalid I/O redirection",0DH,0AH,"$"
MEMERR	  db	  "Insufficient memory",0DH,0AH,"$"
OVFERR	  db	  "*** STACK OVERFLOW ***",0DH,0AH,"$"
DATA	  ends
	  subttl  stack/program segment declaration
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; The stack segment is included to prevent the warning from the
; linker, and also to define the base (lowest address) of the stack.
; ----------------------------------------------------------------------------
STKRSV	  equ	  128		  ; reserved stack size
STKMIN	  equ	  512		  ; minimum run-time stack size
	  IF	  COM
stack	  segment 'data'
	  ELSE
stack	  segment stack 'data'
	  ENDIF
SBASE	  db	  STKRSV dup (?)
stack	  ends




	  IF	  S8086
prog	  segment byte public 'prog'
	  assume  cs:pgroup
	  ENDIF

	  IF	  D8086
code	  segment byte public 'code'
	  assume  cs:cgroup
	  ENDIF

	  IF	  P8086
_code	  segment byte
	  assume  cs:_code
	  ENDIF

	  IF	  L8086
_prog	  segment byte
	  assume  cs:_prog
	  ENDIF

	  assume  ds:dgroup

	  IF	  LPROG EQ 0
	  extrn   _MAIN:near
	  extrn   RBRK:near
	  ENDIF
	  subttl  Main line code of startup logic
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The main program must set up the initial segment registers and the stack
; pointer, and set up a far return to the MS-DOS exit point at ES:0.  The
; command line bytes from the program segment prefix are moved onto the stack,
; and a pointer to them supplied to the C main module _main (which calls main).
; COM file organization must start at 0FCh in order to satisfy EXE2BIN program,
; and is 0FCh because BASE segment has 2 WORDs defined in it (code starts 100h).
; ----------------------------------------------------------------------------
	  IF	  COM
	  org	  0FCh
	  ENDIF

	  public  C
C	  proc	  far
	  cli
	  IF	  COM
	  lea	  ax,pgroup:tail       ; set up DS/SS for .COM file
	  add	  ax,16
	  mov	  cl,4
	  shr	  ax,cl
	  mov	  bx,cs
	  add	  ax,bx
	  mov	  ds,ax
	  mov	  ss,ax
	  mov	  sp,offset dgroup:SBASE+STKRSV
	  ELSE
	  mov	  ax,dgroup	       ; set up DS/SS for .EXE file
	  mov	  ds,ax
	  mov	  ax,STACK
	  mov	  ss,ax
	  mov	  sp,STKRSV
	  ENDIF

	  sti
	  mov	  ah,DOS_VERSION       ; get DOS version number
	  int	  DOS
	  or	  al,al
	  jnz	  m0
	  mov	  ax,1		       ; assume 1.00 if null return
m0:	  mov	  word ptr _DOS,ax
	  mov	  word ptr _PSP+2,es	       ; set up ptr. to prog seg prefix
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; Look at the environment area for information about this run.
; ----------------------------------------------------------------------------
	  cmp	  _DOS,2
	  jl	  m01
	  mov	  ax,es:[2Ch]
	  mov	  word ptr _ENV+2,ax	       ; set up pointer to environment
	  or	  ax,ax
	  jz	  m01		       ; branch if null
	  push	  es		       ; determine number of bytes
	  cld
	  les	  di,_ENV
	  xor	  ax,ax
	  mov	  cx,7FFFh
m0a:	  repnz   scasb
	  jnz	  m0b
	  scasb
	  jnz	  m0a
	  inc	  di
	  and	  di,0FFFEh
	  mov	  _ESIZE,di
m0b:	  pop	  es

; ----------------------------------------------------------------------------
; Examine command line: look for stdin/stdout, stack size, other params.
; ----------------------------------------------------------------------------
m01:	  mov	  si,80h	       ; check command line in PSP area.
	  mov	  cl,es:[si]
	  xor	  ch,ch
	  jcxz	  m12		       ; branch if null
m1:	  inc	  si		       ; scan for first arg
	  mov	  al,es:[si]
	  cmp	  _dos,2
	  jge	  m10
	  cmp	  al,'<'
	  je	  m2		       ; branch if input file name
	  cmp	  al,'>'
	  je	  m3		       ; branch if output file name
m10:	  cmp	  al,'='
	  je	  m4		       ; branch if stack size
	  cmp	  al,' '
	  je	  m11		       ; branch if white space
	  cmp	  al,tab
	  jne	  m12		       ; branch if normal arg
m11:	  dec	  cx
	  jg	  m1
	  xor	  cx,cx
m12:	  jmp	  short m5	       ; branch if no args found
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; Get input file name variable address, so can move from environment area, OR,
; get output file name variable address, so can move from environment area.
; ----------------------------------------------------------------------------
m2:	  lea	  di,dgroup:_INAME
	  jmp	  short m31
m3:	  lea	  di,dgroup:_ONAME

; ----------------------------------------------------------------------------
; Save file name in data area, using address setup in DI register.
; ----------------------------------------------------------------------------
m31:	  xor	  ah,ah
m32:	  dec	  cx
	  jz	  m33
	  inc	  si
	  mov	  al,es:[si]
	  cmp	  al,' '
	  jz	  m33
	  cmp	  al,tab
	  jz	  m33
	  mov	  ds:[di],al
	  inc	  di
	  inc	  ah
	  cmp	  ah,32
	  je	  m34
	  jmp	  short m32
m33:	  mov	  byte ptr ds:[di],0
	  jmp	  short m11
m34:	  lea	  dx,dgroup:namerr
	  jmp	  ABORT
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; Get stack size from command line, put into control variables.
; ----------------------------------------------------------------------------
m4:	  xor	  bx,bx
m41:	  dec	  cx
	  jz	  m42
	  inc	  si
	  mov	  al,es:[si]
	  cmp	  al,' '
	  je	  m42
	  cmp	  al,tab
	  je	  m42
	  sub	  al,'0'
	  jl	  m43
	  cmp	  al,9
	  jg	  m43
	  add	  bx,bx
	  jc	  m43
	  mov	  dx,bx
	  add	  bx,bx
	  jc	  m43
	  add	  bx,bx
	  jc	  m43
	  add	  bx,dx
	  jc	  m43
	  xor	  ah,ah
	  add	  bx,ax
	  jc	  m43
	  jmp	  short m41
m42:	  or	  bx,bx
	  jz	  m43			    ; branch if stack size arg is null.
	  mov	  _STACK,bx
	  jmp	  short m11
m43:	  lea	  dx,dgroup:stkerr
	  jmp	  ABORT
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; Set up the stack, now that know what the size desired is.
; ----------------------------------------------------------------------------
	  IF	  LDATA EQ 0
m5:	  mov	  ax,_ESIZE	       ; reserve space for environment
	  ELSE
m5:	  xor	  ax,ax
	  ENDIF
	  mov	  bx,_STACK	       ; get stack size
	  shr	  bx,1		       ; make size even
	  add	  bx,bx
	  cmp	  bx,stkmin
	  ja	  m51
	  mov	  bx,stkmin	       ; use default if too small
	  mov	  _STACK,bx
m51:	  add	  bx,ax 	       ; add environment size
	  jc	  m54		       ; ABORT if overflow
	  mov	  dx,es:2	       ; compute available paragraphs
	  mov	  ax,ss
	  sub	  dx,ax
	  test	  dx,0F000h
	  jnz	  m52		       ; branch if greater than 64Kbytes
	  shl	  dx,1		       ; convert to bytes
	  shl	  dx,1
	  shl	  dx,1
	  shl	  dx,1
	  jmp	  short m53
m52:	  mov	  dx,0FFF0h	       ; use largest value
	  if	  LDATA
m53:	  cmp	  dx,bx 	       ; check if stack will fit
	  ja	  m55
	  ELSE
m53:	  add	  bx,_BASE	       ; check if stack will fit
	  jc	  m54
	  cmp	  dx,ax
	  ja	  m55
	  ENDIF
m54:	  lea	  dx,dgroup:memerr     ; ABORT if insufficient memory
	  jmp	  ABORT
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The stack fits, so wear it.
; ----------------------------------------------------------------------------
m55:	  cli
	  mov	  _TOP,bx	       ; set top-of-stack
	  mov	  sp,bx 	       ; set stack pointer
	  IF	  LDATA EQ 0
	  mov	  ax,ds
	  mov	  ss,ax
	  ENDIF
	  mov	  _SS,ss
	  sti

; ----------------------------------------------------------------------------
; Set up memory allocation pointers
; ----------------------------------------------------------------------------
	  push	  cx		       ; save command byte count
	  add	  bx,15 	       ; compute mem pool base segment number
	  mov	  cl,4
	  shr	  bx,cl
	  mov	  ax,ss
	  add	  ax,bx
	  mov	  word ptr _MBASE+2,ax
	  mov	  word ptr _MNEXT+2,ax
	  mov	  bx,es:2	       ; get top segment number
	  sub	  bx,ax 	       ; compute memory pool size
	  jbe	  m54		       ; branch if insufficient memory
	  mov	  cl,4		       ; compute number of bytes
	  rol	  bx,cl
	  mov	  ax,bx
	  and	  ax,15
	  and	  bx,0FFF0h
	  mov	  word ptr _MSIZE,bx
	  mov	  word ptr _MSIZE+2,ax
	  call	  RBRK		       ; reset memory pool
	  or	  ax,ax
	  jnz	  m54
	  pop	  dx		       ; restore command line length

; ----------------------------------------------------------------------------
; Put return address at top of stack
; ----------------------------------------------------------------------------
	  push	  es		       ; return is to 1st word of prog prefix
	  xor	  ax,ax
	  push	  ax
	  mov	  bp,sp 	       ; BP contains stack linkage
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; Move environment to stack for small data models
; ----------------------------------------------------------------------------
	  IF	  LDATA EQ 0
	  les	  di,_ENV
	  mov	  di,_ESIZE
m6:	  sub	  di,2
	  jl	  m61
	  push	  es:[di]
	  jmp	  short m6
m61:	  mov	  es,word ptr _PSP+2
	  mov	  word ptr _ENV,sp
	  mov	  word ptr _ENV+2,ss
	  ENDIF

; ----------------------------------------------------------------------------
; copy command line to stack
; ----------------------------------------------------------------------------
	  mov	  bx,dx 	       ; get residual command line length
	  mov	  cx,dx
	  add	  bx,4		       ; 3 bytes additional, 1 for rounding
	  and	  bx,0FFFEh	       ; force even number of bytes
	  sub	  sp,bx 	       ; allocate space on stack
	  mov	  di,sp
	  mov	  byte ptr ss:[di],'c' ; store dummy program name
	  inc	  di
	  jcxz	  m8		       ; skip if no bytes to move
	  mov	  byte ptr ss:[di],' '
	  inc	  di
m7:	  mov	  al,es:[si]	       ; move bytes to stack
	  mov	  ss:[di],al
	  inc	  si
	  inc	  di
	  loop	  m7
m8:	  mov	  byte ptr ss:[di],0   ; append null byte
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; after setup control break stuff, set up arguments for '_main' and call it.
; ----------------------------------------------------------------------------
	  cmp	  _dos,2
	  jl	  k9
	  push	  es		  ; save the current es register value
	  mov	  ax,GET_VECT*256+CTRBRK_VEC	;get old ^C and ^break address
	  int	  DOS		  ; please.
	  mov	  word ptr CTRL_ADDR,es	  ; store returned vlaue
	  mov	  word ptr CTRL_ADDR+2,bx  ;
	  pop	  es

	  lea	  bx,pgroup:_ABORT_DS
	  mov	  pgroup:[bx],ds  ; save off ds value for interrupt routine.
	  push	  ds
	  push	  cs		  ; move cs value into ds register
	  pop	  ds
	  mov	  dx,offset pgroup:CNTRL_BRK
	  mov	  ax,SET_VECT*256+CTRBRK_VEC	;set the conrol-break address
	  int	  DOS
	  pop	  ds		  ; restore the proper value of ds register.

k9:	  mov	  _SP,sp	       ; save stack pointer reset value
	  mov	  ax,sp 	       ; push pointer to command line
	  IF	  LDATA
	  push	  ss
	  ENDIF

	  sti
	  push	  ds		       ; make ES same as DS
	  pop	  es
	  push	  ax		;argument - pointer to command line
	  call	  _MAIN 	       ; call 'c' main-line code, calls 'main'.

; ----------------------------------------------------------------------------
; We're BACK!!  if running DOS 2.0 or greater, terminate control-break stuff.
; ----------------------------------------------------------------------------
	  cmp	  _dos,2
	  jl	  m9
	  sti
	  mov	  dx, word ptr CTRL_ADDR+2  ; better move before mangle DS register!!!
	  mov	  ax, word ptr CTRL_ADDR
	  mov	  ds,ax
	  mov	  ax,SET_VECT*256+CTRBRK_VEC	;set the conrol-break address
	  int	  DOS			;no need to save/restore DS for exit

	  mov	  ax,DOS_EXIT*256+0	;no error
	  int	  DOS			;exit with zero code
m9:	  mov	  sp,bp 		;restore ptr to far return
	  ret				;return to MS-DOS
	  subttl  abort, control-break and stack overflow routines
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; name	  ABORT label (near, still within 'C' proc)
; description:
; This label is jumped to by routines in this file that find bad error state.
; ----------------------------------------------------------------------------
ABORT:	  mov	  ah,DISP_STRG	       ; want to display error message
	  int	  DOS
	  cmp	  _dos,2
	  jl	  a1
	  mov	  ax,DOS_EXIT*256+1	;error return code 1
	  int	  DOS
a1:	  push	  es
	  xor	  ax,ax
	  push	  ax
	  ret
C	  endp

; ----------------------------------------------------------------------------
; name	  CNTRL_BRK
; description:
; This routine will be branched to (hopefully) when the user enters control-C
; or control-BREAK at the keyboard.  No action is taken, so that keyboard
; input will hopefully catch the character and continue on as is.  MUST set
; up the data segment register for the _ABORT routine, else will use current.
; ----------------------------------------------------------------------------

CNTRL_BRK proc	  near
	  sti
	  push	  bx
	  push	  ds

	  lea	  bx,pgroup:_ABORT_DS
	  mov	  ds,pgroup:[bx]
	  mov	  _ABORT,1	  ; store TRUE in the variable.
	  pop	  ds
	  pop	  bx
	  iret			  ; just trap out ^break and ^C and return
CNTRL_BRK endp



; ----------------------------------------------------------------------------
; name	  XCOVF -- terminate execution after stack overflow
;
; description:
; This entry point is reached from a C module when its entry sequence detects
; a stack overflow.  Control is passed to XCOVF by a direct jump.
; ----------------------------------------------------------------------------
	  public  XCOVF
XCOVF	  proc	  far
	  mov	  ax,_TOP
	  sub	  ax,4
	  mov	  sp,ax 	       ; reset stack pointer for exit
	  lea	  dx,dgroup:ovferr     ; load offset of error message
	  mov	  ah,DISP_STRG
	  int	  DOS		       ; display error message
	  cmp	  _DOS,2
	  jl	  ovf1
	  mov	  ax,DOS_EXIT*256+1	;error return code 1
	  int	  DOS		       ; exit, also return error code.
ovf1:	  ret
XCOVF	  endp
	  subttl  Exit routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; name	  XCEXIT -- terminate execution of C program
;
; description
; This function terminates execution of the current program by returning to
; MS-DOS.  The error code argument normally supplied to XCEXIT is ignored
; in this implementation.
;
; ----------------------------------------------------------------------------
	  public  XCEXIT
XCEXIT	  proc	  far
	  cmp	  _DOS,2
	  jl	  exit1
	  cli
	  mov	  ax, word ptr CTRL_ADDR
	  mov	  ds,ax
	  mov	  dx, word ptr CTRL_ADDR+2
	  mov	  ax,SET_VECT*256+CTRBRK_VEC	;restore ^C and ^BREAK address
	  int	  DOS			;no need to save/restore DS for exit

	  mov	  bp,sp
	  mov	  ax,[bp+ecode]        ; get error code parameter from stack
	  mov	  ah,DOS_EXIT	       ;no error
	  int	  DOS		       ; exit, returning error code
exit1:	  mov	  ax,_TOP
	  sub	  ax,4
	  mov	  sp,ax
	  ret			       ; far return to DOS, no error code.
XCEXIT	  endp




; ----------------------------------------------------------------------------
; Dummy segment to establish top of program for small program models
; ----------------------------------------------------------------------------
	  IF	  S8086
TAIL	  segment word 'PROG'
	  dw	  -1
TAIL	  ends
	  ENDIF

	  ENDPS
	  END	  C
