/*                                Cyclic Redundancy Check

	CRC is a function that generates a 16-bit value from an 8-bit code.
	It is called by two functions, that compute CRC1 and CRC2.  They,
	in turn are called by two funtions that compute CRC1/2 for sending
	and for receiving.

	------  LINK LAYER ------     ----- PACKET LAYER -----  --- DATA ----
	RECEIVER         SENDER      RECEIVER      SENDER        DEFINE CALL
	crc1_chk        crc1_gen     crc2_chk     crc1_gen      *buf      buf
	   |_______  _______|_____      |___________|___        ndx     &ndx
		   \/             \             \  /        ||  bytes    bytes
		crc_loop ___________\____________\/         ||
	     ______|______              \  _________________||
	    |             |              \/                  |
	get_byte,       crc_add---->mirror              put_byte


	crc2_gen sets a buffer pointer (ccb->outp_buf) and index (ccb->outp_
	ndx) that are used by put_byte and get_byte.  Both must be able to
	change the caller's caller's buf and ndx, so that crc2_gen can subse-
	quently call put_byte to store the CRC that is produced.

	Thus, pointers to there variables are passed to CRCx and to xxt_byte.
	.____________________.  .____________________.  .____________________.
	|     ddddddddddddddd|  |  dddddddddddddddddc|  |c                   |
	crc_gen store needs:     \buf (may change)   \ndx       (may overflow)

	Common CRC routines
	mirror   reverses the order of the bits (left to/from right) in a byte.
		 (#ifdef'd out in this version, replaced by a table lookup)
	crc_loop computes calls crc_add for every byte  in link/packet header.
	get_byte provides each  byte, using the caller's buffer address and
		 index (on the stack) into each bufferlet.
	put_byte stores each byte (of CRC2), using the caller's buffer address
		 and index (on the stack) into each bufferlet.

	Link Layer CRC Routines
	crc1_gen calls crc-loop for output, from the byte after STX to the byte
		 before the CRC1 field, and stores the total into the CRC1
		 field.
	crc1_chk calls crc_loop for input, from the byte after STX through the
		 CRC1 field, and compares the total with a constant.

	Packet Layer CRC Routines
	crc2_gen calls crc-loop for output, from the byte after CRC1 to the byte
		 before the crc2 field, and stores the total into the crc2
		 field.
	crc2_chk calls crc-loop for input, from the byte after CRC1 through the
		 crc2 field, and compares the total with a constant.

	08/17 Mike Remove ret_val = TRUE from crc1_chk and crc2_chk
	09/26 Ed M Change to get rid of generating crc table.
	10/23 Ed M Changed mirror into a lookup table and got rid of crc_add
*/
#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
#define USETBL
#ifndef USETBL
unsigned int    crctbl[256];
#else

	/* initialize the table ourselves */
unsigned int    crctbl[256] = {
    0x0, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0xA50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0xC60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0xE70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0xA1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x2B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x8E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0xAF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0xCC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0xED1, 0x1EF0
};
#endif

#ifndef USETBL
/* -----------------------------------------------------------------------
     mirror is called to invert the bit order of the resultant CRC
 -------------------------------------------------------------------*/

byte
mirror (data)
byte data;
{
    byte ret;
    static  byte xlat[] =
    {
	0, 8, 4, 0xC, 2, 0xA, 6, 0xE, 1, 9, 5,  0xD, 3, 0xB, 7, 0xF
    };
    ret = (xlat[data & 0xF] << 4) | xlat[(data >> 4) & 0xF];
    return (ret);
}


/* --------------------------------------------------------------------------
   crc_add is called by crc_loop to "add" a 8-bit code to a total.
-------------------------------------------------------------------------- */
unsigned
crc_add (code, total)
byte code;
unsigned    total;
{
    unsigned    temp;
 /* get XOR of next byte and upper 8 bits of running CRC */
    temp = (total >> 8) ^ mirror (code);
    return (((total << 8) & 0xFF00) ^ crctbl[temp]);
}

#else

