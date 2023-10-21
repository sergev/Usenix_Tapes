/*
 * Pseudo functions for use with TGRAPH
 * #include <curses.h> before use
 */

#define graph(str)		 wgraph(stdscr, str)
#define mvgraph(y, x, str)	 (move(y,x)==ERR?ERR:graph(str))
#define mvwgraph(win, y, x, str) (wmove(win,y,x)==ERR?ERR:wgraph(win,str))

#ifndef ACS_HLINE

/*
 * SVr3-compatible ACS indexes
 */

extern struct __g__ch_ {
	char g_name[6];
	short g_graph;
} __graph[];

#define ACS_HLINE	(__graph[0].g_graph)
#define ACS_VLINE	(__graph[1].g_graph)
#define ACS_PLUS	(__graph[2].g_graph)
#define ACS_LTEE	(__graph[3].g_graph)
#define ACS_RTEE	(__graph[4].g_graph)
#define ACS_TTEE	(__graph[5].g_graph)
#define ACS_BTEE	(__graph[6].g_graph)
#define ACS_LLCORNER	(__graph[7].g_graph)
#define ACS_ULCORNER	(__graph[8].g_graph)
#define ACS_URCORNER	(__graph[9].g_graph)
#define ACS_LRCORNER	(__graph[10].g_graph)
#define ACS_S9	(__graph[11].g_graph)
#define ACS_S1	(__graph[12].g_graph)
#define ACS_UARROW	(__graph[13].g_graph)
#define ACS_DARROW	(__graph[14].g_graph)
#define ACS_LARROW	(__graph[15].g_graph)
#define ACS_RARROW	(__graph[16].g_graph)
#define ACS_DIAMOND	(__graph[17].g_graph)
#define ACS_CKBOARD	(__graph[18].g_graph)
#define ACS_DEGREE	(__graph[19].g_graph)
#define ACS_PLMINUS	(__graph[20].g_graph)
#define ACS_BULLET	(__graph[21].g_graph)
#define ACS_BOARD	(__graph[22].g_graph)
#define ACS_LANTERN	(__graph[23].g_graph)
#define ACS_BLOCK	(__graph[24].g_graph)

#endif ACS_HLINE
