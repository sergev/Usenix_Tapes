/***************************************************************/
3dsubrs.c
/***************************************************************/
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>

#include <graphics/layers.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include "threed.h"

extern UBYTE title[] ;

extern struct Custom custom;

extern struct TmpRas tmpras;

extern struct BitMap bitmap0;
extern struct BitMap bitmap1;

extern struct RastPort r[2];
extern struct RastPort *rp[2];

extern struct RasInfo ri[2];
extern struct RasInfo *rip[2];

extern struct RasInfo *irip;

extern WORD pcount ;
extern WORD vcount ;

extern UWORD frametoggle ;

extern struct Objectinfo *objectinfo ;
extern struct Objectinfo *firstobjectinfo ;
extern struct Objectinfo *cameraobjectinfo ;

extern BPTR objectsegment ;

extern struct Object *Amiga ;

extern struct UV *cameramatrix ;
extern struct Coordinate *cameraposition ;

extern long GfxBase;
extern long DosBase;

int nullproc();
int addvect();
int subvect();
int roll();
int pitch();
int yaw();
int transpose();

int (*subroutines[])() = 
{
    nullproc,
    addvect,
    subvect,
    roll,
    pitch,
    yaw,
    transpose,
};


/*****************************************************************************/

nullproc()
{
    return(FALSE);
}

WORD mul3d(a,b)
WORD a,b;
{
LONG c;

    c = a * b;
    c += 0x2000;
    c >>= 14;

    return((WORD)c);
}

roll(bm,sine,cosine)
struct UV *bm;
WORD sine;
WORD cosine;
{
    struct UV tmp;

    tmp.uv11 = (WORD)(mul3d(bm->uv11,cosine)+mul3d(bm->uv21,sine));
    tmp.uv21 = (WORD)(mul3d(-bm->uv11,sine)+mul3d(bm->uv21,cosine));
    tmp.uv12 = (WORD)(mul3d(bm->uv12,cosine)+mul3d(bm->uv22,sine));
    tmp.uv22 = (WORD)(mul3d(-bm->uv12,sine)+mul3d(bm->uv22,cosine));
    tmp.uv13 = (WORD)(mul3d(bm->uv13,cosine)+mul3d(bm->uv23,sine));
    tmp.uv23 = (WORD)(mul3d(-bm->uv13,sine)+mul3d(bm->uv23,cosine));

    bm->uv11 = tmp.uv11;
    bm->uv21 = tmp.uv21;
    bm->uv12 = tmp.uv12;
    bm->uv22 = tmp.uv22;
    bm->uv13 = tmp.uv13;
    bm->uv23 = tmp.uv23;

}

yaw(bm,sine,cosine)
struct UV *bm;
WORD sine;
WORD cosine;
{
    struct UV tmp;

    tmp.uv11 = (WORD)(mul3d(bm->uv11,cosine)+mul3d(bm->uv31,sine));
    tmp.uv31 = (WORD)(mul3d(-bm->uv11,sine)+mul3d(bm->uv31,cosine));
    tmp.uv12 = (WORD)(mul3d(bm->uv12,cosine)+mul3d(bm->uv32,sine));
    tmp.uv32 = (WORD)(mul3d(-bm->uv12,sine)+mul3d(bm->uv32,cosine));
    tmp.uv13 = (WORD)(mul3d(bm->uv13,cosine)+mul3d(bm->uv33,sine));
    tmp.uv33 = (WORD)(mul3d(-bm->uv13,sine)+mul3d(bm->uv33,cosine));

    bm->uv11 = tmp.uv11;
    bm->uv31 = tmp.uv31;
    bm->uv12 = tmp.uv12;
    bm->uv32 = tmp.uv32;
    bm->uv13 = tmp.uv13;
    bm->uv33 = tmp.uv33;

}

