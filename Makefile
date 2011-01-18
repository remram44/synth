CC = gcc -pedantic -W -Wall -Wextra
OBJ = main.o audio.o
BIN = audio
CFLAGS = -g
LIBS = -g -lSDL
#RM = rm -f
RM = del /F

.PHONY : all clean

all: $(BIN)


$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJ)
