#Makefile

#Variable declaration
TARGET = yvonne-remote
OBJECTS = yvonne-remote.o yvonne-remote-lib.o

VERSIONFILE = installed.version

CFLAGS = -O2 -Wall -g
CFLAGS += `pkg-config --cflags MagickWand`
CFLAGS += `pkg-config --cflags libgphoto2`

LDFLAGS = `pkg-config --libs MagickWand`
LDFLAGS += `pkg-config --libs libgphoto2`

CC = gcc

RM = rm -f

prefix = /usr/local
bindir = $(prefix)/bin
#End of variable declaration

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^


# clean the project
clean:
	$(RM) $(TARGET) $(OBJECTS)

# install the project
install:
	cp $(TARGET) $(bindir)
	git log -1 --abbrev-commit --pretty=oneline > $(VERSIONFILE)

#unsintall the project
uninstall:
	$(RM) $(bindir)/$(TARGET)
	rm -f $(VERSIONFILE)
