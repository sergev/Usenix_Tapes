	.file	"term2init.c"
	.data
	.globl	_nulluser
	.comm	_nulluser,4
	.globl	_ply0
	.comm	_ply0,100
	.globl	_ply1
	.comm	_ply1,4
	.text
	.align	2
	.globl	_term2init
_term2init:
	bra	.L87
.L88:
	jbsr	_getlogin
	movl	d0,_ply1
	tstl	_ply1
	bne	.L91
	.text
	movl	#.L93,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addqw	#8,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L91:
.L97:
	.text
	movl	#.L99,sp@-
	jbsr	_printf
	addqw	#4,sp
	movl	#_ply0,sp@-
	jbsr	_gets
	addqw	#4,sp
	tstb	_ply0
	beq	.L100
	bra	.L96
.L100:
	bra	.L97
.L96:
	movl	#_ply0,sp@-
	jbsr	_iswizard
	addqw	#4,sp
	movl	d0,_iswiz
	movl	_ply1,sp@-
	jbsr	_iswizard
	addqw	#4,sp
	movl	d0,_iswiz+4
	.text
	movl	#.L103,sp@-
	movl	#_ply0,sp@-
	jbsr	_strcmp
	addqw	#8,sp
	tstl	d0
	bne	.L104
	movl	#1,_nulluser
.L104:
	tstl	_nulluser
	beq	.L105
	.text
	movl	#.L106,sp@-
	pea	a6@(-600)
	jbsr	_strcpy
	addqw	#8,sp
	bra	.L107
.L105:
	.text
	movl	#_ply0,sp@-
	jbsr	_getuser
	addqw	#4,sp
	movl	d0,sp@-
	movl	#.L108,sp@-
	pea	a6@(-600)
	jbsr	_sprintf
	addw	#12,sp
.L107:
	.text
	movl	#.L109,a6@(-604)
	movl	a6@(-604),sp@-
	pea	a6@(-600)
	jbsr	_strcmp
	addqw	#8,sp
	tstl	d0
	bne	.L110
	.text
	movl	#.L111,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addqw	#8,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L110:
	movl	_ply1,sp@-
	movl	#_ply0,sp@-
	pea	a6@(-600)
	jbsr	_page
	addw	#12,sp
	.text
	movl	#.L113,sp@-
	jbsr	_printf
	addqw	#4,sp
	movl	a6@(-604),sp@-
	pea	a6@(-600)
	jbsr	_do2term
	addqw	#8,sp
	pea	a6@(-600)
	jbsr	_geterm
	addqw	#4,sp
	bra	.L86
.L86:
	unlk	a6
	rts|0
.L87:
	link	a6,#-604
	tstw	sp@(-150)
	bra	.L88
	.data
	.text
	.align	2
	.globl	_geterm
_geterm:
	bra	.L117
.L118:
	.text
	movl	#.L119,sp@-
	movl	a6@(8),sp@-
	jbsr	_strcmp
	addqw	#8,sp
	tstl	d0
	bne	.L120
	.text
	movl	#_ttycs,sp@-
	movl	#.L123,sp@-
	jbsr	_getenv
	addqw	#4,sp
	movl	d0,sp@-
	jbsr	_fillterm
	addqw	#8,sp
	bra	.L116
.L120:
	.text
	movl	#.L124,sp@-
	movl	a6@(8),sp@-
	jbsr	_strcmp
	addqw	#8,sp
	tstl	d0
	bne	.L125
	.text
	movl	#.L126,a6@(-4)
	bra	.L127
.L125:
	.text
	movl	#8,sp@-
	movl	#.L129,sp@-
	movl	a6@(8),sp@-
	jbsr	_strncmp
	addw	#12,sp
	tstl	d0
	bne	.L10000
	movl	a6@(8),sp@-
	jbsr	_strlen
	addqw	#4,sp
	cmpl	#10,d0
	beq	.L131
.L10000:
	.text
	movl	a6@(8),sp@-
	movl	#.L132,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addw	#12,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
	bra	.L133
.L131:
	movl	a6@(8),d0
	addql	#8,d0
	movl	d0,a6@(-4)
