#!/usr/bin/perl

# This script forks twice, and the middle process exits immediately. The third
# process will then gets pid 1 as parent, and exit later. At that point, pid 1
# receives a SIGCHLD signal. The first process keeps running so we can actually
# observe this effect.

if (fork == 0) {
	if (fork == 0) {
		sleep 1;
		exit 1;
	} else {
		exit 2;
	}
} else {
	sleep 2;
	system "ps xf";
	exit 3;
}
