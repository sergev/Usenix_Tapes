extern int cgoof;
int area[16], rbuf[256];
int quit, color, fd, yroll, palmode;

/* to compile this program: load tempera -lp -lg  */
/* to execute, then type:   a.out                 */
/* Program written by Marshall Schaffer - all rights reserved */

main() {

	int buf[4];

	paintopen();    /* initialize Grinnell and program */

/* start of program */

	canvas();

	while (1)   {

		grcur(area,buf,0);

		if(buf[0] <= 255 && buf[1] >= 236)      yroll = 0;

			else {yroll =  236;}

		switch (palmode) {

		     case '1':
				subpal();
				break;

		     case '2':
				addpal();
				break;

		     case '3':
				prepal();
				break;

		     default:
				prepal();
				break;

		     }          /* end switch */

		if(quit == 1)  break;

		draw();

	}       /* end while loop */

	paintclose();   /* close Grinnell */

}       /* end main program */


canvas()  {

	int k;
	char c;

for(k=1;   k<30; k++)   {
	printf("\n");
			}

	drawbox(8,18,503,453);          /* draw border around picture */

/* draw box for current color - color sum */
	if(drawbox(89,454,155,479))  {
		printf("couldnt write curcol box\n");
		exit(); }

     printf("Type 1 for subtractive color mixing (yellow, cyan, magenta)\n");
     printf("     2 for additive color mixing (red, green, blue)\n");
     printf("     3 for 'premixed' palette\n");

	palmode = getchar();

	printf("Do you want the 'canvas' (the color tv screen)\n");
	printf("to be white or some other color?\n");
	printf("      -----    ---- ----- -----\n");
	printf("\n");

	prompt:
		printf("(Please type w or c on the keyboard,\n");
		printf("followed by a carriage return.)\n");

		c = getchar();
		switch(c)  {

			case 'w':
					color = 07777;
					break;

			case 'c':
					yroll = 236;
					switch (palmode) {

					  case '1':
						     subpal();
						     break;

					  case '2':
						     addpal();
						     break;

					  case '3':
						     prepal();
						     break;


					  default:
						     prepal();
						     break;

		     }          /* end switch */


					break;

			default:
					goto prompt;

		     }          /* end switch */


/* paint the screen the desired color */
	genter(area,3,color,0,0,0);
	gwvec(area,11,21,500,450,1);

for(k=1;   k<30; k++)   {
	printf("\n");
			}

/* prompt user, and pause. */
rep:
	printf("If you are ready to begin\n");
	printf("painting, type c and hit carriage return.\n");

	c = getchar();
	if(c != 'c')  goto rep;

	gwcur(area,1,260,230,1,0);      /* initialize cursor 1 position */
	gwcur(area,2,270,230,1,0);      /* initialize cursor 2 position */

for(k=1;   k<30; k++)   {
	printf("\n");
			}

}       /* end canvas subprogram */


draw()  {

	int curbuf[4], oldc1x,oldc1y,oldc2x,oldc2y;

	genter(area,3,color,2,2,0);

	while (1)  {
		     grcur(area,curbuf,1);

		     if(curbuf[0] <= 10)  curbuf[0] = 11;
		     if(curbuf[1] <= 20)  curbuf[1] = 21;
		     if(curbuf[2] >= 500) curbuf[2] = 499;
		     if(curbuf[3] >= 450) curbuf[3] = 449;

		     if(oldc1x == curbuf[0] && oldc1y == curbuf[1]
			&& oldc2x == curbuf[2] && oldc2y == curbuf[3])
				break;

		     oldc1x = curbuf[0];        oldc1y = curbuf[1];
		     oldc2x = curbuf[2];        oldc2y = curbuf[3];

		     gwvec(area,curbuf[0],curbuf[1],curbuf[2],curbuf[3],0);
		    }

}       /* end draw subprogram */


