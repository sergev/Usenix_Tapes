{
{ {
     { Interactor is the base class for all interactive objects. } ^ . {
     Every interactor has a shape member variable that defines
     the desired characteristics of screen space in terms of
     size, shrinkability, and stretchability. } ^ . { This information
     is used to allocate display space for the interactor and the
     interactor's canvas member variable is set to the actual
     space obtained. } ^ . { The lower left corner of the canvas is
     addressed by (0,0) the upper right by the member variables
     (xmax,ymax). } ^ .
} 2 + . } _ 0 0 ~
<
{ {
     { The input member variable is the normal sensor for reading
     events. } ^ . { The output member variable is the standard painter
     for performing graphics operations. } ^ . { The cursor member
     variable controls the format and interpretation of the input
     pointing device (e.g., mouse cursor) when it is inside the
     interactor's canvas. } ^ .
} 2 + . } _ 0 0 ~
<
{ {
     { An interactor may optionally define the perspective member
     variable to represent the portion of total area that the
     interactor is displaying. } ^ . { Perspectives allow interactors to
     coordinate with other interactors, such as scrollers, that
     want to control the display area. } ^ .
} 2 + . } _ 0 0 ~
<
{ {
     { To access the workstation, an interactor must be inserted
     into a scene, called its parent. } ^ . { The root scene is the
     instance of the World class that must be created at the
     start of your program. } ^ .
} 2 + . } _ 0 0 ~
} < ^ _ _ ;