.L133:
.L127:
	.text
	movl	a6@(-4),sp@-
	movl	#.L134,sp@-
	pea	a6@(-205)
	jbsr	_sprintf
	addw	#12,sp
	.text
	movl	#.L135,sp@-
	pea	a6@(-205)
	jbsr	_popen
	addqw	#8,sp
	movl	d0,a6@(-210)
	tstl	a6@(-210)
	bne	.L136
	.text
	pea	a6@(-205)
	movl	#.L137,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addw	#12,sp
	movl	#9,sp@-
	jbsr	_getpid
	movl	d0,sp@-
	jbsr	_kill
	addqw	#8,sp
.L136:
	.text
	movl	#0,sp@-
	movl	#.L141,sp@-
	movl	a6@(-210),sp@-
	jbsr	_strfind
	addw	#12,sp
	tstl	d0
	beq	.L142
	.text
	pea	a6@(-205)
	movl	#.L143,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addw	#12,sp
	movl	#9,sp@-
	jbsr	_getpid
	movl	d0,sp@-
	jbsr	_kill
	addqw	#8,sp
.L142:
	.text
	movl	#1,sp@-
	movl	#.L144,sp@-
	movl	a6@(-210),sp@-
	jbsr	_strfind
	addw	#12,sp
	tstl	d0
	beq	.L145
	.text
	movl	#.L146,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addqw	#8,sp
	movl	#9,sp@-
	jbsr	_getpid
	movl	d0,sp@-
	jbsr	_kill
	addqw	#8,sp
.L145:
	lea	a6@(-105),a0
	movl	a0,a6@(-4)
.L149:
	movl	a6@(-210),a0
	subql	#1,a0@
	bge	.L10001
	movl	a6@(-210),sp@-
	jbsr	__filbuf
	addqw	#4,sp
	bra	.L10002
.L10001:
	movl	a6@(-210),a0
	movl	a0@(4),d0
	addql	#1,a0@(4)
	movl	d0,a0
	movb	a0@,d0
	andl	#255,d0
.L10002:
	movb	d0,a6@(-5)
	cmpb	#32,d0
	beq	.L148
	cmpb	#10,a6@(-5)
	beq	.L148
	cmpb	#-1,a6@(-5)
	bne	.L150
	.text
	movl	#.L151,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addqw	#8,sp
	movl	#9,sp@-
	jbsr	_getpid
	movl	d0,sp@-
	jbsr	_kill
	addqw	#8,sp
	bra	.L152
.L150:
	movl	a6@(-4),a0
	movb	a6@(-5),a0@
	addql	#1,a6@(-4)
.L152:
	bra	.L149
.L148:
	movl	a6@(-4),a0
	clrb	a0@
.L154:
	movl	a6@(-210),a0
	subql	#1,a0@
	bge	.L10003
	movl	a6@(-210),sp@-
	jbsr	__filbuf
	addqw	#4,sp
	bra	.L10004
.L10003:
	movl	a6@(-210),a0
	movl	a0@(4),d0
	addql	#1,a0@(4)
	movl	d0,a0
	movb	a0@,d0
	andl	#255,d0
.L10004:
	cmpl	#-1,d0
	beq	.L153
	bra	.L154
.L153:
	movl	a6@(-210),sp@-
	jbsr	_fclose
	addqw	#4,sp
	movl	#_ttycs,sp@-
	pea	a6@(-105)
	jbsr	_fillterm
	addqw	#8,sp
	bra	.L116
.L116:
	unlk	a6
	rts|0
.L117:
	link	a6,#-212
	tstw	sp@(-148)
	bra	.L118
	.data
	.text
	.align	2
	.globl	_strfind
_strfind:
	bra	.L157
.L158:
	movl	a6@(12),a6@(-4)
.L161:
	movl	a6@(8),a0
	subql	#1,a0@
	bge	.L10006
	movl	a6@(8),sp@-
	jbsr	__filbuf
	addqw	#4,sp
	bra	.L10007
.L10006:
	movl	a6@(8),a0
	movl	a0@(4),d0
	addql	#1,a0@(4)
	movl	d0,a0
	movb	a0@,d0
	andl	#255,d0
.L10007:
	movb	d0,a6@(-9)
	cmpb	#-1,d0
	beq	.L10005
	cmpb	#10,a6@(-9)
	bne	.L162
	tstl	a6@(16)
	beq	.L162
