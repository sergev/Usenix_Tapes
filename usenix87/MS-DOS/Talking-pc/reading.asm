
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	reading.asm: reading words for the talking console device driver

	include parms.h

PGROUP	group	PROG, DATA
	assume cs:PGROUP, ds:PGROUP

DATA	segment	word public 'DATA'

	extrn bufhead:word, buftail:word, bufcur:word, buftop:word, bufbot:word
	extrn wdbuf:byte
	public inspeech, rdflag
	public rdlines, onesymb

inspeech db 0 ; set when physically reading
rdflag	db 0 ; set when scheduled to read (mode)
rdlines	db 0 ; reading one line at a time, or to the end
onesymb	db 0 ; read only one symbol
waspunc	db 0 ; read non alphanumeric

roottype db 0 ; type of suffix removed to produce root word
vowels db 1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0
;	tables to reconstruct words
suftab	db "s   es  ies ing ing ing d   ed  ed  ied "
sufdrop	db "  y  e   y"
sufadd	db 1,2,3,3,4,3,1,3,2,3
sufdub	db 0,0,0,0,1,0,0,1,0,0
len	dw 0 ; length of word
;	legal english three letter initial consonent clusters
iclu	db "chrchlphrphlsclschscrshlshrshwsphsplsprstrthrthw~"

DATA	ends

UDATA   segment public 'UDATA' ; for finding end of data segment
UDATA   ends


PROG	segment	byte public 'PROG'

	extrn ss_text:near
	extrn incbptr:near, decbptr:near
	extrn curchar:near
	public reading

;	reading the buffer while application programs run
reading	proc near
	sti
	cmp rdflag,0
	jnz rdenb
	sti
	ret
rdenb: ; reading enabled
	mov waspunc,0
	mov bx,bufcur
rd3:
	mov al,[bx]
	call incbptr
;	Skip whitespace,
;	and read symbols.
;	if it is a letter, construct and read the word,
	cmp al,' '
	jz rd3
	mov ah,al
	or al,20h
	jns noteof
	call decbptr
rdend:	cmp bx,bufhead
	jz rd1
	cmp onesymb,0
	jz adv
	cmp waspunc,0
	jnz rd1
	mov al,[bx]
	cmp al,'0'
	jb rd1
	or al,20h
	cmp al,'z'
	ja rd1
	cmp al,'9'
	jbe adv
	cmp al,'a'
	jae adv
;	one symbol at a time, or EOF
rd1:	call decbptr
	mov rdflag,0
	mov onesymb,0
adv:	mov bufcur,bx
	sti
	ret

noteof:
	mov di,ADDR: wdbuf
	mov cx,WDLEN
	call decbptr
	cmp al,'z'
	ja rdsymb
	cmp al,'a'
	jae rdwd
	cmp al,'0'
	jb rdsymb
	cmp al,'9'
	ja rdsymb
;	read number, up to WDLEN digits
num1:	mov [di],al
	inc di
	call incbptr
	mov al,[bx]
	cmp al,'0'
	jb endnum
	cmp al,'9'
	ja endnum
	loop num1
endnum:	mov byte ptr [di],0
	call rdend
	jmp spkstring

rdsymb:	inc waspunc ; read symbol
	mov bufcur,bx
	sti
	call curchar
	cli
	mov bx,bufcur
	mov al,[bx]
	cmp al,13
	jnz rd2
	cmp rdlines,0
	jz rd2
	mov rdflag,0
	mov onesymb,0
	call decbptr
rd2:	call incbptr
	jmp rdend

rdwd: ; read word
	mov [di],al
	inc di
	call incbptr
	mov al,[bx]
	or al,20h
	cmp al,'a'
	jb endwd
	cmp al,'z'
	ja endwd
	loop rdwd
endwd:	mov byte ptr [di],0
	call rdend

;	determine length of word
	sub di,ADDR: wdbuf
	mov len,di

	call lookup
	jc spkstring
	call mkroot
	jnc acrchk
	call lookup
	pushf
	call reconst
	popf
	jc spkstring

acrchk:	call acron

spkstring:
	mov bx,ADDR: wdbuf
	jmp ss_text
reading	endp

;	extract the root word
mkroot	proc near
	mov bh,0
	mov roottype,bh
	mov ax,len
	mov si,ADDR: wdbuf
sub ax,5
	jb noroot ; word to short to safely rootinize
	add si,ax
	mov al,[si+3]
	mov ah,[si+2]
	cmp byte ptr [si+4],'s'
	jnz nopl
;	possible plural
	cmp al,'s'
	jz noroot
	cmp al,'i'
	jz noroot
	cmp al,'a'
	jz noroot
	cmp al,'u'
	jz noroot
	cmp al,'e'
	jnz pl1
	cmp ah,'i'
	jz ies
	cmp ah,'h'
	jz ches
	cmp ah,'z'
	jz ches
	cmp ah,'s'
	jz ches
;	normal plural
pl1:mov byte ptr [si+4],0
	dec len
	inc roottype
	stc
	ret
ies:	mov byte ptr [si+2],'y'
	inc roottype
;	churches type plural
ches:	mov byte ptr [si+3],0
	sub len,2
	add roottype,2
	stc
	ret
