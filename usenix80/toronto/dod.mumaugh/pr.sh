(pwd; ls -l ) | pr -h  "Core Graphics Directory"
pr Read_me
pr doc
pr run
pr -i -n auxiliary.c \
batch.c \
defaults.c \
errors.c \
genisco.c \
init_termin.c \
inquiry.c \
new_aux.c \
prim_attrib.c \
primitives.c \
seg_attrib.c \
segments.c \
view_surface.c \
view_trans.c
nncref auxiliary.c \
batch.c \
defaults.c \
errors.c \
genisco.c \
init_termin.c \
inquiry.c \
new_aux.c \
prim_attrib.c \
primitives.c \
seg_attrib.c \
segments.c \
view_surface.c \
view_trans.c
ar tv libcore.a | pr -h "Library directory"
pr run-example
pr user_program.c
