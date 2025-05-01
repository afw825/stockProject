CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LIBS = -lcurl -lm -g

SRC = project.c stock_analyzer.c
OBJ = $(SRC:.c=.o)
EXEC = project

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJ) $(EXEC)
