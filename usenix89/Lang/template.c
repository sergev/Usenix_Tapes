#define MAXSTRING 160
#include <stdio.h>
#include <ctype.h>

char *from_words[]={
/*1*/
	"CleartoEOLN",
	"_cleartoeoln",
	"_transmit_on",
	"address1",
	"addressII",
	"alternate_prompt",
	"alternative_addresses",
	"current_length",
	"current_record",
	"current_time",
	"default_editor",
	"default_weedlist",
	"define_softkeys",
	"display_central_message",
	"display_error",
	"display_headers",
	"display_title",
	"expand_env",
	"expand_filename",
	"expand_group",
	"expand_site",
	"expand_system",
	"expanded",
	"expanded_to",
	"filename",
	"forwarded",
	"get_return",
	"header_page",
	"header_rec",
	"header_table",
	"last_line",
	"machine_group",
	"mailbox_defined",
	"message_count",
	"message_number",
	"newaliases",
	"optimize_and_add",
	"optimize_arpa",
	"optimize_cmplx_arpa",
	"optimize_return",
	"optionally_enter",
	"original_cc",
	"original_msg_num",
	"parse_arpa_date",
	"parse_arpa_from",
	"pattern_enter",
	"pattern_match",
	"read_alias_files",
	"remove_domains",
	"remove_header",
	"reply_to",
	"resolve_received",
	"ret_addr",
	"return_value",
	"return_value_of",
	"sendmail",
	"show_menu",
	"show_msg_status",
	"softkeys_off",
	"subject_matches",
	"subjectbuffer",
	"system_call",
	"system_data",
	"system_data_file",
	"system_files",
	"system_hash_file",
	"system_hash_table",
	"system_record",
	"tail_of_string",
	"talk_to_sys",
	"temp_file",
	"timebuff",
	"timebuffer",
	"top_of_screen_left",
	"unexpanded_to"
/*E*/
};
char *to_words[]={
/*2*/
	"cleartoEOLN",
	"_Cleartoeoln",
	"_Transmit_on",
	"Address1",
	"aDdressII",
	"Alternate_prompt",
	"aLternative_addresses",
	"CUrrent_length",
	"cUrrent_record",
	"Current_time",
	"Default_editor",
	"dEfault_weedlist",
	"Define_softkeys",
	"DIsplay_central_message",
	"Display_error",
	"dIsplay_headers",
	"diSplay_title",
	"ExPand_env",
	"exPand_filename",
	"eXpand_group",
	"EXpand_site",
	"Expand_system",
	"Expanded",
	"eXpanded_to",
	"Filename",
	"Forwarded",
	"Get_return",
	"HEader_page",
	"Header_rec",
	"hEader_table",
	"Last_line",
	"Machine_group",
	"Mailbox_defined",
	"Message_count",
	"mEssage_number",
	"Newaliases",
	"oPtimize_and_add",
	"opTimize_arpa",
	"OPtimize_cmplx_arpa",
	"Optimize_return",
	"Optionally_enter",
	"oRiginal_cc",
	"Original_msg_num",
	"pArse_arpa_date",
	"Parse_arpa_from",
	"Pattern_enter",
	"pAttern_match",
	"Read_alias_files",
	"Remove_domains",
	"rEmove_header",
	"Reply_to",
	"Resolve_received",
	"Ret_addr",
	"rEturn_value",
	"Return_value_of",
	"Sendmail",
	"Show_menu",
	"Show_msg_status",
	"Softkeys_off",
	"sUbject_matches",
	"Subjectbuffer",
	"sYStem_call",
	"SYstem_data",
	"syStem_data_file",
	"SyStem_files",
	"System_hash_file",
	"sYstem_hash_table",
	"SYStem_record",
	"Tail_of_string",
	"Talk_to_sys",
	"Temp_file",
	"Timebuff",
	"tImebuffer",
	"Top_of_screen_left",
	"Unexpanded_to"
/*E*/
};

main(argc,argv)
int argc;
char **argv;
{   char word[MAXSTRING], *wnext_ch = word;
    int c;

    while ((c = getchar()) != EOF) {
	if (isalpha(c) || (wnext_ch != word && isdigit(c)) || c == '_') {
	    *(wnext_ch++) = c;
	}
	else {
	    if (wnext_ch != word) {
		*wnext_ch = '\0';
		output_word(word);
		wnext_ch = word;
	    }
	    putchar(c);
	}
    }
    if (wnext_ch != word) {
	*wnext_ch = '\0';
	output_word(word);
	wnext_ch = word;
    }
}
 
output_word(word)
char *word;

{   register int low_word, high_word, cmp_result, word_num;

    low_word = 0;
    high_word =
/*3*/
/*E*/
;
    while (high_word >= low_word) {
	word_num = (high_word + low_word) >> 1;
	if (!(cmp_result = strcmp(word, from_words[word_num]))) {
	    fputs(to_words[word_num], stdout);
	    return;
	}
	else if (cmp_result < 0) {
	    high_word = word_num - 1;
	}
	else {
	    low_word = word_num + 1;
	}
    }
    fputs(word, stdout);
}
