CC = cc
CFLAGS = -g -O2 -Wall -Werror
PREFIX = /usr/local

newpid: newpid.o

install: newpid
	install newpid $(DESTDIR)$(PREFIX)/bin/newpid

clean:
	rm -f newpid newpid.o
