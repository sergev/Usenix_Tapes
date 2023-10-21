1207a
	cmp	ybuf,r1
	bne	3f
	sys	open; dev; 0
	bes	3f
.
1206c
	mov	ybuf,r1
	sys	stat; dev; ybuf
.
5a
/	modified 05-Jun-1980 by D A Gwyn:
/	1) "ttyn" fix across devices.

.
4c
/ nroff1.s - "nroff" text formatter part 1
.
w
q
