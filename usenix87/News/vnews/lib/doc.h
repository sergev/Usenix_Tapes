.na
Newsgroups may be refered to by name or my number.
The name is identical across all machines,
the number is specific to a particular machine.

Articles are refered to by message id, by number in newsgroup,
or by offset into data base.
The message id is the same on all machines.
The number is specific to a given machine.
The offset into the data base is specific to a particular
machine and a particular open call to the data base.
(When expire runs, it moves the article records.
A program which has the data base open will continue
to use the old data base, however, so that the same offsets
will still work.)

The article data base contains a record for each article.
It consists of a single file divided into several sections.
First is the header, which gives the size of the other sections.
Second is a hash table for message ids.
Third is a hash table of reference lines which is used to deal with
orphaned articles.
Fourth is a table of pointers to newsgroup chains.
All the articles in a newsgroup are connected by a singly linked list.
Finally, the article records appear.

There is one article record for each article.
