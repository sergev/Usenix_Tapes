	.TITLE	UX.SML


;***************************

.MACRO	$PARAM
	R0=%0
	R1=%1
	R2=%2
	R3=%3
	R4=%4
	R5=%5
	SP=%6
	PC=%7
.ENDM	$PARAM

;***************************

.MACRO	$INDIR
	104400 +	0
.ENDM	$INDIR

;***************************

.MACRO	$EXIT
	104400 +	1
.ENDM	$EXIT

;***************************

.MACRO	$FORK
	104400 +	2
.ENDM	$FORK

;***************************

.MACRO	$READ
	104400 +	3
.ENDM	$READ

;***************************

.MACRO	$WRITE
	104400 +	4
.ENDM	$WRITE

;***************************

.MACRO	$OPEN
	104400 +	5
.ENDM	$OPEN

;***************************

.MACRO	$CLOSE
	104400 +	6
.ENDM	$CLOSE

;***************************

.MACRO	$WAIT
	104400 +	7
.ENDM	#WAIT

;***************************

.MACRO	$CREAT
	104400 +	8.
.ENDM	$CREAT


;***************************

.MACRO	$LINK
	104400 +	9.
.ENDM	$LINK


;***************************

.MACRO	$UNLINK
	104400 +	10.
.ENDM	$UNLINK

;******************************

.MACRO	$EXEC
	104400 +	11.
.ENDM	$EXEC

;***************************

.MACRO	$CHDIR
	104400 +	12.
.ENDM	$CHDIR


;****************************

.MACRO	$TIME
	104400 +	13.
.ENDM	$TIME

;****************************

.MACRO	$MKNOD
	104400 +	14.
.ENDM	$MKNOD

;****************************

.MACRO	$CHMOD
	104400 +	15.
.ENDM	$CHMOD

;***************************

.MACRO	$CHOWN
	104400 +	16.
.ENDM	$CHOWN

;***************************

.MACRO	$BREAK
	104400 +	17.
.ENDM	$BREAK

;***************************

.MACRO	$STAT
	104400 +	18.
.ENDM	$STAT

;***************************

.MACRO	$SEEK
	104400 +	19.
.ENDM	$SEEK

;******************************

.MACRO	$MOUNT
	104400 +	21.
.ENDM	$MOUNT

;*****************************

.MACRO	$UMOUNT
	104400 +	22.
.ENDM	$UMOUNT

;****************************

.MACRO	$SETUID
	104400 +	23.
.ENDM	$SETUID

;****************************

.MACRO	$GETUID
	104400 +	24.
.ENDM	$GETUID

;***************************

.MACRO	$STIME
	104400 +	25.
.ENDM	$STIME

;******************************

.MACRO	$FSTAT
	104400 +	28.
.ENDM	$FSTAT

;*****************************

.MACRO	$SMDATE
	104400 +	30.
.ENDM	$SMDATE

;*****************************

.MACRO	$STTY
	104400 +	31.
.ENDM	$STTY

;****************************

.MACRO	$GTTY
	104400 +	32.
.ENDM	$GTTY

;****************************

.MACRO	$NICE
	104400 +	34.
.ENDM	$NICE

;****************************

.MACRO	$SLEEP
	104400 +	35.
.ENDM	$SLEEP

;*****************************

.MACRO	$SYNC
	104400 +	36.
.ENDM	$SYNC

;*****************************

.MACRO	$KILL
	104400 +	37.
.ENDM	$KILL

;*****************************

.MACRO	$SWITCH
	104400 +	38.
.ENDM	$SWITCH

;****************************

.MACRO	$DUP
	104400 +	41.
.ENDM	$DUP

;*****************************

.MACRO	$PIPE
	104400 +	42.
.ENDM	$PIPE

;*****************************

.MACRO	$TIMES
	104400 +	43.
.ENDM	$TIMES

;****************************

.MACRO	$PROF
	104400 +	44.
.ENDM	$PROF

;*****************************

.MACRO	$SETGID
	104400 +	46.
.ENDM	$SETGID

;****************************

.MACRO	$GETGID
	104400 +	47.
.ENDM	$GETGID

;****************************

.MACRO	$SIG
	104400 +	48.
.ENDM	$SIG

;******************************


.END
