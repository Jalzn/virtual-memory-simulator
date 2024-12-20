#ifndef HPAGE_TABLE_H
#define HPAGE_TABLE_H

#include "Types.h"
#include <stdbool.h>

// Entrada da tabela de segundo nível
typedef struct {
  bool valid;
  bool referenced;
  int frame;
  int accessed_at;
  int created_at;
} Level2Entry;

// Entrada da tabela de primeiro nível
typedef struct {
  bool present;
  Level2Entry *entries;
} Level1Entry;

// Tabela de páginas hierárquica
typedef struct {
  Level1Entry *level1;
  int l1_size;
  int l2_size;
  Algorithm algorithm;
  int total_frames;
  int access;
  int faults;
} HPageTable;

HPageTable *hpt_create(int l1_size, int l2_size, int total_frames,
                       Algorithm algorithm);
void hpt_access_page(HPageTable *pt, int page_id, int clock);

#endif
