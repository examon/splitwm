cflags+= -std=c99 -Wall -Werror -pedantic -Os
LDFLAGS=-lX11
EXEC=cleanwm
CC=cc

all: $(EXEC)

cleanwn: cleanwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+

clean:
	rm -f cleanwm *.o
