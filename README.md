newpid
======

Very simple wrapper around clone(CLONE_NEWPID) that launches a command in a new
PID namespace. /proc is also remounted so it sees the new process space. Needs
root to run.

I haven't seen this functionality as a standalone command elsewhere. If you
find something else, please let me know.

Example: $ sudo newpid ps aux
<pre>
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root         1  0.0  0.0   4080    84 pts/3    S+   12:15   0:00 newpid ps aux
root         2  0.0  0.0  19984  1316 pts/3    R+   12:15   0:00 ps aux
</pre>

 -- Christoph Berg <myon@debian.org>
