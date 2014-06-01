#Makefile
TARGET = yvonne-remote
OBJECTS = yvonne-remote.o yvonne-remote-lib.o

CFLAGS = -O2 -Wall -g
CC = gcc

REMOVE = rm -f

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


# Target: clean project.
clean:
	$(REMOVE) $(TARGET) $(OBJECTS)
