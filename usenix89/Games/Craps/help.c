#include "types.h"
#include "ext.h"

help()
{
	clear(); refresh();
	mvaddstr(0 ,20,"Key        Action");
	mvaddstr(1 ,20,"---        ------");
	mvaddstr(2, 20," r         Roll the Dice");
	mvaddstr(3 ,20," p         Pass Line bet");
	mvaddstr(4 ,20," d         Dont Pass bet");
	mvaddstr(5 ,20," c         Come bet");
	mvaddstr(6 ,20," D         Dont Come bet");
	mvaddstr(7 ,20," o         Take odds on Pass Line/Come bets");
	mvaddstr(8 ,20," l         Lay odds on Dont Pass/Dont Come bets");
	mvaddstr(9 ,20," b         Place bet");
	mvaddstr(10,20," f         Field bet");
	mvaddstr(11,20," h         Hardway bet");
	mvaddstr(12,20," s         Any Seven bet");
	mvaddstr(13,20," a         Any Craps bet");
	mvaddstr(14,20," 2         Snake Eyes (two) bet");
	mvaddstr(15,20," 3         Three bet");
	mvaddstr(16,20," y         Eeyo-'Leven (Eleven) bet");
	mvaddstr(17,20," u         Boxcars (Twelve) bet");
	mvaddstr(18,20," C         Cheat, set the dice for the next roll");
	mvaddstr(19,20," q         Quit");
	mvaddstr(20,20," ?         This List");
	mvaddstr(21,20," !         Shell escape");
	refresh();
	msg("Press [space] to continue",23,1);
	print_board();
	if(point) mvaddstr(4,pcol[point],"----");
	pr_bets();
	cheat=1;
	roll();
	cheat=0;
	update(1);
}
