	  title   CIO_CTRL.ASM	Character Input/Output routines
	  page	  60,132
; ------------------------------------------------------------------------
;		  Character Input/Output Routine
;
; filename = cio_ctrl.asm
;
; cbrk_inp   Gets a character from standard in, includes ^C checking.
; cbrk_out   Puts a character to standard out, includes ^C checking.
; nbrk_out   Puts a character to standard out, no ^C checking.
; nbrk_inp   Returns TRUE if a character is ready for input, no ^C checking.
;
; wn_curcl   Returns the current column number of the cursor.
; wn_currw   Returns the current row number of the cursor.
;
; wn_setsz   initializes the rows/columns to use when wn_dchar is called.
; wn_setdf   initializes the rows/columns to the standard 1-24, 1-80 settings.
;
; wn_dchar   displays a character, with scroll control in current window.
; wn_dstrg   displays a string, with scroll control in current window.
; wn_clear   Blanks out the current window, leaving cursor at (0, 0).
; ------------------------------------------------------------------------
;   Date    Change#   by     Reason
; 08/19/84     01    curt    Initial version of object module.
; ------------------------------------------------------------------------
VIDEO	  EQU	10h	;supervisor call to BIOS.  output to display
STCUR_POS EQU	2
RDCUR_POS EQU	3
SCRL_UP   EQU	6
DISP_ATTR EQU	9
WRITE_CUR EQU	0Ah
DOS	  EQU	21h	;supervisor call to DOS
STND_OUT  EQU	2	;output via ANSI.SYS (it must be loaded)
CONSOL_IO EQU	6	;if (dx)=FF, input no wait, else output
CONINP_WT EQU	8	;input wait, no echo (does NOT use ANSI.SYS)
ENDSTRING EQU	  00h
LINEFEED  EQU	  0Ah
CRETURN   EQU	  0Dh
BACKSP	  EQU	  08h
SPACE	  EQU	  20h
DEF_FCOL  EQU	  0
DEF_FROW  EQU	  0		       ; These are default values for control.
DEF_LCOL  EQU	  79
DEF_LROW  EQU	  23

; ----------------------------------------------------------------------------
; The dgroup items are private.  The variables 'first...' and 'last...' are
; control variables for scrolling region, they define the current window.
; ----------------------------------------------------------------------------
dgroup	  group   data
data	  segment byte public 'data'
	  assume  DS:dgroup
firstcol  db	  DEF_FCOL	       ; Row/Column are zero based values...
firstrow  db	  DEF_FROW
lastcol   db	  DEF_LCOL
lastrow   db	  DEF_LROW
data	  ends
	  subttl  control-break checking console Input/Output routines
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; This structure is used by all routines, defines map into memory for stack.
; ----------------------------------------------------------------------------
stkstr	  struc
oldbp	  dw	  ?
retrn_ip  dw	  ?
dchar	  dw	  ?
param2	  dw	  ?
param3	  dw	  ?
param4	  dw	  ?
stkstr	  ends


pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup
; ----------------------------------------------------------------------------
; This function returns a character input from standard input device.  If there
; is no character waiting, the DOS routine called will wait for one.  The DOS
; routine will also check the input character for ^C processing.
; ----------------------------------------------------------------------------
	  public  cbrk_inp
cbrk_inp  proc	  near
	  push	  bp

	  mov	  ah,CONINP_WT		;DOS call to get key input, check ^C.
	  int	  DOS
	  sti				;re-enable interupts.
	  xor	  ah,ah 		;just in case DOS sloppy in use.

	  pop	  bp
	  ret
cbrk_inp  endp



; ----------------------------------------------------------------------------
; This function outputs a character to the standard output device.  The DOS
; routine will also check the input buffer for ^C processing.
; ----------------------------------------------------------------------------
	  public  cbrk_out
cbrk_out  proc	  near
	  push	  bp
	  mov	  bp,sp

	  mov	  ax,[bp].dchar 	;get character to output into al.
	  mov	  ah,STND_OUT		;DOS call to output char., check ^C.
	  int	  DOS
	  sti

	  pop	  bp
	  ret
cbrk_out  endp
	  subttl  NON-control-break checking console Input/Output routines
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; Returns ah=1 if no character read. If ah=0, al=char.	No ^C checking done.
; ----------------------------------------------------------------------------
	  public  nbrk_inp
nbrk_inp  proc	  near
	  push	  bp
	  push	  dx

	  mov	  dl,0FFh
	  mov	  ah,CONSOL_IO		;DOS call to check keyboard.
	  int	  DOS

	  sti
	  jz	  nochar		;no - return NULL
	  xor	  ah,ah 		;return input in AL
	  jmp	  short gotchar
nochar:   mov	  ah,1			;256 means nothing input
	  xor	  al,al

