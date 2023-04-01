# YmirDB
COMP2017 - Systems Programming second assignment. This was a quite complex database system written in C, designed to teach more complex memory operations.
<br /><br />
BYE - clear database and exit<br />
HELP - display this help message<br />
LIST KEYS - displays all keys in current state<br />
LIST ENTRIES - displays all entries in current state<br />
LIST SNAPSHOTS - displays all snapshots in the database<br />
GET <key> - displays entry values<br />
DEL <key> - deletes entry from current state<br />
PURGE <key> - deletes entry from current state and snapshots<br />
SET <key> <value ...> - sets entry values<br />
PUSH <key> <value ...> - pushes values to the front<br />
APPEND <key> <value ...> - appends values to the back<br />
PICK <key> <index> - displays value at index<br />
PLUCK <key> <index> - displays and removes value at index<br />
POP <key> - displays and removes the front value<br />
DROP <id> - deletes snapshot<br />
ROLLBACK <id> - restores to snapshot and deletes newer snapshots<br />
CHECKOUT <id> - replaces current state with a copy of snapshot<br />
SNAPSHOT - saves the current state as a snapshot<br />
MIN <key> - displays minimum value<br />
MAX <key> - displays maximum value<br />
SUM <key> - displays sum of values<br />
LEN <key> - displays number of values<br />
REV <key> - reverses order of values (simple entry only)<br />
UNIQ <key> - removes repeated adjacent values (simple entry only)<br />
SORT <key> - sorts values in ascending order (simple entry only)<br />
FORWARD <key> - lists all the forward references of this key<br />
BACKWARD <key> - lists all the backward references of this key<br />
TYPE <key> - displays if the entry of this key is simple or general<br />
