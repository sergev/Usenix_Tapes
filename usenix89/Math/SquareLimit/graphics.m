%list
||
|| graphics.m: Copyright Michael S. Parsons, September 1985
||
|| A collection of functions performing the primitive vector operations.
|| All these functions work on line end points. Each point has coordinates
|| which are real values 0 <= x,y <= 1.
||
%nolist

||
|| The algebraic data types.
||

%insert "line.m" || The point and line algebraic data types.

|| polar is the same in a polar coordinate format. Angle then length.
polar ::= Po num num

||
|| A couple of type synonyms for brevity.
||

|| string is the normal string type.
string == [char]

|| An image is a one-level list of lines.
image == [line]

||
|| Assorted constants.
||

twopi = 2 * pi
pibytwo = pi / 2
threepibytwo = 3 * pi / 2

|| imsize is the multiplier for writing out the coordinates as integers.
imsize :: num
imsize = 1024

|| The top left corner of the picture.
topleft :: point
topleft = Pt 0 0

|| The top right of the picture.
topright :: point
topright = Pt 1 0

|| The middle of the picture.
middle = Pt 0.5 0.5

|| border is the square round the perimeter of the picture.
border :: image
border = square topleft 1

||
|| Some useful functions.
||

|| sqr is the normal square function.
sqr :: num -> num
sqr x = x * x

|| ctop converts the cartesian coords to polar coords.
ctop :: point -> polar
ctop (Pt x y) = Po (a x y) l
                where
                a 0 0 = 0
                a 0 y = pi/2, y >= 0
                a 0 y = -pi/2
                a x y = arctan(y / x), x >= 0 & y >= 0
                a x y = pi + arctan(y / x),  x < 0
                a x y = 2 * pi + arctan(y / x)
                l = sqrt(sqr x + sqr y)

|| pri prints out the image as line endpoints: 4 numbers to a line.
pri :: image -> string
pri = concat.(map prl)
      where
      prl (Ln p1 p2) = concat [prp p1, " ", prp p2, "\n"]
                       where
                       prp (Pt x y) = concat [ip x, " ", ip y]
                                      where
                                      ip p = show(entier(p * imsize))

|| imap maps the given function onto each point of the image.
imap :: (point -> point) -> image -> image
imap f i = map imap' i
           where
           imap' (Ln p1 p2) = Ln (f p1) (f p2)

|| join produces a list of lines connecting the points given.
join :: [point] -> image
join pts = join' (hd pts) pts
           where
           join' h [x]     = [Ln h x]
           join' h (a:b:x) = (Ln a b):join' h (b:x)

|| square produces a square given top left hand corner and size.
square :: point -> num -> image
square (Pt x y) s = join [Pt x y, Pt (x+s) y, Pt (x+s) (y+s), Pt x (y+s)]

||
|| Start of graphics primitives.
||

|| tr translates an image by amount t.
tr :: point -> image -> image
tr t = imap (trp t)
       where
       trp (Pt x y) (Pt p q) = Pt (p+x) (q+y)

|| rot rotates an image by th radians anti-clockwise, about the image centre.
rot :: num -> image -> image
rot th i = tr middle (imap rotp (tr (negpoint middle) i))
           where
           rotp p = Pt (coord cos) (coord sin)
                    where
                    coord f = l * (f (a + th))
                              where
                              (Po a l) = ctop p

|| ref90 reflects everything about the line x = 0.5
ref90 :: image -> image
ref90 = imap ref90p
        where
        ref90p (Pt x y) = Pt (1-x) y

|| negpoint changes the sign of the point coordinates.
negpoint :: point -> point
negpoint (Pt x y) = Pt (-x) (-y)

|| scale scales an image up or down in x or y directions with one fixed point.
scale :: point -> point -> image -> image
scale s t i = tr t (imap (scalep s) (tr (negpoint t) i))
              where
              scalep (Pt sx sy) (Pt x y) = Pt (x * sx) (y * sy)

|| beside squashes one image against the other in the ratio r1:r2
beside :: num -> num -> image -> image -> image
beside r1 r2 i1 i2 = scale (sfac r1) topleft i1 ++ scale (sfac r2) topright i2
                     where
                     sfac r = Pt (r / (r1 + r2)) 1

|| above squashes one image above the other in the ratio r1:r2
above :: num -> num -> image -> image -> image
above r1 r2 i1 i2 = rot threepibytwo (beside r1 r2 (rot pibytwo i1) (rot pibytwo i2))

|| seq produces a sequence of images from a list of images.
seq :: (num -> num -> image -> image -> image) -> [image] -> image
seq f [i] = i
seq f (a:x) = f 1 (#x) a (seq f x)

|| row produces a row of (different) images.
row :: [image] -> image
row = seq beside

|| likewise col
col :: [image] -> image
col = seq above

|| sameseq produces a sequence of images all the same.
sameseq :: (num -> num -> image -> image -> image) -> num -> image -> image
sameseq f 0 i = []
sameseq f n i = f 1 (n-1) i (sameseq f (n-1) i)

|| samerow uses sameseq to give a row of identical images.
samerow :: num -> image -> image
samerow = sameseq beside

|| likewise samecol.
samecol :: num -> image -> image
samecol = sameseq above

|| samegrid produces a grid of images, all the same.
samegrid :: num -> num -> image -> image
samegrid x y = (samecol y).(samerow x)

|| mirror90 combines an image with its reflection about the line x = 0.5
mirror90 :: image -> image
mirror90 i = i ++ ref90 i

|| squashes produces a sequence of squashed images, getting more squashed.
squashes :: num -> image -> image
squashes 0 i = []
squashes n i = above 1 (n-1) (samerow n i) (squashes (n-1) i)

|| star merges a sequence of rotations of the same object.
star :: num -> image -> image
star n i = star' 0
           where
           incth = twopi/n
           star' th = [], th >= twopi
           star' th = rot th i ++ star' (th+incth)
