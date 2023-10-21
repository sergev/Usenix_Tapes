/* plot.c -- plot routines for visual star trek */

#include "vtrek.h"
#include curses
/* replot screen */
replot()
{
	cls();
	plt_stat(ALL);
	plt_srs(ALL);
	plt_dam(ALL);
	plt_gal(ALL);
	mvaddstr(12,18,"READOUT");
	mvaddstr(13,18,"-------");
	refresh();
	plt_num(ALL);
}

/* plot status (upper left) */
plt_stat(op, item)
int op, item;
{
	static char *text[9] = {
	    "      Status", "      ------", "Stardate     :",
	    "Condition    :", "Quadrant     :", "Sector       :",
	    "Energy       :", "Photon torps :", "Shields      :"
	};
	static char *ctext[4] = {
	    "Green", "Yellow", "Red", "Docked"
	};
	int i, high, low;

	char buf[80];

	if (op & TEXT)
	    for (i = 0; i < 9; i++) {
		mvaddstr(i+2,1,text[i]);
		    }
		refresh();

	if (op & (INFO | ELEMENT)) {
	    if (op & INFO) {
		low = STARDATE;
		high = SHIELDS;
	    }
	    else {
		low = item;
		high = item;
	    }

	    for (i = low; i <= high; i++) {
		switch (i) {
		case STARDATE :
		    sprintf(buf,"%-.1f", stardate);
		    mvaddstr(4,16,buf);
		    refresh();
		    break;
		case CONDITION :
		    sprintf(buf,"%-6s", ctext[condition]);
		    mvaddstr(5,16,buf);
		    refresh();
		    break;
		case QUADRANT :
		    sprintf(buf,"[%d,%d]", xquad + 1, yquad + 1);
		    mvaddstr(6,16,buf);
		    refresh();
		    break;
		case SECTOR :
		    sprintf(buf,"[%d,%d]", xsect + 1, ysect + 1);
		    mvaddstr(7,16,buf);
		    refresh();
		    break;
		case ENERGY :
		    sprintf(buf,"%-4d", energy);
		    mvaddstr(8,16,buf);
		    refresh();
		    break;
		case TORPS :
		    sprintf(buf,"%-2d", torps);
		    mvaddstr(9,16,buf);
		    refresh();
		    break;
		case SHIELDS :
		    sprintf(buf,"%-4d", shields);
		    mvaddstr(10,16,buf);
		    refresh();
		    break;
		}
	    }
	}
}

/* plot short range scan */
plt_srs(op, xs, ys)
int op, xs, ys;
{
	static char *htext = "-1--2--3--4--5--6--7--8-";
	static char *stext[6] = {
	    "   ", "<K>", "<S>", " * ", "???", " + "
	};
	int i, j;
	char buf[80];
	int slen;

	if (op & TEXT) {
	    mvaddstr(1,28,htext);
	    for (i = 1; i < 9; i++) {
		sprintf(buf,"%d", i);
		mvaddstr(i+1,27,buf);
		sprintf(buf,"%d", i);
		mvaddstr(i+1,52,buf);
	    }
	    mvaddstr(10,28,htext);
	    refresh();
	}

	strcpy(stext[PLAYER], playership);

	if (op & INFO) {
	    for (i = 0; i < 8; i++) {
		slen=0;
		for (j = 0; j < 8; j++){
		    sprintf(buf,"%s", 
                        stext[(damage[SRS] <= 0) ? EMPTY : quadrant[j][i]]);
	            mvaddstr(i+2,28+slen,buf);
		    slen+=strlen(buf);
                  }
	refresh();
	}
       }
	else if (op & ELEMENT) {
	    sprintf(buf,"%s", stext[(damage[SRS] <= 0) ? EMPTY : quadrant[xs][ys]]);
            mvaddstr(ys+2,28+3*xs,buf);
	    refresh();
	}
}

