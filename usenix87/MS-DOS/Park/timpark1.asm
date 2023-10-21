code	Segment Para
	Assume cs:code, ds:code
	org	100h
park	proc far
	jmp	init
x1coff 	dw	0
x1cseg 	dw	0
x13off 	dw	0
x13seg 	dw	0
parked	dw	0
count	dw	0
value	dw	0
flag	dw	0
oldax	dw	0
oldip	dw	0
oldcs	dw	0
oldflgs	dw	0
	even
x1cint:	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	mov	ax,cs
	mov	ds,ax
	xor	ax,ax
	cmp	parked,ax
	jne	x1cext
	dec	word ptr count
	cmp	count,ax
	jne	x1cext
	cmp	flag,1
	je	x1cext
	cli
	mov	ax,1
	mov	parked,ax
	mov	flag,ax
	mov	ax,value
	mov	count,ax
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	mov	cs:oldax,ax
	pop	ax	;ip
	mov	cs:oldip,ax
	pop	ax      ;cs
	mov	cs:oldcs,ax
	pop	ax      ;flags
	mov	cs:oldflgs,ax
	mov	ax,offset parks
 ;Generate an interrupt return to the parker
	pushf
	push	cs
	push	ax
	mov	ax,cs:oldax
	sti
	jmp	dword ptr cs:[x1coff]
x1cext:	jmp	x1cout
parks:	sti
	push	ax
	push	cx
	push	dx
	push	ds
	mov	ax,cs
	mov	ds,ax
;
;
;	Do the BIOS's dirty work so this code will work!
;
	mov	al,020h
	out	020h,al
;
;
;
	mov	ah,08
	mov	dx,80h
	int	13h
	mov	dx,80h
 	inc	ch
	jnc	p1
	add	cl,40h
p1:	mov	ax,0c01h
	int	13h
;
	mov	ah,08
	mov	dx,81h
	int	13h
	mov	dx,81h
  	inc	ch
	jnc	p2
	add	cl,40h
p2:	mov	ax,0c01h
	int	13h
;
	xor	ax,ax
	mov	flag,ax
	pop	ds
	pop	dx
	pop	cx
	mov	ax,cs:oldflgs
	push	ax
	popf
	pop	ax
 	jmp	dword ptr cs:[oldip]
x1cout:	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
x1cou1:	jmp	dword ptr cs:[x1coff]
x13int:	push	ax
	push	ds
	sti
	mov	ax,cs
	mov	ds,ax
	cmp	flag,1
	je	x13in1
	mov	ax,value
	mov	count,ax
	xor	ax,ax
	mov	parked,ax
x13in1:	pop	ds
	pop	ax
	jmp	dword ptr cs:[x13off]
	dw	0
table	dw	0
init:	xor	ax,ax
	mov	parked,ax
	mov	bx,80h
	mov	al,[bx]
	cmp	al,0
	jne	init1
	int	21h
init1:	inc	bx
	mov	al,[bx]
	cmp	al,32
	je	init1
	cmp	al,13
	jne	init3
init2:	xor	ax,ax
	int	21h
init3:	cmp	al,49
	jb	init2
	cmp	al,57
	ja	init2
	lea	dx,mess
	mov	cx,ax
	mov	ah,09
	int	21h
	mov	ax,cx
	xor	ah,ah
	mov	bx,0fh
	and	ax,bx
	mov	dx,1091
	mul	dx
	mov	value,ax
	mov	count,ax
	cli
	push	es
	xor	ax,ax
	mov	es,ax
	mov	bx,76
	mov	ax,es:[bx]
	mov	x13off,ax
	mov	ax,offset x13int
	mov	es:[bx],ax
	inc	bx
	inc	bx
	mov	ax,es:[bx]
	mov	x13seg,ax
	mov	ax,cs
	mov	es:[bx],ax
	xor	ax,ax
	mov	flag,ax
	mov	es,ax
	mov	bx,112
	mov	ax,es:[bx]
	mov	x1coff,ax
	mov	ax,offset x1cint
	mov	es:[bx],ax
	inc	bx
	inc	bx
	mov	ax,es:[bx]
	mov	x1cseg,ax
	mov	ax,cs
	mov	es:[bx],ax
	pop	es
	sti
	mov	dx,offset table
	int	27h
mess	db	'Alpha Computer Service timed parker is activated'
	db	13,10,'for hard disks C and, if present, D'
	db	13,10,'$'
park	endp
code	ends
	end park
