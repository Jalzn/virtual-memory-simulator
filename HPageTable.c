#include "HPageTable.h"
#include <stdio.h>
#include <stdlib.h>

HPageTable *hpt_create(int l1_size, int l2_size, int total_frames,
                       Algorithm algorithm) {
  HPageTable *pt = (HPageTable *)malloc(sizeof(HPageTable));

  pt->level1 = (Level1Entry *)malloc(sizeof(Level1Entry) * l1_size);

  // Inicializa tabela de primeiro nível
  for (int i = 0; i < l1_size; i++) {
    pt->level1[i].present = false;
    pt->level1[i].entries = NULL;
  }

  pt->l1_size = l1_size;
  pt->l2_size = l2_size;
  pt->algorithm = algorithm;
  pt->total_frames = total_frames;
  pt->access = 0;
  pt->faults = 0;

  return pt;
}

// Funções auxiliares para algoritmos de substituição
int hpt_2a_algorithm(HPageTable *pt, int l1_idx) {
  static int pointer = 0;
  int init_pointer = pointer;
  Level2Entry *entries = pt->level1[l1_idx].entries;

  do {
    if (!entries[pointer].referenced && entries[pointer].valid) {
      int target = pointer;
      pointer = (pointer + 1) % pt->l2_size;
      return target;
    }
    if (entries[pointer].valid) {
      entries[pointer].referenced = false;
    }
    pointer = (pointer + 1) % pt->l2_size;
  } while (pointer != init_pointer);

  // Se todos estiverem referenciados, retorna o primeiro válido
  for (int i = 0; i < pt->l2_size; i++) {
    if (entries[i].valid) {
      return i;
    }
  }
  return 0;
}

int hpt_lru_swap(HPageTable *pt, int l1_idx) {
  int target_entry = -1;
  int last_access = 9999999;
  Level2Entry *entries = pt->level1[l1_idx].entries;

  for (int i = 0; i < pt->l2_size; i++) {
    if (entries[i].valid && entries[i].accessed_at < last_access) {
      last_access = entries[i].accessed_at;
      target_entry = i;
    }
  }

  return target_entry;
}

int hpt_fifo_swap(HPageTable *pt, int l1_idx) {
  int target_entry = -1;
  int last_created = 9999999;
  Level2Entry *entries = pt->level1[l1_idx].entries;

  for (int i = 0; i < pt->l2_size; i++) {
    if (entries[i].valid && entries[i].created_at < last_created) {
      last_created = entries[i].created_at;
      target_entry = i;
    }
  }

  return target_entry;
}

int hpt_random_swap(HPageTable *pt, int l1_idx) {
  Level2Entry *entries = pt->level1[l1_idx].entries;
  int valid_count = 0;

  // Conta entradas válidas
  for (int i = 0; i < pt->l2_size; i++) {
    if (entries[i].valid)
      valid_count++;
  }

  if (valid_count == 0)
    return 0;

  // Escolhe aleatoriamente entre as entradas válidas
  int target = rand() % valid_count;
  int current = 0;

  for (int i = 0; i < pt->l2_size; i++) {
    if (entries[i].valid) {
      if (current == target)
        return i;
      current++;
    }
  }

  return 0;
}

void hpt_access_page(HPageTable *pt, int page_id, int clock) {
  pt->access++;

  // Calcula índices para as tabelas de primeiro e segundo nível
  int l1_idx = page_id / pt->l2_size;
  int l2_idx = page_id % pt->l2_size;

  // Verifica se a tabela de segundo nível existe
  if (!pt->level1[l1_idx].present) {
    pt->level1[l1_idx].entries =
        (Level2Entry *)malloc(sizeof(Level2Entry) * pt->l2_size);
    pt->level1[l1_idx].present = true;

    // Inicializa entradas do segundo nível
    for (int i = 0; i < pt->l2_size; i++) {
      pt->level1[l1_idx].entries[i].valid = false;
      pt->level1[l1_idx].entries[i].referenced = false;
      pt->level1[l1_idx].entries[i].frame = 0;
      pt->level1[l1_idx].entries[i].accessed_at = 0;
      pt->level1[l1_idx].entries[i].created_at = 0;
    }
  }

  // Verifica se a página já está mapeada
  if (pt->level1[l1_idx].entries[l2_idx].valid) {
    pt->level1[l1_idx].entries[l2_idx].referenced = true;
    pt->level1[l1_idx].entries[l2_idx].accessed_at = clock;
    return;
  }

  pt->faults++;

  // Procura por uma entrada livre no segundo nível
  int free_entry = -1;
  for (int i = 0; i < pt->l2_size; i++) {
    if (!pt->level1[l1_idx].entries[i].valid) {
      free_entry = i;
      break;
    }
  }

  // Se encontrou entrada livre, usa ela
  if (free_entry != -1) {
    pt->level1[l1_idx].entries[free_entry].valid = true;
    pt->level1[l1_idx].entries[free_entry].referenced = false;
    pt->level1[l1_idx].entries[free_entry].frame =
        pt->faults % pt->total_frames; // Simples alocação de frame
    pt->level1[l1_idx].entries[free_entry].accessed_at = clock;
    pt->level1[l1_idx].entries[free_entry].created_at = clock;
    return;
  }

  // Se não encontrou entrada livre, precisa fazer swap
  int target_entry = -1;

  if (pt->algorithm == SECOND_CHANCE) {
    target_entry = hpt_2a_algorithm(pt, l1_idx);
  } else if (pt->algorithm == LRU) {
    target_entry = hpt_lru_swap(pt, l1_idx);
  } else if (pt->algorithm == FIFO) {
    target_entry = hpt_fifo_swap(pt, l1_idx);
  } else if (pt->algorithm == RANDOM) {
    target_entry = hpt_random_swap(pt, l1_idx);
  } else {
    printf("Falha ao detectar algoritmo\n");
    exit(1);
  }

  if (target_entry == -1) {
    printf("Falha ao realizar o algoritmo de swap.\n");
    exit(1);
  }

  // Atualiza a entrada escolhida
  pt->level1[l1_idx].entries[target_entry].valid = true;
  pt->level1[l1_idx].entries[target_entry].referenced = false;
  pt->level1[l1_idx].entries[target_entry].frame =
      pt->faults % pt->total_frames;
  pt->level1[l1_idx].entries[target_entry].accessed_at = clock;
  pt->level1[l1_idx].entries[target_entry].created_at = clock;
}
