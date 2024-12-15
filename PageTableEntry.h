#ifndef PAGE_TABLE_ENTRY_H
#define PAGE_TABLE_ENTRY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t page;
  uint32_t frame;
  bool referenced;
} PageTableEntry;

#endif
