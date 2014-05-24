#Makefile
TARGET = yvonne-remote
OBJECTS = yvonne-remote.o yvonne-remote-lib.o

CFLAGS = -O2 -Wall -g
CC = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

