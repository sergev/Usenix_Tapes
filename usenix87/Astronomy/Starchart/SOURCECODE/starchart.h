typedef struct
    {
    float racen, dlcen, scale;
    float north, south, east, west;
    float maglim;
    int wx, wy, ww, wh;
    float yscale;
    } mapblock, *map;

extern mapblock master, thumbnail;
