#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
  uint32_t page;
  uint32_t frame;
  uint32_t inserted_at;
  uint32_t referenced_at;
} PageTableEntry;

typedef struct {
  PageTableEntry *entries;
  int total;
} DensePageTable;

uint32_t decode_page_from_address(uint32_t address, Config *config) {
  uint32_t tmp = config->page_size;
  uint32_t s = 0;

  while (tmp > 1) {
    tmp = tmp >> 1;
    s++;
  }

  return address >> s;
}

DensePageTable *create_dense_page_table(Config *config) {
  DensePageTable *page_table = (DensePageTable *)malloc(sizeof(DensePageTable));

  int num_pages = config->memory_size / config->page_size;

  page_table->entries = malloc(sizeof(PageTableEntry) * num_pages);
  for (int i = 0; i < num_pages; i++) {
    page_table->entries[i].page = 0;
    page_table->entries[i].frame = 0;
    page_table->entries[i].inserted_at = clock_time;
    page_table->entries[i].referenced_at = clock_time;
  }

  page_table->total = num_pages;

  return page_table;
}

PageTableEntry *get_page_table_entry(DensePageTable *page_table,
                                     uint32_t page) {
  for (int i = 0; i < page_table->total; i++) {
    PageTableEntry *entry = &page_table->entries[i];

    if (entry->page == page) {
      entry->referenced_at = clock_time;
      return entry;
    }
  }
  return NULL;
}

PageTableEntry *find_free_page_table_entry(DensePageTable *page_table) {
  for (int i = 0; i < page_table->total; i++) {
    if (page_table->entries[i].frame == 0) {
      return &page_table->entries[i];
    }
  }
  return NULL;
}

PageTableEntry *swap_with_lru(DensePageTable *page_table) {
  int index = -1;
  uint32_t oldest_time = 3999999999;

  for (int i = 0; i < page_table->total; i++) {
    if (page_table->entries[i].referenced_at < oldest_time) {
      oldest_time = page_table->entries[i].referenced_at;
      index = i;
    }
  }

  if (index == -1) {
    printf("Failed in LRU Algorithm, id is less than 0\n");
    exit(1);
  }

  return &page_table->entries[index];
}
PageTableEntry *swap_with_fifo(DensePageTable *page_table) {
  int index = -1;
  uint32_t oldest_time = 3999999999;
  for (int i = 0; i < page_table->total; i++) {
    if (page_table->entries[i].inserted_at < oldest_time) {
      oldest_time = page_table->entries[i].inserted_at;
      index = i;
    }
  }

  if (index == -1) {
    printf("Failed in FIFO Algorithm, id is less than 0\n");
    exit(1);
  }

  return &page_table->entries[index];
}

PageTableEntry *swap_with_random(DensePageTable *page_table) {
  int index = rand() % page_table->total;
  return &page_table->entries[index];
}

// Simulate a single access in memory
void MMU(uint32_t address, DensePageTable *page_table, Config *config,
         Stats *stats) {
  // Increments virtual clock
  clock_time++;

  if (config->debug_mode) {
    printf("[MMU] Address %x, Mode: %d\n", address, 0);
  }

  uint32_t page = decode_page_from_address(address, config);

  PageTableEntry *entry = get_page_table_entry(page_table, page);

  if (entry) {
    return;
  }

  if (config->debug_mode) {
    printf("\tCould not locate page (%x) in table\n", page);
  }

  if (config->debug_mode) {
    printf("\tTrying to allocate new page\n");
  }

  // TODO: Only find free entry if condition
  entry = find_free_page_table_entry(page_table);

  if (entry != NULL && config->debug_mode) {
    printf("\tFound a empty page in table: (%p)\n", entry);
    return;
  }

  // Should realocate a page
  stats->page_faults++;

  if (config->algorithm == FIFO) {
    entry = swap_with_fifo(page_table);
  }

  if (config->algorithm == LRU) {
    entry = swap_with_lru(page_table);
  }

  if (config->algorithm == RANDOM) {
    entry = swap_with_random(page_table);
  }

  entry->page = page;
  entry->frame = 1;
  entry->inserted_at = clock_time;
  entry->referenced_at = clock_time;
}

void parse_arguments(int argc, char **argv, Config *config);

int main(int argc, char **argv) {
  Config config;

  parse_arguments(argc, argv, &config);

  srand(1);

  Stats stats = {0};

  DensePageTable *page_table = create_dense_page_table(&config);

  if (config.debug_mode) {
    printf("Initing virtual memory simulator\n");
    printf("Total page table entries available: %d\n", page_table->total);

    switch (config.algorithm) {
    case FIFO: {
      printf("Algorithm selected: FIFO\n");
      break;
    }
    case RANDOM: {
      printf("Algorithm selected: RANDOM\n");
      break;
    }
    default: {
      printf("Failed to detect algorithm\n");
      exit(1);
    }
    }
  }

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
