/*
 * The metaview customizes the squares frame.
 */

#ifndef metaview_h
#define metaview_h

#include <InterViews/frame.h>

class SquaresMetaView : public BorderFrame {
    class SquaresFrame* view;	/* metaview's "subject" */
    class ButtonState* vwidth;	/* width, 0 means no scroller */
    ButtonState* right;		/* otherwise on left */
    ButtonState* hwidth;	/* width, 0 means no scroller */
    ButtonState* below;		/* otherwise above */
    ButtonState* accept;	/* close dialog, update view */
    ButtonState* cancel;	/* close dialog, ignore changes  */
public:
    SquaresMetaView(SquaresFrame*, Painter* out);
    void Dialog(Event&);
    void Update();
};

#endif