.L10005:
	moveq	#-1,d0
	bra	.L156
.L162:
	movb	a6@(-9),d0
	movl	a6@(-4),a0
	movb	a0@,d1
	cmpb	d1,d0
	bne	.L163
	addql	#1,a6@(-4)
	movl	a6@(-4),a0
	tstb	a0@
	bne	.L164
	clrl	d0
	bra	.L156
.L164:
	bra	.L165
.L163:
	movl	a6@(-4),d0
	cmpl	a6@(12),d0
	beq	.L166
	movl	a6@(12),a6@(-8)
.L169:
	movl	a6@(-8),d0
	cmpl	a6@(-4),d0
	bge	.L168
	movb	a6@(-9),d0
	movl	a6@(-4),d1
	subl	a6@(-8),d1
	addl	a6@(12),d1
	movl	d1,a0
	movb	a0@,d1
	cmpb	d1,d0
	bne	.L170
	movl	a6@(-4),d0
	subl	a6@(-8),d0
	movl	d0,sp@-
	movl	a6@(12),sp@-
	movl	a6@(-8),sp@-
	jbsr	_strncmp
	addw	#12,sp
	tstl	d0
	bne	.L170
	bra	.L168
.L170:
	addql	#1,a6@(-8)
	bra	.L169
.L168:
	movl	a6@(-8),d0
	cmpl	a6@(-4),d0
	blt	.L10008
	movb	a6@(-9),d0
	movl	a6@(12),a0
	movb	a0@,d1
	cmpb	d1,d0
	bne	.L171
.L10008:
	movl	a6@(-4),d0
	subl	a6@(-8),d0
	addql	#1,d0
	addl	a6@(12),d0
	movl	d0,a6@(-4)
	bra	.L172
.L171:
	movl	a6@(12),a6@(-4)
.L172:
.L166:
.L165:
	bra	.L161
.L160:
	bra	.L156
.L156:
	unlk	a6
	rts|1
.L157:
	link	a6,#-12
	tstw	sp@(-148)
	bra	.L158
	.data
	.globl	_dups
	.comm	_dups,3600
	.align	2
	.globl	_lastdup
_lastdup:
	.long	0
	.text
	.align	2
	.globl	_getuser
_getuser:
	bra	.L176
.L177:
	.data
	.align	2
.L178:
	.long	-1
	.align	4
.L179:
	.space	36
	.text
	clrl	_lastdup
	moveq	#-1,d0
	cmpl	.L178,d0
	bne	.L180
	.text
	movl	#0,sp@-
	movl	#.L182,sp@-
	jbsr	_open
	addqw	#8,sp
	movl	d0,.L178
	bra	.L183
.L180:
	movl	#0,sp@-
	movl	#0,sp@-
	movl	.L178,sp@-
	jbsr	_lseek
	addw	#12,sp
.L183:
	moveq	#-1,d0
	cmpl	.L178,d0
	bne	.L184
	clrl	d0
	bra	.L175
.L184:
.L187:
	movl	#36,sp@-
	movl	#.L179,sp@-
	movl	.L178,sp@-
	jbsr	_read
	addw	#12,sp
	cmpl	#36,d0
	bne	.L186
	movl	#8,sp@-
	movl	a6@(8),sp@-
	movl	#.L179,sp@-
	jbsr	_strncmp
	addw	#12,sp
	tstl	d0
	bne	.L188
	movl	#36,sp@-
	movl	_lastdup,d0
	addql	#1,_lastdup
	movl	d0,sp@-
	jbsr	lmul
	addqw	#8,sp
	addl	#_dups,d0
	movl	d0,a6@(-108)
	movl	#.L179,a0
	movl	a6@(-108),a1
	addw	#36,a0
	addw	#36,a1
	movl	#8,d0
.L10009:
	movl	a0@-,a1@-
	dbra	d0,.L10009
.L188:
	bra	.L187
.L186:
	moveq	#1,d0
	cmpl	_lastdup,d0
	bne	.L189
	movl	#_dups+12,d0
	bra	.L175
