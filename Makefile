CFLAGS += -g -O2 -Wall -Werror
PREFIX = /usr

all: newpid newpid.1

newpid: newpid.o

newpid.1: newpid.pod
	pod2man --center "" -r "" --quotes=none --section 1 $< > $@

install: newpid newpid.1
	install -d $(DESTDIR)$(PREFIX)/bin
	install newpid $(DESTDIR)$(PREFIX)/bin/newpid
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install newpid.1 $(DESTDIR)$(PREFIX)/share/man/man1/newpid.1

clean:
	rm -f newpid newpid.o newpid.1 z.out
