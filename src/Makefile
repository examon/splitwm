CFLAGS = -lX11 -std=c99 $(OPT)
CC = gcc
SRC = splitwm.c mouse.c tile.c desktop.c draw.c client.c grab.c event.c view.c dbg.c
OBJ = $(SRC:.c=.o)
EXEC = splitwm
OPT = -Os

cleanwn: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) 
	@echo "done: $(EXEC)"

clean:
	@echo "cleaning"
	rm -f $(EXEC) *.o
	@echo "done"
