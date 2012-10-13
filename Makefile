CFLAGS+= -Wall
LDFLAGS=-lX11
EXEC=cleanwm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

cleanwn: cleanwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+

clean:
	rm -f cleanwm *.o