.L189:
	tstl	_lastdup
	bne	.L190
	.text
	movl	#.L192,sp@-
	movl	#2,sp@-
	jbsr	_writes
	addqw	#8,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L190:
	.text
	movl	a6@(8),sp@-
	movl	#.L193,sp@-
	jbsr	_printf
	addqw	#8,sp
	clrl	a6@(-104)
.L196:
	movl	a6@(-104),d0
	cmpl	_lastdup,d0
	bge	.L195
	.text
	movl	#36,sp@-
	movl	a6@(-104),sp@-
	jbsr	lmul
	addqw	#8,sp
	addl	#_dups+12,d0
	movl	d0,sp@-
	movl	a6@(8),sp@-
	movl	#.L197,sp@-
	jbsr	_printf
	addw	#12,sp
	pea	a6@(-100)
	jbsr	_gets
	addqw	#4,sp
	cmpb	#121,a6@(-100)
	bne	.L198
	movl	#36,sp@-
	movl	a6@(-104),sp@-
	jbsr	lmul
	addqw	#8,sp
	addl	#_dups+12,d0
	bra	.L175
.L198:
	cmpb	#113,a6@(-100)
	bne	.L199
	clrl	d0
	bra	.L175
.L199:
	addql	#1,a6@(-104)
	bra	.L196
.L195:
	.text
	movl	#.L200,sp@-
	movl	#2,sp@-
	jbsr	_writes
	addqw	#8,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
	bra	.L175
.L175:
	unlk	a6
	rts|1
.L176:
	link	a6,#-108
	tstw	sp@(-152)
	bra	.L177
	.data
	.globl	_xpipe
	.comm	_xpipe,8
	.globl	_chand
	.comm	_chand,8
	.text
	.align	2
	.globl	_do2term
_do2term:
	bra	.L203
.L204:
	movl	#2,sp@-
	movl	a6@(8),sp@-
	jbsr	_open
	addqw	#8,sp
	movl	d0,_chand
	movl	#2,sp@-
	movl	a6@(12),sp@-
	jbsr	_open
	addqw	#8,sp
	movl	d0,_chand+4
	moveq	#-1,d0
	cmpl	_chand,d0
	bne	.L205
	movl	a6@(8),sp@-
	jbsr	_pout
	addqw	#4,sp
.L205:
	moveq	#-1,d0
	cmpl	_chand+4,d0
	bne	.L207
	movl	a6@(12),sp@-
	jbsr	_pout
	addqw	#4,sp
.L207:
	movl	#_xpipe,sp@-
	jbsr	_pipe
	addqw	#4,sp
	cmpl	#-1,d0
	bne	.L209
	.text
	movl	#.L210,sp@-
	jbsr	_pout
	addqw	#4,sp
.L209:
	movl	_chand,sp@-
	jbsr	_dochan
	addqw	#4,sp
	movl	_chand+4,sp@-
	jbsr	_dochan
	addqw	#4,sp
	bra	.L202
.L202:
	unlk	a6
	rts|0
.L203:
	link	a6,#0
	tstw	sp@(-144)
	bra	.L204
	.data
	.text
	.align	2
	.globl	_pout
_pout:
	bra	.L213
.L214:
	movl	a6@(8),sp@-
	jbsr	_perror
	addqw	#4,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
	bra	.L212
.L212:
	unlk	a6
	rts|0
.L213:
	link	a6,#0
	tstw	sp@(-140)
	bra	.L214
	.data
	.text
	.align	2
	.globl	_dochan
_dochan:
	bra	.L217
.L218:
	.data
	.align	2
.L219:
	.long	0
	.text
	.text
	movl	.L219,d0
	addql	#1,.L219
	movl	d0,sp@-
	movl	#.L220,sp@-
	pea	a6@(-50)
	jbsr	_sprintf
	addw	#12,sp
	jbsr	_fork
	bra	.L223
.L224:
	.text
	movl	#.L225,sp@-
	jbsr	_pout
	addqw	#4,sp
.L226:
	movl	.L219,d0
	subql	#1,d0
	bra	.L216
.L227:
	movl	#0,sp@-
	movl	#0,sp@-
	movl	a6@(8),sp@-
	jbsr	_fcntl
	addw	#12,sp
	cmpl	#-1,d0
	bne	.L229
	.text
	movl	#.L230,sp@-
	jbsr	_pout
	addqw	#4,sp
