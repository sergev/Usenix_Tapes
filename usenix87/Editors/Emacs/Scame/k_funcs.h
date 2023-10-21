/*	SCAME k_funcs.h				*/

/*	Revision 1.0.0  1985-02-09		*/

/*	Copyright 1985 by Leif Samuelsson	*/

extern int
	k_appendnextkill(), k_ascii(), k_backchar(),
	k_backdelchar(), k_backkillword(), k_backtoindent(),
	k_backword(), k_begofbuf(), k_begli(),
	k_begse(), k_bell(),
	k_c_m_prefix(), k_c_prefix(), k_c_x_dispatch(),
	k_capitalizeword(), k_comment(), k_copyregion(),
	k_cntlpg(), k_countlregion(), k_delblanklines(),
	k_delchar(), k_delhorizspace(), k_delindent(), k_digit(),
	k_dired(), k_dispdate(), k_downline(), k_edt_0(),
	k_edt_8(), k_edt_comma(), k_edt_dash(), k_edt_pf4(),
	k_enddefkbdmac(), k_endofbuf(), k_endli(),
	k_endse(), k_exchgdotmark(), k_executekbdmac(),
	k_exit(), k_exit_to_shell(), k_extended(), k_fillregion(),
	k_findfile(), k_forwchar(), k_forwword(), k_growwindow(),
	k_help(), k_indentregion(), k_indnewline(), k_insertc(),
	k_instab(), k_isearch(), k_killbuffer(), k_killline(),
	k_killregion(), k_killsentence(), k_killword(),
	k_listbuffers(), k_listdir(), k_logout(),
	k_lowcaseregion(), k_locasw(), k_m_o_dispatch(),
	k_m_prefix(), k_mailbuffer(), k_makeparens(),
	k_markbuffer(), k_markword(), k_metadigit(), k_negarg(), k_newline(),
	k_newwindow(), k_nextscreen(), k_notmodified(),
	k_onewindow(), k_openline(), k_otherwindow(),
	k_prevscreen(), k_push(), k_pushpopmark(),
	k_queryreplace(), k_quote(), k_readmail(),
	k_risearch(), k_savefile(), k_scrollotherwindow(),
	k_selectbuffer(), k_setcommcol(), k_setfcol(),
	k_setfillprefix(), k_shellcmd(), k_sis(),
	k_startdefkbdmac(), k_tab(), k_transchar(), k_trnsword(),
	k_twowindows(),
	k_ubk(), k_univarg(), k_upcaseregion(), k_upcwd(),
	k_upline(), k_visitfile(), k_void(), k_whatcursorpos(),
	k_writefile(), k_yank();
