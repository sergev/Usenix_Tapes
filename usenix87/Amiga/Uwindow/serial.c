#include "exec/types.h"
#include "exec/ports.h"
#include "exec/devices.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "devices/serial.h"

/* Open a serial device */
      int
OpenSerial(readrequest,writerequest)
      struct IOExtSer *readrequest;
      struct IOExtSer *writerequest;
      {
            int error;
            readrequest->io_SerFlags = NULL;
            error = OpenDevice(SERIALNAME, NULL, readrequest, NULL);
            writerequest->IOSer.io_Device = readrequest->IOSer.io_Device;
            writerequest->IOSer.io_Unit = readrequest->IOSer.io_Unit;
            writerequest->io_CtlChar = readrequest->io_CtlChar;
            writerequest->io_ReadLen = readrequest->io_ReadLen;
            writerequest->io_BrkTime = readrequest->io_BrkTime;
            writerequest->io_Baud = readrequest->io_Baud;
            writerequest->io_WriteLen = readrequest->io_WriteLen;
            writerequest->io_StopBits = readrequest->io_StopBits;
            writerequest->io_RBufLen = readrequest->io_RBufLen;
            writerequest->io_SerFlags = readrequest->io_SerFlags;
            writerequest->io_TermArray.TermArray0
            = readrequest->io_TermArray.TermArray0;
            writerequest->io_TermArray.TermArray1
            = readrequest->io_TermArray.TermArray1;
            /* clone required parts of the request */
            return(error);
      }

      int
SerPutChar(request,character)
      struct IOExtSer *request;
      char character;
      {
            request->IOSer.io_Command = CMD_WRITE;
            request->IOSer.io_Data = (APTR)&character;
            request->IOSer.io_Length = 1;
            DoIO(request);
            return(0);
      }

      int
QueueSerRead(request, whereto)
      struct IOExtSer *request;
      char *whereto;
      {
            request->IOSer.io_Command = CMD_READ;
            request->IOSer.io_Data = (APTR)whereto;
/*            request->IOSer.io_Flags = IOF_QUICK; */
            request->IOSer.io_Length = 1;
            BeginIO(request);
            return(0);
      }

      int
QuerySer(request)
	struct IOExtSer *request;
	{
		request->IOSer.io_Command = SDCMD_QUERY;
		BeginIO(request);
		WaitIO(request);
/*		DoIO(request); */
		return((int)(request->IOSer.io_Actual));
	} /* end of QuerySer */

   int
SetParams(io,rbuf_len,rlen,wlen,brk,baud,sf,ta0,ta1)
   struct IOExtSer *io;
   unsigned long rbuf_len;
   unsigned char rlen;
   unsigned char wlen;
   unsigned long brk;
   unsigned long baud;
   unsigned char sf;
   unsigned long ta0;
   unsigned long ta1;
   {
      io->io_ReadLen = rlen;
      io->io_WriteLen = wlen;
      io->io_Baud = baud;
      io->io_BrkTime = brk;
      io->io_StopBits = 0x01;
      io->io_RBufLen = rbuf_len;
      io->io_SerFlags = sf;
      io->io_TermArray.TermArray0 = ta0;
      io->io_TermArray.TermArray1 = ta1;
      io->IOSer.io_Command = SDCMD_SETPARAMS;
   
      return(DoIO(io));
}