.L229:
	movl	#1,sp@-
	movl	#0,sp@-
	movl	_xpipe+4,sp@-
	jbsr	_fcntl
	addw	#12,sp
	cmpl	#-1,d0
	bne	.L231
	.text
	movl	#.L232,sp@-
	jbsr	_pout
	addqw	#4,sp
.L231:
	.text
	.text
	movl	#0,sp@-
	pea	a6@(-50)
	movl	#.L235,sp@-
	movl	#.L234,sp@-
	jbsr	_execl
	addw	#16,sp
	.text
	movl	#.L236,sp@-
	jbsr	_pout
	addqw	#4,sp
	bra	.L222
.L223:
	moveq	#-1,d1
	cmpl	d1,d0

	beq	.L224
	moveq	#0,d1
	cmpl	d1,d0

	beq	.L227
	bra	.L226
.L222:
	bra	.L216
.L216:
	unlk	a6
	rts|1
.L217:
	link	a6,#-52
	tstw	sp@(-152)
	bra	.L218
	.data
	.text
	.align	2
	.globl	_readc
_readc:
	bra	.L239
.L240:
	movl	#6,sp@-
	movl	a6@(8),sp@-
	movl	_xpipe,sp@-
	jbsr	_read
	addw	#12,sp
	bra	.L238
	bra	.L238
.L238:
	unlk	a6
	rts|1
.L239:
	link	a6,#0
	tstw	sp@(-148)
	bra	.L240
	.data
	.text
	.align	2
	.globl	_page
_page:
	bra	.L242
.L243:
	tstl	_nulluser
	beq	.L245
	bra	.L241
.L245:
	movl	a6@(8),sp@-
	jbsr	_okterm
	addqw	#4,sp
	tstl	d0
	bne	.L247
	.text
	movl	a6@(8),sp@-
	movl	a6@(12),sp@-
	movl	#.L248,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addw	#16,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L247:
	movl	#2,sp@-
	movl	a6@(8),sp@-
	jbsr	_access
	addqw	#8,sp
	cmpl	#-1,d0
	bne	.L250
	.text
	movl	a6@(12),sp@-
	movl	#.L251,sp@-
	movl	#__iob+28,sp@-
	jbsr	_fprintf
	addw	#12,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L250:
	jbsr	_fork
	tstl	d0
	bne	.L252
	.text
	.text
	movl	#0,sp@-
	movl	a6@(16),sp@-
	movl	a6@(8),sp@-
	movl	#.L254,sp@-
	movl	#.L253,sp@-
	jbsr	_execl
	addw	#20,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
.L252:
	.text
	movl	#.L255,sp@-
	movl	#1,sp@-
	jbsr	_writes
	addqw	#8,sp
	movl	#120,sp@-
	jbsr	_alarm
	addqw	#4,sp
	movl	#_onalrm,sp@-
	movl	#14,sp@-
	jbsr	_signal
	addqw	#8,sp
	.text
	movl	a6@(8),sp@-
	movl	#.L257,sp@-
	jbsr	_printf
	addqw	#8,sp
.L259:
	movl	a6@(8),sp@-
	jbsr	_okterm
	addqw	#4,sp
	tstl	d0
	beq	.L258
	bra	.L259
.L258:
	movl	#1,sp@-
	movl	#14,sp@-
	jbsr	_signal
	addqw	#8,sp
	bra	.L241
	bra	.L241
.L241:
	unlk	a6
	rts|0
.L242:
	link	a6,#0
	tstw	sp@(-156)
	bra	.L243
	.data
	.text
	.align	2
	.globl	_okterm
_okterm:
	bra	.L261
.L262:
	pea	a6@(-30)
	movl	a6@(8),sp@-
	jbsr	_stat
	addqw	#8,sp
	btst	#5,a6@(-25)
	bne	.L10010
	btst	#2,a6@(-25)
	beq	.L264
.L10010:
	clrl	d0
	bra	.L260
.L264:
	moveq	#1,d0
	bra	.L260
	bra	.L260
.L260:
	unlk	a6
	rts|1
.L261:
	link	a6,#-32
	tstw	sp@(-144)
	bra	.L262
	.data
	.text
	.align	2
	.globl	_onalrm
_onalrm:
	bra	.L266