/* lookup table, value is index with bit order reversed. */
static byte mirror[256] = {
 '\x0', '\x80', '\x40', '\xC0', '\x20', '\xA0', '\x60', '\xE0', '\x10', '\x90', '\x50', '\xD0', '\x30', '\xB0', '\x70', '\xF0',
 '\x8', '\x88', '\x48', '\xC8', '\x28', '\xA8', '\x68', '\xE8', '\x18', '\x98', '\x58', '\xD8', '\x38', '\xB8', '\x78', '\xF8',
 '\x4', '\x84', '\x44', '\xC4', '\x24', '\xA4', '\x64', '\xE4', '\x14', '\x94', '\x54', '\xD4', '\x34', '\xB4', '\x74', '\xF4',
 '\xC', '\x8C', '\x4C', '\xCC', '\x2C', '\xAC', '\x6C', '\xEC', '\x1C', '\x9C', '\x5C', '\xDC', '\x3C', '\xBC', '\x7C', '\xFC',
 '\x2', '\x82', '\x42', '\xC2', '\x22', '\xA2', '\x62', '\xE2', '\x12', '\x92', '\x52', '\xD2', '\x32', '\xB2', '\x72', '\xF2',
 '\xA', '\x8A', '\x4A', '\xCA', '\x2A', '\xAA', '\x6A', '\xEA', '\x1A', '\x9A', '\x5A', '\xDA', '\x3A', '\xBA', '\x7A', '\xFA',
 '\x6', '\x86', '\x46', '\xC6', '\x26', '\xA6', '\x66', '\xE6', '\x16', '\x96', '\x56', '\xD6', '\x36', '\xB6', '\x76', '\xF6',
 '\xE', '\x8E', '\x4E', '\xCE', '\x2E', '\xAE', '\x6E', '\xEE', '\x1E', '\x9E', '\x5E', '\xDE', '\x3E', '\xBE', '\x7E', '\xFE',
 '\x1', '\x81', '\x41', '\xC1', '\x21', '\xA1', '\x61', '\xE1', '\x11', '\x91', '\x51', '\xD1', '\x31', '\xB1', '\x71', '\xF1',
 '\x9', '\x89', '\x49', '\xC9', '\x29', '\xA9', '\x69', '\xE9', '\x19', '\x99', '\x59', '\xD9', '\x39', '\xB9', '\x79', '\xF9',
 '\x5', '\x85', '\x45', '\xC5', '\x25', '\xA5', '\x65', '\xE5', '\x15', '\x95', '\x55', '\xD5', '\x35', '\xB5', '\x75', '\xF5',
 '\xD', '\x8D', '\x4D', '\xCD', '\x2D', '\xAD', '\x6D', '\xED', '\x1D', '\x9D', '\x5D', '\xDD', '\x3D', '\xBD', '\x7D', '\xFD',
 '\x3', '\x83', '\x43', '\xC3', '\x23', '\xA3', '\x63', '\xE3', '\x13', '\x93', '\x53', '\xD3', '\x33', '\xB3', '\x73', '\xF3',
 '\xB', '\x8B', '\x4B', '\xCB', '\x2B', '\xAB', '\x6B', '\xEB', '\x1B', '\x9B', '\x5B', '\xDB', '\x3B', '\xBB', '\x7B', '\xFB',
 '\x7', '\x87', '\x47', '\xC7', '\x27', '\xA7', '\x67', '\xE7', '\x17', '\x97', '\x57', '\xD7', '\x37', '\xB7', '\x77', '\xF7',
 '\xF', '\x8F', '\x4F', '\xCF', '\x2F', '\xAF', '\x6F', '\xEF', '\x1F', '\x9F', '\x5F', '\xDF', '\x3F', '\xBF', '\x7F', '\xFF' 
};
#endif

/* --------------------------------------------------------------------------
   get_byte returns next byte in the bufferlet chain and sets fin, if last.
   If packet size is zero, do not call this routine!  Get next bufferlet before
   using it.  Caller supplies 3 variables that get_byte may write into.
   get_byte goes beyound the end of data bytes to get 2 CRC bytes for crcX_chk
-------------------------------------------------------------------------- */
get_byte (buf, index)
BUF_PTR * buf;
int    *index;
{
    BUF_PTR nbuf;
    BUF_PTR get_buff();
    int     ret_val;
    ret_val = (*buf) -> data[*index];
    (*index)++;
    if (*index >= BFLT_DATA_SIZE) {
	if ((*buf) -> bnext ==  NULL) {
	    int_off ();
	    if  ((nbuf = get_buff ()) == NULL) {
		(*index)--;
	    }
	    else {
		(*buf) -> bnext = nbuf;
		*buf = nbuf;
		(*buf) -> num_bytes =   0;
		*index = 0;
	    }
	    int_on ();
	}
	else {
	    *buf = (*buf) -> bnext;
	    *index = 0;
	}
    }
    return (ret_val);
}

/* --------------------------------------------------------------------------
   put_byte is called by crc2_gen, after it has computed the CRC, to store the
   next byte in the bufferlet chain.  Caller supplies the bufferlet, index and
   buffer-size pointers that put_byte may write into, and the byte to store.
-------------------------------------------------------------------------- */
put_byte (buf, index, size, bits)
BUF_PTR * buf;
int    *index,
       *size,
	bits;
{
    BUF_PTR nbuf;
    BUF_PTR get_buff();
    (*buf) -> data[*index] = bits;
    (*index)++;
    (*buf) -> num_bytes++;
    if (*index >= BFLT_DATA_SIZE) {
	int_off ();
	if ((nbuf = get_buff ()) == NULL) {             
	    (*buf) -> num_bytes--;
	    (*index)--; 
	}
	else {
	    (*buf) -> bnext = nbuf;
	    *buf = nbuf;
	    (*buf) -> num_bytes = 0;
	    *index = 0;
	}
	int_on  ();
    }
 /* else let CRC fail, retransmit later --- don't store */
    return;
}



