/* window init functions and data */

#include "term.h"

/* GLOBALS ****************************************************** */
long IntuitionBase=0;
long GfxBase=0;

struct NewWindow nw = {
   0, 0,             /* start position                  */
   639, 199,         /* width, height                   */
   -1, -1,           /* detail pen, block pen           */
   MENUPICK | CLOSEWINDOW | GADGETUP,
                     /* IDCMP flags                     */
   ACTIVATE | WINDOWDRAG | WINDOWDEPTH | WINDOWSIZING | WINDOWCLOSE,
                     /* window flags                    */
   NULL,             /* pointer to first user gadget    */
   NULL,             /* pointer to user checkmark       */
   "uwterm 1.00",    /* window title                    */
   NULL,             /* pointer to screen    (later)    */
   NULL,             /* pointer to superbitmap          */
   50,40,1000,1000,  /* sizing limits min and max       */
   WBENCHSCREEN      /* type of screen in which to open */
   };

InitLibs()
{
   GfxBase = OpenLibrary("graphics.library", 0);
   if (GfxBase == NULL) Cleanup();
   IntuitionBase = OpenLibrary("intuition.library", 0);
   if (IntuitionBase == NULL) Cleanup();
}
   struct Window *
NewTermWin()
{
   struct Window *win;

   win = OpenWindow(&nw);
   if ( win == NULL ) Cleanup();
   return(win);
}
