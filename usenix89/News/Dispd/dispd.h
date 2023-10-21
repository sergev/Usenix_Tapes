#define USERS "/u/lme/du/users" /* only users in this file will be displayed */
#define MFILE "/usr/spool/message/dumess" /* file for backing up the display */
#define TITLE "XT/DU ERICSSON COMPUTER SCIENCE LABORATORY" /* title */
#define UPDATE "Last updated at:" /* text on the display */
#define DISTTY "/dev/tty25" /* terminal to which the display is connected */
#define GLOB_MESS "/u/lme/du/message" /* file for the global message */
#define WHDIR "/usr/spool/rwho" /* standard 4.2 */
#define HOST "erix" /* host for the deamon */
#define DISTYPE "vt100"
struct user_data {
	char user_name[10];
	char user_message[81];
	char real_name[20];
	char machtty[15][9];
	int idle, lastmach;
}; 