subpal()       {

	int mval,yval,cval,nval;
	int blast,glast,rlast;
	int bcolor,rcolor,gcolor;
	int pbuf[4],oldpos[4];
	int rval,bval,gval;
	int pricolor,lastcolor;
	int cx, cy, r;

/* initialize disk file */
	fd = creat("/tmp/quarter",0666);
	close(fd);
	fd = open("/tmp/quarter",2);
	unlink("/tmp/quarter");
/* roll out the appropriate quadrant of the picture */
	rollout();

/* do a gclear where palette will go */
	gclear(area,11,21+yroll,245,195,07777);

/* write the primary colors */

	mval = 255;     yval = 3855;    cval = 4080;    nval = 07777;

	for(r=20;r <=230; r = r + 15)   {

		genter(area,3,mval,0,2,0);
		gwvec(area,r,185+yroll,r+15,200+yroll,1);
		mval = mval - 17;

		genter(area,3,yval,0,2,0);
		gwvec(area,r,160+yroll,r+15,175+yroll,1);
		yval = yval - 257;

		genter(area,3,cval,0,2,0);
		gwvec(area,r,135+yroll,r+15,150+yroll,1);
		cval = cval - 272;

		genter(area,3,nval,0,2,0);
		gwvec(area,r,110+yroll,r+15,125+yroll,1);
		nval = nval - 273;               }

/* draw and label "control boxes"                         */
   dlboxes();

/* save cursor position */
	grcur(area,oldpos,0);

/* move cursor1 into SELECT box;  turn cursor2 off. */
	gwcur(area,1,225,75+yroll,1,1);
	gwcur(area,2,300,100,0,1);

/* palette command interpriter.                 */
lastcolor = color;pricolor = 0;color = 0;  /* initialize */
gcolor = 15; bcolor = 15; rcolor = 15;
update();
loop:
	grcur(area,pbuf,1);
	cx = pbuf[0];   cy = pbuf[1];
	if(cy < 55+yroll || cx > 253)          /*ignore out of bounds*/
	    goto loop;

	    else { if(cy > 105+yroll)
			{pricolor =grpnt(area,cx,cy);
			 lastcolor = color;

			 gval = pricolor / 256;
			 bval = (pricolor - (gval*256)) / 16;
			 rval = pricolor - (256*gval) - (16 * bval);

			 glast = gcolor; blast = bcolor; rlast = rcolor;

			 if(gval == 0) {if((15-gcolor) < rval)
					{gcolor = gcolor - (rval-(15-gcolor));
					 if(gcolor < 0) gcolor = 0;  }}
		     if(bval == 0) {if((15-bcolor) < gval)
				    {bcolor = bcolor - (gval-(15-bcolor));
				     if(bcolor < 0) bcolor = 0;   }}
		     if(rval == 0) {if((15-rcolor) < bval)
				    {rcolor = rcolor - (bval-(15-rcolor));
				     if(rcolor < 0) rcolor = 0;   }}

		      color = (256*gcolor) + (16*bcolor) + (1*rcolor);

			 update();                      }

		     else { if(cx <= 58 && cx >= 9)
			      { quit = 1;   goto roll; }

			 else { if(cx <= 123 && cx >= 73)
				     {color = lastcolor;
				      update();         }

				else { if(cx <= 188 && cx >= 138)
					   {color = 0;
					    update();   }

				       else { if(cx <= 253 &&
						 cx >= 203) goto roll;}
	}}}}
goto loop;

roll:

/* do a gclear over where palette is */
	gclear(area,9,21+yroll,247,195,07777);

/* rollin picture from memory */
	rollin();

/* close disk file */
	close(fd);

/* restore cursor positions prior to palette */
	gwcur(area,1,oldpos[0],oldpos[1],1,0);
	gwcur(area,2,oldpos[2],oldpos[3],1,0);

}              /* end palette subprogram */

drawbox(xlo,ylo,xhi,yhi)
int xlo,ylo,xhi,yhi;    {

	gclear(area,xlo,ylo,xhi-xlo+1,yhi-ylo+1,07777);
	genter(area,3,07777,0,0,0);
	gwvec(area,xlo,ylo,xhi,yhi,1);

	gclear(area,xlo+1,ylo+1,xhi-xlo-1,yhi-ylo-1,07777);
	genter(area,3,010000,0,0,0);
	gwvec(area,xlo+1,ylo+1,xhi-1,yhi-1,1);

}


