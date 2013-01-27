CFLAGS = -lX11 -std=c99 $(OPT)
CC = gcc
SRC = splitwm.c mouse.c tile.c desktop.c draw.c client.c grab.c event.c view.c dbg.c
OBJ = $(SRC:.c=.o)
EXEC = splitwm
OPT = -Os
BINPREFIX = /usr/local/bin

cleanwn: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

clean:
	@echo "cleaning"
	rm -f $(EXEC) *.o

install:
	@echo "installing $(EXEC) to $(BINPREFIX)"
	@mkdir -p "$(BINPREFIX)"
	@cp $(EXEC) "$(BINPREFIX)"
	@chmod 755 "$(BINPREFIX)/$(EXEC)"

uninstall:
	@echo "removing $(EXEC) from $(BINPREFIX)"
	@rm -f "$(BINPREFIX)/$(EXEC)"