.L267:
	.text
	movl	#.L268,sp@-
	movl	#1,sp@-
	jbsr	_writes
	addqw	#8,sp
	movl	#1,sp@-
	jbsr	_exit
	addqw	#4,sp
	bra	.L265
.L265:
	unlk	a6
	rts|0
.L266:
	link	a6,#0
	tstw	sp@(-144)
	bra	.L267
	.data
.L93:
	.byte	0x44,0x6f,0x6e,0x27,0x74,0x20,0x6b,0x6e
	.byte	0x6f,0x77,0x20,0x77,0x68,0x6f,0x20,0x79
	.byte	0x6f,0x75,0x20,0x61,0x72,0x65,0x2e,0xa
	.byte	0x0
.L99:
	.byte	0x57,0x68,0x6f,0x20,0x77,0x6f,0x75,0x6c
	.byte	0x64,0x20,0x79,0x6f,0x75,0x20,0x6c,0x69
	.byte	0x6b,0x65,0x20,0x74,0x6f,0x20,0x70,0x6c
	.byte	0x61,0x79,0x20,0x77,0x69,0x74,0x68,0x20
	.byte	0x3f,0x20,0x0
.L103:
	.byte	0x2d,0x6e,0x75,0x6c,0x6c,0x0
.L106:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x6e,0x75,0x6c
	.byte	0x6c,0x0
.L108:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x25,0x73,0x0
.L109:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x74,0x74,0x79
	.byte	0x0
.L111:
	.byte	0x59,0x6f,0x75,0x20,0x63,0x61,0x6e,0x27
	.byte	0x74,0x20,0x65,0x78,0x70,0x65,0x63,0x74
	.byte	0x20,0x74,0x6f,0x20,0x70,0x6c,0x61,0x79
	.byte	0x20,0x77,0x69,0x74,0x68,0x20,0x79,0x6f
	.byte	0x75,0x72,0x73,0x65,0x6c,0x66,0x21,0xa
	.byte	0x0
.L113:
	.byte	0x52,0x65,0x73,0x70,0x6f,0x6e,0x73,0x65
	.byte	0x2e,0x2e,0x2e,0x2e,0xa,0xd,0x0
.L119:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x6e,0x75,0x6c
	.byte	0x6c,0x0
.L123:
	.byte	0x54,0x45,0x52,0x4d,0x0
.L124:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x63,0x6f,0x6e
	.byte	0x73,0x6f,0x6c,0x65,0x0
.L126:
	.byte	0x63,0x6f,0x0
.L129:
	.byte	0x2f,0x64,0x65,0x76,0x2f,0x74,0x74,0x79
	.byte	0x0
.L132:
	.byte	0x75,0x6e,0x72,0x65,0x63,0x6f,0x67,0x6e
	.byte	0x69,0x7a,0x65,0x64,0x20,0x74,0x74,0x79
	.byte	0x20,0x6e,0x61,0x6d,0x65,0x20,0x25,0x73
	.byte	0xa,0x0
.L134:
	.byte	0x70,0x73,0x20,0x77,0x77,0x65,0x74,0x25
	.byte	0x73,0x0
.L135:
	.byte	0x72,0x0
.L137:
	.byte	0x63,0x61,0x6e,0x27,0x74,0x20,0x65,0x78
	.byte	0x65,0x63,0x75,0x74,0x65,0x20,0x25,0x73
	.byte	0xa,0x0
.L141:
	.byte	0x6f,0x6b,0x67,0x61,0x6c,0x61,0x78,0x79
	.byte	0x0
.L143:
	.byte	0x63,0x61,0x6e,0x27,0x74,0x20,0x66,0x69
	.byte	0x6e,0x64,0x20,0x6f,0x6b,0x67,0x61,0x6c
	.byte	0x61,0x78,0x79,0x20,0x69,0x6e,0x20,0x25
	.byte	0x73,0xa,0x0
.L144:
	.byte	0x20,0x54,0x45,0x52,0x4d,0x3d,0x0
.L146:
	.byte	0x63,0x61,0x6e,0x27,0x74,0x20,0x66,0x69
	.byte	0x6e,0x64,0x20,0x54,0x45,0x52,0x4d,0x20
	.byte	0x74,0x79,0x70,0x65,0xa,0x0
