newpid
======

Newpid is a wrapper around clone(CLONE_NEWPID) that launches a command
in a new PID namespace. Child processes exiting are properly reaped so no
zombie processes stay around. /proc is also remounted so it sees the new
process space; CLONE_NEWNS is used to make sure this doesn't affect the host
system. Newpid can safely be installed as a suid binary, it will drop
privileges after executing the necessary system calls.

With -n, CLONE_NEWNET starts a new network namespace. This can be used to test
multiple daemons that all use the same local port at the same time. Invoking
"newnet" is equivalent to "newpid -n".

-N is similar to -n, but joins a preconfigured network namespace whose name
must start with "newpid". See below for an example.

With -i, CLONE_NEWIPC starts a new IPC namespace.

With -u, CLONE_NEWUTS starts a new UTS namespace.

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

Joining a namespace
-------------------

newpid -N newpidns1 will join the namespace configured at
/var/run/netns/newpidns1.  To set up namespaces, do something like this:

<pre>
ip netns add newpidns1

ip link add veth0 type veth peer name veth1
ip link set veth1 netns newpidns1
ip a add 10.1.1.1/24 dev veth0
ip link set veth0 up
ip netns exec newpidns1 ip link set lo up
ip netns exec newpidns1 ip a add 10.1.1.2/24 dev veth1
ip netns exec newpidns1 ip link set veth1 up
</pre>

Requirements
------------

The setns() system call first appeared in Linux in kernel 3.0; library support
was added to glibc in version 2.14.

 -- Christoph Berg <myon@debian.org>
