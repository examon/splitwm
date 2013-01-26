CFLAGS = -lX11 -lXtst -std=c99 $(OPT)
CC = gcc
SRC = xfakeevent.c
OBJ = $(SRC:.c=.o)
EXEC = xfakeevent
OPT = -Os

cleanwn: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) 
	@echo "done: $(EXEC)"

clean:
	@echo "cleaning"
	rm -f $(EXEC) *.o
	@echo "done"
