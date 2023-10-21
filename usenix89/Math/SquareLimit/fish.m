%insert "graphics.m" || All the graphics primitives.
%list
||
|| The following are all the functions required to produce the famous 
|| "Square Limit" drawing by M. C. Escher. These functions were derived by 
|| Peter Henderson. See his paper in 1982 Conf. on LISP and Func. Prog.
|| (C) Miranda coding by Michael Parsons, September 1985
||
%nolist

|| quartet combines 4 images, one image in each quadrant, to produce one image.
quartet :: image -> image -> image -> image -> image
quartet a b c d = col [row [a,b], row [c,d]]

|| nonet arranges 9 images in equal sized squares to form 1 image.
nonet :: image -> image -> image -> image -> image -> image -> image -> image -> image -> image
nonet a b c d e f g h i = col [row [a,b,c], row [d,e,f], row [g,h,i]]

|| cycle combines 4 smaller copies of an image, each rotated by a different
|| multiple of 90 degrees.
cycle :: image -> image
cycle i = quartet i (rot threepibytwo i) (rot pibytwo i) (rot pi i)

|| t is one of the basic fish rearrangements.
t :: image
t = quartet p q r s

|| u is another of the basic fish rearrangements.
u :: image
u = cycle (rot pibytwo q)

|| side defines one side of the picture at level n.
side :: num -> image
side 0 = quartet [] [] rt t
         where                  || This where due to compiler bug?
         rt = rot pibytwo t
side n = quartet sn sn rt t
         where
         rt = rot pibytwo t
         sn = side (n-1)

|| corner defines one corner of the picture at level n.
corner :: num -> image
corner 0 = quartet [] [] [] u
corner n = quartet (corner (n-1)) sn (rot pibytwo sn) u
           where
           sn = side (n-1)

|| quadcorner forms one quarter of the image at level n;
quadcorner :: num -> image
quadcorner n = nonet (corner n) sn sn rsn u rt rsn rt (rot pibytwo q)
               where
               sn = side n
               rsn = rot pibytwo sn
               rt = rot pibytwo t

|| squarelimit produces the fish drawing.
squarelimit :: num -> image
squarelimit n = cycle (quadcorner n)

|| fish gives a border to produce the final drawing.
fish :: num -> image
fish n = (squarelimit n) ++ border

|| All the data needed to draw the fish.
p :: image
p = [Ln (Pt 0 0.8125) (Pt 0 0.5),
    Ln (Pt 0 0.8125) (Pt 0.1875 0.75),
    Ln (Pt 0.75 0) (Pt 0.8125 6.25e-2),
    Ln (Pt 0.1875 0.75) (Pt 0 0.5),
    Ln (Pt 0.375 1) (Pt 0.25 0.75),
    Ln (Pt 0.25 0.6875) (Pt 0.4375 0.625),
    Ln (Pt 0.25 0.6875) (Pt 0.25 0.375),
    Ln (Pt 0.4375 0.625) (Pt 0.25 0.375),
    Ln (Pt 0.6875 1) (Pt 0.625 0.75),
    Ln (Pt 0.6875 1) (Pt 0.875 0.875),
    Ln (Pt 0.875 0.875) (Pt 1 0.875),
    Ln (Pt 0.625 0.75) (Pt 0.8125 0.6875),
    Ln (Pt 0.8125 0.6875) (Pt 1 0.75),
    Ln (Pt 0.625 0.75) (Pt 0.5 0.5),
    Ln (Pt 0.5625 0.625) (Pt 0.75 0.5625),
    Ln (Pt 0.75 0.5625) (Pt 1 0.625),
    Ln (Pt 0.5 0.5) (Pt 0.75 0.4375),
    Ln (Pt 0.75 0.4375) (Pt 1 0.5),
    Ln (Pt 0.5 0.5) (Pt 0.25 0.1875),
    Ln (Pt 0.25 0.1875) (Pt 0 0),
    Ln (Pt 0.5 0.25) (Pt 1 0.375),
    Ln (Pt 1 0.25) (Pt 0.75 0.25),
    Ln (Pt 0.75 0.25) (Pt 0.5 0),
    Ln (Pt 0.5 0) (Pt 0.375 6.25e-2),
    Ln (Pt 0.375 6.25e-2) (Pt 0 0),
    Ln (Pt 0.625 0) (Pt 0.75 0.125),
    Ln (Pt 0.75 0.125) (Pt 1 0.1875),
    Ln (Pt 1 0.125) (Pt 0.8125 6.25e-2),
    Ln (Pt 1 6.25e-2) (Pt 0.875 0)]