pitch(bm,sine,cosine)
struct UV *bm;
WORD sine;
WORD cosine;
{
    struct UV tmp;

    tmp.uv21 = (WORD)(mul3d(bm->uv21,cosine)-mul3d(bm->uv31,sine));
    tmp.uv31 = (WORD)(mul3d(bm->uv21,sine)+mul3d(bm->uv31,cosine));
    tmp.uv22 = (WORD)(mul3d(bm->uv22,cosine)-mul3d(bm->uv32,sine));
    tmp.uv32 = (WORD)(mul3d(bm->uv22,sine)+mul3d(bm->uv32,cosine));
    tmp.uv23 = (WORD)(mul3d(bm->uv23,cosine)-mul3d(bm->uv33,sine));
    tmp.uv33 = (WORD)(mul3d(bm->uv23,sine)+mul3d(bm->uv33,cosine));

    bm->uv21 = tmp.uv21;
    bm->uv31 = tmp.uv31;
    bm->uv22 = tmp.uv22;
    bm->uv32 = tmp.uv32;
    bm->uv23 = tmp.uv23;
    bm->uv33 = tmp.uv33;

}

transform(dest)
struct Coordinate *dest;
{
    LONG zinv = 0x00400000;

    dest->z = (dest->z<64) ? 64 : dest->z ;

    /* dest->x = (WORD)( ((long)dest->x << 8) / dest->z ); */ 
    /* dest->y = (WORD)( ((long)dest->y << 8) / dest->z ); */

    /* new algorithm - figure inverse of z and multiply */

    zinv /= dest->z;

     dest->x = mul3d(dest->x,(WORD)zinv); 
     dest->y = mul3d(dest->y,(WORD)zinv); 


#ifdef DEBUG
    printf("transform: dest->x = %lx\n",dest->x);
    printf("transform: dest->y = %lx\n",dest->y);
    printf("transform: dest->z = %lx\n",dest->z);
#endif

}

perspect(objectnumpoints,objectbufpoints)
WORD objectnumpoints;
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;
    struct Coordinate *nextpoint;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("perspect: pointcount = %lx\n",pointcount);
#endif
   transform(&objectbufpoints[pointcount]);
    }
}

subvect(bp,src,dest)
struct Coordinate *bp;
struct Coordinate *src;
struct Coordinate *dest;
{

#ifdef DEBUG
    printf("subvect: src->x = %lx\n",src->x);
    printf("subvect: src->y = %lx\n",src->y);
    printf("subvect: src->z = %lx\n",src->z);
#endif

    dest->x = (WORD)(src->x - bp->x);
    dest->y = (WORD)(src->y - bp->y);
    dest->z = (WORD)(src->z - bp->z);

#ifdef DEBUG
    printf("subvect: dest->x = %lx\n",dest->x);
    printf("subvect: dest->y = %lx\n",dest->y);
    printf("subvect: dest->z = %lx\n",dest->z);
#endif

}

addvect(bp,src,dest)
struct Coordinate *bp;
struct Coordinate *src;
struct Coordinate *dest;
{

#ifdef DEBUG
    printf("addvect: src->x = %lx\n",src->x);
    printf("addvect: src->y = %lx\n",src->y);
    printf("addvect: src->z = %lx\n",src->z);
    printf("addvect: bp->x = %lx\n",bp->x);
    printf("addvect: bp->y = %lx\n",bp->y);
    printf("addvect: bp->z = %lx\n",bp->z);
#endif

    dest->x = (WORD)(src->x + bp->x);
    dest->y = (WORD)(src->y + bp->y);
    dest->z = (WORD)(src->z + bp->z);

#ifdef DEBUG
    printf("addvect: dest->x = %lx\n",dest->x);
    printf("addvect: dest->y = %lx\n",dest->y);
    printf("addvect: dest->z = %lx\n",dest->z);
#endif

}

translate(bp,objectnumpoints,objectbufpoints)
struct Coordinate *bp;
WORD objectnumpoints;
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("translate: pointcount = %lx\n",pointcount);
#endif
   addvect(bp,&objectbufpoints[pointcount],&objectbufpoints[pointcount]);
    }
}

transpose(bm)
struct UV *bm;
{
    WORD tmp;

    tmp = bm->uv12; bm->uv12 = bm->uv21; bm->uv21 = tmp;
    tmp = bm->uv13; bm->uv13 = bm->uv31; bm->uv31 = tmp;
    tmp = bm->uv23; bm->uv23 = bm->uv32; bm->uv32 = tmp;
}

cat(dest,src1,src2)
struct UV *dest;
struct UV *src1;
struct UV *src2;
{
   matrix(&dest->uv11,&src1->uv11,src2);
   matrix(&dest->uv21,&src1->uv21,src2);
   matrix(&dest->uv31,&src1->uv31,src2);
}