paintopen()  {

	gopen(area,0,0,512,480);

	gclear(area,0,0,512,480,07777);

	quit = 0;

}

paintclose()  {

	gclose(area);

}

rollin()        {

	int i, ymax;

	ymax = yroll + 216;
	for(i=yroll; i< ymax; i++)   {

	     if(read(fd,rbuf,512) < 0)
		printf("read messed up\n");

	     if(gwrow(area,rbuf,i,0,256,1))
		printf("gwrow messed up\n");

	}       /* end for loop */

	seek(fd,0,0);

}       /* end rollin subprogram */

rollout()       {

	int i, ymax;

	ymax = yroll + 216;
	for(i = yroll;i < ymax; i++) {

	     if(grrow(area,rbuf,i,0,256,1))
		printf("grrow messed up\n");

	     if(write(fd,rbuf,512) < 0)
		printf("write messed up\n");

	}       /* end for loop */

	seek(fd,0,0);

}       /* end rollout subprogram */



prepal()       {


	int oldpos[4], curbuf[4];
	int rval,bval,gval,nval;
	int lastcolor;
	int cx, cy, r, i;

/* initialize disk file */
	fd = creat("/tmp/quarter",0666);
	close(fd);
	fd = open("/tmp/quarter",2);
	unlink("/tmp/quarter");

/* save cursor position */
	grcur(area,oldpos,0);

/* roll out the appropriate quadrant of the picture */
	rollout();

/* do a gclear where palette will go */
A:      gclear(area,11,21+yroll,245,195,07777);

/* move cursor1 into SELECT box;  turn cursor2 off. */
	gwcur(area,1,225,80+yroll,1,1);
	gwcur(area,2,300,100,0,1);

/* draw and label control boxes */
   dlboxes();

/* pre-mixed palette */
   rval = 15;
for(i=110; i <= 192; i = i + 6)        {
	genter(area,3,rval,0,0,0);
	gwvec(area,20,i+yroll,245,i+yroll+5,1);
	rval = rval - 1;        }

bval = 240;
for(i=20;i <= 230; i = i + 15)   {
	genter(area,3,bval,0,1,0);
	gwvec(area,i,110+yroll,i+14,198+yroll,1);
	bval = bval - 16;       }

grcur(area,curbuf,1);
if(curbuf[1] <= 89+yroll)  {goto C;}

lastcolor = color;
color = grpnt(area,curbuf[0],curbuf[1]);
update();

genter(area,3,color,0,2,0);
gwvec(area,20,110+yroll,245,198+yroll,1);

gval = 3840;
for(i=20; i <= 230; i = i + 15)   {
     genter(area,3,gval,0,1,0);
     gwvec(area,i,110+yroll,i+14,198+yroll,1);
     gval = gval - 256;          }

grcur(area,curbuf,1);

if(curbuf[1] <= 89+yroll)  {goto C;}

lastcolor = color;
color = grpnt(area,curbuf[0],curbuf[1]);
update();

/* palette command interpriter. */
loop:
	grcur(area,curbuf,1);
C:      cx = curbuf[0];   cy = curbuf[1];
	if(cy < 55+yroll || cx > 253)          /*ignore out of bounds*/
	    goto loop;

	       else { if(cx <= 58 && cx >= 9)     /* QUIT */
			{ quit = 1;   goto roll; }

		   else { if(cx <= 123 && cx >= 73) /* LAST COLOR */
			       {color = lastcolor;
				update();      }

			 else { if(cx <= 188 && cx >= 138)  /* NEW COLOR */
				     {color = 0;
				      update();
					    goto A;         }

	      /* SELECT */             else { if(cx <= 253 &&
						 cx >= 203) goto roll;}
	}}}
goto loop;

roll:

/* do a gclear over where palette is */
	gclear(area,9,21+yroll,247,195,07777);

/* rollin picture from memory */
	rollin();

/* close disk file */
	close(fd);

/* restore cursor positions prior to palette */
	gwcur(area,1,oldpos[0],oldpos[1],1,0);
	gwcur(area,2,oldpos[2],oldpos[3],1,0);

}              /* end palette subprogram */



