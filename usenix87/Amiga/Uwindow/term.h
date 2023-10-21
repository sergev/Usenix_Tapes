/* INCLUDES ********************************************************** */

#include "exec/types.h"
#include "exec/ports.h"
#include "exec/devices.h"
#include "exec/io.h"
#include "exec/memory.h"

#include "libraries/dos.h"
#include "graphics/text.h"
#include "libraries/diskfont.h"
#include "intuition/intuition.h"
 
/* EXTERNALS ***************************************************** */

extern struct Window *OpenWindow();
extern struct Screen *OpenScreen();
extern struct MsgPort *CreatePort();
extern struct IOStdReq *CreateStdIO();
extern struct IORequest *CreateExtIO();

/* DATA TYPES *****************/
struct uw_struct {
	int free;		/* is this uw struct free? */
	struct MsgPort *ConReadPort, *ConWritePort;
	struct IOStdReq *ConWriteReq, *ConReadReq;
	struct Window *win;
	int winsig, consig;
};

extern struct uw_struct uw[];
extern int uw_read, uw_write, uw_count;