matrix(dest,src,bm)
struct Coordinate *dest;
struct Coordinate *src;
struct UV *bm;
{

#ifdef DEBUG
    printf("matrix: src->x = %lx\n",src->x);
    printf("matrix: src->y = %lx\n",src->y);
    printf("matrix: src->z = %lx\n",src->z);
#endif

    dest->x = (WORD)(mul3d(src->x,bm->uv11)+mul3d(src->y,bm->uv12)+mul3d(src->z,bm->uv13));
    dest->y = (WORD)(mul3d(src->x,bm->uv21)+mul3d(src->y,bm->uv22)+mul3d(src->z,bm->uv23));
    dest->z = (WORD)(mul3d(src->x,bm->uv31)+mul3d(src->y,bm->uv32)+mul3d(src->z,bm->uv33));

#ifdef DEBUG
    printf("matrix: dest->x = %lx\n",dest->x);
    printf("matrix: dest->y = %lx\n",dest->y);
    printf("matrix: dest->z = %lx\n",dest->z);
#endif

}

rotate(bm,objectnumpoints,pointstart,objectbufpoints)
struct UV *bm;
WORD objectnumpoints;
struct Coordinate *pointstart[];
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("rotate: pointcount = %lx\n",pointcount);
#endif
   matrix(&objectbufpoints[pointcount],pointstart[pointcount],bm);
    }
}

copynormals(objectnumpoints,pointstart,objectbufpoints)
WORD objectnumpoints;
struct Coordinate *pointstart[];
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("copynormals: pointcount = %lx\n",pointcount);
#endif
   objectbufpoints[pointcount] = *(pointstart[pointcount]);
    }

}

camera(bm,bp,objectnumpoints,srcbufpoints,destbufpoints)
struct UV *bm;
struct Coordinate *bp;
WORD objectnumpoints;
struct Coordinate srcbufpoints[];
struct Coordinate destbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("camera: pointcount = %lx\n",pointcount);
#endif
   subvect(bp,&destbufpoints[pointcount],&srcbufpoints[pointcount]);
   matrix(&destbufpoints[pointcount],&srcbufpoints[pointcount],bm);
   transform(&destbufpoints[pointcount]);
    }
}

notransformdopoints(bm,bp,objectnumpoints,pointstart,objectbufpoints)
struct UV *bm;
struct Coordinate *bp;
WORD objectnumpoints;
struct Coordinate *pointstart[];
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("notransformdopoints: pointcount = %lx\n",pointcount);
#endif
   matrix(&objectbufpoints[pointcount],pointstart[pointcount],bm);
   addvect(bp,&objectbufpoints[pointcount],&objectbufpoints[pointcount]);
    }
}

dopoints(bm,bp,objectnumpoints,pointstart,objectbufpoints)
struct UV *bm;
struct Coordinate *bp;
WORD objectnumpoints;
struct Coordinate *pointstart[];
struct Coordinate objectbufpoints[];
{
    WORD pointcount = 0;

    for(pointcount = 0; pointcount < objectnumpoints; pointcount++)
    {
#ifdef DEBUG
   printf("dopoints: pointcount = %lx\n",pointcount);
#endif
   matrix(&objectbufpoints[pointcount],pointstart[pointcount],bm);
   addvect(bp,&objectbufpoints[pointcount],&objectbufpoints[pointcount]);
   transform(&objectbufpoints[pointcount]);
    }
}

/***************************************************************/
amiga.c
/***************************************************************/
#include <exec/types.h>
#include "threed.h"

extern struct Object Universe;
extern struct UV universematrix;
extern struct Coordinate universeposition;

extern int universeprocedure();

extern struct Object AmigaObject;
extern struct UV amigamatrix;
extern struct Coordinate amigaposition;
extern struct Coordinate *amigapoints[];
extern struct Coordinate *amiganormals[];
extern struct Polygon *amigapolygons[];

extern int amigaprocedure();

struct Object Universe =
{
    NULL,
    &AmigaObject,
    &universematrix,
    &universeposition,
    0,NULL,
    0,NULL,
    0,NULL,
    NULL
};

