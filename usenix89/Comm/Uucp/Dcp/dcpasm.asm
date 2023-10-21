; Support rotuines for DCP a uucp clone. 
; Copyright Richard H. Lamb, 1985,1986,1987
; dos dir scan for a file
;""""""""""""""""""""""""""""""
_text 	segment	byte	public	'code'
_text	ends
_data	segment	byte	public	'data'
_data	ends
const	segment	byte	public	'const'
const	ends
_bbs	segment	byte	public	'bbs'
_bbs	ends
dgroup	group	const,_bbs,_data
	assume	cs:_text,ds:dgroup,ss:dgroup,es:dgroup
_text	SEGMENT
	public	_dir_open
_dir_open	proc	near 
;_dir_open:
	push	bp
	mov	bp,sp
	push	ds
	push	dx
	push	cx
; set DTA
	mov	dx,word ptr [bp+6]
	mov	ah,01ah
	int	21h
; find first file
	mov	dx,word ptr [bp+4]
	mov	ah,04eh
	mov	cx,010h
	int	21h
; return 0=found 
	sub	ah,ah
	pop	cx
	pop	dx
	pop	ds
	pop	bp
	ret
_dir_open	endp
;*******************************
_text	ends
	end
