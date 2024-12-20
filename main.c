#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DPageTable.h"
#include "HPageTable.h"
#include "IPageTable.h"
#include "Types.h"

#define MAX_FILENAME 256

int clock_time = 0;

#define MAX_FILENAME 256

typedef struct {
  Algorithm algorithm;
  char filename[MAX_FILENAME];
  int page_size;   // KB
  int memory_size; // KB
  int debug_mode;
} Config;

int decode_page_from_address(uint32_t address, Config *config) {
  uint32_t tmp = config->page_size;
  uint32_t s = 0;

  while (tmp > 1) {
    tmp = tmp >> 1;
    s++;
  }

  int total_bits = 32;

  // Calcula o número de bits a descartar à direita
  int shift = total_bits - s;

  return address >> shift;
}

int derivar_valor_s(int page_size) {
  int tmp = page_size;
  int s = 0;

  while (tmp > 1) {
    tmp = tmp >> 1;
    s++;
  }

  return s;
}

void parse_arguments(int argc, char **argv, Config *config);

int main(int argc, char **argv) {
  Config config;

  parse_arguments(argc, argv, &config);

  srand(1);

  int num_pages = pow(2, derivar_valor_s(config.page_size));

  DPageTable *dpt = dpt_create(num_pages, config.memory_size / config.page_size,
                               config.algorithm);

  int num_pages_per_level = pow(2, derivar_valor_s(config.page_size) / 2);
  HPageTable *hpt =
      hpt_create(num_pages_per_level, num_pages_per_level,
                 config.memory_size / config.page_size, config.algorithm);

  IPageTable *ipt =
      ipt_create(config.memory_size / config.page_size, config.algorithm);

  FILE *file_input = fopen(config.filename, "r");

  uint32_t addr;
  char rw;

  while (fscanf(file_input, "%x %c", &addr, &rw) == 2) {
    clock_time++;
    int page_id = decode_page_from_address(addr, &config);
    dpt_access_page(dpt, page_id, clock_time);
    hpt_access_page(hpt, page_id, clock_time);
    ipt_access_page(ipt, page_id, clock_time);
  }

  printf("Usando a tabela densa\n");
  printf("Acessos a memoria: %d\n", dpt->access);
  printf("Paginas Lidas: %d\n", dpt->faults);
  printf("Tamanho da tabela: %d\n", dpt->total);

  printf("\n");
  printf("Usando a tabela invertida\n");
  printf("Acessos a memoria: %d\n", hpt->access);
  printf("Paginas Lidas: %d\n", hpt->faults);
  // printf("Tamanho da tabela: %d\n", ipt->total);

  printf("\n");
  printf("Usando a tabela invertida\n");
  printf("Acessos a memoria: %d\n", ipt->access);
  printf("Paginas Lidas: %d\n", ipt->faults);
  printf("Tamanho da tabela: %d\n", ipt->total);
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
