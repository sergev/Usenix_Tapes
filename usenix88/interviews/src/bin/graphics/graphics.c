/*
 * Persistent structured graphics demonstration program.
 */

#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/graphic.h>
#include <InterViews/rubrect.h>
#include <InterViews/scroller.h>
#include <InterViews/sensor.h>
#include <InterViews/world.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/grblock.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/lines.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/splines.h>

World* world;
Graphics* graphics;

/*
 * Graphics class used in main program.
 */
class Graphics {
    boolean alive;
    class Picture* pict;
    
    void CreateView();
public:
    Graphics();
    void Run();
    void Kill();
};

/*
 * View is a GraphicBlock with its Handle member function redefined to detect
 * hits on the Graphic objects it contains.
 */
class View : public GraphicBlock {
public:
    View(Graphic*);
    void Handle(Event&);
    void Move(Graphic*, Event&);
};

/*
 * Persistent object creation function which creates an instance of any
 * persistent object used in the application, given a ClassId.  A pointer
 * to this function is passed to the object manager constructor.
 *
 * In this program, we don't derive any new classes from Persistent, so
 * GraphicsConstruct simply calls GraphicConstruct. (We could have eliminated
 * GraphicsConstruct altogether and simply passed a pointer to GraphicConstruct
 * to the object manager constructor.)
 */
Persistent* GraphicsConstruct (ClassId id) {
    switch (id) {
	// id's of newly derived persistent classes would be cased here, e.g.:
	// case DERIVED_PERSISTENT_ID_NUMBER:	return new DerivedPersistent;
	default: {
	    return GraphicConstruct(id);
	}
    }
}

Coord x[] = { 50, 100, 75, 100,  50,  0 };
Coord y[] = {  0,  50, 75, 100, 125, 25 };

/*
 * Primordial root object creation and initialization function.  A pointer
 * to this function is passed to the object manager constructor.
 */
void GraphicsInitialize (RefList* root) {
    Picture* pict = new Picture;
    FullGraphic dfault;
    Line* line;
    MultiLine* multiline;
    BSpline* spline;
    Rect* rect;
    FillRect* frect;
    Circle* circle;
    FillCircle* fcircle;
    Polygon* poly;
    FillPolygon* fpoly;
    ClosedBSpline* cspline;
    FillBSpline* fspline;
    Label* label;
	    
    InitPPaint();
    root->Append(new RefList(pict));
    root->Append(new RefList(pblack));
    root->Append(new RefList(pwhite));
    root->Append(new RefList(psolid));
    root->Append(new RefList(pclear));
    root->Append(new RefList(psingle));
    root->Append(new RefList(pstdfont));

    dfault.FillBg(true);
    dfault.SetColors(pblack, pwhite);
    dfault.SetPattern(psolid);
    dfault.SetBrush(psingle);
    dfault.SetFont(pstdfont);

    line = new Line (0, 0, 75, 75, &dfault);
    multiline = new MultiLine (x, y, 6, &dfault);
    spline = new BSpline (x, y, 6, &dfault);
    rect = new Rect (0, 0, 100, 100, &dfault);
    frect = new FillRect (0, 0, 100, 100, &dfault);
    circle = new Circle (0, 0, 50, &dfault);
    fcircle = new FillCircle (0, 0, 50, &dfault);
    poly = new Polygon (x, y, 6, &dfault);
    fpoly = new FillPolygon (x, y, 6, &dfault);
    cspline = new ClosedBSpline (x, y, 6, &dfault);
    fspline = new FillBSpline (x, y, 6, &dfault);
    label = new Label ("This text comprises a label object.", &dfault);
    
    line->Rotate(37.0);
    line->Translate(0, 300);
    multiline->Rotate(37.0);
    multiline->Translate(100, 300);
    spline->Rotate(37.0);
    spline->Translate(250, 300);
    rect->Rotate(37.0);
    rect->Translate(100, 150);
    frect->Rotate(37.0);
    frect->Translate(100, 0);
    circle->Scale(1.0, 0.6);
    circle->Rotate(37.0);
    circle->Translate(0, 150);
    fcircle->Scale(1.0, 0.6);
    fcircle->Rotate(37.0);
    fcircle->Translate(0, 0);
    poly->Rotate(37.0);
    poly->Translate(250, 150);
    fpoly->Rotate(37.0);
    fpoly->Translate(250, 0);
    cspline->Rotate(37.0);
    cspline->Translate(400, 150);
    fspline->Rotate(37.0);
    fspline->Translate(400, 0);
    label->Translate(350, 175);
    label->Rotate(22.0);

    pict->Append(line, multiline, spline, rect);
    pict->Append(frect, circle, fcircle, poly);
    pict->Append(fpoly, cspline, fspline, label);
}

View::View (Graphic* g) : (nil, g) {
    input = new Sensor(updownEvents);
    input->Catch(KeyEvent);
}

void View::Handle (Event& e) {
    Coord slop = round(cm/15);
    Picture* pict = (Picture*) graphic;
    Graphic* g;

    if (e.eventType == DownEvent) {
	BoxObj b = BoxObj(e.x-slop, e.y-slop, e.x+slop, e.y+slop);
	if ((g = pict->LastGraphicIntersecting(b)) != nil) {
	    Move(g, e);
	}
    } else if (e.eventType == KeyEvent && *e.keystring == 'q') {
	graphics->Kill();
    }    
}

void View::Move (Graphic* g, Event& e) {
    SlidingRect* sr;
    Coord l, b, r, t, dx, dy, dummy;
    
    g->GetBox(l, b, r, t);
    sr = new SlidingRect(output, canvas, l, b, r, t, e.x, e.y);
    sr->Draw();

    Listen(allEvents);
    do {
	sr->Track(e.x, e.y);
	Read(e);
    } while (e.eventType != UpEvent);
    Listen(input);

    sr->GetCurrent(dx, dy, dummy, dummy);
    delete sr;
    dx -= l;
    dy -= b;
    g->Translate(dx, dy);
    g->Touch();
    Update();
}

void Graphics::CreateView () {
    Banner* banner;
    View* view;
    HBox* interior;
    HScroller* hscroller;
    VScroller* vscroller;
    Coord width = round(cm/2);

    pict->SetTransformer(nil);
    pict->Translate(50, 50);

    view = new View(pict);
    hscroller = new HScroller(view, width);
    vscroller = new VScroller(view, width);

    banner = new Banner(stdpaint, "graphics demo", nil, "type 'q' to quit");
    interior = new HBox(
	new VBox(view, new HBorder(stdpaint), hscroller),
	new VBorder(stdpaint),
	new VBox(
	    vscroller, new HBorder(stdpaint), new VGlue(stdpaint, width, 0, 0)
	)
    );
    
    world->Insert(new TitleFrame(stdpaint, banner, interior));
}

Graphics.Graphics () {
    RefList* root;

    alive = true;
    TheManager = new ObjectMan(
	"graphics", &GraphicsInitialize, &GraphicsConstruct
    );
    root = TheManager->GetRoot();
    pict = (Picture*) (*(*root)[1])();
    CreateView();
}

void Graphics::Run () {
    Event e;

    while (alive) {
	world->Read(e);
	e.target->Handle(e);
    }
}

void Graphics::Kill () {
    alive = false;
}

void main () {
    world = new World;
    graphics = new Graphics;
    graphics->Run();
    delete graphics;
    delete TheManager;			    // write out persistent stuff
    delete world;
}
