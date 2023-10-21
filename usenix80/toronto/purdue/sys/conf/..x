: copies /deadstart/new-unix to /deadstart/no-rs04-unix
: and patches swapdev, nswap, and swplo
: to use rp04 swap area
cp /deadstart/new-unix /deadstart/no-rs04-unix
db /deadstart/no-rs04-unix
_pswapdev/
3006!
_sswapdev/
3006!
_pswplo/
6500.!
_sswplo/
1!
_nswap/
5000.!
_paddrx/
547.!
_pipedev/
3000!
%
echo /deadstart/no-rs04-unix  is ready