noroot:	clc
	ret
	nopl:cmp len,6
	jb noroot ; too short
	cmp byte ptr [si+4],'g'
	jnz noing
;	possible present progressive
	cmp al,'n'
	jnz noroot
	cmp ah,'i'
	jnz noroot
	mov al,[si+1]
	mov ah,[si]
	mov bl,al
	cmp vowels[bx-'a'],0
	jnz ing1
	cmp al,ah
	jnz no1pair
	mov byte ptr [si+1],0
	sub len,4
	mov roottype,5
	stc
	ret
ing1:	mov byte ptr [si+2],0
	sub len,3
	mov roottype,4
	stc
	ret
no1pair:mov bl,ah
	cmp vowels[bx-'a'],0
	jz ing1
	cmp ah,'w'
	jae ing1
	mov bl,[si-1]
	cmp vowels[bx-'a'],0
	jnz ing1
	mov byte ptr [si+3],0
	mov byte ptr [si+2],'e'
	sub len,2
	mov roottype,6
	stc
	ret
noing:	cmp byte ptr [si+4],'d'
	jnz noroot
;	possible past tense
	cmp al,'e'
	jnz noroot
	cmp ah,'i'
	jz ied
	mov bl,ah
	mov al,[si+1]
	cmp vowels[bx-'a'],0
	jnz ed1
	cmp al,ah
	jnz no2pair
	mov byte ptr [si+2],0
	sub len,3
	mov roottype,8
	stc
	ret
no2pair:	mov bl,al
	cmp vowels[bx-'a'],0
	jz ched
	cmp al,'w'
	jae ched
	mov bl,[si]
	cmp vowels[bx-'a'],0
	jnz ched
ed1:	mov roottype,6
	jmp pl1
ied:	mov byte ptr [si+2],'y'
	inc roottype
ched:	mov byte ptr [si+3],0
	sub len,2
	add roottype,9
	stc
	ret
mkroot	endp

;	reconstruct word, based on root and removed suffixes
reconst	proc near
	mov si,ADDR: wdbuf-1
	add si,len
	mov bl,roottype
	mov bh,0
	dec bx
	mov ah,0
	mov al,sufadd[bx]
	add len,ax
	mov al,[si]
	cmp sufdub[bx],0
	jz nodub
	inc si
	mov [si],al
nodub:	cmp al,sufdrop[bx]
	jnz nodrop
	dec si
	dec len
nodrop:	shl bx,1
	shl bx,1
	add bx,ADDR: suftab
	mov cx,3
sf1:	mov al,[bx]
	inc si
	mov [si],al
	inc bx
	loop sf1
	mov bx,len
	mov wdbuf[bx],0
	ret
	reconst	endp

;	look up word in pronounciation table
lookup	proc near
	mov dx,len ; length of word
	mov si,ADDR: UDATA
	mov ch,0
lk1:	mov cl,[si]
	jcxz lk2
	cmp dx,cx
	jz lk3 ; same length?
lk4:	add si,cx
	inc si
	mov cl,[si]
	add si,cx
	inc si
	jnc lk1 ; next word in table
lk3:	mov di,si
	mov bx,ADDR: wdbuf-1
	sub di,bx
lk5:	inc bx
	mov al,[bx]
	cmp al,[bx+di]
	jnz lk6
	loop lk5
	mov cl,[si]
	add si,cx
	inc si
	mov cl,[si]
	mov len,cx
	mov bx,ADDR: wdbuf-1
	sub si,bx
lk7:	inc bx
	mov al,[bx+si]
	mov [bx],al
	loop lk7
	mov byte ptr [bx+1],0
	stc ; word replaced
	ret
lk6:	mov cl,[si]
	jnz lk4
lk2:clc ; not found
	ret
lookup	endp

;	if it is an acronym, insert blanks to pronounce letters
acron	proc near
;	any vowels in the first four letters?
	mov ax,len
	cmp ax,4
	jb ac2
	mov ax,4
ac2:	xchg ax,si
	xor bx,bx
	xor ax,ax
ac3:	mov bl,wdbuf[si-1]
	add al,vowels[bx-'a']
	dec si
	jnz ac3
	cmp al,0
	jz blank ; forget it, no vowels
	cmp al,4
	jz blank ; too many vowels
	cmp ax,len
	jz blank ; all vowels

	cmp al,1
	jnz nostr
	cmp len,4
	jb nostr
	mov bl,wdbuf+3
	cmp byte ptr vowels[bx-'a'],0
	jz nostr
	mov bx,ADDR: iclu
ac4:mov si,0
ac5:	mov al,[bx+si]
	cmp al,wdbuf[si]
	jnz ac6
	inc si
	cmp si,3
	jnz ac5
nostr:	ret ; no acronym
ac6:	add bx,3
	cmp byte ptr [bx],'~' ; end of table?
	jnz ac4

blank: ; fill with blanks, so letters are pronounced
	mov si,len
	mov bx,si
	add bx,si
	mov di,bx
ac1:	dec bx
	mov byte ptr wdbuf[bx],' '
	dec bx
	mov al,wdbuf[si-1]
	mov wdbuf[bx],al
	dec si
	jnz ac1
	mov wdbuf[di-1],0
	ret
acron	endp

PROG	ends

	end
