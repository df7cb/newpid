/*
 * newpid: launch a subprocess in a new PID namespace
 * Copyright (C) 2013 Christoph Berg <myon@debian.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
run (void *argv_void)
{
	char *const *argv = argv_void;
	char *argv_sh[] = { NULL, NULL };

	if (umount ("/proc") != 0) {
		perror ("umount /proc");
		exit (1);
	}

	if (mount ("proc", "/proc", "proc", 0, NULL) != 0) {
		perror ("mount proc");
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

	if (execvp (argv[0], argv) < 0) {
		perror ("execvp");
		exit (1);
	}

	/* NOT REACHED */
	return 0;
}

int
main (int argc, char *argv[], char *envp[])
{
	char cstack[2048];
	int child;
	int status;

	if ((child = clone (run,
			cstack + 1024, /* middle of array so we don't care which way the stack grows */
			CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, /* new pid & mount namespace, send SIGCHLD on termination */
			argv + 1) /* skip argv[0] */
	) < 0) {
		perror ("clone");
		exit (1);
	}

	wait (&status);

	return WEXITSTATUS (status);
}
