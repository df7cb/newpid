newpid
======

Newpid is a simple wrapper around clone(CLONE_NEWPID) that launches a command
in a new PID namespace. Child processes exiting are properly reaped so no
zombie processes stay around. /proc is also remounted so it sees the new
process space; CLONE_NEWNS is used to make sure this doesn't affect the host
system. Newpid can safely be installed as a suid binary, it will drop
privileges after executing the necessary system calls.

With -n, CLONE_NEWNET starts a new network namespace. This can be used to test
multiple daemons that all use the same local port at the same time. Invoking
"newnet" is equivalent to "newpid -n".

With -i, CLONE_NEWIPC starts a new IPC namespace.

Needed capabilities are CAP_SYS_ADMIN and CAP_NET_ADMIN. Alternatively, newpid
will drop privileges when installed suid root.

I haven't seen this functionality as a standalone command elsewhere. If you
find something else, please let me know.

Examples:
<pre>
$ newpid ps aux
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
myon         1  0.0  0.0   4080    84 pts/3    S+   12:15   0:00 newpid ps aux
myon         2  0.0  0.0  19984  1316 pts/3    R+   12:15   0:00 ps aux
</pre>

<pre>
$ newpid -n ip link
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
</pre>

 -- Christoph Berg <myon@debian.org>
