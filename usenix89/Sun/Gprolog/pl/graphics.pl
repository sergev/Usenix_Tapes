% Graphics interface

:- op(910,fx,'#').
:- op(911,fx,'##').

'#'(def(X,Y)) :- '##'def(X,Y).
'#'(def(X,Y)) :- !, write('plgraphics: '), write(X), write(' is not defined'),
		nl, fail.

% The following are special cases:
% To make the usage of the functions as similar to the C usage as
% possible and to avoid kludging up the code for 'plgraphics', we introduce
% extra arguments to these calls at this point

% '#'(set_world_coordinate_matrix_2(X)) :-
%	plgraphics(set_world_coordinate_matrix_2(3,3,X)).
% '#'(set_world_coordinate_matrix_3(X)) :-
%	plgraphics(set_world_coordinate_matrix_3(4,4,X)).
% '#'(inquire_world_coordinate_matrix_2(X)) :-
%	plgraphics(inquire_world_coordinate_matrix_2(3,3,X)).
% '#'(inquire_world_coordinate_matrix_3(X)) :-
%	plgraphics(inquire_world_coordinate_matrix_3(4,4,X)).
% '#'(inquire_inverse_composite_matrix(X)) :-
%	plgraphics(inquire_inverse_composite_matrix(4,4,X)).

'#'(X) :- plgraphics(X).

%	@(#)usercore.h 1.8 83/08/31 SMI
% Copyright (c) 1983 by Sun Microsystems, Inc.

'##'def(pi,3.141592654).

'##'def(true,1).
'##'def(false,0).
'##'def(on,1).				% same as true - bjb

'##'def(string,0).
'##'def(character,1).

'##'def(maxvsurf,5).			% view surfaces; maximum number of

'##'def(parallel,0).			% transform constants
'##'def(perspective,1).

'##'def(none,1).			% segment types
'##'def(xlate2,2).
'##'def(xform2,3).
'##'def(xlate3,2).
'##'def(xform3,3).

'##'def(solid,0).			% line styles
'##'def(dotted,1).
'##'def(dashed,2).
'##'def(dotdashed,3).

'##'def(constant,0).			% polygon shading modes
'##'def(gouraud,1).
'##'def(phong,2).

'##'def(pick,0).			% input device constants
'##'def(keyboard,1).
'##'def(button,2).
'##'def(locator,3).
'##'def(valuator,4).
'##'def(stroke,5).

'##'def(roman,0).			% Font select constants
'##'def(greek,1).
'##'def(script,2).
'##'def(oldenglish,3).
'##'def(stick,4).
'##'def(symbols,5).

'##'def(gallant,0).			% raster font constants
'##'def(gacha,1).
'##'def(sail,2).
'##'def(gachabold,3).
'##'def(cmr,4).
'##'def(cmrbold,5).

'##'def(off,0).				% char justify constants
'##'def(left,1).
'##'def(center,2).
'##'def(right,3).

'##'def(normal,0).			% rasterop selection
'##'def(xorrop,1).
'##'def(orrop,2).

'##'def(plain,0).			% polygon interior style
'##'def(shaded,1).

'##'def(basic,0).			% Core output levels
'##'def(buffered,1).
'##'def(dynamica,2).
'##'def(dynamicb,3).
'##'def(dynamicc,4).

'##'def(noinput,0).			% Core input levels
'##'def(synchronous,1).
'##'def(complete,2).

'##'def(twod,0).			% Core dimensions
'##'def(threed,1).

%static struct {			% default primitive attributes
%	int lineindx;
%	int fillindx;
%	int textindx;
%	int linestyl;
%	int polyintstyl;
%	int polyedgstyl;
%	float linwidth;
%	int pen;
%	int font;
%	float chwidth,chheight;
%	float chup[4], chpath[4], chspace[4];
%	int chjust;
%	int chqualty;
%	int marker;
%	int pickid;
%	int rasterop;
%	} PRIMATTS = {1,1,1,SOLID,PLAIN,SOLID,0.0,0,STICK,11.,11.,
%		{0.,1.,0.,1.},{1.,0.,0.,1.}, {0.,0.,0.,1.},
%		OFF,STRING,42,0,NORMAL};
%
