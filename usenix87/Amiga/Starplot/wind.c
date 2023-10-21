#include <intuition/intuition.h>
#include <stdio.h>

struct GfxBase *GfxBase;
struct RastPort *rp;
extern struct IntuitionBase *IntuitionBase;

struct Screen *myscreen,*OpenScreen();
struct NewScreen newscr = {
    0,0,640,200,4,
    0,1,
    HIRES,
    CUSTOMSCREEN,
    NULL,
    "Stars' Screen!",
    NULL,NULL
};

struct Window *mywindow,*OpenWindow();
struct NewWindow newwind = {
    0,0,640,200,
    1,2,
    CLOSEWINDOW | NEWSIZE /* |  REFRESHWINDOW */,
    WINDOWSIZING | WINDOWDEPTH | WINDOWCLOSE | WINDOWDRAG |
    ACTIVATE,
    NULL, NULL,
    "Stars! (c) Darrin West.",
    NULL, /* changed later */
    NULL,
    10,10,640,200,
    CUSTOMSCREEN
};

unsigned short colours[] = {
    0, 0x55f, 0x338, 0x224,	/* blue */
       0xfff, 0x888, 0x444,	/* white */
       0xff5, 0x883, 0x442,	/* yellow */
       0xf55, 0x833, 0x422,	/* red */
    0x0f0, 0x0f0, 0x0f0		/*junk */
};
    
int MAX_X,MAX_Y;
int rewrite = 0;

int window_init()
{
    GfxBase = (struct GFxBase *) OpenLibrary("graphics.library",0);
    if (GfxBase == NULL) {
        printf("Can't find graphics.library\n");
	return(0);
    }
    
    IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",0);
    if (IntuitionBase == NULL){
        printf("Can't find intuition.library\n");
	return(0);
    }
    
    myscreen = OpenScreen(&newscr);
    if (myscreen == NULL){
        printf("Can't open screen\n");
	return(0);
    }
    
    myscreen->ViewPort.ColorMap = GetColorMap(16);
    LoadRGB4(&(myscreen->ViewPort),colours,16);
    
    newwind.Screen = myscreen;
    
    mywindow = OpenWindow(&newwind);
    if (mywindow == NULL){
        printf("Can't open window\n");
	return(0);
    }
    MAX_X = mywindow->Width;
    MAX_Y = mywindow->Height;
    rp = mywindow->RPort;
}

window_deinit()
{
    CloseWindow(mywindow);
    CloseScreen(myscreen);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

check_input()
{
    int class,code;
    struct IntuiMessage *message;

	while (message = GetMsg(mywindow->UserPort)){
	    class = message->Class;
	    code = message->Code;
	    ReplyMsg(message);
	    
	    switch(class) {
		case CLOSEWINDOW:
		    printf("close\n");
		    fflush(stdout);
		    window_deinit();
		    exit(0);
		    break;
		case NEWSIZE:
		    MAX_X = mywindow->Width;
		    MAX_Y = mywindow->Height;
		    printf("size: %d %d\n",MAX_X,MAX_Y);
		    fflush(stdout);
		    rewrite = 1;
		    break;
		default:
		    printf("other\n");
		    fflush(stdout);
		    break;
	    }
	}
}

wt()
{
    Wait(1<<mywindow->UserPort->mp_SigBit);
}

point(x,y,cl)
int x,y,cl;
{
    SetAPen(rp,cl);
    WritePixel(rp,x,y);
}

wind_clear()
{
    SetRast(rp,0);
}
