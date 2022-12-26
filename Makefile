CC=cc
DEBUG=-ggdb -O0
PROD=-O3
CFLAGS=-I. -W -Wall -Wextra $(PROD)

TARG=cmenu
DEPS=term.h buf.h
OBJS=rawm.o ctrl.o $(TARG).o

PREFIX=~/.local/bin
#PREFIX=/usr/local/bin

.o: $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS)

clean:
	rm *.o
	rm $(TARG)

install: $(TARG)
	install $(TARG) $(PREFIX)/$(TARG)

uninstall: install
	rm $(PREFIX)/$(TARG)

.PHONY: clean