/* plot damage info */
plt_dam(op, item)
int op, item;
{
	static char *text[10] = {
	    "    Damage Report", "    -------------",
	    "Warp engines    :", "S.R. sensors    :", "L.R. sensors    :",
	    "Phaser control  :", "Damage control  :", "Defense control :",
	    "Computer        :", "Photon tubes    :"
	};
	char buf[80];
	int i;

	if (op & TEXT)
	    for (i = 0; i < 10; i++) {
		sprintf(buf,"%s", text[i]);
                mvaddstr(i+1,56,buf);
	    }
		refresh();

	if (op & INFO)
	    for (i = 0; i < 8; i++) {
		if (damage[DAMAGE] <= 0){
	            mvaddstr(i+3,74,"    ");
		    refresh();
		}else{
		    sprintf(buf,"%4d", damage[i]);
	            mvaddstr(i+3,74,buf);
		    refresh();
	    }
        }
	else if (op & ELEMENT) {
	    if (damage[DAMAGE] <= 0){
	            mvaddstr(item+3,74,"    ");
		    refresh();
	    }else{
		    sprintf(buf,"%4d", damage[item]);
	            mvaddstr(item+3,74,buf);
		    refresh();
	}
}
}
/* plot galaxy map */
plt_gal(op, xq, yq)
int op, xq, yq;
{
	static char *htext = "-1- -2- -3- -4- -5- -6- -7- -8-";
	int i, j, fedquad;
	char buf[80];

	if (op & TEXT) {
            mvaddstr(13,47,htext);
            mvaddstr(22,47,htext);
	    for (i = 1; i < 9; i++) {
		sprintf(buf,"%d:", i);
	        mvaddstr(i+13,45,buf);
		for (j = 0; j < 7; j++) {
		    mvaddch(i + 13, 50 + (j << 2),':');
		}
		sprintf(buf,"%d:", i);
	        mvaddstr(i+13,78,buf);
	    }
		refresh();
	}

	if (op & INFO) {
	    for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++) {
		    if (damage[COMPUTER] <= 0 || !galaxy[j][i].known){
		        mvaddstr(i+14,47 + (j <<2),"   ");
			refresh();
		    }else{
			sprintf(buf,"%01d%01d%01d", galaxy[j][i].nkling, 
			   galaxy[j][i].nbase,
			   galaxy[j][i].nstar);
		        mvaddstr(i+14,47 + (j <<2),buf);
			refresh();
		}
             }
	    mvaddch(yquad + 14, 46 + (xquad << 2),'[');
	    mvaddch(yquad + 14, 50 + (xquad << 2),']');
	    refresh();
	}
	else if (op & ELEMENT) {
	    fedquad = (xq == xquad && yq == yquad);
	    mvaddch(yq + 14, 46 + (xq << 2),fedquad ? '[' : ':');
	    refresh();

	    if (damage[COMPUTER] <= 0){
	        mvaddstr(yq + 14, 47 + (xq << 2),"   ");
		refresh();
	    }else{
		sprintf(buf,"%01d%01d%01d", galaxy[xq][yq].nkling, 
                        galaxy[xq][yq].nbase, galaxy[xq][yq].nstar);
	        mvaddstr(yq + 14, 47 + (xq << 2),buf);
       	        mvaddch(yq+14,47 + (xq << 2)+strlen(buf),fedquad ? ']' : ':');
		refresh();
	}
    }
}

/* plot number of star bases & klingons */
plt_num(op)
int op;
{
	float kf;
	char buf[132];

	if (op & TEXT) {
	   mvaddstr(12,47,"Base stars = ");
	   mvaddstr(12,63,"Klingons = ");
	   mvaddstr(23,54,"Kill Factor = ");
	   refresh();
	}

	if (op & INFO) {
	    sprintf(buf," %d", numbases);
 	    mvaddstr(12,59,buf);
	    sprintf(buf," %d/%d  ", numkling, begkling);
 	    mvaddstr(12,73,buf);
	    if (begdate != stardate)
		kf = (begkling - numkling) / (stardate - begdate);
	    else
		kf = 0.0;
	    sprintf(buf,"%5.3f  ", kf);
 	    mvaddstr(23,68,buf);
		refresh();
	}
}

/* change readout */
readout(op, str)
int op;
char *str;
{
	int i, j;
	char buf[132];

	switch (op) {

	case CLEAR :		/* clear readout */
	    for (i = 14; i <= 13 + rolines; i++) {
		for (j = 0; j < 44; j++)
		    mvaddch(i,j+1,' ');
	    }
	    rolines = 0;
		refresh();
	    break;

	case ADDLINE :		/* add line to readout */
	    if (rolines >= 10)
		readout(CLEAR, NULL);
	    sprintf(buf,"%-.44s", str);
		        mvaddstr(rolines+14,1,buf);
			refresh();
	    rolines++;
	    break;
	}
	refresh();
}
