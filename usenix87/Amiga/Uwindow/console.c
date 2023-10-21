#include "exec/types.h"
#include "exec/ports.h"
#include "exec/devices.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "devices/console.h"

/* These functions are taken directly from the console.device chapter
 * in the Amiga V1.1 ROM KERNEL manual.  See manual for documentation. */


/* Open a console device */
      int
OpenConsole(writerequest,readrequest,window)
      struct IOStdReq *writerequest;
      struct IOStdReq *readrequest;
      struct Window *window;
      {
            int error; 
            writerequest->io_Data = (APTR) window;
            writerequest->io_Length = sizeof(*window);
            error = OpenDevice("console.device", 0, writerequest, 0);
            readrequest->io_Device = writerequest->io_Device;
            readrequest->io_Unit   = writerequest->io_Unit;
                  /* clone required parts of the request */
            return(error);
      }

/* Output a single character to a specified console */ 

      int
ConPutChar(request,character)
      struct IOStdReq *request;
      char character;
      {
            request->io_Command = CMD_WRITE;
            request->io_Data = (APTR)&character;
            request->io_Length = 1;
            request->io_Flags = IOF_QUICK;
            DoIO(request);
            return(0);
      }
 
/* Output a NULL-terminated string of characters to a console */ 

      int
ConPutStr(request,string)
      struct IOStdReq *request;
      char *string;
      {
            request->io_Command = CMD_WRITE;
            request->io_Data = (APTR)string;
            request->io_Length = -1;
            DoIO(request);
            return(0);
      }
 
      /* queue up a read request to a console */

      int
QueueRead(request,whereto)
      struct IOStdReq *request;
      char *whereto;
      {
            request->io_Command = CMD_READ;
            request->io_Data = (APTR)whereto;
            request->io_Length = 1;
            SendIO(request);
            return(0);
      }

