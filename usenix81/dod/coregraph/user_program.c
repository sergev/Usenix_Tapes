#define TRUE 1
#define FALSE 0
#define EMPTY -2
#define DELETED -1
#define NORETAIN 0
#define RETAIN 1
#define XLATE2 2
#define XFORM2 3
#define PICK 0
#define KEYBOARD 1
#define BUTTON 2
#define LOCATOR 3
#define VALUATOR 4
#define MEDIUM 1

main()
   {
   int levelnum;
   int retrncde;
   int segtype;
   int i,j;
   char *surfname;
   char *segname,*newname;
   char *string;
   float x,y;
   float deltax,deltay;
   float chwidth,cheight;
   float scwidth,scheight;
   float xmin,xmax,ymin,ymax;
/* float vwmatrix[2][2];          */
   float xcoord[5],ycoord[5];
   float angle;

   struct chsize
      {
      float width;
      float height;
      };

   struct chspace
      {
      float dx;
      float dy;
      };

   struct
      {
      float color[3];
      float intensty;
      float linwidth;
      int linestyl;
      int font;
      struct chsize charsize;
      struct chspace chrspace;
      int chqualty;
      int pick2id;   /*** VERSION 7: PICKID ***/
      } primstruct;


/* struct temp1
      {
      float xmin;
      float xmax;
      float ymin;
      float ymax;
      };

   struct
      {
      struct temp1 vwport;
      struct temp1 wynndow;
      float vwplnorm[3];
      float vwrefpt[3];
      float vwupdir[3];
      float viewdis;
      float frontdis;
      float backdis;
      int projtype;
      float projdir;
      } viewparm;
*/

   levelnum = 4;

   retrncde = initcore(levelnum);
/* printf("initcore:Return code = %d\n",retrncde);                          */

   surfname = "vwsurf1";
   retrncde = initvwsurface(surfname);
/* printf("initvwsurface:Return code = %d\n",retrncde);                     */

   retrncde = selectvwsurface(surfname);
/* printf("selectvwsurface:Return code = %d\n",retrncde);                   */

/* scwidth = 0.8;scheight = 1.0;
   retrncde = ndcsp2(scwidth,scheight);
   printf("ndcsp2:Return code = %d\n",retrncde);

   xmin = .2;xmax = .6;ymin = .2;ymax = .8;
   retrncde = vwport2(xmin,xmax,ymin,ymax);
   printf("vwport2:Return code = %d\n",retrncde);
*/
   xmin = 0.;xmax = 511.;
   ymin = 0.;ymax = 511.;
   retrncde = window2(xmin,xmax,ymin,ymax);
/* printf("window2:Return code = %d\n",retrncde);                           */

/* retrncde = inqvwtransformation(vwmatrix);
   printf("inqvwtransformation:Return code = %d\n",retrncde);
   printf("Following are viewing transformation values\n");
   for(i = 0;i < 2;i++)
      {
      for(j = 0;j < 2;j++)
	 {
	 printf("%f ",vwmatrix[i][j]);
	 }
      }
   printf("\n");
*/
/*** INPUT FUNCTIONS USED IN LEVEL 3 IMPLEMENTATION ***/
/* retrncde = enabledevice(BUTTON,3);
   printf("enabledevice:Return code = %d\n",retrncde);

   retrncde = disabldevice(BUTTON,3);
   printf("disabldevice:Return code = %d\n",retrncde);

   retrncde = enableclass(BUTTON);
   printf("enableclass:Return code = %d\n",retrncde);

   retrncde = disablclass(BUTTON);
   printf("disablclass:Return code = %d\n",retrncde);
*/
   retrncde = clipwindow(TRUE);
/* printf("clipwindow:Return code = %d\n",retrncde);                        */

/* retrncde = inqvwparameters(&viewparm);
   printf("inqvwparameters:Return code = %d\n",retrncde);
   printf("Window X-maximum equals %f\nViewport Y-maximum equals %f\n",viewparm.wynndow.xmax,viewparm.vwport.ymax);
*/
   segname = "subpicture";
   retrncde = createseg(segname);
/* printf("createseg:Return code = %d\n",retrncde);                         */

   primstruct.color[0] = 1.0;primstruct.color[1] = 0.0;primstruct.color[2] = 0.0;
   retrncde = setcolor(primstruct.color);
/* printf("setcolor:Return code = %d\n",retrncde);                          */

   xcoord[0] = 50.;ycoord[0] = 250.;
   xcoord[1] = 50.;ycoord[1] = 300.;
   xcoord[2] = 150.;ycoord[2] = 300.;
   xcoord[3] = 150.;ycoord[3] = 250.;
   xcoord[4] = 50.;ycoord[4] = 250.;
   retrncde = plyabs2(xcoord,ycoord,5);
/* printf("plyabs2:Return code = %d\n",retrncde);                           */

   retrncde = movabs2(60.,275.);
/* printf("movabs2:Return code = %d\n",retrncde);                           */

   primstruct.color[0] = 1.0;primstruct.color[1] = 1.0;primstruct.color[2] = 1.0;
   retrncde = setcolor(primstruct.color);
/* printf("setcolor:Return code = %d\n",retrncde);                          */

   string = "Non-Retained";
   retrncde = text(string);
/* printf("text:Return code = %d\n",retrncde);                              */

   retrncde = closeseg();
/* printf("closeseg:Return code = %d\n",retrncde);                          */

/* retrncde = setsegtype(XLATE2);
   printf("setsegtype:Return code = %d\n",retrncde);
*/
   retrncde = setsegtype(XFORM2);
/* printf("setsegtype:Return code = %d\n",retrncde);                        */

   segname = "picture";
   retrncde = createseg(segname);
/* printf("createseg:Return code = %d\n",retrncde);                         */

   primstruct.color[0] = 0.0;primstruct.color[1] = 1.0;primstruct.color[2] = 0.0;
   retrncde = setcolor(primstruct.color);
/* printf("setcolor:Return code = %d\n",retrncde);                          */


   chwidth = 10.;
   cheight = 14.;
   retrncde = setchsize(chwidth,cheight);
/* printf("setchsize:Return code = %d\n",retrncde);                         */
   retrncde = inqchsize(&chwidth,&cheight);
/* printf("inqchsize:Return code = %d\n",retrncde);                         */
/* printf("Current width equals %f\nCurrent height equals %f\n",chwidth,cheight);
*/
   retrncde = setchspc(14.,0.);
/* printf("setchspc:Return code = %d\n",retrncde);                          */

   retrncde = movabs2(200.,200.);
/* printf("movabs2:Return code = %d\n",retrncde);                           */

   retrncde = linabs2(300.,300.);
/* printf("linabs2:Return code = %d\n",retrncde);                           */

   xcoord[0] = 0.;ycoord[0] = -100.;
   xcoord[1] = -100.;ycoord[1] = 0.;
   retrncde = plyrel2(xcoord,ycoord,2);
/* printf("plyrel2:Return code = %d\n",retrncde);                           */

   retrncde = inqpos2(&x,&y);
/* printf("inqpos2:Return code = %d\n",retrncde);                           */
/* printf("Current drawing position is %f,%f\n",x,y);                       */

   retrncde = setchquality(MEDIUM);
/* printf("setchquality:Return code = %d\n",retrncde);                      */

   primstruct.color[0] = 1.0;primstruct.color[1] = 0.7;primstruct.color[2] = 0.2;
   retrncde = setcolor(primstruct.color);
/* printf("setcolor:Return code = %d\n",retrncde);                          */

   retrncde = movrel2(-100.,-100.);
/* printf("movrel2:Return code = %d\n",retrncde);                           */

   string = "Andrew Greenholt";
   retrncde = text(string);
/* printf("text:Return code = %d\n",retrncde);                              */

   retrncde = inqpos2(&x,&y);
/* printf("inqpos2:Return code = %d\n",retrncde);                           */
/* printf("Current drawing position is %f,%f\n",x,y);                       */

   retrncde = mrkabs2(100.,200.,43);
/* printf("mrkabs2:Return code = %d\n",retrncde);                           */

/* angle = 1.570796;
   retrncde = setro2(segname,angle);
   printf("setro2:Return code = %d\n",retrncde);
*/
   deltax = .2;deltay = .2;
   retrncde = settr2(segname,deltax,deltay);
/* printf("settr2:Return code = %d\n",retrncde);                            */

   retrncde = inqtr2(segname,&deltax,&deltay);
/* printf("inqtr2:Return code = %d\n",retrncde);                            */
/* printf("Segment %s has translation values %f,%f\n",segname,deltax,deltay);
*/
   retrncde = closeseg();
/* printf("closeseg:Return code = %d\n",retrncde);                          */

/* retrncde = newframe();
   printf("newframe:Return code = %d\n",retrncde);
*/
   newname = "show";
   retrncde = renameseg(segname,newname);
/* printf("renameseg:Return code = %d\n",retrncde);                         */

/* retrncde = deleteseg(newname);
   printf("deleteseg:Return code = %d\n",retrncde);                         */

   retrncde = deselectvwsurface(surfname);
/* printf("deselectvwsurface:Return code = %d\n",retrncde);                 */

/* retrncde = termvwsurface(surfname);
   printf("termvwsurface:Return code = %d\n",retrncde);                     */

/* retrncde = termcore();
   printf("termcore:Return code = %d\n",retrncde);                          */

   }