struct Object *UniverseObject = &Universe;

struct UV universematrix =
{
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000
};

struct Coordinate universeposition =
{
    0,
    0,
    0
};

struct Object AmigaObject =
{
    NULL,
    NULL,
    &amigamatrix,
    &amigaposition,
    54,amigapoints,
    8,amiganormals,
    11,amigapolygons,
    amigaprocedure
};


struct UV amigamatrix =
{
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000
};


struct Coordinate amigaposition =
{
    0xfff8,
    0xffe8,
    0xfc60
};

struct Coordinate p0 =
{
    0xfef8,
    0xffd0,
    0xffe8
};

struct Coordinate p1 =
{
    0xfef8,
    0x0048,
    0xffe8
};

struct Coordinate p2 =
{
    0x0120,
    0x0048,
    0xffe8
};

struct Coordinate p3 =
{
    0x0120,
    0xffd0,
    0xffe8
};

struct Coordinate p4 =
{
    0xfef8,
    0xffd0,
    0x0018
};

struct Coordinate p5 =
{
    0xfef8,
    0x0048,
    0x0018
};

struct Coordinate p6 =
{
    0x0120,
    0x0048,
    0x0018
};

struct Coordinate p7 =
{
    0x0120,
    0xffd0,
    0x0018
};

struct Coordinate p8 =
{
    0xff40,
    0xffe8,
    0xffc8
};

struct Coordinate p9 =
{
    0xff30,
    0x0000,
    0xffc8
};

struct Coordinate p10 =
{
    0xff08,
    0x0000,
    0xffc8
};

struct Coordinate p11 =
{
    0xff20,
    0x0018,
    0xffc8
};

struct Coordinate p12 =
{
    0xff10,
    0x0030,
    0xffc8
};

struct Coordinate p13 =
{
    0xff30,
    0x0030,
    0xffc8
};

struct Coordinate p14 =
{
    0xff40,
    0x0018,
    0xffc8
};

struct Coordinate p15 =
{
    0xff50,
    0x0030,
    0xffc8
};

struct Coordinate p16 =
{
    0xff70,
    0x0030,
    0xffc8
};

struct Coordinate p17 =
{
    0xff88,
    0xffe8,
    0xffc8
};

struct Coordinate p18 =
{
    0xff88,
    0x0030,
    0xffc8
};

struct Coordinate p19 =
{
    0xffa8,
    0x0030,
    0xffc8
};

struct Coordinate p20 =
{
    0xffa8,
    0x0018,
    0xffc8
};

struct Coordinate p21 =
{
    0xffb8,
    0x0020,
    0xffc8
};

struct Coordinate p22 =
{
    0xffc8,
    0x0018,
    0xffc8
};

struct Coordinate p23 =
{
    0xffc8,
    0x0030,
    0xffc8
};

struct Coordinate p24 =
{
    0xffe8,
    0x0030,
    0xffc8
};

struct Coordinate p25 =
{
    0xffe8,
    0xffe8,
    0xffc8
};

struct Coordinate p26 =
{
    0xffb8,
    0x0000,
    0xffc8
};

struct Coordinate p27 =
{
    0x0000,
    0x0000,
    0xffc8
};

struct Coordinate p28 =
{
    0x0000,
    0x0030,
    0xffc8
};

struct Coordinate p29 =
{
    0x0018,
    0x0030,
    0xffc8
};

struct Coordinate p30 =
{
    0x0018,
    0xffe8,
    0xffc8
};

struct Coordinate p31 =
{
    0x0048,
    0xffe8,
    0xffc8
};

struct Coordinate p32 =
{
    0x0030,
    0x0000,
    0xffc8
};

struct Coordinate p33 =
{
    0x0030,
    0x0018,
    0xffc8
};

struct Coordinate p34 =
{
    0x0048,
    0x0030,
    0xffc8
};

struct Coordinate p35 =
{
    0x0078,
    0x0030,
    0xffc8
};

struct Coordinate p36 =
{
    0x0088,
    0x0018,
    0xffc8
};

struct Coordinate p37 =
{
    0x00a0,
    0x0018,
    0xffc8
};

struct Coordinate p38 =
{
    0x0090,
    0x0008,
    0xffc8
};

struct Coordinate p39 =
{
    0x0068,
    0x0008,
    0xffc8
};

