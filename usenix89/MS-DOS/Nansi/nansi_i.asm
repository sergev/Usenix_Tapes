;------ nansi_i.asm ----------------------------------------------
; Contains code only needed at initialization time.
; (C) 1986 Daniel Kegel
; May be distributed for educational and personal use only
;-----------------------------------------------------------------

	include nansi_d.asm		; definitions

	; to nansi.asm
	public	dosfn0

	; from nansi.asm
	extrn	break_handler:near
	extrn	int_29:near
	if	takeBIOS
	extrn	new_vid_bios:near
	extrn	old_vid_bios:dword
	endif
	extrn	req_ptr:dword
	extrn	xlate_tab_ptr:word

	; from nansi_p.asm
	extrn	param_buffer:word	; adr of first byte free for params
	extrn	param_end:word		; adr of last byte used for params
	extrn	redef_end:word		; adr of last used byte for redefs


code	segment byte public 'CODE'
	assume	cs:code, ds:code

;-------- dos function # 0 : init driver ---------------------
; Initializes device driver interrupts and buffers, then
; passes ending address of the device driver to DOS.
; Since this code is only used once, the buffer can be set up on top
; of it to save RAM.

dosfn0	proc	near
	; Install BIOS keyboard break handler.
	xor	ax, ax
	mov	ds, ax
	mov	bx, 6Ch
	mov	word ptr [BX],offset break_handler
	mov	[BX+02], cs
	; Install INT 29 quick putchar.
	mov	bx, 0a4h
	mov	word ptr [bx], offset int_29
	mov	[bx+2], cs
	if	takeBIOS
	; Install INT 10h video bios replacement.
	mov	bx, 40h
	mov	ax, [bx]
	mov	word ptr cs:old_vid_bios, ax
	mov	ax, [bx+2]
	mov	word ptr cs:old_vid_bios[2], ax
	mov	word ptr [bx], offset new_vid_bios
	mov	word ptr [bx+2], cs
	endif

	push	cs
	pop	ds
	push	cs
	pop	es			; es=cs so we can use stosb
	cld				; make sure stosb increments di

	; Calculate addresses of start and end of parameter/redef buffer.
	; The buffer occupies the same area of memory as this code!
	; ANSI parameters are accumulated at the lower end, and
	; keyboard redefinitions are stored at the upper end; the variable
	; param_end is the last byte used by params (changes as redefs added);
	; redef_end is the last word used by redefinitions.
	mov	di, offset dosfn0
	mov	param_buffer, di
	add	di, 512
	mov	param_end, di	; addr of last byte in free area
	inc	di

	; Build the default redefinition table:
	;	control-printscreen -> control-P
	; (Must be careful not to write over ourselves here!)
	mov	al, 16		; control P
	stosb
	mov	ax, 7200h	; control-printscreen
	stosw
	mov	ax, 1		; length field
	mov	redef_end, di	; address of last used word in table
	stosw

	; Build a 1:1 output character translation table.
	; It is 256 bytes long, starts just after the param/redef buffer,
	; and is the last thing in the initialized device driver.
	mov	xlate_tab_ptr, di
	xor	ax, ax
init_loop:
	stosb
	inc	al
	jnz	init_loop

	xor	ax, ax
	; Return ending address of this device driver.
	; Status is in AX.
	lds	si, req_ptr
	mov	word ptr [si+0Eh], di
	mov	[si+10h], cs
	; Return exit status in ax.
	ret

dosfn0	endp

code	ends
	end				; of nansi_i.asm
