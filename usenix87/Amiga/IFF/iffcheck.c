/*---------------------------------------------------------------------*
 * IFFCheck.C  Print out the structure of an IFF-85 file,      11/19/85
 * checking for structural errors.
 *
 * DO NOT USE THIS AS A SKELETAL PROGRAM FOR AN IFF READER!
 * See ShowILBM.C for a skeletal example.
 *
 * This version for the Commodore-Amiga computer.
 *----------------------------------------------------------------------*/
#include "iff/iff.h"


/* ---------- IFFCheck -------------------------------------------------*/
/* [TBD] More extensive checking could be done on the IDs encountered in the
 * file. Check that the reserved IDs "FOR1".."FOR9", "LIS1".."LIS9", and
 * "CAT1".."CAT9" aren't used. Check that reserved IDs aren't used as Form
 * types. Check that all IDs are made of 4 printable characters (trailing
 * spaces ok). */

typedef struct {
    ClientFrame clientFrame;
    int levels;         /* # groups currently nested within.*/
    } Frame;

char MsgOkay[] = { "----- (IFF_OKAY) A good IFF file." };
char MsgEndMark[] = {"----- (END_MARK) How did you get this message??" };
char MsgDone[] = { "----- (IFF_DONE) How did you get this message??" };
char MsgDos[] = { "----- (DOS_ERROR) The DOS gave back an error." };
char MsgNot[] = { "----- (NOT_IFF) not an IFF file." };
char MsgNoFile[] = { "----- (NO_FILE) no such file found." };
char MsgClientError[] = {"----- (CLIENT_ERROR) IFF Checker bug."};
char MsgForm[] = { "----- (BAD_FORM) How did you get this message??" };
char MsgShort[] = { "----- (SHORT_CHUNK) How did you get this message??" };
char MsgBad[] = { "----- (BAD_IFF) a mangled IFF file." };

/* MUST GET THESE IN RIGHT ORDER!!*/
char *IFFPMessages[-LAST_ERROR+1] = {
    /*IFF_OKAY*/  MsgOkay,
    /*END_MARK*/  MsgEndMark,
    /*IFF_DONE*/  MsgDone,
    /*DOS_ERROR*/ MsgDos,
    /*NOT_IFF*/   MsgNot,
    /*NO_FILE*/   MsgNoFile,
    /*CLIENT_ERROR*/ MsgClientError,
    /*BAD_FORM*/  MsgForm,
    /*SHORT_CHUNK*/  MsgShort,
    /*BAD_IFF*/   MsgBad
    };

/* FORWARD REFERENCES */
extern IFFP GetList(GroupContext *);
extern IFFP GetForm(GroupContext *);
extern IFFP GetProp(GroupContext *);
extern IFFP GetCat (GroupContext *);

void IFFCheck(name)  char *name; {
    IFFP iffp;
    BPTR file = Open(name, MODE_OLDFILE);
    Frame frame;

    frame.levels = 0;
    frame.clientFrame.getList = GetList;
    frame.clientFrame.getForm = GetForm;
    frame.clientFrame.getProp = GetProp;
    frame.clientFrame.getCat  = GetCat ;

    printf("----- Checking file '%s' -----\n", name);
    if (file == 0)
        iffp = NO_FILE;
    else
        iffp = ReadIFF(file, (ClientFrame *)&frame);

    Close(file);
    printf("%s\n", IFFPMessages[-iffp]);
    }

main(argc, argv)   int argc;  char **argv; {
    if (argc != 1+1) {
        printf("Usage: 'iffcheck filename'\n");
        exit(0);
        }
    IFFCheck(argv[1]);
    }

/* ---------- Put... ---------------------------------------------------*/

PutLevels(count)   int count; {
    for ( ;  count > 0;  --count) {
        printf(".");
        }
    }

PutID(id)  ID id; {
    printf("%c%c%c%c", (id>>24)&0x7f, (id>>16)&0x7f, (id>>8)&0x7f, id&0x7f);
    }

PutN(n)   int n; {
    printf(" %d ", n);
    }

/* Put something like "...BMHD 14" or "...LIST 14 PLBM". */
PutHdr(context)  GroupContext *context; {
    PutLevels( ((Frame *)context->clientFrame)->levels );
    PutID(context->ckHdr.ckID);
    PutN(context->ckHdr.ckSize);

    if (context->subtype != NULL_CHUNK)
        PutID(context->subtype);

    printf("\n");
    }

/* ---------- AtLeaf ---------------------------------------------------*/

/* At Leaf chunk.  That is, a chunk which does NOT contain other chunks.
 * Print "ID size".*/
IFFP AtLeaf(context)  GroupContext *context; {

    PutHdr(context);
    /* A typical reader would read the chunk's contents, using the "Frame"
     * for local data, esp. shared property settings (PROP).*/
    /* IFFReadBytes(context, ...buffer, context->ckHdr->ckSize); */
    return(IFF_OKAY);
    }

/* ---------- GetList --------------------------------------------------*/
/* Handle a LIST chunk.  Print "LIST size subTypeID".
 * Then dive into it.*/
IFFP GetList(parent)  GroupContext *parent; {
    Frame newFrame;

    newFrame = *(Frame *)parent->clientFrame;  /* copy parent's frame*/
    newFrame.levels++;

    PutHdr(parent);

    return( ReadIList(parent, (ClientFrame *)&newFrame) );
    }

/* ---------- GetForm --------------------------------------------------*/
/* Handle a FORM chunk.  Print "FORM size subTypeID".
 * Then dive into it.*/
IFFP GetForm(parent)   GroupContext *parent; {
    /*CompilerBug register*/ IFFP iffp;
    GroupContext new;
    Frame newFrame;

    newFrame = *(Frame *)parent->clientFrame;  /* copy parent's frame*/
    newFrame.levels++;

    PutHdr(parent);

    iffp = OpenRGroup(parent, &new);
    CheckIFFP();
    new.clientFrame = (ClientFrame *)&newFrame;

    /* FORM reader for Checker. */
    /* LIST, FORM, PROP, CAT already handled by GetF1ChunkHdr. */
    do {if ( (iffp = GetF1ChunkHdr(&new)) > 0 )
            iffp = AtLeaf(&new);
        } while (iffp >= IFF_OKAY);

    CloseRGroup(&new);
    return(iffp == END_MARK ? IFF_OKAY : iffp);
    }

/* ---------- GetProp --------------------------------------------------*/
/* Handle a PROP chunk.  Print "PROP size subTypeID".
 * Then dive into it.*/
IFFP GetProp(listContext)  GroupContext *listContext; {
    /*CompilerBug register*/ IFFP iffp;
    GroupContext new;

    PutHdr(listContext);

    iffp = OpenRGroup(listContext, &new);
    CheckIFFP();

    /* PROP reader for Checker. */
    ((Frame *)listContext->clientFrame)->levels++;

    do {if ( (iffp = GetPChunkHdr(&new)) > 0 )
            iffp = AtLeaf(&new);
        } while (iffp >= IFF_OKAY);

    ((Frame *)listContext->clientFrame)->levels--;

    CloseRGroup(&new);
    return(iffp == END_MARK ? IFF_OKAY : iffp);
    }

/* ---------- GetCat ---------------------------------------------------*/
/* Handle a CAT chunk.  Print "CAT size subTypeID".
 * Then dive into it.*/
IFFP GetCat(parent)  GroupContext *parent;  {
    IFFP iffp;

    ((Frame *)parent->clientFrame)->levels++;

    PutHdr(parent);

    iffp = ReadICat(parent);

    ((Frame *)parent->clientFrame)->levels--;
    return(iffp);
    }

