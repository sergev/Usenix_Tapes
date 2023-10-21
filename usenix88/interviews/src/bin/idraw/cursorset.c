/* idraw cursor definitions */

#include <InterViews/cursor.h>
#include <InterViews/paint.h>

static const int NUMIVCURSORS = 9;

static CursorPattern iv0 = {
    0x0000, 0x7FFC, 0x47C4, 0x4FE4, 0x57D4, 0x7BBC, 0x6D6C, 0x06C0, 
    0x6D6C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv1 = {
    0x0000, 0x7EFC, 0x46FC, 0x4C7C, 0x56FC, 0x7BFC, 0x6DEC, 0x06C0, 
    0x6D6C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv2 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x6D7C, 0x06FC, 
    0x6D7C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv3 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x6D6C, 0x06C0, 
    0x6DEC, 0x7BFC, 0x56FC, 0x4C7C, 0x46FC, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv4 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x6D6C, 0x06C0, 
    0x6D6C, 0x7BBC, 0x57D4, 0x4FE4, 0x47C4, 0x7FFC, 0x0000, 0x0000, 
};

static CursorPattern iv5 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x6D6C, 0x06C0, 
    0x6F6C, 0x7FBC, 0x7ED4, 0x7C64, 0x7EC4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv6 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x7D6C, 0x7EC0, 
    0x7D6C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv7 = {
    0x0000, 0x7EFC, 0x7EC4, 0x7C64, 0x7ED4, 0x7FBC, 0x6F6C, 0x06C0, 
    0x6D6C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern iv8 = {
    0x0000, 0x7EFC, 0x46C4, 0x4C64, 0x56D4, 0x7BBC, 0x6D6C, 0x06C0, 
    0x6D6C, 0x7BBC, 0x56D4, 0x4C64, 0x46C4, 0x7EFC, 0x0000, 0x0000, 
};

static CursorPattern ivmask = {
    0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 
    0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0x0000 
};

static Cursor *IVCursor [NUMIVCURSORS];

void MakeIVCursors () {
    IVCursor[0] = new Cursor (7, 7, iv0, ivmask, black, white);
    IVCursor[1] = new Cursor (7, 7, iv1, ivmask, black, white);
    IVCursor[2] = new Cursor (7, 7, iv2, ivmask, black, white);
    IVCursor[3] = new Cursor (7, 7, iv3, ivmask, black, white);
    IVCursor[4] = new Cursor (7, 7, iv4, ivmask, black, white);
    IVCursor[5] = new Cursor (7, 7, iv5, ivmask, black, white);
    IVCursor[6] = new Cursor (7, 7, iv6, ivmask, black, white);
    IVCursor[7] = new Cursor (7, 7, iv7, ivmask, black, white);
    IVCursor[8] = new Cursor (7, 7, iv8, ivmask, black, white);
}

int NumIVCursors () {
    return NUMIVCURSORS;
}

Cursor *GetIVCursor (int i) {
    return IVCursor[i];
}

void cursorset__init() {}