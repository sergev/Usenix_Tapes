;---- setraw.asm ----------------------------------------------
	public	getraw, setraw
;----- dos ----------------------
; Call DOS function # n.
dos	macro	fn
	mov	ah, fn
	int	21h
	endm

code	segment para public 'CODE'
assume cs:code
;----- Getraw ---------------------------------------------
; Returns AX=1 if file BX is a device in raw mode; 0 otherwise.
; Returns Carry set & errorcode in AX if DOS error.

getraw	proc	near
	mov	al, 0
	DOS	44h		; Get attributes
	jc	gr_exit		; bad file handle?
	mov	ax, 20h
	and	al, dl		; get that bit
	mov	cl, 4
	shr	ax, cl
	clc
gr_exit:
	ret
getraw	endp

;----- Setraw -------------------------------------------
; Sets Raw state of file BX to (AX != 0) if file is a device.
; Returns Carry set & errorcode in AX if DOS error.
setraw	proc	near
	mov	cx, ax
	mov	al, 0
	DOS	44h
	jc	sr_exit
	test	al, 80h		; It it a device?
	jz	sr_exit		; nope- do nothing.
	; Current mode in DX; set CX = 20H if CX nonzero.
	or	cx, cx
	jz	sr_ax0
		mov	cx, 20h
sr_ax0: and	dx, 00cfh	; clear old raw bit and hi byte,
	or	dx, cx		; set new value.
	mov	al, 1
	DOS	44h
sr_exit:
	ret
setraw	endp
code	ends
	end
;---- end of setraw.asm ------------------------------------------
