CC=cc
DEBUG=-ggdb -O0
PROD=-O3
CFLAGS=-I. -W -Wall -Wextra $(PROD)

TARG=cmenu
DEPS=term.h buf.h
OBJS=rawm.o ctrl.o buf.o $(TARG).o

MAN=cmenu.1

PREFIX=/usr/local
MANPREFIX=$(PREFIX)/man
BINPREFIX=$(PREFIX)/bin

.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS)

clean:
	rm $(TARG)
	rm *.o
	rm *.core

install: $(TARG)
	mkdir -p $(MANPREFIX)
	cp -f $(MAN) $(MANPREFIX)/man1
	install $(TARG) $(BINPREFIX)/$(TARG)

install-dired:
	install dired $(BINPREFIX)/dired

uninstall: install
	rm $(BINPREFIX)/$(TARG)

.PHONY: clean
