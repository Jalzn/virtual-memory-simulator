#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILENAME 256

typedef enum { LRU, SECOND_CHANCE, FIFO, RANDOM } Algorithm;

typedef struct {
  Algorithm algorithm;
  char filename[MAX_FILENAME];
  int page_size;   // KB
  int memory_size; // KB
  int debug_mode;
} Config;

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

int decode_page_from_address(int address, int page_size) {
  int tmp = page_size;
  int s = 0;

  while (tmp > 1) {
    tmp = tmp >> 1;
    s++;
  }

  return address >> s;
}

typedef struct Entry {
  int page;
  int frame;
  bool referenced;
  struct Entry *next;
} Entry;

Entry *create_entry(int page) {
  Entry *e = (Entry *)malloc(sizeof(Entry));
  e->page = page;
  e->frame = 0;
  e->referenced = false;
  e->next = NULL;
  return e;
}

typedef struct PageTable {
  Entry *root;
  int size;
} PageTable;

bool pt_access_page(PageTable *pt, int page) {
  Entry *root = pt->root;

  for (int i = 0; i < pt->size; i++) {
    if (root->page == page) {
      root->referenced = true;
      return true;
    }

    root = root->next;
  }

  return false;
}

void pt_enqueue(PageTable *pt, int page) {
  if (pt->size == 0) {
    pt->root = create_entry(page);
    pt->size++;
    return;
  }

  Entry *root = pt->root;

  while (root->next != NULL) {
    root = root->next;
  }

  root->next = create_entry(page);
  pt->size++;
}

Entry pt_dequeue(PageTable *pt) {
  Entry e;

  e.page = pt->root->page;
  e.frame = pt->root->frame;
  e.next = NULL;

  pt->root = pt->root->next;

  pt->size--;

  return e;
}

int pt_find_index(PageTable *pt, int page) {
  Entry *root = pt->root;

  for (int i = 0; i < pt->size; i++) {
    if (root->page == page) {
      return i;
    }

    root = root->next;
  }

  return -1;
}

void pt_remove_at(PageTable *pt, int index) {
  if (index < 0 || index > pt->size) {
    printf("Invalid index to remove from page table\n");
    exit(1);
  }

  Entry *prev = NULL;
  Entry *current = pt->root;

  if (index == 0) {
    pt->root = current->next;
    free(current);
    pt->size--;
    return;
  }

  for (int i = 0; i < index; i++) {
    prev = current;
    current = current->next;
  }

  prev->next = current->next;
  free(current);
  pt->size--;
}

int main(int argc, char **argv) {
  srand(1);

  Config config;
  int page_faults = 0;

  parse_arguments(argc, argv, &config);

  PageTable page_table;

  page_table.root = NULL;
  page_table.size = 0;

  FILE *file_input = fopen(config.filename, "r");

  int addr;
  char rw;

  clock_t start = clock();
  while (fscanf(file_input, "%x %c", &addr, &rw) == 2) {
    // Tenta acessar pagina
    int page = decode_page_from_address(addr, config.page_size);

    bool found = pt_access_page(&page_table, page);

    // FIFO Algorithm
    if (!found) {
      page_faults++;

      if (page_table.size < config.memory_size / config.page_size) {
        pt_enqueue(&page_table, page);
        continue;
      }

      if (config.algorithm == SECOND_CHANCE) {
        Entry *root = page_table.root;

        for (int i = 0; i < page_table.size; i++) {
          if (root->referenced) {
            int page = root->page;
            root = root->next;
            pt_remove_at(&page_table, i);
            pt_enqueue(&page_table, page);
          } else {
            pt_remove_at(&page_table, i);
            pt_enqueue(&page_table, page);
          }
        }

      } else if (config.algorithm == LRU) {
        Entry *root = page_table.root;

        for (int i = 0; i < page_table.size; i++) {
          if (!root->referenced) {
            root->page = page;
            root->frame = 1;
            root->referenced = 0;
            break;
          }

          root = root->next;
        }
      } else if (config.algorithm == FIFO) {
        pt_dequeue(&page_table);
        pt_enqueue(&page_table, page);
      } else if (config.algorithm == RANDOM) {
        int random_index = rand() % page_table.size;
        pt_remove_at(&page_table, random_index);
        pt_enqueue(&page_table, page);
      }
    }
  }

  clock_t end = clock();

  printf("Page faults: %d\n", page_faults);
  printf("Tempo gasto: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

  return 0;
}
