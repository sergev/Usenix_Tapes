#define PAGE_MID ":more (%2d%%):"
#define PAGE_NEXT ":next article:"
#define PAGE_END ":end:"
#define PAGE_NO ":?:"
#define PPR_MAX 18	/* maximum length of PAGE prompts */

/*
	reading commands: no control chars, add help message to helppg
	SAVE, HEADTOG and SETROT are also recognized
*/
#define HPG_HEAD "toggle header print flag"
#define HPG_ROT "toggle rotation"
#define HPG_SAVE "save article in a file"
#define PG_NEXT 'n'
#define HPG_NEXT "next article, if any"
#define PG_QUIT 'q'
#define HPG_QUIT "quit reading articles, if any more to read"
#define PG_FLIP 'Q'
#define HPG_FLIP "quit reading, and turn to next page of articles"
#define PG_FOLLOW 'f'
#define HPG_FOLLOW "post followup to article"
#define PG_REPLY 'm'
#define HPG_REPLY "send mail to author of article"
#define PG_HELP '?'
#define HPG_HELP "see this help menu"
#define PG_REWIND 'r'
#define HPG_REWIND "rewind article to beginning"
#define PG_WIND 'e'
#define HPG_WIND "seek to end of article (to next/end prompt)"
#define PG_STEP '\n'
#define HPG_STEP "next line"
#define HPG_DEF "\n anything else to continue normal reading"
#define HPG_EDEF "\n anything else to try reading next article, if any"