struct Coordinate p40 =
{
    0x0078,
    0x0018,
    0xffc8
};

struct Coordinate p41 =
{
    0x0060,
    0x0018,
    0xffc8
};

struct Coordinate p42 =
{
    0x0048,
    0x0000,
    0xffc8
};

struct Coordinate p43 =
{
    0x0060,
    0x0000,
    0xffc8
};

struct Coordinate p44 =
{
    0x0078,
    0xffe8,
    0xffc8
};

struct Coordinate p45 =
{
    0x00d8,
    0xffe8,
    0xffc8
};

struct Coordinate p46 =
{
    0x00c8,
    0x0000,
    0xffc8
};

struct Coordinate p47 =
{
    0x00a0,
    0x0000,
    0xffc8
};

struct Coordinate p48 =
{
    0x00b8,
    0x0018,
    0xffc8
};

struct Coordinate p49 =
{
    0x00a8,
    0x0030,
    0xffc8
};

struct Coordinate p50 =
{
    0x00c8,
    0x0030,
    0xffc8
};

struct Coordinate p51 =
{
    0x00d8,
    0x0010,
    0xffc8
};

struct Coordinate p52 =
{
    0x00e8,
    0x0030,
    0xffc8
};

struct Coordinate p53 =
{
    0x0108,
    0x0030,
    0xffc8
};

struct Coordinate *amigapoints[54] =
{
    &p0,
    &p1,
    &p2,
    &p3,
    &p4,
    &p5,
    &p6,
    &p7,
    &p8,
    &p9,
    &p10,
    &p11,
    &p12,
    &p13,
    &p14,
    &p15,
    &p16,
    &p17,
    &p18,
    &p19,
    &p20,
    &p21,
    &p22,
    &p23,
    &p24,
    &p25,
    &p26,
    &p27,
    &p28,
    &p29,
    &p30,
    &p31,
    &p32,
    &p33,
    &p34,
    &p35,
    &p36,
    &p37,
    &p38,
    &p39,
    &p40,
    &p41,
    &p42,
    &p43,
    &p44,
    &p45,
    &p46,
    &p47,
    &p48,
    &p49,
    &p50,
    &p51,
    &p52,
    &p53
};

struct Coordinate n0 =
{
    0x0000,
    0x0000,
    0xc000
};

struct Coordinate n1 =
{
    0x4000,
    0x0000,
    0x0000
};

struct Coordinate n2 =
{
    0x0000,
    0x0000,
    0x4000
};

struct Coordinate n3 =
{
    0xc000,
    0x0000,
    0x0000
};

struct Coordinate n4 =
{
    0x0000,
    0xc000,
    0x0000
};

struct Coordinate n5 =
{
    0x0000,
    0x4000,
    0x0000
};

struct Coordinate n6 =
{
    0x0000,
    0x0000,
    0xc000
};

struct Coordinate n7 =
{
    0x0000,
    0x0000,
    0xc000
};

struct Coordinate *amiganormals[8] =
{
    &n0,
    &n1,
    &n2,
    &n3,
    &n4,
    &n5,
    &n6,
    &n7
};

struct Coordinate *v0[4] =
{
    &p0,
    &p1,
    &p2,
    &p3
};

struct Coordinate *v1[4] =
{
    &p3,
    &p2,
    &p6,
    &p7
};

struct Coordinate *v2[4] =
{
    &p7,
    &p6,
    &p5,
    &p4
};

struct Coordinate *v3[4] =
{
    &p4,
    &p5,
    &p1,
    &p0
};

struct Coordinate *v4[4] =
{
    &p4,
    &p0,
    &p3,
    &p7
};

struct Coordinate *v5[4] =
{
    &p2,
    &p1,
    &p5,
    &p6
};

struct Coordinate *v6[9] =
{
    &p9,
    &p10,
    &p11,
    &p12,
    &p13,
    &p14,
    &p15,
    &p16,
    &p8
};

struct Coordinate *v7[10] =
{
    &p17,
    &p18,
    &p19,
    &p20,
    &p21,
    &p22,
    &p23,
    &p24,
    &p25,
    &p26
};

struct Coordinate *v8[4] =
{
    &p27,
    &p28,
    &p29,
    &p30
};