q :: image
q = [Ln (Pt 0 0) (Pt 0 0.25),
    Ln (Pt 0 0.5) (Pt 0 1),
    Ln (Pt 0 1) (Pt 0.5 1),
    Ln (Pt 0.75 1) (Pt 1 1),
    Ln (Pt 0.125 1) (Pt 0.25 0.6875),
    Ln (Pt 0.25 1) (Pt 0.375 0.6875),
    Ln (Pt 0.375 1) (Pt 0.5 0.6875),
    Ln (Pt 0.5 1) (Pt 0.625 0.625),
    Ln (Pt 0.25 0.6875) (Pt 0.25 0.5625),
    Ln (Pt 0.375 0.6875) (Pt 0.375 0.5625),
    Ln (Pt 0.5 0.6875) (Pt 0.5 0.5),
    Ln (Pt 0.625 0.625) (Pt 0.625 0.4375),
    Ln (Pt 0.625 1) (Pt 0.875 0.3125),
    Ln (Pt 0.75 1) (Pt 0.8125 0.75),
    Ln (Pt 0.8125 1) (Pt 1 0.625),
    Ln (Pt 0.875 1) (Pt 1 0.75),
    Ln (Pt 0.9375 1) (Pt 1 0.875),
    Ln (Pt 0 0.25) (Pt 0.1875 0.1875),
    Ln (Pt 0 0.375) (Pt 0.4375 0.3125),
    Ln (Pt 0 0.5) (Pt 0.25 0.5625),
    Ln (Pt 0.125 0) (Pt 0.1875 0.1875),
    Ln (Pt 0.1875 0.1875) (Pt 0.3125 0.125),
    Ln (Pt 0.3125 0.125) (Pt 0.25 0),
    Ln (Pt 0.3125 0.125) (Pt 0.4375 6.25e-2),
    Ln (Pt 0.4375 6.25e-2) (Pt 0.5 0),
    Ln (Pt 0.5 6.25e-2) (Pt 0.6875 6.25e-2),
    Ln (Pt 0.6875 6.25e-2) (Pt 0.5625 0.1875),
    Ln (Pt 0.5625 0.1875) (Pt 0.5 6.25e-2),
    Ln (Pt 0.5625 0.25) (Pt 0.75 0.25),
    Ln (Pt 0.75 0.25) (Pt 0.625 0.375),
    Ln (Pt 0.625 0.375) (Pt 0.5625 0.25),
    Ln (Pt 1 0) (Pt 0.75 0.375),
    Ln (Pt 0.375 0) (Pt 0.4375 6.25e-2),
    Ln (Pt 0.75 0.375) (Pt 0.375 0.5625),
    Ln (Pt 0.375 0.5625) (Pt 0.25 0.5625),
    Ln (Pt 1 0) (Pt 0.9375 0.375),
    Ln (Pt 0.9375 0.375) (Pt 1 0.5),
    Ln (Pt 1 0.5) (Pt 0.8125 0.75)]

r :: image
r = [Ln (Pt 0.5 0.5) (Pt 0.125 0.25),
    Ln (Pt 0.125 0.25) (Pt 0 0),
    Ln (Pt 0 0.5) (Pt 0.125 0.25),
    Ln (Pt 0 0.25) (Pt 6.25e-2 0.125),
    Ln (Pt 0 1) (Pt 0.5 0.5),
    Ln (Pt 0 0.75) (Pt 0.3125 0.375),
    Ln (Pt 6.25e-2 0.9375) (Pt 0.25 1),
    Ln (Pt 0.125 0.875) (Pt 0.5 1),
    Ln (Pt 0.1875 0.8125) (Pt 0.5 0.875),
    Ln (Pt 0.3125 0.6875) (Pt 0.75 0.8125),
    Ln (Pt 0.5 0.875) (Pt 0.75 1),
    Ln (Pt 0.75 0.8125) (Pt 1 1),
    Ln (Pt 0.5 0.5) (Pt 0.875 0.625),
    Ln (Pt 1 0.625) (Pt 0.6875 0.375),
    Ln (Pt 0.6875 0.375) (Pt 0.375 0),
    Ln (Pt 1 0.5) (Pt 0.75 0.25),
    Ln (Pt 1 0.75) (Pt 0.875 0.625),
    Ln (Pt 0.75 0.25) (Pt 0.6875 0),
    Ln (Pt 0.75 0.25) (Pt 1 0),
    Ln (Pt 1 0.375) (Pt 0.8125 0.1875),
    Ln (Pt 1 0.25) (Pt 0.875 0.125),
    Ln (Pt 1 0.125) (Pt 0.9375 6.25e-2)]

s :: image
s = [Ln (Pt 0 1) (Pt 0.25 0.875),
    Ln (Pt 0.125 0.9375) (Pt 0 0.75),
    Ln (Pt 0.25 0.875) (Pt 0.5 0.875),
    Ln (Pt 0.5 0.875) (Pt 1 1),
    Ln (Pt 1 1) (Pt 0.625 0.75),
    Ln (Pt 0.625 0.75) (Pt 0.5 0.625),
    Ln (Pt 0 0.625) (Pt 0.4375 0.75),
    Ln (Pt 0 0.5) (Pt 0.5 0.625),
    Ln (Pt 0 0.375) (Pt 0.4375 0.5),
    Ln (Pt 0 0.25) (Pt 0.4375 0.375),
    Ln (Pt 0 0.125) (Pt 0.4375 0.1875),
    Ln (Pt 0.5 0.625) (Pt 0.4375 0.5),
    Ln (Pt 0.4375 0.5) (Pt 0.4375 0.1875),
    Ln (Pt 0.4375 0.1875) (Pt 0.5 0),
    Ln (Pt 0.625 0) (Pt 0.6875 0.375),
    Ln (Pt 0.75 0) (Pt 0.8125 0.1875),
    Ln (Pt 0.8125 0.1875) (Pt 0.9375 0.4375),
    Ln (Pt 0.9375 0.4375) (Pt 1 0.5),
    Ln (Pt 1 0.125) (Pt 0.8125 0.1875),
    Ln (Pt 1 0.25) (Pt 0.875 0.3125),
    Ln (Pt 1 0.375) (Pt 0.9375 0.4375),
    Ln (Pt 0.75 0.5625) (Pt 0.75 0.75),
    Ln (Pt 0.75 0.75) (Pt 0.625 0.625),
    Ln (Pt 0.625 0.625) (Pt 0.75 0.5625),
    Ln (Pt 0.8125 0.5625) (Pt 0.9375 0.5),
    Ln (Pt 0.9375 0.5) (Pt 0.9375 0.6875),
    Ln (Pt 0.9375 0.6875) (Pt 0.8125 0.5625)]
