#ifndef DENSE_PAGE_TABLE_H
#define DENSE_PAGE_TABLE_H

#include "PageTableEntry.h"
#include "stdint.h"

typedef struct {
  PageTableEntry *entries;
  int size;
  int front;
  int rear;
  int count;
} DensePageTable;

DensePageTable *dense_page_table_create(int num_pages);
int dense_page_table_access_page(DensePageTable *page_table, uint32_t page);

// Removes page at index and add a new in queue
void dense_page_table_remove_page_at(DensePageTable *page_table, int index);

void dense_page_table_add_page(DensePageTable *page_table, uint32_t page);
int dense_page_table_dequeue(DensePageTable *page_table);

#endif
