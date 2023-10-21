#include "inclusions.c"


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setdefaultattributes                                       */
/*                                                                          */
/*     PURPOSE: SET THE DEFAULT PRIMITIVE AND SEGMENT ATTRIBUTE VALUES      */
/*              TO THOSE VALUES CONTAINED IN THE SPECIFIED STRUCTURES.      */
/*                                                                          */
/****************************************************************************/

setdefaultattributes(defprim,segatval)
   struct primattr *defprim;
   struct segattr *segatval;
   {
   char *funcname;
   int errnum,i;

   funcname = "setdefaultattributes";
   if(dfaltset)   /*** FUNCTION BEEN CALLED ALREADY? ***/
      {
      errnum = 11;
      errhand(funcname,errnum);
      return(1);
      }
   if(prevseg)   /*** SEGMENT BEEN PREVIOUSLY OPENED? ***/
      {
      errnum = 12;
      errhand(funcname,errnum);
      return(2);
      }
   /**************************************************/
   /***                                            ***/
   /*** CHECK FOR VALIDITY OF PRIMITIVE ATTRIBUTE  ***/
   /*** PARAMETERS PASSED.                         ***/
   /***                                            ***/
   /**************************************************/

   for(i = 0;i < 3;i++)
      {
      if(fldefck(defprim->color[i],minimum.color[i],maximum.color[i],funcname) == 3) return(3);
      }
   if(fldefck(defprim->intensty,minimum.intensty,maximum.intensty,funcname) == 3) return(3);
   if(fldefck(defprim->linwidth,minimum.linwidth,maximum.linwidth,funcname) == 3) return(3);
   if(indefck(defprim->linestyl,minimum.linestyl,maximum.linestyl,funcname) == 3) return(3);
   if(indefck(defprim->font,minimum.font,maximum.font,funcname) == 3) return(3);
   if(fldefck(defprim->charsize.width,minimum.charsize.width,maximum.charsize.width,funcname) == 3) return(3);
   if(fldefck(defprim->charsize.height,minimum.charsize.height,maximum.charsize.height,funcname) == 3) return(3);
   if(fldefck(defprim->chrspace.dx,minimum.chrspace.dx,maximum.chrspace.dx,funcname) == 3) return(3);
   if(fldefck(defprim->chrspace.dy,minimum.chrspace.dy,maximum.chrspace.dy,funcname) == 3) return(3);
   if(indefck(defprim->chqualty,minimum.chqualty,maximum.chqualty,funcname) == 3) return(3);
   if(indefck(defprim->pick2id,minimum.pick2id,maximum.pick2id,funcname) == 3) return(3);

   /**************************************************/
   /***                                            ***/
   /*** CHECK FOR VALIDITY OF SEGMENT ATTRIBUTE    ***/
   /*** PARAMETERS PASSED.                         ***/
   /***                                            ***/
   /**************************************************/

   if(sattrck(segatval->visbil2ty,funcname) == 3) return(3);
   if(sattrck(segatval->detect2bl,funcname) == 3) return(3);
   if(sattrck(segatval->high2lght,funcname) == 3) return(3);

   /*************************************************************/
   /***                                                       ***/
   /*** CHECK THAT EACH VALUE WITHIN THE IMAGE TRANSFORMATION ***/
   /*** MATRICES ARE BETWEEN -1 AND +1.                       ***/
   /***                                                       ***/
   /*************************************************************/

   if(attrchk(segatval->scale2[0],funcname) == 3) return(3);
   if(attrchk(segatval->scale2[1],funcname) == 3) return(3);
   if(attrchk(segatval->trans2lat[0],funcname) == 3) return(3);
   if(attrchk(segatval->trans2lat[1],funcname) == 3) return(3);
   if(attrchk(segatval->rotate2,funcname) == 3) return(3);

   /**************************************************/
   /***                                            ***/
   /*** DEFAULT PRIMITIVE ATTRIBUTE TRANSFERS      ***/
   /***                                            ***/
   /**************************************************/

   for(i = 0;i < 3;i++)
      {
      defaultt.color[i] = defprim->color[i];
      }
   defaultt.intensty = defprim->intensty;
   defaultt.linwidth = defprim->linwidth;
   defaultt.linestyl = defprim->linestyl;
   defaultt.font = defprim->font;
   defaultt.charsize.width = defprim->charsize.width;
   defaultt.charsize.height = defprim->charsize.height;
   defaultt.chrspace.dx = defprim->chrspace.dx;
   defaultt.chrspace.dy = defprim->chrspace.dy;
   defaultt.chqualty = defprim->chqualty;
   defaultt.pick2id = defprim->pick2id;

   /******************************************/
   /***                                    ***/
   /*** SEGMENT ATTRIBUTE VALUES TRANSFERS ***/
   /***                                    ***/
   /******************************************/

   defsegat.visbil2ty = segatval->visbil2ty;
   defsegat.detect2bl = segatval->detect2bl;
   defsegat.high2lght = segatval->high2lght;
   defsegat.scale2[0] = segatval->scale2[0];
   defsegat.scale2[1] = segatval->scale2[1];
   defsegat.trans2lat[0] = segatval->trans2lat[0];
   defsegat.trans2lat[1] = segatval->trans2lat[1];
   defsegat.rotate2 = segatval->rotate2;

   dfaltset = TRUE;
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqattr                                                    */
/*                                                                          */
/*     PURPOSE: INQUIRE DEFAULT PRIMITIVE AND SEGMENT ATTRIBUTES.           */
/*                                                                          */
/****************************************************************************/

inqattr(defprim,segatval)
   struct primattr *defprim;
   struct segattr *segatval;
   {
   register int i;

   /**************************************************/
   /***                                            ***/
   /*** DEFAULT PRIMITIVE ATTRIBUTE TRANSFERS      ***/
   /***                                            ***/
   /**************************************************/

   for(i = 0;i < 3;i++)
      {
      defprim->color[i] = defaultt.color[i];
      }
   defprim->intensty = defaultt.intensty;
   defprim->linwidth = defaultt.linwidth;
   defprim->linestyl = defaultt.linestyl;
   defprim->font = defaultt.font;
   defprim->charsize.width = defaultt.charsize.width;
   defprim->charsize.height = defaultt.charsize.height;
   defprim->chrspace.dx = defaultt.chrspace.dx;
   defprim->chrspace.dy = defaultt.chrspace.dy;
   defprim->chqualty = defaultt.chqualty;
   defprim->pick2id = defaultt.pick2id;

   /******************************************/
   /***                                    ***/
   /*** SEGMENT ATTRIBUTE VALUES TRANSFERS ***/
   /***                                    ***/
   /******************************************/

   segatval->visbil2ty = defsegat.visbil2ty;
   segatval->detect2bl = defsegat.detect2bl;
   segatval->high2lght = defsegat.high2lght;
   segatval->scale2[0] = defsegat.scale2[0];
   segatval->scale2[1] = defsegat.scale2[1];
   segatval->trans2lat[0] = defsegat.trans2lat[0];
   segatval->trans2lat[1] = defsegat.trans2lat[1];
   segatval->rotate2 = defsegat.rotate2;

   return(0);
   }
