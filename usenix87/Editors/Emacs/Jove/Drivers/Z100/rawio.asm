; raw i/o returns using ibm bios ints.
; k. mitchum 12/84.

	include model.h
	include prologue.h

@z100	equ	TRUE

	public rawgetc, rawchkc

if @bigmodel
rawchkc	proc far
else
rawchkc	proc near
endif
if @z100
		mov dl,0ffh
		mov	ah,6
		int 21h
else
		mov	ah,1
		int	16h
endif
		jz	rawchk1	;if no character
		mov	ax,1
		ret
rawchk1:
		mov	ax,0
		ret
rawchkc	endp

if @bigmodel
rawgetc	proc far
else
rawgetc	proc near
endif
if @z100
		mov	ah,7
		int	21h
else
		mov	ah,0
		int	16h
endif
		ret
rawgetc	endp

	include epilogue.h

	end

