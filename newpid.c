/*
 * newpid: launch a subprocess in a new PID namespace
 * Copyright (C) 2013, 2014 Christoph Berg <myon@debian.org>
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
#include <errno.h>
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
	pid_t child;
	pid_t pid;

	if (umount ("/proc") != 0) {
		/* ignore errors here, /proc could be busy
		perror ("umount /proc");
		exit (1);
		*/
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

	if (waitpid (child, &status, 0) < 0) {
		perror ("waitpid");
	}

	if (WIFEXITED (status))
		return WEXITSTATUS (status);
	if (WIFSIGNALED (status))
		return 128 + WTERMSIG (status);
	return -1;
}
