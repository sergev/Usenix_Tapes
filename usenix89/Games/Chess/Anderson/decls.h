/*
 * global declarations 
 */

/* board.c */

extern void InitBoard(), DoMove(), UnDoMove(), ShowLastPlay(), 
    SaveGame(), WriteTranscript(), RestoreGame(), DoSetupChange(),
    DoResignation();
extern BOOL InitialTurn, IsOurPieceAt(), IsSrcPieceAt(), 
    IsMoveLegal(), IHaveMoved(), InCheck(), GameOver;
extern Square * GetSquare(), * GetSrcSquare();
extern int PromotePawn(), Turn;

/* boardsw.c */

extern void InitBoardSW(), DrawBoard(), DrawSquare(), AddVictim(),
    DeleteVictim(), KillMouseActivity(), RequestUndo();
extern MouseState Mouse;
extern BOOL Flashing;

/* chessprocess.c */

extern void InitChessProcess(), ReapChessProcesses(), KillChessProcesses(),
    SendMachineMove(), MachineUndo(), MachineRestore(), MachineFirst();
extern BOOL MachineSetup(), MachineDebug;
extern int GetMachineMove(), MachineSave(), ChessProcessFDs[];

/* controlsw.c */

extern void InitControlSW();
extern BOOL SaveWanted;

/* ipc.c */

extern int MyColor, PeerColor;
extern unsigned long PeerProgNum; 
extern void InitRPC(), SendResignation(), SendUndoRequest(), SendTalkMsg(),
    SendRestoreMove(), SendEndRestore(), SendSetupChange(),
    SendUndoAcknowledgement();
extern BOOL UndoWanted, SendMove(), RestoringGame;
extern char * PeerUserName;

/* main.c */

#ifdef FILE
extern FILE * RestoreFile;
#endif
extern int errno, TranscriptType;
extern char * TranscriptFileName, * SaveFileName, * PlayerName[];
extern BOOL SetupMode, IsMachine[2];
extern struct passwd * UserPWEntry;

/* msgsw.c */

extern void InitMsgSW(), Message(), ClearMessage(), WhoseMoveMessage();

/* rpcsw.c */

extern void AddRPCSubwindow(), DeleteRPCSubwindow();

/* select.c */

extern void SelectAll();

/* talksw.c */

extern void RecvTalkMsg(), InitTalkSW();

/* tool.c */

/* the following is a kludge, but hides enormous organizational problems
 * with Sun's headers */
#ifdef TOOL_NULL
extern Tool * NchessTool;
#endif
extern void RunTool(), InitTool(), ParseToolArgs();

/* xdr.c */

extern int XdrGameReq(), XdrMove(), XdrString(), XdrSetup();

/* undeclared system calls and library fxns */

extern int getpid();
extern long random();
extern char * malloc();
