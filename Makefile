#Makefile

#Variable declaration
TARGET = yvonne-remote
SRC= yvonne-remote-lib.c \
	 yvonne-remote-ui.c
#OBJECTS=$(subst .c,.o,$(SRC)) // https://stackoverflow.com/questions/20514587/text0x20-undefined-reference-to-main-and-undefined-reference-to-function
OBJECTS = yvonne-remote.o yvonne-remote-lib.o yvonne-remote-ui.o

VERSIONFILE = installed.version

CFLAGS = -O2 -Wall -g
CFLAGS += `pkg-config --cflags MagickWand`
CFLAGS += `pkg-config --cflags libgphoto2`

LDFLAGS = `pkg-config --libs MagickWand`
LDFLAGS += `pkg-config --libs libgphoto2`
LDFLAGS += -lncurses

# Compilateur C Ansi :
CC = gcc
AR = @ar –rv
RMOBJ = @rm –f $(OBJ)/ *.o
ECHO = @echo
MKREP = @mkdir $(TARGETDIR)
MKDROIT = @chmod u= rwx
RM = rm -f

prefix = /usr/local
bindir = $(prefix)/bin
#End of variable declaration

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^

# clean the project
clean:
	$(RM) $(TARGET)
	$(RM) $(OBJECTS)

# install the project
install:
	cp $(TARGET) $(bindir)
	git log -1 --abbrev-commit --pretty=oneline > $(VERSIONFILE)

#unsintall the project
uninstall:
	$(RM) $(bindir)/$(TARGET)
	rm -f $(VERSIONFILE)