.L151:
	.byte	0x70,0x73,0x20,0x6f,0x75,0x74,0x70,0x75
	.byte	0x74,0x20,0x66,0x6f,0x72,0x6d,0x61,0x74
	.byte	0x20,0x65,0x72,0x72,0x6f,0x72,0xa,0x0
.L182:
	.byte	0x2f,0x65,0x74,0x63,0x2f,0x75,0x74,0x6d
	.byte	0x70,0x0
.L192:
	.byte	0x6e,0x6f,0x74,0x20,0x6c,0x6f,0x67,0x67
	.byte	0x65,0x64,0x20,0x69,0x6e,0x2e,0xa,0xd
	.byte	0x0
.L193:
	.byte	0x25,0x73,0x20,0x6c,0x6f,0x67,0x67,0x65
	.byte	0x64,0x20,0x69,0x6e,0x20,0x6d,0x6f,0x72
	.byte	0x65,0x20,0x74,0x68,0x65,0x6e,0x20,0x6f
	.byte	0x6e,0x63,0x65,0x2e,0xa,0x0
.L197:
	.byte	0x25,0x73,0x20,0x6f,0x6e,0x20,0x25,0x73
	.byte	0x20,0x3f,0x20,0x0
.L200:
	.byte	0x6e,0x6f,0x20,0x6d,0x6f,0x72,0x65,0x20
	.byte	0x74,0x74,0x79,0x73,0xa,0xd,0x0
.L210:
	.byte	0x70,0x69,0x70,0x65,0x0
.L220:
	.byte	0x25,0x64,0x0
.L225:
	.byte	0x66,0x6f,0x72,0x6b,0x0
.L230:
	.byte	0x64,0x75,0x70,0x32,0x20,0x31,0x0
.L232:
	.byte	0x64,0x75,0x70,0x32,0x20,0x32,0x0
.L234:
	.byte	0x2f,0x75,0x73,0x72,0x2f,0x67,0x61,0x6d
	.byte	0x65,0x73,0x2f,0x6c,0x69,0x62,0x2f,0x67
	.byte	0x61,0x6c,0x61,0x78,0x79,0x2f,0x6c,0x6f
	.byte	0x63,0x61,0x6c,0x0
.L235:
	.byte	0x6c,0x6f,0x63,0x61,0x6c,0x0
.L236:
	.byte	0x6c,0x6f,0x63,0x61,0x6c,0x0
.L248:
	.byte	0x25,0x73,0x27,0x73,0x20,0x74,0x65,0x72
	.byte	0x6d,0x69,0x6e,0x61,0x6c,0x2c,0x20,0x25
	.byte	0x73,0x2c,0x20,0x6d,0x6f,0x64,0x65,0x20
	.byte	0x69,0x73,0x20,0x62,0x61,0x64,0x20,0x66
	.byte	0x6f,0x72,0x20,0x67,0x61,0x6c,0x61,0x78
	.byte	0x79,0xa,0x0
.L251:
	.byte	0x43,0x61,0x6e,0x27,0x74,0x20,0x77,0x72
	.byte	0x69,0x74,0x65,0x20,0x25,0x73,0x2e,0xa
	.byte	0x0
.L253:
	.byte	0x2f,0x75,0x73,0x72,0x2f,0x67,0x61,0x6d
	.byte	0x65,0x73,0x2f,0x6c,0x69,0x62,0x2f,0x67
	.byte	0x61,0x6c,0x61,0x78,0x79,0x2f,0x70,0x61
	.byte	0x67,0x65,0x72,0x0
.L254:
	.byte	0x70,0x61,0x67,0x65,0x72,0x0
.L255:
	.byte	0x50,0x6c,0x65,0x61,0x73,0x65,0x20,0x77
	.byte	0x61,0x69,0x74,0x2e,0x2e,0x2e,0x2e,0x2e
	.byte	0xa,0x0
.L257:
	.byte	0x53,0x6c,0x65,0x65,0x70,0x69,0x6e,0x67
	.byte	0x20,0x6f,0x6e,0x20,0x25,0x73,0xa,0x0
.L268:
	.byte	0x54,0x69,0x6d,0x65,0x6f,0x75,0x74,0x2e
	.byte	0xd,0xa,0x0
