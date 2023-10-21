typedef enum { P_CONTINUE, P_NEXT_PERSON, P_ABORT, P_HELP } P_Choices;

typedef enum { 
        
        M_SEARCH_BY_OTHER, M_SEARCH_BY_NAME, M_PRINT_TO_LASER_PRINTER,
        M_HELP, M_EXIT, M_PERUSE, M_SAVE, M_ADD

      } Main_Choices;

typedef enum { 

        A_ABORT_ADD, A_BACKUP, A_FILL_IN_REST,
        A_ABORT_ROLO, A_HELP, A_NO_DATA

      } Add_Choices;

typedef enum { 
        
        E_ABORT, E_UPDATE, E_DELETE, E_CONTINUE, E_PREV, E_HELP, E_SCAN
        
      } E_Choices;

typedef enum { U_ABORT, U_HELP, U_END_UPDATE } U_Choices;

typedef enum { S_ABORT, S_HELP, S_SCAN_ONE_BY_ONE } S_Choices;

typedef enum { O_ABORT, O_HELP, O_BACKUP, O_DONE_OTHERS } O_Choices;
