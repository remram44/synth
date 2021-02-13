CC = gcc 
CFLAGS = -g -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -pedantic -W -Wall -Wextra
LDLIBS = -lSDL -lpthread -lm

SRC = main.c audio.c
OBJ = $(SRC:.c=.o)

all: main

tetris:
	./main ./samples/tetris.txt

detail:
	./main ./samples/details.txt

instru:
	./main ./samples/instruments.txt

main: $(OBJ)

$(BIN): $(OBJ)

clean:
	$(RM) $(OBJ) main

.PHONY : all clean tetris instru detail
