#include "types.h"
#include "ext.h"

pr_bets()
{
	int i;

	mvaddstr(R+15,C+40,"       ");	/* clear pass line */
	mvaddstr(R+13,C+40,"       ");	/* clear dont pass */
	mvaddstr(R+15,C+47,"        ");	/* clear pass line odds */
	mvaddstr(R+13,C+47,"        "); /* clear dont pass odds */
	mvaddstr(R+11,C+48,"       ");	/* clear field */
	mvaddstr(R+8 ,C+48,"       "); 	/* clear come bar */
	mvaddstr(R+6 ,C+2 ,"       ");	/* clear dont come bar */
	mvaddstr(R+2 ,D+5 ,"       ");	/* clear any seven */
	mvaddstr(R+6 ,D+1 ,"       ");	/* clear hard 6 */
	mvaddstr(R+6 ,D+9 ,"       ");	/* clear hard 10 */
	mvaddstr(R+10,D+1 ,"       ");	/* clear hard 8 */
	mvaddstr(R+10,D+9 ,"       ");	/* clear hard 4 */
	mvaddstr(R+14,D+1 ,"       ");  /* clear eleven */
	mvaddstr(R+14,D+9 ,"       ");	/* clear twelve */
	mvaddstr(R+18,D+1 ,"       ");	/* clear snake eyes */
	mvaddstr(R+18,D+9 ,"       ");	/* clear ace-duece */
	mvaddstr(R+21,D+5 ,"       ");	/* clear any craps */
	for(i=0;i<11;i++) {
		if(chk_plc(i)) {
			mvaddstr(R+7,pcol[i]-1,"-------"); /* place bets */
			mvaddstr(R+6,pcol[i]-1,"       "); /* come bet */
			mvaddstr(R+5,pcol[i]-1,"       "); /* come bet odds */
			mvaddstr(R+1,pcol[i]-1,"       "); /* done come bet */
			mvaddstr(R  ,pcol[i]-1,"-------"); /* dont come odds */
		}
	}
	if(pass!=0.0) {move(R+15,C+40); printw("$%.2f",pass);}
	if(dont!=0.0) {move(R+13,C+40); printw("$%.2f",dont);}
	if(point && odds[point]!=0.0) {
		move(R+15,C+47); printw("/$%.2f",odds[point]);
	}
	if(point && lodds[point]!=0.0) {
		move(R+13,C+47); printw("/$%.2f",lodds[point]);
	}
	if(field!=0.0) {move(R+11,C+48); printw("$%.2f",field);}
	if(comeb!=0.0) {move(R+8,C+48); printw("$%.2f",comeb);}
	if(dcomeb!=0.0) {move(R+6,C+2); printw("$%.2f",dcomeb);}
	if(aseven!=0.0) {move(R+2,D+5); printw("$%.2f",aseven);}
	if(hways[6]!=0.0) {move(R+6,D+1); printw("$%.2f",hways[6]);}
	if(hways[10]!=0.0) {move(R+6,D+9); printw("$%.2f",hways[10]);}
	if(hways[8]!=0.0) {move(R+10,D+1); printw("$%.2f",hways[8]);}
	if(hways[4]!=0.0) {move(R+10,D+9); printw("$%.2f",hways[4]);}
	if(eeyo!=0.0) {move(R+14,D+1); printw("$%.2f",eeyo);}
	if(boxcars!=0.0) {move(R+14,D+9); printw("$%.2f",boxcars);}
	if(aces!=0.0) {move(R+18,D+1); printw("$%.2f",aces);}
	if(aceduece!=0.0) {move(R+18,D+9); printw("$%.2f",aceduece);}
	if(acraps!=0.0) {move(R+21,D+5); printw("$%.2f",acraps);}
	for(i=0;i<11;i++) {
		if(place[i]!=0.0 && chk_plc(i)) {
			move(R+7,pcol[i]-1);
			printw("$%.2f",place[i]);
		}
		if(come[i]!=0.0 && chk_plc(i)) {
			move(R+6,pcol[i]-1);
			printw("$%.2f",come[i]);
			if(odds[i]!=0.0) {
				move(R+5,pcol[i]-1);
				printw("$%.2f",odds[i]);
			}
		}
		if(dcome[i]!=0.0 && chk_plc(i)) {
			move(R+1,pcol[i]-1);
			printw("$%.2f",dcome[i]);
			if(lodds[i]!=0.0) {
				move(R,pcol[i]-1);
				printw("$%.2f",lodds[i]);
			}
		}
	}
	refresh();
}

print_board()
{

	clear();
mvaddstr(R   ,C,"----------------------------------------------------------");
mvaddstr(R+1 ,C,"|        |       |       |       |       |       |       |");
mvaddstr(R+2 ,C,"|  DONT  -------------------------------------------------");
mvaddstr(R+3 ,C,"|  COME  |   4   |   5   |   6   |   8   |   9   |  10   |");
mvaddstr(R+4 ,C,"|        |       |       |       |       |       |       |");
mvaddstr(R+5 ,C,"| BAR 12 |       |       |       |       |       |       |");
mvaddstr(R+6 ,C,"|        |       |       |       |       |       |       |");
mvaddstr(R+7 ,C,"----------------------------------------------------------");
mvaddstr(R+8 ,C,"|                            C O M E                     |");
mvaddstr(R+9 ,C,"----------------------------------------------------------");
mvaddstr(R+10,C,"         |   double     F I E L D      triple            |");
mvaddstr(R+11,C,"         |     2    3   4   9  10  11    12              |");
mvaddstr(R+12,C,"         -------------------------------------------------");
mvaddstr(R+13,C,"         |        DONT PASS BAR 12                       |");
mvaddstr(R+14,C,"         -------------------------------------------------");
mvaddstr(R+15,C,"         |        P A S S    L I N E                     |");
mvaddstr(R+16,C,"         -------------------------------------------------");
mvaddstr(R   ,D,"-----------------");
mvaddstr(R+1 ,D,"| Any Seven 4-1 |");
mvaddstr(R+2 ,D,"|               |");
mvaddstr(R+3 ,D,"-----------------");
mvaddstr(R+4 ,D,"|Hard 6 |Hard 10|");
mvaddstr(R+5 ,D,"|  9-1  |  7-1  |");
mvaddstr(R+6 ,D,"|       |       |");
mvaddstr(R+7 ,D,"-----------------");
mvaddstr(R+8 ,D,"|Hard 8 |Hard 4 |");
mvaddstr(R+9 ,D,"|  9-1  |  7-1  |");
mvaddstr(R+10,D,"|       |       |");
mvaddstr(R+11,D,"-----------------");
mvaddstr(R+12,D,"|Eleven |Twelve |");
mvaddstr(R+13,D,"| 14-1  | 29-1  |");
mvaddstr(R+14,D,"|       |       |");
mvaddstr(R+15,D,"-----------------");
mvaddstr(R+16,D,"|  Two  | Three |");
mvaddstr(R+17,D,"| 29-1  |  14-1 |");
mvaddstr(R+18,D,"|       |       |");
mvaddstr(R+19,D,"-----------------");
mvaddstr(R+20,D,"| Any Craps 8-1 |");
mvaddstr(R+21,D,"|               |");
mvaddstr(R+22,D,"-----------------");
}