struct Coordinate *v9[14] =
{
    &p31,
    &p32,
    &p33,
    &p34,
    &p35,
    &p36,
    &p37,
    &p38,
    &p39,
    &p40,
    &p41,
    &p42,
    &p43,
    &p44
};

struct Coordinate *v10[9] =
{
    &p46,
    &p47,
    &p48,
    &p49,
    &p50,
    &p51,
    &p52,
    &p53,
    &p45
};

struct Polygon f0 =
{
    4,
    v0,
    &n7,
    7
};

struct Polygon f1 =
{
    4,
    v1,
    &n1,
    1
};

struct Polygon f2 =
{
    4,
    v2,
    &n2,
    2
};

struct Polygon f3 =
{
    4,
    v3,
    &n3,
    3
};

struct Polygon f4 =
{
    4,
    v4,
    &n4,
    4
};

struct Polygon f5 =
{
    4,
    v5,
    &n5,
    5
};

struct Polygon f6 =
{
    9,
    v6,
    &n6,
    6
};

struct Polygon f7 =
{
    10,
    v7,
    &n6,
    6
};

struct Polygon f8 =
{
    4,
    v8,
    &n6,
    6
};

struct Polygon f9 =
{
    14,
    v9,
    &n6,
    6
};

struct Polygon f10 =
{
    9,
    v10,
    &n6,
    6
};

struct Polygon *amigapolygons[11] =
{
    &f0,
    &f1,
    &f2,
    &f3,
    &f4,
    &f5,
    &f6,
    &f7,
    &f8,
    &f9,
    &f10
};

/***************************************************************/
amigaprocedures.c
/***************************************************************/
#include <exec/types.h>
#include "threed.h"

/* #define PROCDEBUG */

universeprocedure(argc,argv)
int argc;
char *argv[];
{
    static int error;
    static int count;
    int i;

#ifdef PROCDEBUG
    printf("universeprocedure: executing universeprocedure()...\n");
#endif

    for(i=0; i<argc; i++)
    {

#ifdef PROCDEBUG
    printf("argv[%lx] = %lx\n",argc,argv[i]);
#endif
        ;
    }

    switch(argc)
    {
   case 0 : error = TRUE;  /* no objectinfopointer */
       return(error);
   case 1 : error = FALSE; /* no subroutines to execute */
       return(error);
   case 2 : count++;
       {
           struct Objectinfo ** oipp;
           int (**subp)();
           int (*sub)();

           oipp = *argv;
           subp = *(argv+1);
#ifdef PROCDEBUG
     printf("oipp = %lx; *oipp = %lx; **oipp = %lx\n",oipp,*oipp,**oipp);
     printf("subp = %lx; *subp = %lx; **subp = %lx\n",subp,*subp,**subp);
#endif
       }
       break; 
   default: break;
    }

    return(error);
}

amigaprocedure(argc,argv)
int argc;
char *argv[];
{
    static int error;
    static int count;
    int i;

#ifdef PROCDEBUG
    printf("amigaprocedure: executing amigaprocedure()...\n");
#endif

    for(i=0; i<argc; i++)
    {

#ifdef PROCDEBUG
    printf("argv[%lx] = %lx\n",argc,argv[i]);
#endif
        ;
    }

    switch(argc)
    {
   case 0 : error = TRUE;  /* no objectinfopointer */
       return(error);
   case 1 : error = FALSE; /* no subroutines to execute */
       return(error);
   case 2 : count++;
       {
           struct Objectinfo ** oipp;
           int (**subp)();
           int (*sub)();

           oipp = *argv;
           subp = *(argv+1);
#ifdef PROCDEBUG
     printf("oipp = %lx; *oipp = %lx; **oipp = %lx\n",oipp,*oipp,**oipp);
     printf("subp = %lx; *subp = %lx; **subp = %lx\n",subp,*subp,**subp);
#endif

           /* transpose objectmatrix */

           sub = *(subp+TRANSPOSE);

           (*sub)( (*oipp)->objectmatrix);

           /* yaw objectmatrix */
           
           sub = *(subp+YAW);

           (*sub)( (*oipp)->objectmatrix, SINA, COSA);

           /* pitch objectmatrix */

           sub = *(subp+PITCH);

           (*sub)( (*oipp)->objectmatrix, -SINB, COSB);

           /* transpose objectmatrix */

           sub = *(subp+TRANSPOSE);

           (*sub)( (*oipp)->objectmatrix);
           
       }
       break; 
   default: break;
    }

    return(error);
}

