#ifndef DPAGE_TABLE_H
#define DPAGE_TABLE_H

#include "Types.h"
#include <stdbool.h>

typedef struct {
  int frame;
  bool valid;
} DPageEntry;

typedef struct {
  int page;
  int accessed_at;
  int created_at;
  bool free;
  bool referenced;
} DFrameEntry;

typedef struct {
  DPageEntry *pages;
  int total;
  DFrameEntry *frames;
  Algorithm algorithm;
  int num_frames;
  int access;
  int faults;
} DPageTable;

DPageTable *dpt_create(int num_pages, int num_frames, Algorithm algorithm);

void dpt_access_page(DPageTable *pt, int page, int clock);

#endif
