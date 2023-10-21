/*
 * Comport.h
 *
 * defines the bit masking for the get_mcr()
 *
 * @(#) comport.h	Version hoptoad-1.3	87/03/24
 *
 * Copyright (C) Tim M. Pozar 1987
 * Anyone can use this code for anything, but it is copyright by Tim
 * and you must leave his copyright in the code.
 *
 */

/*
 * get_msr()
 *   Function to read (get) the byte located in the Modem Status 
 * Register (3FEh).  The table below describes the byte returned.
 *   bit  description
 *    0   Delta Clear to Send (DCTS)
 *        Indicates that the !CTS input to the chip has changed state
 *        since the last time it was read by the processor.
 *    1   Delta Data Set Ready (DDSR)
 *        Indicates that the !DRS input to the chip has changed since 
 *        last time it was read by the processor.
 *    2   Trailing Edge Ring Indicator (TERI)
 *        Indicates that the !RI input to the chip has changed from
 *        an on (logical 1) to an off (logical 0) condition.
 *    3   Delta Rx Line Signal detect (DRLSD)
 *        Indicates that the !RLSD input to the chip has changed state.
 * NOTE: Whenever bit 0, 1, 2, or 3 is set to a logical 1, a modem status
 *       interrupt is generated.
 *
 *    4   Clear to Send (CTS)
 *        This bit is the complement of the clear to send (!CTS) input.
 *        If bit 4 (LOOP) of the MCR is set to a logical 1, this is 
 *        equivalent to RTS in the MCR.
 *    5   Data Set Ready (DSR)
 *        This bit is the complement of the data set ready (!DSR) input.
 *        If bit 4 (LOOP) of the MCR is set to a logical 1, this is 
 *        equivalent to DTR in the MCR.
 *    6   Ring Indicator (RI)
 *        This bit is the complement of the ring indicator (!RI) input.
 *        If bit 4 (LOOP) of the MCR is set to a logical 1, this is 
 *        equivalent to OUT 1 in the MCR.
 *    7   Receive Line Signal Detect (RLSD) or Carrier Detect (CD).
 *        This bit is the complement of the received line signal detect
 *        (!RLSD) input. If bit 4 (LOOP) of the MCR is set to a logical 1,
 *        this is equivalent to OUT 2 in the MCR.
 */

#define  DCTS       1
#define  DDSR       2
#define  TERI       4
#define  DRLSD      8
#define  CTS       16
#define  DST       32
#define  RI        64
#define  RLSD     128   /* Also known as ... */
#define  CD       128   
