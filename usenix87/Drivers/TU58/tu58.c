#include "tu58.h"
#include <stdio.h>
#include "debug.h"

 extern int file[]; 
 extern char ronly[];

 int state = TUS_IDLE;
 struct packet pk, dk, ek;

/*
 * get requests from host
 */

tu58()
{
	dk.pk_flag = TUF_DATA;		/* only one data packet flag */
	ek.pk_flag = TUF_CMD;
	ek.pk_mcount = CMDLEN;
	ek.pk_sw = 0;
	ek.pk_seq = 0;

	while( state = evalflag() );
}

evalflag()		/* get flag & evaluate type of packet & execute */
{
	pk.pk_flag = cget();
	debugd("flag:",pk.pk_flag);
	switch (pk.pk_flag) {
		case EOF:		/* finished */
			return TUS_QUIT;
		case TUF_NULL:		/* send during an break & init */
			break;
		case TUF_DATA:
			if( state & TUS_HTT ) {
				if( hosttu() )
					return TUS_IDLE;
				return TUS_HTT;
			}
			fprintf(stderr, "Data packet not expected\n");
			break;
		case TUF_CMD:			/* control packet */
			if( state & TUS_IDLE ) {
				state = TUS_HTT;
				return cmd();
			}
			fprintf(stderr, "Command pack not expected\n");
			break;
		case TUF_INITF:			/* a 'single byte' packet */
			cget();	/* discard first initf char (p 3-3) */
			clearbuf();
			cput(TUF_CONT);   /* send back 'single byte' */
			cflush();
			break;
		case TUF_BOOT:
			fprintf(stderr,"will read in boot block\n");
			bootio(file[cget()]);
			break;
		case TUF_CONT:			/* a 'single byte' packet */
		case TUF_XON:
			state &= ~TUS_WAIT;
			return state;
		case TUF_XOFF:			/* a 'single byte' packet */
			state |= TUS_WAIT;
			return state;
		default:
			fprintf(stderr, "Unknown packet flag %o\n", pk.pk_flag);
			cput(TUF_INITF);
			cput(TUF_INITF);
			cflush();
	}
	return TUS_IDLE;
}

cmd()			 /* judge op code of command packet */
{
	pk.pk_mcount = cget();		/* mcount = CMDLEN */
	if ((pk.pk_mcount < 0) || /* sanity check mcount */
	  (pk.pk_mcount > (sizeof(struct packet)-(sizeof(int)+sizeof(char))))) {
		debugd("bad mcount in cmd",pk.pk_mcount);
		return TUS_IDLE;
	}
	/* bloody hack follows:
		A pointer into a packet struct is passed to getpacket (arg2)
		These elements should be acquired one by one and placed
			more delicately into the struct, but this works
			if the struct is packed, and bytes are in vax
			order, etc.
	*/
	if( !getpacket(&pk,(u_char *)&pk.pk_op) ) {
		debugs("cmd: checksum err");
		return TUS_IDLE;
	}

	debugd("op code:",pk.pk_op);
	switch(pk.pk_op){
	case TUOP_NOOP:
		state = TUS_TTH;
		sendend( 0, TUE_SUC, 0, 0);
		break;
	case TUOP_INIT:
		clearbuf();
	case TUOP_DIAGNOSE:
	case TUOP_END:
		state = TUS_TTH;
		sendend( TUOP_END, TUE_SUC, 0, 0);
		break;
	case TUOP_READ:
		state =  TUS_TTH;
		debugd("block",pk.pk_block); debugd("count",pk.pk_count);
		if( fposition(pk.pk_unit,pk.pk_mod,pk.pk_block) ) {
			frclear();
			tuhost();
		}
		break;
	case TUOP_WRITE:
		if( ronly[pk.pk_unit] ) {
			state =  TUS_TTH;
			sendend(TUOP_END, TUE_WPRO,0,0);
			break;
		}
		fposition(pk.pk_unit,pk.pk_mod,pk.pk_block);
		ek.pk_count = 0;
		fwclear();
		cput(TUF_CONT); cflush();
		return TUS_HTT;
	case TUOP_SEEK:
		state = TUS_TTH;
		if( fposition(pk.pk_unit,pk.pk_mod,pk.pk_block) )
			sendend(TUOP_END, TUE_SUC,0,0);
		break;
	default:
		fprintf(stderr,"Incorrect op code %o\n",pk.pk_op);
		state = TUS_TTH;
		sendend( TUOP_END, TUE_BADO, 0, 0);
	}
	return TUS_IDLE;
}

sendend(op, succ,count,sum)		/* tu58 sends end packet to host */
char op, succ; u_short count, sum;
{
	ek.pk_op = op;
	ek.pk_succ = succ;
	ek.pk_unit = pk.pk_unit;
	ek.pk_count = count;
	ek.pk_stat = sum;
	debugx("sendend: succ",succ);
	/* see comment below in cmd() re: getpacket */
	sendpacket( &ek, (u_char *)&ek.pk_op);
	return 1;
}

tuhost()				/* host reads from tu58 */
{
	register int cnt, peak;
	char *cp, *fread();

	debugd("tuhost: count",pk.pk_count);
	for( cnt = pk.pk_count; cnt>0; cnt -= DATALEN) {
		dk.pk_mcount = DATALEN < cnt ? DATALEN : cnt;
		if( (cp = fread(pk.pk_unit)) )
		   sendpacket(&dk,(u_char *)cp);
		else {
		   sendend(TUOP_END, TUE_DERR,pk.pk_count - cnt,0);
		   return;
		}
#ifndef PWBTTY
		if( (peak = cpeak()) != -1 && (peak == 0 || peak == TUF_INITF) )
			return;
#endif
	}
	sendend(TUOP_END, TUE_SUC, pk.pk_count, 0);
}

hosttu()				/* host writes to tu58 */
{
	char *cp, *fwrite();

	if( (dk.pk_mcount = cget()) > DATALEN )
		dk.pk_mcount = DATALEN;
	debugd("hosttu: total count",0377&dk.pk_mcount);
	if( !(cp = fwrite(pk.pk_unit)) ) {
		debugs("hosttu: write error");
		sendend(TUOP_END, TUE_DERR,ek.pk_count,0);
		return 0;
	}
	if( !getpacket( &dk, (u_char *)cp) ) {
		debugs("hosttu: Checksum error");
		return 0;
	}
	ek.pk_count += 0377&dk.pk_mcount;
	pk.pk_count -= 0377&dk.pk_mcount;
	if(pk.pk_mod & TUMD_WRV);	/* verify write */
	
	if(pk.pk_count > 0) {
		cput(TUF_CONT); cflush();
		return 0;
	}

	if( fwflush(pk.pk_unit) ) {
		if(pk.pk_mod & TUMD_WRV);	/* verify write */
		sendend(TUOP_END, TUE_SUC,ek.pk_count,0);
	}
	else
		sendend(TUOP_END, TUE_DERR,ek.pk_count,0);
	return 1;
}

clearbuf()
{
	state &= ~(TUS_TTH | TUS_HTT );	    /* stop transfer to io buffers */
	pk.pk_count = 0;		/* zero command count */
	clearcbuf();			/* empty serial line io buffers */
}