gotchar:  pop	  dx
	  pop	  bp
	  ret
nbrk_inp  endp



; ----------------------------------------------------------------------------
; This function outputs a character to the standard output device.  The DOS
; routine will not check the input buffer for ^C processing.
; ----------------------------------------------------------------------------
	  public  nbrk_out
nbrk_out  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  dx

	  mov	  dx,[bp].dchar 	;get character to output into dl.
	  mov	  ah,CONSOL_IO		;DOS call to output char., ignore ^C.
	  int	  DOS

	  sti
	  pop	  dx
	  pop	  bp
	  ret
nbrk_out  endp
	  subttl  current column/row reporting routines
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; This routine returns a 1-based current column number to the caller.
; ----------------------------------------------------------------------------
	  public  wn_curcl
wn_curcl  proc	  near
	  push	  bp

	  xor	  bh,bh 	       ; all setting done for page zero.
	  mov	  ah,RDCUR_POS	       ; read current cursor position,
	  int	  VIDEO 	       ; leaves row in dh, col in dl.
	  sti
	  xor	  dh,dh
	  mov	  ax,dx
	  inc	  ax		       ; return 1-based column number.

	  pop	  bp
	  ret
wn_curcl  endp






; ----------------------------------------------------------------------------
; This routine returns a 1-based current row number to the caller.
; ----------------------------------------------------------------------------
	  public  wn_currw
wn_currw  proc	  near
	  push	  bp

	  xor	  bh,bh 	       ; all setting done for page zero.
	  mov	  ah,RDCUR_POS	       ; read current cursor position,
	  int	  VIDEO 	       ; leaves row in dh, col in dl.
	  sti
	  mov	  al,dh
	  xor	  ah,ah
	  inc	  ax		       ; return 1-based row number.

	  pop	  bp
	  ret
wn_currw  endp
	  subttl  routine to set up window size
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine allows the caller to redefine the size of the current window.
; The input values are expected in the following order (for C): first column
; of first row, first row, last column of last row, last row.  The values
; are expected to be based on rows 1-24, columns 1-80.	No checks are made on
; the ranges, they are taken to be specified correctly.
; ----------------------------------------------------------------------------
	  public  wn_setsz
wn_setsz  proc	  near
	  push	  bp
	  mov	  bp,sp

	  mov	  ax,[bp].dchar
	  dec	  al
	  mov	  firstcol,al	       ; 0 based upper left column #.

	  mov	  ax,[bp].param2
	  dec	  al
	  mov	  firstrow,al	       ; 0 based upper left row #.

	  mov	  ax,[bp].param3
	  dec	  al
	  mov	  lastcol,al	       ; 0 based lower right column #.

	  mov	  ax,[bp].param4
	  dec	  al
	  mov	  lastrow,al	       ; 0 based lower right row #.

	  pop	  bp
	  ret
wn_setsz  endp




; ----------------------------------------------------------------------------
; This routine will set the default sizes up for the caller.
; ----------------------------------------------------------------------------
	  public  wn_setdf
wn_setdf  proc	  near
	  mov	  firstcol,DEF_FCOL
	  mov	  firstrow,DEF_FROW
	  mov	  lastcol,DEF_LCOL
	  mov	  lastrow,DEF_LROW
	  ret
wn_setdf  endp
	  subttl  'inside the window' display character routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine accepts a character to be output, within the current window.
; If the character is line feed, the cursor is moved to the next line, and
; scrolling is done if necessary.  If the character is a carriage return, the
; cursor is moved to the left edge of the window, on the same line.  If the
; character is neither of the above, it is displayed.  After the character is
; displayed, the cursor is moved over one position.  If the movement causes
; the cursor to pass the right edge of the window, the cursor is moved down
; to the beginning of the next line.  If moving to the next line requires a
; scroll, that is done as well.
; ----------------------------------------------------------------------------
	  public  wn_dchar
wn_dchar  proc	  near
	  push	  bp
	  mov	  bp,sp

	  xor	  bx,bx 	       ; all setting done for page zero.
	  mov	  ah,RDCUR_POS	       ; read current cursor position,
	  int	  VIDEO 	       ; leaves row in dh, col in dl.

	  mov	  ax,[bp].dchar        ; get character to be displayed.
	  cmp	  al,LINEFEED	       ; compare to linefeed character, if not
	  je	  linefd	       ; Is LF: do scroll, or next line.
	  cmp	  al,CRETURN	       ; see if carriage return.
	  je	  newline	       ; yes, set cursor for next line
	  cmp	  al,BACKSP
	  jne	  donorm	       ; not backspace, just do call.

	  cmp	  dl,firstcol	       ; see if already at left edge, if so,
	  jle	  alldone	       ; just ignore the character.
	  dec	  dl		       ; move cursor over to left one pos.
	  mov	  ah,STCUR_POS
	  int	  VIDEO
	  mov	  al,SPACE	       ; now output a space.
	  push	  ax
	  call	  wn_dchar	       ; call recursively, output the char.
	  mov	  sp,bp 	       ; restore the stack after the call.
	  dec	  dl		       ; move cursor over to left once again.
	  jmp	  short movecurs       ; set cursor position logic.
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

