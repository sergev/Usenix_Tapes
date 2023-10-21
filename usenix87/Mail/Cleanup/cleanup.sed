echo "Cleaning up $1"
ed $1 <<!
g/Internet:/d
g/^---/d
g/Reply-To:/d
g/Nf-/d
g/Relay-Version:/d
g/Path:/d
g/Posting-Version:/d
g/Message-ID:/d
g/Message-Id:/d
g/Date:/d
g/Date-Received:/d
g/References:/d
g/Lines:/d
g/^>>/d
g/^> >/d
g/Xref:/d
g/Sender:/d
g/Expires:/d
g/Distribution:/d
g/From uucp/d
g/Received:/d
g/	id/d
w
q
!
