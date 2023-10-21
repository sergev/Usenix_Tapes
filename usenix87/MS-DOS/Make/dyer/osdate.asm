dos	=	21h

arg1	=	4			; lattice argument indexes
arg2	=	arg1+2
arg3	=	arg2+2

pgroup	group	prog
prog	segment byte public 'prog'
	public	osdate
	assume	cs:pgroup

;
;------
; OSDATE - return file's creation-date (called from Lattice), or -1
;	   if can't find the file.
; Synopsis:
;		int osdate(filename, time1, time2)
;			char *filename;
;			int *time1, *time2;
;
osdate proc near
	push	bp
	mov	bp,sp

;--- Open the file
	mov	dx,[bp+arg1]
	xor	al,al
	mov	ah,3dh
	int	dos
	jc	osd$err			; can't, so complain

;--- Get file's creation date and time
	mov	bx,ax			; get handle's date info
	xor	al,al
	mov	ah,57h
	int	dos
	jc	osd$cls			; "can't happen" (but close it)

;--- Install date/time info into caller's variables
	mov	si,[bp+arg2]		; *arg2 = time (least significant)
	mov	[si],cx
	mov	si,[bp+arg3]		; *arg3 = date (most significant)
	mov	[si],dx

;--- Close file & return (ok)
	mov	ah,3eh
	int	dos
	xor	ax,ax
	pop	bp
	ret

;--- Close file & return error condition
osd$cls:
	mov	ah,3eh
	int	dos
osd$err:
	mov	ax,-1
	pop	bp
	ret
osdate endp

prog	ends
	end
