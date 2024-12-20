#ifndef IPAGE_TABLE_H
#define IPAGE_TABLE_H

#include <stdbool.h>

#include "Types.h"

typedef struct {
  int page;
  int accessed_at;
  int created_at;
  bool referenced;
  bool free;
} IPageEntry;

typedef struct {
  IPageEntry *frames;
  int total;
  Algorithm algorithm;
  int access;
  int faults;
} IPageTable;

IPageTable *ipt_create(int num_frames, Algorithm algorithm);
void ipt_access_page(IPageTable *pt, int page_id, int clock);

#endif
