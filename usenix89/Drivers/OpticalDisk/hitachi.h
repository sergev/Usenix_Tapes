/*  hitachi.h  4/23/85	*/

/*
	Test program for GPIB 796 and Hitachi (OFC)
	Sends LOCK and UNLOCK commands to OFC
 */
/* Author:  Jungbo Yang  4/23/85  */

/* read only register and write only registers */

#define dir     cdor
#define isr1    imr1
#define isr2    imr2
#define spsr    spmr
#define adsr    admr
#define cptr    auxmr
#define adr0    adr
#define adr1    eosr
#define sr      cr0

/* Control masks for hidden registers (auxmr) */

#define LISTEN  0x13
#define SB      0x10
#define ICR     0040
#define PPR     0140
#define AUXRA   0200
#define AUXRB   0240
#define AUXRE   0300

/* Hardware register bit definitions */
/*      Name            Bit(s)          register       */

#define HR_DI           (1<<0)          /* isr1         */
#define HR_DO           (1<<1)          /* isr1         */
#define HR_END          (1<<4)          /* isr1         */
#define HR_DOIE         (1<<1)          /* imr1         */
#define HR_ENDIE        (1<<4)          /* imr1         */
#define HR_CO           (1<<3)          /* isr2         */
#define HR_SRQI         (1<<6)          /* isr2         */
#define HR_INTR         (1<<7)          /* isr2         */
#define HR_COIE         (1<<3)          /* imr2         */
#define HR_DMAI         (1<<4)          /* imr2         */
#define HR_DMAO         (1<<5)          /* imr2         */
#define HR_SRQIE        (1<<6)          /* imr2         */
#define HR_ADM0         (1<<0)          /* admr         */
#define HR_TRM0         (1<<4)          /* admr         */
#define HR_TRM1         (1<<5)          /* admr         */
#define HR_TON          (1<<7)          /* admr         */
#define HR_LON          (1<<6)          /* admr         */
#define HR_DL           (1<<5)          /* adr          */
#define HR_DT           (1<<6)          /* adr          */
#define HR_ARS          (1<<7)          /* adr          */
#define HR_GO           (1<<0)          /* cr0          */
#define HR_DMAE         (1<<1)          /* cr0          */
#define HR_LMR          (1<<2)          /* cr0          */
#define HR_FINIE        (1<<3)          /* cr0          */
#define HR_MEMRD        (1<<4)          /* cr0          */
#define HR_ECC          (1<<6)          /* cr0          */
#define HR_NSRQ         (1<<0)          /* sr , this bit not used! */
#define HR_NFIN         (1<<5)          /* sr           */
#define HR_NGPIBIR      (1<<6)          /* sr           */
#define HR_DONE         (1<<7)          /* sr           */
#define HR_CBRQ         (1<<5)          /* cr1          */
#define HR_SC           (1<<3)          /* cr1          */
#define HR_MIE          (1<<7)          /* cr1          */
#define HR_BURST	(1<<6)		/* cr1 */
#define HR_BTIME	(7)		/* cr1 */
#define HR_HLDA         (1<<0)          /* auxra        */
#define HR_HLDE         (1<<1)          /* auxra        */
#define HR_INV          (1<<3)          /* auxrb        */
#define HR_PPU          (1<<4)          /* ppr          */
#define HR_CIC		(1<<7)		/* adsr */
#define HR_TA		(1<<1)		/* adsr */
#define HR_LA		(1<<2)		/* adsr */

/* 7210 Auxiliary Commands */

#define AUX_PON         000     /* Immediate Execute pon        */
#define AUX_FH          003     /* Finish handshake             */
#define AUX_SEOI        006     /* Send EOI                     */
#define AUX_TCA         021     /* Take Control Asynchronously  */
#define AUX_GTS         020     /* Go To Standby                */
#define AUX_EPP         035     /* Execute Parallel Poll        */
#define AUX_SIFC        036     /* Set IFC                      */
#define AUX_CIFC        026     /* Clear IFC                    */
#define AUX_SREN        037     /* Set REN                      */
#define AUX_CREN        027     /* Clear REN                    */

/* ieep 488 codes */
#define UNT '_'
#define UNL '?'
#define SPE '\030'
#define SPD '\031'
#define DCL '\024'	/* device clear */

/* define talker and listers addresses */
#define MTA 0100
#define MLA 040

/* define hitachi commands */
/* control operations */
#define CONTROL_OP 0x89
#define CONTROL_OR 0x3
#define REZERO 0x0		/* rezero */
#define MOUNT 0x1		/* mount/lock */
#define DEMOUNT 0x2		/* demount/unlock */
#define EJECT 0x3		/* eject */
#define CHANGE 0x4		/* change */
#define NOP 0xf			/* nop */
#define CRC1 0x5d   		 /* default. all zero data */
#define CRC2 0x2a
/* send status operations */
#define SS_OP 0x49		/* no CRC? . use CONTROL_SS */
#define SS_OR 0x4
#define READSS 0x0
#define SECTOR 0x1
#define ODSTAT 0xc
#define CELLSTAT 0x8
#define DEVID 0x9
#define SS_POP 0x69		/** no CRC */

/*
 * For converting addresses (long) 
 */
#define lobyte(x)        ((long)(x) & 0377)
#define midbyte(x)      (((long)(x) >> 8) & 0377)
#define hibyte(x)       (((long)(x) >> 16) & 0377)
