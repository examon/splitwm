cflags+= -std=c99 -Wall -pedantic -Os
LDFLAGS=-lX11
EXEC=cleanwm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=cc

all: $(EXEC)

cleanwn: cleanwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+

clean:
	rm -f cleanwm *.o
