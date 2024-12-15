CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

TARGET = tp2virtual
SOURCES = main.c DensePageTable.c
HEADERS = DensePageTable.h PageTableEntry.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) ${HEADERS}
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
