#include "DPageTable.h"
#include "Types.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

DPageTable *dpt_create(int num_pages, int num_frames, Algorithm algorithm) {
  DPageTable *pt = (DPageTable *)malloc(sizeof(DPageTable));

  pt->pages = (DPageEntry *)malloc(sizeof(DPageEntry) * num_pages);
  for (int i = 0; i < num_pages; i++) {
    pt->pages[i].frame = 0;
    pt->pages[i].valid = false;
  }

  pt->frames = (DFrameEntry *)malloc(sizeof(DFrameEntry) * num_frames);
  for (int i = 0; i < num_frames; i++) {
    pt->frames[i].page = 0;
    pt->frames[i].created_at = 0;
    pt->frames[i].accessed_at = 0;
    pt->frames[i].free = true;
    pt->frames[i].referenced = false;
  }

  pt->algorithm = algorithm;
  pt->total = num_pages;
  pt->num_frames = num_frames;
  pt->access = 0;
  pt->faults = 0;
  return pt;
}

int dpt_2a_swap(DPageTable *pt) {
  static int current_position = 0; // Mantém a posição do ponteiro circular

  // Primeira passagem: procura por frames não referenciados
  int start_position = current_position;
  do {
    // Verifica se o frame atual não está referenciado
    if (!pt->frames[current_position].referenced) {
      int target_frame = current_position;
      // Move o ponteiro para a próxima posição
      current_position = (current_position + 1) % pt->num_frames;
      return target_frame;
    }
    // Se está referenciado, dá uma segunda chance
    pt->frames[current_position].referenced = false;
    // Move o ponteiro para o próximo frame
    current_position = (current_position + 1) % pt->num_frames;
  } while (current_position != start_position);

  // Se todos os frames estavam referenciados e receberam segunda chance,
  // retorna o frame atual (política FIFO)
  int target_frame = current_position;
  // Move o ponteiro para a próxima posição
  current_position = (current_position + 1) % pt->num_frames;
  return target_frame;
}

int dpt_lru_swap(DPageTable *pt) {
  int target_frame = -1;
  int last_access = 9999999;

  for (int i = 0; i < pt->num_frames; i++) {
    if (pt->frames[i].accessed_at < last_access) {
      last_access = pt->frames[i].accessed_at;
      target_frame = i;
    }
  }

  return target_frame;
}

int dpt_fifo_swap(DPageTable *pt) {
  int target_frame = -1;
  int last_created = 9999999;

  for (int i = 0; i < pt->num_frames; i++) {
    if (pt->frames[i].created_at < last_created) {
      last_created = pt->frames[i].created_at;
      target_frame = i;
    }
  }

  return target_frame;
}

int dpt_random_swap(DPageTable *pt) {
  int target_frame = rand() % pt->num_frames;
  return target_frame;
}

void dpt_access_page(DPageTable *pt, int page_id, int clock) {
  pt->access++;

  if (pt->pages[page_id].valid) {
    if (pt->frames[pt->pages[page_id].frame].page != page_id) {
      printf("Accessing invalid frame\n");
      printf("\tPage (%d): [frame: %d]\n", page_id, pt->pages[page_id].frame);
      printf("\tFrame (%d): [page: %d]\n", pt->pages[page_id].frame,
             pt->frames[pt->pages[page_id].frame].page);
      exit(1);
    }
    pt->frames[pt->pages[page_id].frame].referenced = true;
    pt->frames[pt->pages[page_id].frame].accessed_at = clock;
    return;
  }

  pt->faults++;

  int free_frame = -1;
  for (int i = 0; i < pt->num_frames; i++) {
    if (pt->frames[i].free) {
      free_frame = i;
      break;
    }
  }

  // Nao precisa fazer swap
  if (free_frame != -1) {
    pt->pages[page_id].frame = free_frame;
    pt->pages[page_id].valid = true;

    pt->frames[free_frame].page = page_id;
    pt->frames[free_frame].created_at = clock;
    pt->frames[free_frame].accessed_at = clock;
    pt->frames[free_frame].free = false;
    pt->frames[free_frame].referenced = false;
    return;
  }

  // Faz um swap baseado no algoritmo escolhido
  int target_frame = -1;

  if (pt->algorithm == SECOND_CHANCE) {
    target_frame = dpt_2a_swap(pt);
  } else if (pt->algorithm == LRU) {
    target_frame = dpt_lru_swap(pt);
  } else if (pt->algorithm == FIFO) {
    target_frame = dpt_fifo_swap(pt);
  } else if (pt->algorithm == RANDOM) {
    target_frame = dpt_random_swap(pt);
  } else {
    printf("Falha ao detectar algoritmo\n");
    exit(1);
  }

  if (target_frame == -1) {
    printf("Falha ao realizar o algortimo de swap.\n");
    exit(1);
  }

  int old_page = pt->frames[target_frame].page;

  pt->frames[target_frame].page = page_id;
  pt->frames[target_frame].created_at = clock;
  pt->frames[target_frame].accessed_at = clock;
  pt->frames[target_frame].free = false;
  pt->frames[free_frame].referenced = false;

  pt->pages[old_page].valid = false;

  pt->pages[page_id].frame = target_frame;
  pt->pages[page_id].valid = true;
}
