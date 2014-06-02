#Makefile

#Variable declaration
TARGET = yvonne-remote
OBJECTS = yvonne-remote.o yvonne-remote-lib.o

CFLAGS = -O2 -Wall -g
CC = gcc

RM = rm -f

prefix = /usr/local
bindir = $(prefix)/bin
#End of variable declaration

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


# clean the project
clean:
	$(RM) $(TARGET) $(OBJECTS)
	
# install the project
install:
	cp $(TARGET) $(bindir)
  
#unsintall the project
uninstall:
	$(RM) $(bindir)/$(TARGET)
