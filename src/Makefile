CFLAGS = -lX11 -std=c99 $(OPT)
CC = gcc
OPT = -Os
SRC = splitwm.c mouse.c tile.c desktop.c draw.c client.c grab.c event.c view.c dbg.c
OBJ = $(SRC:.c=.o)
EXEC = splitwm
PREFIX ?= /usr/local

cleanwn: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

clean:
	@echo cleaning
	rm -f $(EXEC) *.o

install:
	@echo installing $(EXEC) to $(DESTDIR)$(PREFIX)/bin
	@install -Dm755 ${EXEC} $(DESTDIR)$(PREFIX)/bin/${EXEC}

uninstall:
	@echo removing $(EXEC) from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(EXEC)
