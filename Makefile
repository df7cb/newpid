CFLAGS += -g -O2 -Wall -Werror
PREFIX = /usr/local

all: newpid newpid.1 newnet.1

newpid: newpid.o

%.1: %.pod
	pod2man --center "" -r "" --quotes=none --section 1 $< > $@

install: newpid newpid.1
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 newpid $(DESTDIR)$(PREFIX)/bin/newpid
ifeq ($(DESTDIR),)
	setcap "CAP_SYS_ADMIN,CAP_NET_ADMIN=pe" $(DESTDIR)$(PREFIX)/bin/newpid
endif
	install -m 755 newnet $(DESTDIR)$(PREFIX)/bin/newnet
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	install newpid.1 $(DESTDIR)$(PREFIX)/share/man/man1/newpid.1
	install newnet.1 $(DESTDIR)$(PREFIX)/share/man/man1/newnet.1

check:
	$(MAKE) -C test check

clean:
	rm -f newpid newpid.o newpid.1 newnet.1
	$(MAKE) -C test clean
