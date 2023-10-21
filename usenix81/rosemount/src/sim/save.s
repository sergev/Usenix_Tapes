/ Routines to 'save' and 'restore' the stack enviroment.
/
/ 'save' stores the stack in an area designated by 'malloc'
/ and returns a pointer to the area.
/
/ 'restore' takes the data from a previous call to 'save' and
/ writes it back in the stack area.  When 'restore' returns
/ it looks like a return from the call to 'save' except zero
/ is returned instead of the pointer.
/
/ 'save' returns -1 if there is no room left from malloc.

 .globl _save,_restore
 .globl _malloc,_free
 .globl csv,cret


_save:
	jsr r5,csv
	clr r0									/ get length of stack
	sub sp,r0
	add $4,r0
	mov r0,(sp)
	jsr pc,*$_malloc
	cmp $-1,r0
	beq 2f									/ - error?
	mov sp,r1
	mov r0,r2
	mov sp,(r2)+
	mov r5,(r2)+
1:
	mov (r1)+,(r2)+
	tst r1
	bne 1b
2:
	jmp cret





_restore:
	jsr r5,csv
	mov 4(r5),r0
	mov r0,r1
	mov (r1)+,sp
	mov (r1)+,r5
	mov sp,r2
1:
	mov (r1)+,(r2)+
	tst r2
	bne 1b
	mov r0,(sp)
	jsr pc,*$_free
	clr r0
	jmp cret
