cflags+= -std=c99 -Wall -Werror -pedantic -Os
LDFLAGS=-lX11
EXEC=splitwm
CC=cc

all: $(EXEC)

cleanwn: splitwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+

clean:
	rm -f splitwm *.o