/***************************************************************/
camera.c
/***************************************************************/
#include <exec/types.h>
#include <exec/memory.h>
#include "threed.h"

extern struct Object Camera1;
extern struct UV camera1matrix;
extern struct Coordinate camera1position;

extern int camera1procedure();

struct Object Camera1 =
{
    NULL,
    NULL,
    &camera1matrix,
    &camera1position,
    0,NULL,
    0,NULL,
    0,NULL,
    NULL
};

struct UV camera1matrix =
{
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000,
    0x0000,
    0x0000,
    0x0000,
    0x4000
};

struct Coordinate camera1position =
{
    0,
    0,
    0
};

/***************************************************************/
deallocate.c
/***************************************************************/
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>

#include <graphics/layers.h>
#include <intuition/intuition.h>
#include "threed.h"

/* #define DEBUG */
/* #define ODEBUG */
/* #define DEBUGDRAW */
/* #define TICKDEBUG */
/* #define TICKPRINT */
#define FIXEDILLUMINATION
#define CYCLECOLORS

#ifndef FIXEDILLUMINATION

extern UWORD colorpalette[];

#else

extern UWORD colorpalette[];

#endif

extern UBYTE title[];

extern struct Custom custom;
 
extern struct TmpRas tmpras;

extern struct BitMap bitmap0;
extern struct BitMap bitmap1;

extern struct RastPort r[];
extern struct RastPort *rp[];

extern struct RasInfo ri[];
extern struct RasInfo *rip[];

extern struct RasInfo *irip;

extern WORD pcount;
extern WORD vcount;

extern UWORD frametoggle;

extern long GfxBase;

/******************************************************************************/

objectdeallocate(view,screen,window,objectinfo)
struct View *view;
struct Screen *screen;
struct Window *window;
struct Objectinfo *objectinfo;
{


   if (objectinfo->displaymatrix)
     FreeMem(objectinfo->displaymatrix,sizeof(struct UV));

   if (objectinfo->displayposition)
     FreeMem(objectinfo->displayposition,sizeof(struct Coordinate));

   if ((objectinfo->objectbufpoints) && (objectinfo->objectbufpointsize))
     FreeMem(objectinfo->objectbufpoints,objectinfo->objectbufpointsize);

   if ((objectinfo->objectbufnormals) && (objectinfo->objectbufnormalsize))
     FreeMem(objectinfo->objectbufnormals,objectinfo->objectbufnormalsize);

   /* deallocate buffers for pptr, ntpr, color lists */

   if ((objectinfo->pptrbuf) && (objectinfo->pptrbufsize))
     FreeMem(objectinfo->pptrbuf,objectinfo->pptrbufsize);

   if ((objectinfo->nptrbuf) && (objectinfo->nptrbufsize))
     FreeMem(objectinfo->nptrbuf,objectinfo->nptrbufsize);

   if ((objectinfo->colorbuf) && (objectinfo->colorbufsize))
     FreeMem(objectinfo->colorbuf,objectinfo->colorbufsize);

   /* deallocate all subobjects dependent on this object */

   while( objectinfo->subobjectinfo )
   {
       struct Objectinfo *soip;

       /* delink the first subobjectinfo from the objectinfo list */

       soip = objectinfo->subobjectinfo;

       objectinfo->subobjectinfo = objectinfo->subobjectinfo->nextobjectinfo;

       /* deallocate the buffers dependent on the current subobjectinfo */

#ifdef ODEBUG
       printf("    deallocate subobjectinfo(%lx)\n",soip);
#endif

       objectdeallocate(view,screen,window,soip);

       /* deallocate this subobjectinfo structure itself */

       FreeMem(soip,sizeof(struct Objectinfo));
   }

}
/***************************************************************/
debug.c
/***************************************************************/
#include <exec/types.h>

printf(a,b,c,d,e,f,g)
APTR a,b,c,d,e,f;
{
    kprintf(a,b,c,d,e,f,g);
}
/***************************************************************/
