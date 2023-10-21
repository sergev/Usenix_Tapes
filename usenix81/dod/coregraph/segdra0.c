#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
segdra0(asegptr,anindex)
   struct segstruc *asegptr;
   int anindex;
   {
    rastererase = TRUE;
    ddstruct.opcode = SETCOL;
    ddstruct.float1 = 0.0;
    ddstruct.float2 = 0.0;
    ddstruct.float3 = 0.0;
    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
    ddstruct.opcode = SETINT;
    ddstruct.float1 = 0.0;
    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
    segdraw(asegptr,anindex);
    rastererase = FALSE;
    return(0);
   }

