CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

TARGET = tp2virtual
SOURCES = main.c
HEADERS =

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) ${HEADERS}
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
