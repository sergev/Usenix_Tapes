; New ANSI terminal driver.
; Optimized for speed in the case of multi-character write requests.
; (C) 1986 Daniel Kegel, Pasadena, CA
; May be distributed for educational and personal use only
; The following files make up the driver:
;	nansi.asm   - all DOS function handlers except init
;	nansi_p.asm - parameter parser for ANSI escape sequences
;	nansi_f.asm - ANSI command handlers
;	nansi_i.asm - init DOS function handler
;
; Daniel Kegel, Bellevue, Washington & Pasadena, California
; Revision history:
; 5  july 85: brought up non-ANSI portion except forgot backspace
; 6  july 85: split off ANSI stuff into other files, added backspace
; 11 july 85: fixed horrible bug in getchar; changed dosfns to subroutines
; 12 july 85: fixed some scrolling bugs, began adding compaq flag
; 9  aug 85:  added cursor position reporting
; 10 aug 85:  added output character translation
; 11 aug 85:  added keyboard redefinition, some EGA 80x43 support
; 10 sept 85: Tandy 2000 support via compaq flag (finding refresh buffer)
; 30 Jan 86:  removed Tandy 2000 stuff, added graphics mode support
; 12 feb 86:  added int 29h handler, added PUSHA/POPA, added direct beep,
;	      direct cursor positioning, takeover of BIOS write_tty,
;	      noticed & squashed 2 related bugs in tab expansion
; 13 feb 86:  Squashed them again, harder
; 24 feb 86:  There is a bug in the timing code used by the BEEP routine.
;	      If the addition of the beep period to the
;	      BIOS low timer word results in an overflow, the beep will be
;	      supressed. Also made code compatible eith earlier versions
;	      of assembler.
;------------------------------------------------------------------------

	include nansi_d.asm	; definitions

	; from nansi_f.asm
	extrn	f_escape:near, f_in_escape:near

	; from nansi_p.asm
	extrn	param_end:word, redef_end:word

	; from nansi_i.asm
	extrn	dosfn0:near

	; to nansi_p.asm
	public	f_loopdone
	public	f_not_ansi
	public	f_ansi_exit

	; to both nansi_p.asm and nansi_f.asm
	public	cur_x, cur_y, max_x, cur_attrib

	; to nansi_f.asm
	public	xy_to_regs, get_blank_attrib
	public	port_6845
	public	wrap_flag
	public	cur_parm_ptr
	public	cur_coords, saved_coords, max_y
	public	escvector, string_term
	public	cpr_esc, cprseq
	public	video_mode
	public	lookup
	public	in_g_mode

	; to nansi_i.asm
	public	req_ptr, break_handler
	public	int_29
	if	takeBIOS
	public	new_vid_bios, old_vid_bios
	endif

	; to all modules
	public	xlate_tab_ptr

;--- seg_cs is the CS: override prefix
; (assembler forgets cs: on second "xlat dummy_cs_byte")
seg_cs	macro
	db	2eh
	endm

;--- push_all, pop_all ------------------------------------------------
; Save/restore all user registers.
push_all	macro
	if	is_8088
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di
	else
	pusha
	endif
	endm

pop_all macro
	if	is_8088
	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	else
	popa
	endif
	endm

keybuf	struc				; Used in getchar
len	dw	?
adr	dw	?
keybuf	ends


ABS40	segment at 40h
	org	1ah
buffer_head	dw	?	; Used in 'flush input buffer' dos call.
buffer_tail	dw	?

	org	49h
crt_mode	db	?
crt_cols	dw	?
crt_len		dw	?
crt_start	dw	?
cursor_posn	dw	8 dup (?)
cursor_mode	dw	?
active_page	db	?
addr_6845	dw	?
crt_mode_set	db	?	; = 7 only if monochrome display adaptor
crt_palette	db	?
	org	6ch
timer_low	dw	?	; low word of time-of-day counter (18.2 hz)

ABS40	ends

	page

CODE	segment byte public 'CODE'
assume	cs:code, ds:code

	; Device Driver Header

	org	0

	dd	-1			; next device
	dw	8013h			; attributes
	dw	strategy		; request header pointer entry
	dw	interrupt		; request entry point
	db	'CON'			; device name (8 char)
	db	5 dup (20h)		;  ... and 5 blanks)

	; Identification- in case somebody TYPEs the assembled driver
	db	27,'[2J'
	db	"Nansi.sys v2.2"
	ife	is_8088
	db	"(80286)"
	endif
	db	': New ANSI driver (C) Daniel Kegel, Pasadena, CA 1986'
	db	13, 10, 26