/* --------------------------------------------------------------------------
   crc_loop is called by crcX_chk or _gen (X=1 or 2) to loop through all
   bufferlets and compute the CRC total that link/packet layer sends/reveived.
-------------------------------------------------------------------------- */
unsigned
crc_loop (buf, ndx, bytes_max)
BUF_PTR * buf;
int    *ndx,
	bytes_max;
{
    unsigned    total;
    int     i;
    byte ch;
    total = INIT_CRC;
    for (i = 0; i < bytes_max; i++) {
	ch = get_byte (buf, ndx);
#ifdef USETBL
	/*
	 Do the crc calculation inline.
	 */
	total = ((total << 8) &0xff00) ^ crctbl[(total >> 8) ^ mirror[ch]];
#else
	total = crc_add (ch, total);
#endif
    }
    return (total);
}



/* --------------------------------------------------------------------------
   crc1_gen computes the CRC total for link layer to send, and stores it.
-------------------------------------------------------------------------- */
crc1_gen (buffirst)
BUF_PTR buffirst;
{
    static  BUF_PTR buf;
    static int  ndx;
    buf = buffirst;
    ndx = B_SIZE;
    lcb.out_crc = ~crc_loop (&buf, &ndx, 4);
    buf -> data[B_CRC] = mirror[(char)(lcb.out_crc >> 8)];
    buf -> data[B_CRC + 1] = mirror[(char)(lcb.out_crc & 0xFF)];
    return;
}

/* --------------------------------------------------------------------------
   crc1_chk computes the CRC total that link layer reveived, including the two
   CRC bytes, and compares it to a constant.
-------------------------------------------------------------------------- */
crc1_chk (buffirst)
BUF_PTR buffirst;
{
    static  BUF_PTR buf;
    static int  ndx;
    int     ret_val;
    buf = buffirst;
    ndx = B_SIZE;
    lcb.in_crc = crc_loop (&buf, &ndx, 6);
    ret_val = (lcb.in_crc & 0xFFFF) == INPUT_CRC;
    return (ret_val);
}

/* --------------------------------------------------------------------------
   crc2_gen computes the CRC total for link layer to send, and stores it.
-------------------------------------------------------------------------- */
crc2_gen (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    static  BUF_PTR buf;
    static int  ndx,
		bytes_max;
    buf = que -> u.cmd.bfirst;
    ndx = B_BYTE0 + 1;
    bytes_max = buf -> data[B_SIZE];
    if (bytes_max > 0) {
	ccb ->  out_crc2 = ~crc_loop (&buf, &ndx, bytes_max);
	put_byte (&buf, &ndx, &bytes_max, mirror[(char)(ccb -> out_crc2 >> 8)]);
	put_byte (&buf, &ndx, &bytes_max, mirror[(char)(ccb -> out_crc2 & 0xff)]);
    }
    return;
}

/* --------------------------------------------------------------------------
   crc2_chk computes the CRC total that link layer reveived, including the two
   CRC bytes, and compares it to a constant.
-------------------------------------------------------------------------- */
crc2_chk (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    static  BUF_PTR buf;
    static int  ndx,
		bytes_max;
    int     ret_val;
    buf = que -> u.cmd.bfirst;
    ndx = B_BYTE0 + 1;
    bytes_max = buf -> data[B_SIZE] + 2;
    if (bytes_max > 2) {
	ccb ->  in_crc2 = crc_loop (&buf, &ndx, bytes_max);
	ret_val = (ccb  -> in_crc2 & 0xFFFF) == INPUT_CRC;
    }
    else
	ret_val = TRUE;
    return (ret_val);
}


crc_gen (polybits)
long int    polybits;
{
 /*
 This program generates the CRC table for a polynomial for Tymshare engine.
	 SEG A.DATA             ;by Mark Akselrod
 PLNM           WC  18005               ;CODE POLYNOMIAL HERE
 TABLE    HS  256
	 SEG A.CODE
 START    XR  R1,R1
 LOOP           EXHR R2,R1
 LOOP1    JFFO R2,LOOP2
	  J         LOOP3
 LOOP2    LIS   R5,0F
	  SR    R5,R3
	  JLFS  LOOP3
	  L     R4,PLNM
	  SLL   R4,0,R5
	  XR    R2,R4
	  JBS   LOOP1
 LOOP3    STH R2,TABLE,R1,R1
	  AIS   R1,1
	  CHI   R1,0FF
	  JLE   LOOP
	  JAL   R10,CRASH
	  HC    0,0
 */
#ifndef USETBL
    long int    r2,
		r3;
    int     r1,
	    r5;
    for (r1 = 0; r1 <= 255; r1++) {
	r2 = r1;
	r2 = r2 << 16;
	while (r2 != 0) {
	    r3  = r2;
	    for (r5 = 7; (r3 &  0x800000) == 0; r5--)
		r3 = r3 << 1;
	    if  (r5 < 0)
		break;
	    r2  = r2 ^ (polybits << r5);
	};
	crctbl[r1] = r2;
    };
#endif
}
