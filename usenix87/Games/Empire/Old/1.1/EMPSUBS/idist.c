idist(x, y)
int x, y;
{
        register int i;
        double dx, dy, sqrt();

        dx = x;
        dy = y;
        i = sqrt(((dx * dx) + (dy * dy))) + 0.5;
        return (i);
}

idsq(x, y)
int x, y;
{

        return (((x * x) + (y * y)));
}
