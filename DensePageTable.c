#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "DensePageTable.h"
#include "PageTableEntry.h"

DensePageTable *dense_page_table_create(int num_pages) {
  DensePageTable *page_table = (DensePageTable *)malloc(sizeof(DensePageTable));

  page_table->entries =
      (PageTableEntry *)malloc(sizeof(PageTableEntry) * num_pages);

  page_table->size = num_pages;
  page_table->front = 0;
  page_table->rear = -1;
  page_table->count = 0;

  return page_table;
}

int dense_page_table_access_page(DensePageTable *page_table, uint32_t page) {
  for (int i = 0; i < page_table->count; i++) {
    int index = (page_table->front + i) & page_table->size;

    if (page_table->entries[index].page == page) {
      page_table->entries[index].referenced = true;
      return index;
    }
  }

  return -1;
}

int dense_page_table_dequeue(DensePageTable *page_table) {
  int page = page_table->entries[page_table->front].page;

  page_table->front = (page_table->front + 1) % page_table->size;
  page_table->count--;

  return page;
}

void dense_page_table_remove_page_at(DensePageTable *page_table, int index) {
  // Verifica se a lista está vazia
  if (page_table->count == 0) {
    printf("Erro: Tabela de paginas vazia\n");
    return;
  }

  // Verifica se o índice é válido
  if (index < 0 || index >= page_table->count) {
    printf("Erro: Indice invalido\n");
    return;
  }

  // Calcula o índice real no array circular
  // int realIndex = (page_table->front + index) % page_table->size;

  // Shift dos elementos após o índice removido
  for (int i = index; i < page_table->count - 1; i++) {
    int currentRealIndex = (page_table->front + i) % page_table->size;
    int nextRealIndex = (page_table->front + i + 1) % page_table->size;
    page_table->entries[currentRealIndex] = page_table->entries[nextRealIndex];
  }

  // Decrementa o contador de elementos
  page_table->count--;
}

void dense_page_table_add_page(DensePageTable *page_table, uint32_t page) {
  if (page_table->count >= page_table->size) {
    printf("Erro: Lista cheia\n");
  }

  int index = (page_table->front + page_table->count) % page_table->size;

  page_table->entries[index].page = page;
  page_table->entries[index].frame = 1;
  page_table->entries[index].referenced = 0;

  page_table->count++;
}
