/*
 * newpid: launch a subprocess in a new PID namespace
 * Copyright (C) 2013-2015 Christoph Berg <myon@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/if.h>

/* squeeze's and lucid's libc do not expose these: */
#ifndef MS_REC
#define MS_REC 16384
#endif
#ifndef MS_SLAVE
#define MS_SLAVE (1<<19)
#endif

#define NETNS_RUN_DIR "/var/run/netns"

/* global flags */
int cloneflags = 0;
char *netns = NULL;

/* get_ctl_fd, do_chflags, netns_switch from iproute2 (C) Alexey Kuznetsov <kuznet@ms2.inr.ac.ru> GPL2+ */
static int
get_ctl_fd (void)
{
	int s_errno;
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	s_errno = errno;
	fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	errno = s_errno;
	perror("Cannot create control socket");
	return -1;
}

static int
do_chflags (const char *dev, unsigned long flags, unsigned long mask)
{
	struct ifreq ifr;
	int fd;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if ((fd = get_ctl_fd()) < 0)
		return -1;
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0) {
		perror("SIOCGIFFLAGS");
		return -1;
	}
	if ((ifr.ifr_flags^flags)&mask) {
		ifr.ifr_flags &= ~mask;
		ifr.ifr_flags |= mask&flags;
		if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0) {
			perror("SIOCSIFFLAGS");
			return -1;
		}
	}
	close(fd);
	return 0;
}

int netns_switch(char *name)
{
	char net_path[PATH_MAX];
	int netns;

	snprintf(net_path, sizeof(net_path), "%s/%s", NETNS_RUN_DIR, name);
	netns = open(net_path, O_RDONLY | O_CLOEXEC);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace \"%s\": %s\n",
			name, strerror(errno));
		return -1;
	}

	if (setns(netns, CLONE_NEWNET) < 0) {
		fprintf(stderr, "setting the network namespace \"%s\" failed: %s\n",
			name, strerror(errno));
		close(netns);
		return -1;
	}
	close(netns);

	return 0;
}

int
run (void *argv_void)
{
	char *const *argv = argv_void;
	char *argv_sh[] = { NULL, NULL };
	pid_t child;
	pid_t pid;

	if (mount("none", "/proc", NULL, MS_SLAVE|MS_REC, NULL) != 0) {
		perror ("remount proc private");
		exit (1);
	}

	if (mount ("proc", "/proc", "proc", 0, NULL) != 0) {
		perror ("mount proc");
		exit (1);
	}

	if (cloneflags & CLONE_NEWNET) {
		if (netns) {
			/* join this network namespace */
			if (netns_switch (netns) < 0)
				exit (1);
		} else {
			/* set loopback device up */
			if (do_chflags ("lo", IFF_UP, IFF_UP) < 0)
				exit (1);
		}
	}

	/* drop privileges */
	if (setuid(getuid()) != 0) {
		perror ("setuid");
		exit (1);
	}

	if (argv[0] == NULL) {
		char *shell = getenv ("SHELL");

		if (shell)
			argv_sh[0] = shell;
		else
			argv_sh[0] = "/bin/sh";
		argv = argv_sh;
	}

	if ((child = fork ()) == 0) {
		if (execvp (argv[0], argv) < 0) {
			perror ("execvp");
			exit (1);
		}
		/* NOT REACHED */
	}
	if (child < 0) {
		perror ("fork");
		exit (1);
	}

	int status;
	while ((pid = wait (&status)) != child) {
		if (pid < 0 && errno != EINTR) {
			perror ("waitpid");
			exit (1);
		}
		/* ignore SIGCHLD for other children and retry */
		// printf ("Reaped child %d with status %d\n", pid, status);
	}

	if (WIFEXITED (status))
		return WEXITSTATUS (status);
	if (WIFSIGNALED (status))
		return 128 + WTERMSIG (status);
	return -1;
}

int
main (int argc, char *argv[], char *envp[])
{
	int opt;
	while ((opt = getopt(argc, argv, "+inN:u")) != -1) {
		switch (opt) {
			case 'i':
				cloneflags |= CLONE_NEWIPC;
				break;
			case 'N':
				if (strncmp(optarg, "newpid", sizeof("newpid")-1)) {
					fprintf(stderr, "-N argument must start with 'newpid'\n");
					exit(EXIT_FAILURE);
				}
				netns = strdup(optarg);
				/* fallthrough */
			case 'n':
				cloneflags |= CLONE_NEWNET;
				break;
			case 'u':
				cloneflags |= CLONE_NEWUTS;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [options] [command args ...]\n",
						argv[0]);
				fprintf(stderr, "Options:\n");
				fprintf(stderr, "  -i           request new IPC namespace (CLONE_NEWIPC)\n");
				fprintf(stderr, "  -n           request new network namespace (CLONE_NEWNET)\n");
				fprintf(stderr, "  -N newpidns  join named network namespace\n");
				fprintf(stderr, "  -u           request new UTS namespace (CLONE_NEWUTS)\n");
				exit(EXIT_FAILURE);
		}
	}

	char cstack[2048];
	int child;
	int status;

	if ((child = clone (run,
			cstack + 1024, /* middle of array so we don't care which way the stack grows */
			CLONE_NEWPID | CLONE_NEWNS | cloneflags | SIGCHLD, /* new pid & mount namespace, send SIGCHLD on termination */
			argv + optind) /* skip argv[0] and options */
	) < 0) {
		perror ("clone");
		exit (1);
	}

	if (waitpid (child, &status, 0) < 0) {
		perror ("waitpid");
	}

	if (WIFEXITED (status))
		return WEXITSTATUS (status);
	if (WIFSIGNALED (status))
		return 128 + WTERMSIG (status);
	return -1;
}
