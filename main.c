#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DensePageTable.h"
#include "PageTableEntry.h"

#define MAX_FILENAME 256

int clock_time = 0;

#define MAX_FILENAME 256

typedef enum { LRU, SECOND_CHANCE, FIFO, RANDOM } Algorithm;

typedef struct {
  Algorithm algorithm;
  char filename[MAX_FILENAME];
  int page_size;   // KB
  int memory_size; // KB
  int debug_mode;
} Config;

typedef struct {
  int total_memory_accesses;
  int page_faults;
  int dirty_pages_written;
} Stats;

uint32_t decode_page_from_address(uint32_t address, Config *config) {
  uint32_t tmp = config->page_size;
  uint32_t s = 0;

  while (tmp > 1) {
    tmp = tmp >> 1;
    s++;
  }

  return address >> s;
}

int dense_page_table_swap_with_lru(DensePageTable *page_table) {
  for (int i = 0; i < page_table->count; i++) {
    int index = (page_table->front + i) % page_table->size;

    if (!page_table->entries[index].referenced) {
      return index;
    }
  }

  printf("Failed in LRU Algorithm, cant find a valid entry to swap\n");
  return -1;
}

int dense_page_table_swap_with_2a(DensePageTable *page_table) {
  for (int i = 0; i < page_table->count; i++) {
    int index = page_table->front;

    if (page_table->entries[index].referenced) {
      int page = dense_page_table_dequeue(page_table);
      dense_page_table_add_page(page_table, page);
    } else {
      return 0;
    }
  }

  printf(
      "Failed in SECOND_CHANCE Algorithm, cant find a valid entry to swap\n");
  return -1;
}

int dense_page_table_swap_with_fifo(DensePageTable *page_table) { return 0; }

int dense_page_tableswap_with_random(DensePageTable *page_table) {
  int index = rand() % page_table->count;
  return index;
}

// Simulate a single access in memory
void MMU(uint32_t address, DensePageTable *page_table, Config *config,
         Stats *stats) {
  clock_time++;

  // Reset referenced bit every 30 clock for LRU algorithms
  if (config->algorithm == LRU && clock_time == 200) {
    clock_time = 0;
    for (int i = 0; i < page_table->size; i++) {
      page_table->entries[i].referenced = false;
    }
  }

  uint32_t page = decode_page_from_address(address, config);

  // Page is in table, success!
  if (dense_page_table_access_page(page_table, page) != -1) {
    return;
  }

  stats->page_faults++;

  if (page_table->count < page_table->size) {
    dense_page_table_add_page(page_table, page);
    return;
  }

  int target_id = -1;

  if (config->algorithm == FIFO) {
    target_id = dense_page_table_swap_with_fifo(page_table);
  }

  if (config->algorithm == SECOND_CHANCE) {
    target_id = dense_page_table_swap_with_2a(page_table);
  }

  if (config->algorithm == LRU) {
    target_id = dense_page_table_swap_with_lru(page_table);
  }

  if (config->algorithm == RANDOM) {
    target_id = dense_page_tableswap_with_random(page_table);
  }

  if (target_id == -1) {
    printf("Failed to find a valid page to swap\n");
    exit(1);
  }

  dense_page_table_remove_page_at(page_table, target_id);
  dense_page_table_add_page(page_table, page);
}

void parse_arguments(int argc, char **argv, Config *config);

int main(int argc, char **argv) {
  Config config;

  parse_arguments(argc, argv, &config);

  srand(1);

  Stats stats = {0};

  DensePageTable *page_table =
      dense_page_table_create(config.memory_size / config.page_size);

  FILE *file_input = fopen(config.filename, "r");

  uint32_t addr;
  char rw;

  while (fscanf(file_input, "%x %c", &addr, &rw) == 2) {
    MMU(addr, page_table, &config, &stats);
  }

  printf("Paginas Lidas: %d\n", stats.page_faults);

  return 0;
}

void parse_arguments(int argc, char **argv, Config *config) {
  if (argc < 5 || argc > 6) {
    fprintf(stderr,
            "Usage: %s <algorithm> <input_file> <page_size_kb> "
            "<memory_size_kb> [debug]\n",
            argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "lru") == 0)
    config->algorithm = LRU;
  else if (strcmp(argv[1], "2a") == 0)
    config->algorithm = SECOND_CHANCE;
  else if (strcmp(argv[1], "fifo") == 0)
    config->algorithm = FIFO;
  else if (strcmp(argv[1], "random") == 0)
    config->algorithm = RANDOM;
  else {
    fprintf(stderr, "Invalid replacement algorithm\n");
    exit(1);
  }

  strncpy(config->filename, argv[2], MAX_FILENAME - 1);
  config->page_size = atoi(argv[3]) * 1024;
  config->memory_size = atoi(argv[4]) * 1024;
  config->debug_mode = (argc == 6);
}
