# output error messages in English
export LC_MESSAGES=C

check: check-zombie check-newnet check-isolation check-netns-name check-signal

check-zombie:
	newpid ./zombie.pl 2>&1 | tee zombie.out
	if grep -q 'version code' zombie.out; then sed -i -e '1,/version code/d' zombie.out; fi # remove "Non-standard uts for running kernel" on lucid
	diff -ub zombie.expected zombie.out

check-newnet:
	# remove time/rtt output for reproducibility
	# iputils-ping 3:20190515-2 added a "ping: " prefix in front of errors, remove it for compatibility
	( $(ROOTCMD) newpid -n ping -q -c 1 -n 127.0.0.1 2>&1 | sed -e 's/\(time\|rtt\).*//'; \
	  $(ROOTCMD) newpid -n ping -q -c 1 -n 192.0.2.1 2>&1 | sed -e 's/^ping: //' \
	) | tee newnet.out
	diff -u newnet.expected newnet.out

check-isolation:
	# The command must run in its own process group/session, so a kill(0, SIGTERM)
	# issued inside the sandbox (like ncat 7.991's signal handler) cannot reach
	# this test harness. The TERM trap turns a leak into a test failure instead
	# of killing the harness. (perl 'kill 15, 0' == kill(0, SIGTERM))
	trap 'echo "FAIL: kill(0) leaked out of the sandbox"; exit 1' TERM; \
	if newpid perl -e 'kill 15, 0'; then \
		echo "isolation: OK"; \
	else \
		echo "isolation: OK (sandbox terminated itself on kill(0), as expected)"; \
	fi; \
	trap - TERM

check-netns-name:
	# -N namespace names must start with 'newpid' AND contain no '/' (both
	# rejections happen in main() before clone(), so no privileges needed).
	if newpid -N 'newpid/../..' true >/dev/null 2>&1; then \
		echo "FAIL: '-N newpid/../..' was not rejected"; exit 1; \
	else \
		echo "netns-name: OK ('newpid/../..' rejected)"; \
	fi
	if newpid -N other true >/dev/null 2>&1; then \
		echo "FAIL: '-N other' was not rejected"; exit 1; \
	else \
		echo "netns-name: OK ('other' rejected)"; \
	fi

check-signal:
	# SIGTERM sent to newpid must be forwarded into the sandbox: the command
	# runs in its own session/process group, so a group kill can no longer
	# reach it and the termination has to be relayed main() -> run() ->
	# command. The command's own TERM trap (exit 42) proves the signal
	# arrived; 143 is accepted too. The timeout guard keeps a broken forward
	# from hanging the test run.
	timeout -k 2 15 newpid sh -c 'trap "exit 42" TERM; while :; do sleep 1; done' & \
	NP=$$!; \
	sleep 1; \
	kill -TERM $$NP; \
	wait $$NP; RC=$$?; \
	if test "$$RC" != "42" && test "$$RC" != "143"; then \
		echo "FAIL: signal forwarding broken (exit=$$RC)"; exit 1; \
	fi; \
	echo "signal: OK (exit=$$RC)"

clean:
	rm -f zombie.out newnet.out