donorm:   mov	  cx,1		       ; (only want to write one character),
	  mov	  ah,WRITE_CUR	       ; write a character at current position,
	  int	  VIDEO 	       ; using character in al.

	  inc	  dl		       ; want to move over 1 column, this row.
	  cmp	  dl,lastcol	       ; see if at last column to use.
	  jle	  movecurs	       ; not last, just set cursor position.
	  mov	  dl,firstcol	       ; want first column of next row.
linefd:   call	  nxtorscr	       ; next line or scroll screen.
	  jmp	  short alldone

newline:  mov	  dl,firstcol	       ; want left column, dh has current row.
movecurs: mov	  ah,STCUR_POS	       ; want to set cursor position.
	  int	  VIDEO

alldone:  pop	  bp
	  sti
	  ret
wn_dchar  endp








; ----------------------------------------------------------------------------
; This routine expects the offset of a string (within DS).  It outputs the
; characters in the string until the character to be output is '\0', or NULL.
; ----------------------------------------------------------------------------
	  public  wn_dstrg
wn_dstrg  proc	  near
	  push	  bp
	  mov	  bp,sp

	  cld			       ; want to go forward, please.
	  mov	  si,[bp].dchar        ; get offset of start of string.
nextchr:  lodsb 		       ; load up the byte pointed to by SI.
	  cmp	  al,ENDSTRING	       ; see if end-of-string character.
	  je	  done_str	       ; if so, exit.
	  push	  ax		       ; not end-of-string, display the char
	  call	  wn_dchar	       ; by calling char display routine.
	  mov	  sp,bp
	  jmp	  nextchr

done_str: pop	  bp
	  sti
	  ret
wn_dstrg  endp
	  subttl  next line or scroll window, plus clear window routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine implements the scrolling logic.	It is assumed that the cursor
; should be moved to the next line when the routine is called.	If moving the
; cursor would require a screen scroll, that is done.  The column of the cursor
; is not changed, only the row.  Note that this routine is NOT public.
; ----------------------------------------------------------------------------
nxtorscr  proc	  near
	  inc	  dh		       ; time to move to next line.
	  cmp	  dh,lastrow	       ; get lastrow allowed, see if beyond.
	  jle	  nextline	       ; no, just move to next line.

scrlit:   xchg	  bl,dl 	       ; need to save dl for nextline routine.
	  mov	  ch,firstrow
	  mov	  cl,firstcol
	  mov	  dh,lastrow
	  mov	  dl,lastcol	       ; scroll all entire window of lines.
	  mov	  al,1		       ; only want to scroll up one line atime.
	  mov	  bh,7		       ; set attribute to normal character.
	  mov	  ah,SCRL_UP	       ; set to scroll screen in window.
	  int	  VIDEO 	       ; did scroll, now move down a line.
	  xchg	  bl,dl 	       ; restore dl (current column value).

nextline: xor	  bh,bh 	       ; use page 0 please!
	  ;	    dl has current column #, dh has new row #.
	  mov	  ah,STCUR_POS	       ; want to set cursor position.
	  int	  VIDEO
scr_done: ret
nxtorscr  endp

; ----------------------------------------------------------------------------
; This routine makes a special-purpose call to the screen scrolling routine,
; to clear the current window to blanks.  The cursor is left in the upper left.
; ----------------------------------------------------------------------------
	  public  wn_clear
wn_clear  proc	  near
	  push	  bp		       ; DOS mangles this register!!!
	  mov	  ch,firstrow	       ; define the window limints for special
	  mov	  cl,firstcol	       ; scroll call that blanks the window.
	  mov	  dh,lastrow
	  mov	  dl,lastcol

	  mov	  bh,7		       ; attribute is normal character.
	  xor	  al,al 	       ; blank the entire window via scroll.
	  mov	  ah,SCRL_UP
	  int	  VIDEO 	       ; blank out the entire window.

	  xor	  bh,bh 	       ; now set cursor, use page 0 please!
	  mov	  dh,firstrow
	  mov	  dl,firstcol	       ; put cursor at top left corner of
	  mov	  ah,STCUR_POS	       ; the current window.
	  int	  VIDEO
	  sti
	  pop	  bp
	  ret
wn_clear  endp
prog	  ends
	  end
