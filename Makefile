CFLAGS += -g -O2 -Wall -Werror
PREFIX = /usr

all: newpid newpid.1 newnet.1

newpid: newpid.o

%.1: %.pod
	pod2man --center "" -r "" --quotes=none --section 1 $< > $@

install: newpid newpid.1
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 4755 newpid $(DESTDIR)$(PREFIX)/bin/newpid
	install -m 755 newnet $(DESTDIR)$(PREFIX)/bin/newnet
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install newpid.1 $(DESTDIR)$(PREFIX)/share/man/man1/newpid.1
	install newnet.1 $(DESTDIR)$(PREFIX)/share/man/man1/newnet.1

clean:
	rm -f newpid newpid.o newpid.1 newnet.1 z.out
