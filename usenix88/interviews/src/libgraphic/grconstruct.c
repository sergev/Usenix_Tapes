/*
 * Implementation of GraphicConstruct object construction function.
 */

#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/grconstruct.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/lines.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/splines.h>

Persistent* GraphicConstruct (ClassId id) {
    switch (id) {
	case GRAPHIC:		return new Graphic;
	case FULL_GRAPHIC:	return new FullGraphic;
	case PICTURE:		return new Picture;
	case INSTANCE:		return new Instance;
	case ELLIPSE:		return new Ellipse;
	case FILLELLIPSE:	return new FillEllipse;
	case CIRCLE:		return new Circle;
	case FILLCIRCLE:	return new FillCircle;
	case POINT:		return new Point;
	case LINE:		return new Line;
	case MULTILINE:		return new MultiLine;
	case RECT:		return new Rect;
	case FILLRECT:		return new FillRect;
	case POLYGON:		return new Polygon;
	case FILLPOLYGON:	return new FillPolygon;
	case BSPLINE:		return new BSpline;
	case CLOSEDBSPLINE:	return new ClosedBSpline;
	case FILLBSPLINE:	return new FillBSpline;
	case LABEL:		return new Label;
	case PCOLOR:		return new PColor;
	case POINTOBJ:		return new PointObj;
	case LINEOBJ:		return new LineObj;
	case BOXOBJ:		return new BoxObj;
	case MULTILINEOBJ:	return new MultiLineObj;
	case FILLPOLYGONOBJ:	return new FillPolygonObj;
	case PPATTERN:		return new PPattern;
	case PFONT:		return new PFont;
	case PBRUSH:		return new PBrush;

	default:		return nil;
    }
}
