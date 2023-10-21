/*
 * Interface to transformation matrices.
 */

#ifndef transformer_h
#define transformer_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

class Transformer : public Resource {
    float mat00, mat01, mat10, mat11, mat20, mat21;

    float Det(Transformer* t);
public:
    Transformer(Transformer* t =nil);	    // returns identity if t == nil
    Transformer(
	float a00, float a01, float a10, float a11, float a20, float a21
    );
    void GetEntries(
	float& a00, float& a01, float& a10, float& a11, float& a20, float& a21
    );
    void Premultiply(Transformer* t);
    void Postmultiply(Transformer* t);
    void Invert();

    void Translate(float dx, float dy);
    void Scale(float sx, float sy);
    void Rotate(float angle);
    boolean Translated () { return mat20 != 0 || mat21 != 0; }
    boolean Scaled () { return mat00 != 1 || mat11 != 1; }
    boolean Stretched () { return mat00 != mat11; }
    boolean Rotated () { return mat01 != 0 || mat10 != 0; }
    boolean Rotated90 () { return Rotated() && mat00 == 0 && mat11 == 0; }

    void Transform(Coord& x, Coord& y);
    void Transform(Coord x, Coord y, Coord& tx, Coord& ty);
    void Transform(float x, float y, float& tx, float& ty);
    void TransformList(Coord x[], Coord y[], int n, Coord tx[], Coord ty[]);
    void InvTransform(Coord& tx, Coord& ty);
    void InvTransform(Coord tx, Coord ty, Coord& x, Coord& y);
    void InvTransform(float tx, float ty, float& x, float& y);
    void InvTransformList(Coord tx[], Coord ty[], int n, Coord x[], Coord y[]);
};

inline float Transformer.Det (Transformer *t) {
    return t->mat00*t->mat11 - t->mat01*t->mat10;
}

#endif