addpal()       {


	int pbuf[4],oldpos[4];
	int rval,bval,gval,nval;
	int pricolor,lastcolor;
	int cx, cy, r;

/* initialize disk file */
	fd = creat("/tmp/quarter",0666);
	close(fd);
	fd = open("/tmp/quarter",2);
	unlink("/tmp/quarter");
/* roll out the appropriate quadrant of the picture */
	rollout();

/* do a gclear where palette will go */
	gclear(area,11,21+yroll,245,195,07777);

/* write the primary colors */

	rval = 15;      bval = 240;     gval = 3840;    nval = 07777;

	for(r=20;r <=230; r = r + 15)   {

		genter(area,3,rval,0,2,0);
		gwvec(area,r,185+yroll,r+15,200+yroll,1);
		rval = rval - 1;

		genter(area,3,bval,0,2,0);
		gwvec(area,r,160+yroll,r+15,175+yroll,1);
		bval = bval - 16;

		genter(area,3,gval,0,2,0);
		gwvec(area,r,135+yroll,r+15,150+yroll,1);
		gval = gval - 256;

		genter(area,3,nval,0,2,0);
		gwvec(area,r,110+yroll,r+15,125+yroll,1);
		nval = nval - 273;               }

/* draw and label "control boxes"                         */
   dlboxes();

/* save cursor position */
	grcur(area,oldpos,0);

/* move cursor1 into SELECT box;  turn cursor2 off. */
	gwcur(area,1,225,80+yroll,1,1);
	gwcur(area,2,300,100,0,1);

/* palette command interpriter.                 */
lastcolor = color;pricolor = 0;color = 0;  /* initialize */
update();
loop:
	grcur(area,pbuf,1);
	cx = pbuf[0];   cy = pbuf[1];
	if(cy < 55+yroll || cx > 253)          /*ignore out of bounds*/
	    goto loop;

	    else { if(cy > 105+yroll)
		       {pricolor =grpnt(area,cx,cy);
			lastcolor = color;
			color = color | pricolor;
			update();                      }

		     else { if(cx <= 58 && cx >= 9)
			      { quit = 1;   goto roll; }

			 else { if(cx <= 123 && cx >= 73)
				     {color = lastcolor;
				      update();         }

				else { if(cx <= 188 && cx >= 138)
					   {color = 0;
					    update();   }

				       else { if(cx <= 253 &&
						 cx >= 203) goto roll;}
	}}}}
goto loop;

roll:

/* do a gclear over where palette is */
	gclear(area,9,21+yroll,247,195,07777);

/* rollin picture from memory */
	rollin();

/* close disk file */
	close(fd);

/* restore cursor positions prior to palette */
	gwcur(area,1,oldpos[0],oldpos[1],1,0);
	gwcur(area,2,oldpos[2],oldpos[3],1,0);

}              /* end palette subprogram */


update()   {
/* update current color */
	gclear(area,90,455,64,24,07777);
	genter(area,3,color | 010000,2,2,0);
	gwvec(area,90,455,154,478,1);

	    }

dlboxes()  {

	/* draw and label 'control' boxes */

	drawbox(9,39+yroll,58,89+yroll);
	drawbox(73,39+yroll,123,89+yroll);
	drawbox(138,39+yroll,188,89+yroll);
	drawbox(203,39+yroll,253,89+yroll);

	genter(area,3,07777,2,2,0);
	gwstr(area,"QUIT",20,61+yroll);
	gwstr(area,"LAST",87,67+yroll);
	gwstr(area,"COLOR",83,57+yroll);
	gwstr(area,"NEW",155,67+yroll);
	gwstr(area,"COLOR",145,57+yroll);
	gwstr(area,"SELECT",209,61+yroll);

	   }
