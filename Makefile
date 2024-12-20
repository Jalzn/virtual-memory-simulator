CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

TARGET = tp2virtual
SOURCES = main.c DPageTable.c IPageTable.c HPageTable.c
HEADERS = DPageTable.h IPageTable.h HPageTable.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) ${HEADERS}
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
