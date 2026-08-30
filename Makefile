CC = gcc
CFLAGS = -Wall -Wextra
TARGET = xkey
SOURCES = main.c core.c virtual.c utils.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) xkey.h
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

clean:
	rm -f $(TARGET) *.o
