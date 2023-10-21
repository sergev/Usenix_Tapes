round(num, quantum)
int num, quantum;
{

        return ((((num + quantum) >> 1) / quantum) * quantum);
}

long
ldround(dnum, quantum)
double dnum;
int quantum;
{
        long lnum;

        lnum = dnum + (quantum / 2);
        return (((lnum / quantum) * quantum));
}
