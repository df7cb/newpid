# output error messages in English
export LC_MESSAGES=C

check: check-zombie check-newnet check-isolation

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

clean:
	rm -f zombie.out newnet.out