;----- variable area --------------------
req_ptr label	dword
req_off dw	?
req_seg dw	?

wrap_flag	db	1	; 0 = no wrap past line end
escvector	dw	0	; state vector of ESCape sequencor
video_mode	db	3	; ROM BIOS video mode (2=BW, 3=color)
max_y		db	24
max_cur_x	label	word	; used to get both max & cur at once
max_x		db	79	; line width (79 for 80x25 modes)
cur_coords	label	word
cur_x		db	0	; cursor position (0 = left edge)
cur_y		db	0	;		  (0 = top edge)
saved_coords	dw	?	; holds XY after a SCP escape sequence
string_term	db	0	; either escape or double quote
cur_attrib	db	7	; current char attributes
cur_page	db	0	; current display page
video_seg	dw	?	; segment of video card
f_cptr_seg	dw	?	; part of fastout write buffer pointer
cur_parm_ptr	dw	?	; last byte of parm area now used
port_6845	dw	?	; port address of 6845 card
xlate_tab_ptr	dw	?	; pointer to output translation table
		if	takeBIOS
old_vid_bios	dd	?	; pointer to old video bios routine
		endif

brkkeybuf	db	3	; control C
fnkeybuf	db	?	; holds second byte of fn key codes
cpr_buf		db	8 dup (?), '['
cpr_esc		db	1bh	; descending buffer for cpr function

; following four keybufs hold information about input
; Storage order determines priority- since the characters making up a function
; key code must never be separated (say, by a Control-Break), they have the
; highest priority, and so on.	Keyboard keys (except ctrl-break) have the
; lowest priority.

