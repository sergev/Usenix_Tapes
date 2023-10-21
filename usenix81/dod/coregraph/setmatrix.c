#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
setmatrix(asegptr)
   struct segstruc *asegptr;
   {

   imxform[0][0] = cos(asegptr->rotate1) * asegptr->scale1[0];
   imxform[0][1] = -sin(asegptr->rotate1) * asegptr->scale1[1];
   imxform[1][0] = sin(asegptr->rotate1) * asegptr->scale1[0];
   imxform[1][1] = cos(asegptr->rotate1) * asegptr->scale1[1];
   imxform[2][0] = asegptr->trans1lat[0];
   imxform[2][1] = asegptr->trans1lat[1];

   if(identchk(imxform) == 0)
      {
      idenflag = TRUE;
      }
   else idenflag = FALSE;

    return(0);
   }

