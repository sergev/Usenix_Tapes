.globl  _sysrebt    	//reboot the system by reading fsboot
                    	// from rk0 block 0 and executing it
//arg:
//	-1	->	boot default system (/unix)
//	0	->	ask console for system name
//	other	->	address of name of system to pass to fsboot

_sysrebt:
	bis	$340,PS		// turn off interrupts
	tst 	(sp)+		//forget return address
	mov	(sp),r4		//get argument
	clr	RKCS		//turn off disk
1:
	bit	$RDY,RKCS	//wait for ready
	beq	1b
	mov	$RKMR,r0
	clr	-(r0)		//disk address
	clr	-(r0)		//bus address
	clr	-(r0)		//word count
	mov	$011,-(r0)	//seek+go
1:
	bit	$DRDY,RKDS	//wait for seek
	beq	1b
1:
	bit	$RDY,RKCS	//wait for controller
	beq	1b
	mov 	$10.,r1
0:
	mov 	$RKMR,r0
	clr 	-(r0)       	//clear disk address
	clr 	-(r0)       	//bus address
	mov 	$-256.,-(r0)    //set word count
	mov 	$5,-(r0)        //read and go
1:
	bit 	$RDY,(r0)       //wait for read to finish
	beq 	1b
	bit 	$RKERR,(r0)     //test for error
	beq	1f		//no error
	sob 	r1,0b           //try 10 times
	halt
1:
	tst	r4		//type of reboot
	beq	3f		//0  -> ask console for system name
	mov	$6,r3		//address in fsboot for string
	mov	$6,reboot	// put pointer in reboot
	cmp	r4,$-1		//-1 -> default boot
	beq	2f		//boot unix
				//other -> arg is ptr to system name
	mov	$18.,r2		//max system name length for fsboot
	mov	r4,(sp)
1:
	jsr	pc,_fubyte	//get one char
	tst	r0
	blt	3f		//bad ptr, ask console
	movb	r0,(r3)+	//put char in fsboot
	cmp	r0,$'\n		//end of string token
	beq	2f		//execute fsboot
	inc	(sp)		//next char
	sob	r2,1b
3:
	clr	reboot		//ask console for name of system
2:
	reset
	bic	$340,PS		// turn on interrupts
	clr	pc		//execute fsboot

RKCS	=	0177404
RKDS	=	0177400
DRDY	=	0100
RKMR	=	177414		//end of rk registers
RDY	=	200		//rk controller ready
RKERR	=	140000		//rk error bits
halt	=	0
reboot	=	4		//reboot address