fnkey	keybuf	<0, fnkeybuf>	; fn key string (0 followed by scan code)
cprseq	keybuf	<0>		; CPR string (ESC [ y;x R)
brkkey	keybuf	<0, brkkeybuf>	; ^C
xlatseq keybuf	<0>		; keyboard reassignment string

;------ xy_to_regs --------------------------------------------
; on entry: x in cur_x, y in cur_y
; on exit:  dx = chars left on line, di = address
; Alters ax, bx.
xy_to_regs	proc	near
	; Find number of chars 'till end of line, keep in DX
	mov	ax, max_cur_x
	mov	bx, ax			; save max_x & cur_x for next block
	mov	ah, 0			; ax = max_x
	xchg	dx, ax
	mov	al, bh
	mov	ah, 0			; ax = cur_x
	sub	dx, ax
	inc	dx			; dx is # of chars till EOL
	; Calculate DI = current address in text buffer
	mov	al, bl			; al = max_x
	inc	al
	mul	cur_y
	add	al, bh			; al += cur_x
	adc	ah, 0			; AX is # of chars into buffer
	add	ax, ax
	xchg	di, ax			; DI is now offset of cursor.
	ret
xy_to_regs	endp


;------- dos_fn_tab -------------
; This table is used in "interrupt" to call the routine that handles
; the requested function.

max_cmd equ	12
dos_fn_tab:
	dw	dosfn0, nopcmd, nopcmd, badcmd, dosfn4, dosfn5, dosfn6
	dw	dosfn7, dosfn8, dosfn8, nopcmd, nopcmd

;------- strategy ----------------------------------------------------
; DOS calls strategy with a request which is to be executed later.
; Strategy just saves the request.

strategy	proc	far
	mov	cs:req_off,BX
	mov	cs:req_seg,ES
	ret
strategy	endp

;------ interrupt -----------------------------------------------------
; This is where the request handed us during "strategy" is
; actually carried out.
; Calls one of 12 subroutines depending on the function requested.
; Each subroutine returns with exit status in AX.

interrupt	proc	far
	sti
	push_all			; preserve caller's registers
	push	ds
	push	es

	; Read requested function information into registers
	lds	bx,cs:req_ptr
	mov	al,[BX+02h]		; al = function code
;
; The next instruction blows up MASM 1.0 but who cares!!
;
	les	si,[BX+0Eh]		; ES:SI = input/output buffer addr
	mov	cx,[BX+12h]		; cx = input/output byte count

	cmp	al, max_cmd
	ja	unk_command		; too big, exit with error code

	xchg	bx, ax
	shl	bx, 1			; form index to table of words
	mov	ax, cs
	mov	ds, ax
	call	word ptr dos_fn_tab[bx]
int_done:
	lds	bx,cs:req_ptr		; report status
	or	ax, 100h		; (always set done bit upon exit)
	mov	[bx+03],ax

	pop	ES			; restore caller's registers
	pop	DS
	pop_all
	ret				; return to DOS.

unk_command:
	call	badcmd
	jmp	int_done

interrupt	endp

;----- BIOS break handler -----------------------------------------
; Called by BIOS when Control-Break is hit (vector was set up in Init).
; Simply notes that a break was hit.  Flag is checked during input calls.

break_handler	proc
	mov	cs:brkkey.len, 1
	iret
break_handler	endp

	page

;------ badcmd -------------------------------------------------------
; Invalid function request by DOS.
badcmd	proc	near
	mov	ax, 813h		; return "Error: invalid cmd"
	ret
badcmd	endp


;------ nopcmd -------------------------------------------------------
; Unimplemented or dummy function request by DOS.
nopcmd	proc	near
	xor	ax, ax			; No error, not busy.
	ret
nopcmd	endp

;------- dos function #4 ----------------------------------------
; Reads CX characters from the keyboard, places them in buffer at
; ES:SI.
dosfn4	proc	near
	jcxz	dos4done
	mov	di, si
dos4lp: push	cx
	call	getchar
	pop	cx
	stosb
	loop	dos4lp
dos4done:
	xor	ax, ax			; No error, not busy.
	ret
dosfn4	endp

;-------- dos function #5: non-destructive input, no wait ------
; One-character lookahead into the keyboard buffer.
; If no characters in buffer, return BUSY; otherwise, get value of first
; character of buffer, stuff into request header, return DONE.
dosfn5	proc	near
	call	peekchar
	jz	dos5_busy

	lds	bx,req_ptr
	mov	[bx+0Dh], al
	xor	ax, ax			; No error, not busy.
	jmp	short dos5_exit
dos5_busy:
	MOV	ax, 200h		; No error, busy.
dos5_exit:
	ret

dosfn5	endp

;-------- dos function #6: input status --------------------------
; Returns "busy" if no characters waiting to be read.
dosfn6	proc	near
	call	peekchar
	mov	ax, 200h		; No error, busy.
	jz	dos6_exit
	xor	ax, ax			; No error, not busy.
dos6_exit:
	ret
dosfn6	endp

;-------- dos function #7: flush input buffer --------------------
; Clears the IBM keyboard input buffer.  Since it is a circular
; queue, we can do this without knowing the beginning and end
; of the buffer; all we need to do is set the tail of the queue
; equal to the head (as if we had read the entire queue contents).
; Also resets all the device driver's stuffahead buffers.
dosfn7	proc	near
	xor	ax, ax
	mov	fnkey.len, ax		; Reset the stuffahead buffers.
	mov	cprseq.len, ax
	mov	brkkey.len, ax
	mov	xlatseq.len, ax

	mov	ax, abs40
	mov	es, ax
	mov	ax, es:buffer_head	; clear queue by making the tail
	mov	es:buffer_tail, ax	; equal to the head

	xor	ax, ax			; no error, not busy.
	ret
dosfn7	endp

	page
	if	takeBIOS
;--- new_vid_bios -------------------------------------------
; New_vid_bios simply replaces the write_tty call.
; All other calls get sent to the old video bios.
; This gives BIOS ANSI capability.
; However, it takes away the escape character.
; If this is not desired, just tell init to not take over the vector.

new_vid_bios	proc
	cmp	ah, 14
	jz	nvb_write_tty
	jmp	dword ptr cs:old_vid_bios
nvb_write_tty:
	push	cx
	mov	cl, cs:cur_attrib
	; If in graphics mode, BL is new color
	call	in_g_mode		; returns carry set if text mode
	jc	nvb_wt_text
		mov	cs:cur_attrib, bl	; ja?
nvb_wt_text:
	int	29h			; write AL
	mov	cs:cur_attrib, cl	; restore color
	pop	cx
	iret

new_vid_bios	endp
	endif

;------ int_29 ----------------------------------------------
; Int 29 handles DOS quick-access putchar.
; Last device loaded with attribute bit 4 set gets accessed for
; single-character writes via int 29h instead of via interrupt.
; Must preserve all registers.
; Installed as int 29h by dosfn0 (init).
int_29_buf	db	?

int_29	proc	near
	sti
	push	ds
	push	es
	push_all
	mov	cx, 1
	mov	bx, cs
	mov	es, bx
	mov	ds, bx
	mov	si, offset int_29_buf
	mov	byte ptr [si], al
	call	dosfn8
	pop_all
	pop	es
	pop	ds
	iret
int_29	endp

	page
;------ dosfn8 -------------------------------------------------------
; Handles writes to the device (with or without verify).
; Called with
;  CX	 = number of bytes to write
;  ES:SI = transfer buffer
;  DS	 = CS, so we can access local variables.

dosfn8	proc	near

	mov	f_cptr_seg, es	; save segment of char ptr

	; Read the BIOS buffer address/cursor position variables.
	mov	ax, abs40
	mov	ds, ax
	assume	ds:abs40

	; Find current video mode and screen size.
	mov	ax,word ptr crt_mode	; al = crt mode; ah = # of columns
	mov	cs:video_mode, al
	dec	ah			; ah = max column
	mov	cs:max_x, ah

	; Find current cursor coordinates.
	mov	al,active_page
	cbw
	add	ax,ax
	xchg	bx,ax
	mov	ax,cursor_posn[bx]
	mov	cs:cur_coords,AX

	; Find video buffer segment address; adjust it
	; so the offset is zero; return in AX.

	; DS is abs40.
	; Find 6845 address.
	mov	ax, addr_6845
	mov	cs:port_6845, ax
	; Find video buffer address.
	MOV	AX,crt_start
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	add	ah, 0B0h		; assume it's a monochrome card...
	CMP	cs:video_mode,07
	jz	d8_gots
	add	ah, 8			; but if not mode 7, it's color.
d8_gots:
	push	cs
	pop	ds
	assume	ds:code
	mov	video_seg, ax
	mov	es, ax
	call	xy_to_regs		; Set DX, DI according to cur_coords.

	; | If in graphics mode, clear old pseudocursor
	call	in_g_mode
	jc	d8_no_cp
		call	pseudocursor	; write block in xor
d8_no_cp:
	mov	bx, xlate_tab_ptr	; get pointer to translation table

	mov	ah, cur_attrib
	mov	ds, f_cptr_seg		; get segment of char ptr
	assume	ds:nothing
	cld				; make sure we'll increment

	; The Inner Loop: 4+12 +4+4 +4+4 +0+15 +4+4 +4+18 = 77 cycles/loop
	; on 8088; at 4.77 MHz, that gives 16.1 microseconds/loop.
	; At that speed, it takes 32 milliseconds to fill a screen.

	; Get a character, put it on the screen, repeat 'til end of line
	; or no more characters.
	jcxz	f_loopdone		; if count = 0, we're already done.
	cmp	cs:escvector, 0		; If in middle of an escape sequence,
	jnz	f_in_escapex		; jump to escape sequence handler.

f_tloop:; | If in graphics mode, jump to alternate loop
	; | What a massive kludge!  A better approach would have been
	; | to collect characters for a "write n chars" routine
	; | which would handle both text and graphics modes.
	call	in_g_mode
	jc	f_t_cloop
	jmp	f_g_cloop

f_t_cloop:
	LODSB				; get char! (al = ds:[si++])
	cmp	al, 28			; is it a control char?
	jb	f_control		;  maybe...
f_t_nctl:
	seg_cs
	xlat
	STOSW				; Put Char! (es:[di++] = ax)
	dec	dx			; count down to end of line
	loopnz	f_t_cloop		; and go back for more.
	jz	f_t_at_eol		; at end of line; maybe do a crlf.
	jmp	short f_loopdone

f_looploop:
f_ansi_exit:				; in case we switched into
	loopnz	f_tloop			; a graphics mode
f_t_at_eol:
	jz	f_at_eol

f_loopdone:

	;--------- All done with write request -----------
	; DI is cursor address; cursor position in cur_y, dl.
	mov	ax, cs
	mov	ds, ax			; get our segment back
	assume	ds:code

	; Restore cur_x = max_x - dx + 1.
	mov	al, max_x
	inc	al
	sub	al, dl
	mov	cur_x, al
	; Set cursor position; cursor adr in DI; cursor pos in cur_x,cur_y
	call	set_pseudocursor
	; Return to DOS.
	xor	ax, ax			; No error, not busy.
	ret

	;---- handle control characters ----
	; Note: cur_x is not kept updated in memory, but can be
	; computed from max_x and dx.
	; Cur_y is kept updated in memory.
f_control:
	cmp	al, 27			; Is it an escape?
	jz	f_escapex
	cmp	al, 13			; carriage return?
	jz	f_cr
	cmp	al, 10			; line feed?
	jz	f_lf
	cmp	al, 9			; tab?
	jz	f_tabx
	cmp	al, 8			; backspace?
	jz	f_bs
	cmp	al, 7			; bell?
	jz	f_bell
	jmp	f_nctl			; then it is not a control char.

f_tabx: jmp	f_tab
f_escapex:
	jmp	f_escape
f_in_escapex:
	jmp	f_in_escape

f_bs:	;----- Handle backspace -----------------
	; Moves cursor back one space without erasing.	No wraparound.
	cmp	dl, cs:max_x		; wrap around to previous line?
	ja	fbs_wrap		; yep; disallow it.
	dec	di			; back up one char & attrib,
	dec	di
	inc	dx			; and note one more char left on line.
fbs_wrap:
	jmp	f_looploop

f_bell: ;----- Handle bell ----------------------
	; Use BIOS to do the beep.  DX is not changed, as bell is nonprinting.
	call	beep
	or	al, al			; clear z
; old (more portable) version:
;	mov	ax, 0e07h		; "write bell to tty simulator"
;	mov	bx, 0			; "page zero, color black"
;	int	10h			; call BIOS
;	mov	ah, cs:cur_attrib	; restore current attribute
;	mov	bx, cs:xlate_tab_ptr	; restore translate table address
;	or	al, al			; al still 7; this clears Z.
	jmp	f_looploop		; Let main loop decrement cx.

f_cr:	;----- Handle carriage return -----------
	; di -= cur_x<<1;		set di= address of start of line
	; dx=max_x+1;			set bx= chars left in line
	mov	al, cs:max_x
	inc	al
	sub	al, dl			; Get cur_x into ax.
	mov	ah, 0
	sub	di, ax
	sub	di, ax
	mov	dl, cs:max_x		; Full line ahead of us.
	inc	dx
	mov	ah, cs:cur_attrib	; restore current attribute
	or	al, 1			; clear z
	jmp	f_looploop		; and let main loop decrement cx

f_at_eol:
	;----- Handle overrunning right end of screen -------
	; cx++;				compensate for double loop
	; if (!wrap_flag) { dx++; di-=2; }
	; else do_crlf;
	inc	cx
	test	cs:wrap_flag, 1
	jnz	feol_wrap
		dec	di
		dec	di
		inc	dx
		jmp	f_looploop
feol_wrap:
	; dx=max_x+1;			set bx= chars left in line
	; di -= 2*(max_x+1);
	; do_lf
	mov	dl, cs:max_x
	inc	dx
	sub	di, dx
	sub	di, dx
	; fall thru to line feed routine

f_lf:	;----- Handle line feed -----------------
	; if (cur_y >= max_y) scroll;		scroll screen up if needed
	; else { cur_y++; di += max_x<<1;	else increment Y

	mov	al, cs:max_y
	cmp	cs:cur_y, al
	jb	flf_noscroll
		call	scroll_up		; preserves bx,cx,dx,si,di
		jmp	short flf_done
flf_noscroll:
	inc	cs:cur_y
	mov	al, cs:max_x
	mov	ah, 0
	inc	ax
	add	ax, ax
	add	di, ax
flf_done:
	mov	ah, cs:cur_attrib		; restore current attribute
	or	al, 1			; clear z
	jmp	f_looploop		; and let main loop decrement cx

f_tab:	;----- Handle tab expansion -------------
	; Get cur_x into al.
	mov	al, cs:max_x
	inc	al
	sub	al, dl
	; Calculate number of spaces to output.
	push	cx			; save cx
	mov	ch, 0
	mov	cl, al			; get zero based x coordinate
	and	cl, 7
	neg	cl
	add	cl, 8			; 0 -> 8, 1 -> 8, ... 7 -> 1
	sub	dx, cx			; update chars-to-eol, maybe set z
	pushf				; || save Z for main loop
	; ah is still current attribute.  Move CX spaces to the screen.
	mov	al, ' '
	call	in_g_mode		; | graphics mode
	jnc	f_tab_putc		; |
	REP	STOSW
	popf				; || restore Z flag for main loop test
	pop	cx			; restore cx
	jmp	f_looploop		; Let main loop decrement cx.

;--------------- graphics mode support -----------------------

f_tab_putc:	; graphics mode- call putc to put the char
	add	dx, cx			; move back to start of tab
f_tp_lp:
	call	putchar
	dec	dx			; go to next cursor position
	loop	f_tp_lp
	popf				; Z set if wrapped around EOL
	pop	cx
	jmp	f_looploop

;---- in_g_mode -------------
; Returns Carry set if not in a graphics mode.
; Preserves all registers.

in_g_mode	proc	near
	cmp	cs:video_mode, 4
	jb	igm_stc
	cmp	cs:video_mode, 7
	jz	igm_stc
	clc
	ret
igm_stc:
	stc
	ret
in_g_mode	endp

;---- Where to go when a character turns out not to be special
f_nctl:
f_not_ansi:
	call	in_g_mode
	jnc	f_g_nctl		; graphics mode
f_jmptnctl:
	jmp	f_t_nctl		; text mode

;---- Alternate main loop for graphics mode ----
f_g_cloop:
	LODSB				; get char! (al = ds:[si++])
	cmp	al, 28			; is it a control char?
	jb	f_g_control		;  maybe...
f_g_nctl:
	seg_cs
	xlat
	call	putchar
	dec	dx			; count down to end of line
	loopnz	f_g_cloop		; and go back for more.
	jz	f_g_at_eol		; at end of line; maybe do a crlf.
	jmp	f_loopdone

f_g_control:	jmp	f_control
f_g_at_eol:	jmp	f_at_eol

;---- putchar ------------------------------------------------
; Writes char AL, attribute AH to screen at (max_x+1-dl), cur_y.
; On entry, registers set up as per xy_to_regs.
; Preserves all registers.
putchar proc	near
	push	dx
	push	cx
	push	bx
	push	ax
	; 1. Set cursor position.
	mov	al, cs:max_x
	inc	al
	sub	al, dl
	mov	cs:cur_x, al
	mov	dx, cs:cur_coords	; get X & Y into DX
	xor	bx, bx			; choose dpy page 0
	mov	ah, 2			; chose "Set Cursor Position"
	int	10h			; call ROM BIOS
	; 2. Write char & attribute.
	mov	cx, 1
	pop	ax			; get char in AL
	push	ax
	mov	bl, ah			; attribute in BL
	mov	bh, 0
	mov	ah, 9
	int	10h
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	ret
putchar endp

;---- set_pseudocursor ------------
; If in graphics mode, set pseudocursor, else set real cursor.
; Destroys DS!!!!

set_pseudocursor	proc	near
	call	in_g_mode
	jnc	pseudocursor
; old (more portable, but slower) version
;	mov	dx, cur_coords		; get X & Y into DX
;	xor	bx, bx			; choose dpy page 0
;	mov	ah, 2			; chose "Set Cursor Position"
;	int	10h			; call ROM BIOS

	; Write directly to 6845 cursor address register.
	mov	bx, di
	shr	bx, 1			; convert word index to byte index

	mov	dx, port_6845
	mov	al, 0eh
	out	dx, al

	jmp	$+2
	inc	dx
	mov	al, bh
	out	dx, al

	jmp	$+2
	dec	dx
	mov	al, 0fh
	out	dx, al

	jmp	$+2
	inc	dx
	mov	al, bl
	out	dx, al

	; Set cursor position in low memory.
	assume	ds:abs40
	mov	ax, abs40
	mov	ds, ax
; Does anybody ever use anything but page zero?
;	mov	al,active_page
;	cbw
;	add	ax,ax
;	xchg	bx,ax
	mov	ax, cs:cur_coords
	mov	cursor_posn,ax
	ret

	assume	ds:code
set_pseudocursor	endp


;---- pseudocursor --------------------------------------------------
; Writes a color 15 block in XOR at the current cursor location.
; Preserves DS, ES, BX, CX, DX, SI, DI.
; Should be disableable- the pseudocursor slows down single-char
; writes by a factor of three.
pseudocursor	proc	near
	mov	ax, 8f16h	; xor, color 15, ^V (small block)
	call	putchar
	ret
pseudocursor	endp

;--------------- end of graphics mode support --------------------

dosfn8	endp

;--- get_blank_attrib ------------------------------------------------
; Determine new attribute and character for a new blank region.
; Use current attribute, just disallow blink and underline.
; (Pretty strange way to do it.  Might want to disallow rev vid, too.)
; Returns result in AH, preserves all other registers.
get_blank_attrib	proc	near
	mov	ah, 0
	call	in_g_mode
	jnc	gb_aok			; if graphics mode, 0 is bkgnd

	mov	ah, cs:cur_attrib
	and	ah, 7fh			; disallow blink
	cmp	cs:video_mode, 7	; monochrome?
	jnz	gb_aok
		cmp	ah, 1		; underline?
		jnz	gb_aok
		mov	ah, 7		; yep- set it to normal.
gb_aok: ret
get_blank_attrib	endp


;---- scroll_up ---------------------------------------------------
; Scroll screen up- preserves ax, bx, cx, dx, si, di, ds, es.
; Moves screen up 1 line, fills the last line with blanks.
; Attribute of blanks is the current attribute sans blink and underline.

scroll_up	proc	near
	push	ax
	push	bx
	push	cx
	push	dx

	call	get_blank_attrib
	mov	bh, ah			; color to use on new blank areas
	mov	al, 1			; AL is number of lines to scroll.
	mov	ah, 6			; BIOS: scroll up
	mov	cl, 0			; upper-left-x of data to scroll
	mov	ch, 0			; upper-left-y of data to scroll
	mov	dl, cs:max_x		; lower-rite-x
	mov	dh, cs:max_y		; lower-rite-y (zero based)
	int	10h			; call BIOS to scroll a rectangle.

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
scroll_up	endp

;---- lookup -----------------------------------------------
; Called by getchar, peekchar, and key to see if a given key has
; been redefined.
; Sets AH to zero if AL is not zero (i.e. if AX is not a function key).
; Returns with Z cleared if no redefinition; otherwise,
; Z is set, SI points to redefinition string, CX is its length.
; Preseves AL, all but CX and SI.
; Redefinition table organization:
;  Strings are stored in reversed order, first char last.
;  The word following the string is the character to be replaced;
;  the next word is the length of the string sans header.
; param_end points to the last byte used by the parameter buffer;
; redef_end points to the last word used by the redef table.

lookup	proc	near
	mov	si, redef_end		; Start at end of table, move down.
	or	al, al
	jz	lu_lp
	mov	ah, 0			; clear extraneous scan code
lu_lp:	cmp	si, param_end
	jbe	lu_notfound		; If below redef table, exit.
	mov	cx, [si]
	cmp	ax, [si-2]		; are you my mommy?
	jz	lu_gotit
	sub	si, 4
	sub	si, cx			; point to next header
	jmp	lu_lp
lu_notfound:
	or	si, si			; clear Z
	jmp	short lu_exit
lu_gotit:
	sub	si, 2
	sub	si, cx			; point to lowest char in memory
	cmp	al, al			; set Z
lu_exit:
	ret
lookup	endp

;---- searchbuf --------------------------------------------
; Called by getchar and peekchar to see if any characters are
; waiting to be gotten from sources other than BIOS.
; Returns with Z set if no chars found, BX=keybuf & SI=keybuf.len otherwise.
searchbuf	proc	near
	; Search the stuffahead buffers.
	mov	cx, 4			; number of buffers to check for chars
	mov	bx, offset fnkey - 4
sbloop: add	bx, 4			; point to next buffer record
	mov	si, [bx].len
	or	si, si			; empty?
	loopz	sbloop			; if so, loop.
	ret
searchbuf	endp

;---- getchar -----------------------------------------------
; Returns AL = next char.
; Trashes AX, BX, CX, BP, SI.
getchar proc	near
gc_searchbuf:
	; See if any chars are waiting in stuffahead buffers.
	call	searchbuf
	jz	gc_trykbd		; No chars?  Try the keyboard.
	; A nonempty buffer was found.
	dec	[bx].len
	dec	si
	mov	bp, [bx].adr		; get pointer to string
	mov	al, byte ptr ds:[bp][si]; get the char
	; Recognize function key sequences, move them to highest priority
	; queue.
	sub	si, 1			; set carry if si=0
	jc	gc_nofnkey		; no chars left -> nothing to protect.
	cmp	bx, offset fnkey
	jz	gc_nofnkey		; already highest priority -> done.
	or	al, al
	jnz	gc_nofnkey		; nonzero first byte -> not fnkey.
	; Found a function key; move it to highest priority queue.
	dec	[bx].len
	mov	ah, byte ptr ds:[bp][si]; get the second byte of fn key code
gc_fnkey:
	mov	fnkey.len, 1
	mov	fnkeybuf, ah		; save it.
gc_nofnkey:
	; Valid char in AL.  Return with it.
	jmp	short gcdone

gc_trykbd:
	; Actually get a character from the keyboard.
	mov	ah, 0
	int	16h			; BIOS returns with char in AX
	; If it's Ctrl-break, it has already been taken care of.
	or	ax, ax
	jz	gc_trykbd

	; Look in the reassignment table to see if it needs translation.
	call	lookup			; Z=found; CX=length; SI=ptr
	jnz	gc_noredef
	; Okay; set up the reassignment, and run thru the translation code.
	mov	xlatseq.len, cx
	mov	xlatseq.adr, si
	jmp	gc_searchbuf
gc_noredef:
	; Is it a function key?
	cmp	al, 0
	jz	gc_fnkey		; yep- special treatment.
gcdone: ret	; with character in AL.

getchar endp

;---- peekchar -----------------------------------------------
; Returns Z if no character ready, AL=char otherwise.
; Trashes AX, BX, CX, BP, SI.
peekchar	proc	near
pc_searchbuf:
	call	searchbuf
	jz	pc_trykbd		; No chars?  Try the keyboard.
	; A nonempty buffer was found.
	dec	si
	mov	bp, [bx].adr		; get pointer to string
	mov	al, byte ptr ds:[bp][si]; get the char
	; Valid char from buffer in AL.  Return with it.
	jmp	short pcdone
pc_trykbd:
	; Actually peek at the keyboard.
	mov	ah, 1
	int	16h			; BIOS returns with char in AX
	jz	pcexit
	; If it's control-break, it's already been taken care of.
	or	ax, ax
	jnz	pc_notbrk
	mov	ah, 0
	int	16h			; so get rid of it!
	jmp	short pc_trykbd
pc_notbrk:
	; Look in the reassignment table to see if it needs translation.
	call	lookup			; Z=found; CX=length; SI=ptr
	jnz	pcdone			; Nope; just return the char.
	; Okay; get the first code to be returned.
	add	si, cx
	mov	al, [si-1]
pcdone: or	ah, 1			; NZ; char ready!
pcexit: ret	; with character in AL, Z true if no char waiting.
peekchar	endp

;---- beep ------------------------------------------------------
; Beep speaker; period given by beep_div, duration by beep_len.
; Preserves all registers.

beep_div	dw	1300		; fairly close to IBM beep
beep_len	dw	3		; 3/18 sec- shorter than IBM

beep	proc	near
	push_all

	mov	al, 10110110b		; select 8253
	mov	dx, 43h			; control port address
	out	dx, al
	dec	dx			; timer 2 address
	mov	ax, cs:beep_div
	jmp	$+2
	out	dx, al			; low byte of divisor
	xchg	ah, al
	jmp	$+2
	out	dx, al			; high byte of divisor
	mov	dx, 61h
	jmp	$+2
	in	al, dx			; get current value of control bits
	push	ax
	or	al, 3
	jmp	$+2
	out	dx, al			; turn speaker on

	; Wait for desired duration by monitoring time-of-day 18 Hz clock
	push	es
	mov	ax, abs40
	mov	es, ax
	assume	es:abs40
	mov	bx, timer_low
	mov	cx, -1
beeplp:	mov	ax, timer_low
	sub	ax, bx
	cmp	ax, cs:beep_len
	jg	beepover
	loop	beeplp
beepover:
	pop	es
	assume	es:code

	; Turn off speaker
	pop	ax
	and	al, not 3		; turn speaker off
	out	dx, al
	pop_all
	ret
beep	endp

CODE	ends

	end				; of nansi.asm
