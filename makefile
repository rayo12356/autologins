CC = gcc
CFLAGS = -c -fPIC
OBJ = main.o formater.o
TARGET = autolog

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) main.c

formater.o: formater.c
	$(CC) $(CFLAGS) formater.c

run: $(TARGET)
	./$(TARGET)

clear:
	rm -f *.o $(TARGET)

.PHONY: all clear run
