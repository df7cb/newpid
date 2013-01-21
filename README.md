newpid
======

Very simple wrapper around clone(CLONE_NEWPID) that launches a command in a new
PID namespace. /proc is also remounted so it sees the new process space. Needs
root to run.

I haven't seen this functionality as a standalone command elsewhere. If you
find something else, please let me know.

Example: # newpid make check

 -- Christoph Berg <myon@debian.org>
