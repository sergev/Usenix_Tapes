/*
 * Implementation of transformation matrix operations.
 */

#include <InterViews/transformer.h>
#include <math.h>

static const float RADPERDEG = PI/180.0;

Transformer::Transformer (Transformer* t) {
    if (t == nil) {
	mat00 = mat11 = 1;			// identity
	mat01 = mat10 = mat20 = mat21 = 0;
    } else {
	mat00 = t->mat00;	mat01 = t->mat01;
	mat10 = t->mat10;	mat11 = t->mat11;
	mat20 = t->mat20;	mat21 = t->mat21;
    }
}

Transformer::Transformer (
    float a00, float a01, float a10, float a11, float a20, float a21
) {
    mat00 = a00;	mat01 = a01;
    mat10 = a10;	mat11 = a11;
    mat20 = a20;	mat21 = a21;
}

void Transformer::GetEntries (
    float& a00, float& a01, float& a10, float& a11, float& a20, float& a21
) {
    a00 = mat00;
    a01 = mat01;
    a10 = mat10;
    a11 = mat11;
    a20 = mat20;
    a21 = mat21;
}

void Transformer::Translate (float dx, float dy) {
    mat20 += dx;
    mat21 += dy;
}

void Transformer::Scale (float sx, float sy) {
    mat00 *= sx;
    mat01 *= sy;
    mat10 *= sx;
    mat11 *= sy;
    mat20 *= sx;
    mat21 *= sy;
}

void Transformer::Rotate (float angle) {
    float tmp1, tmp2, m00, m01, m10, m11, m20, m21;

    angle *= RADPERDEG;
    tmp1 = cos(angle);
    tmp2 = sin(angle);
    
    m00 = mat00*tmp1;
    m01 = mat01*tmp2;
    m10 = mat10*tmp1;
    m11 = mat11*tmp2;
    m20 = mat20*tmp1;
    m21 = mat21*tmp2;

    mat01 = mat00*tmp2 + mat01*tmp1;
    mat11 = mat10*tmp2 + mat11*tmp1;
    mat21 = mat20*tmp2 + mat21*tmp1;
    mat00 = m00 - m01;
    mat10 = m10 - m11;
    mat20 = m20 - m21;
}

void Transformer::Premultiply (Transformer* t) {
    float tmp1 = mat00;
    float tmp2 = mat10;

    mat00  = t->mat00*tmp1 + t->mat01*tmp2;
    mat10  = t->mat10*tmp1 + t->mat11*tmp2;
    mat20 += t->mat20*tmp1 + t->mat21*tmp2;

    tmp1 = mat01;
    tmp2 = mat11;

    mat01  = t->mat00*tmp1 + t->mat01*tmp2;
    mat11  = t->mat10*tmp1 + t->mat11*tmp2;
    mat21 += t->mat20*tmp1 + t->mat21*tmp2;
}

void Transformer::Postmultiply (Transformer* t) {
    float tmp = mat00*t->mat01 + mat01*t->mat11;
    mat00 = mat00*t->mat00 + mat01*t->mat10;
    mat01 = tmp;

    tmp = mat10*t->mat01 + mat11*t->mat11;
    mat10 = mat10*t->mat00 + mat11*t->mat10;
    mat11 = tmp;

    tmp = mat20*t->mat01 + mat21*t->mat11;
    mat20 = mat20*t->mat00 + mat21*t->mat10;
    mat21 = tmp;

    mat20 += t->mat20;
    mat21 += t->mat21;
}    

void Transformer::Invert () {
    float d = Det(this);
    float a = mat00;

    mat20 = (mat10*mat21 - mat11*mat20)/d;
    mat21 = (mat01*mat20 - mat00*mat21)/d;
    mat00 = mat11/d;
    mat11 = a/d;
    mat10 = -mat10/d;
    mat01 = -mat01/d;
}

void Transformer::Transform (Coord& x, Coord& y) {
    Coord tx = x;
    x = round(tx*mat00 + y*mat10 + mat20);
    y = round(tx*mat01 + y*mat11 + mat21);
}

void Transformer::Transform (Coord x, Coord y, Coord& tx, Coord& ty) {
    tx = round(x*mat00 + y*mat10 + mat20);
    ty = round(x*mat01 + y*mat11 + mat21);
}

void Transformer::Transform (float x, float y, float& tx, float& ty) {
    tx = x*mat00 + y*mat10 + mat20;
    ty = x*mat01 + y*mat11 + mat21;
}

void Transformer::InvTransform (Coord& tx, Coord& ty) {
    float d = float(tx) - mat20;
    float fty = float(ty);

    fty = (mat00*(fty - mat21) - mat01*d) / Det(this);
    tx = round((d - fty*mat10) / mat00);
    ty = round(fty);
}

void Transformer::InvTransform (Coord tx, Coord ty, Coord& x, Coord& y) {
    float d = float(tx) - mat20;
    float fy = float(ty);

    fy = (mat00*(fy - mat21) - mat01*d) / Det(this);
    x = round((d - y*mat10) / mat00);
    y = round(fy);
}

void Transformer::InvTransform (float tx, float ty, float& x, float& y) {
    float d = tx - mat20;

    y = (mat00*(ty - mat21) - mat01*d) / Det(this);
    x = (d - y*mat10) / mat00;
}    

void Transformer::TransformList (
    Coord x[], Coord y[], int n, Coord tx[], Coord ty[]
) {
    register Coord* ox, * oy, * nx, * ny;
    Coord* lim;

    lim = &x[n];
    for (ox = x, oy = y, nx = tx, ny = ty; ox < lim; ox++, oy++, nx++, ny++) {
	Transform(*ox, *oy, *nx, *ny);
    }
}

void Transformer::InvTransformList (
    Coord x[], Coord y[], int n, Coord tx[], Coord ty[]
) {
    register Coord* ox, * oy, * nx, * ny;
    Coord* lim;

    lim = &x[n];
    for (ox = x, oy = y, nx = tx, ny = ty; ox < lim; ox++, oy++, nx++, ny++) {
	InvTransform(*ox, *oy, *nx, *ny);
    }
}
