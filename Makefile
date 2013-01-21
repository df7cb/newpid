CC = cc
CFLAGS = -g -O2 -Wall -Werror
PREFIX = /usr

newpid: newpid.o

install: newpid
	install -d $(DESTDIR)$(PREFIX)/bin
	install newpid $(DESTDIR)$(PREFIX)/bin/newpid

clean:
	rm -f newpid newpid.o
