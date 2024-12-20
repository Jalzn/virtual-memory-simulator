#include "IPageTable.h"
#include "Types.h"

#include <stdio.h>
#include <stdlib.h>

IPageTable *ipt_create(int num_frames, Algorithm algorithm) {
  IPageTable *pt = (IPageTable *)malloc(sizeof(IPageTable));

  pt->frames = (IPageEntry *)malloc(sizeof(IPageEntry) * num_frames);

  for (int i = 0; i < num_frames; i++) {
    pt->frames[i].page = 0;
    pt->frames[i].accessed_at = 0;
    pt->frames[i].created_at = 0;
    pt->frames[i].referenced = false;
    pt->frames[i].free = true;
  }

  pt->total = num_frames;
  pt->algorithm = algorithm;
  pt->access = 0;
  pt->faults = 0;

  return pt;
}

int ipt_2a_algorithm(IPageTable *pt) {
  static int pointer = 0;
  int init_pointer = pointer;
  do {
    if (!pt->frames[pointer].referenced) {
      int target = pointer;
      pointer = (pointer + 1) % pt->total;
      return target;
    }
    pt->frames[pointer].referenced =
        false; // Corrigido: reseta o bit referenced
    pointer = (pointer + 1) % pt->total;
  } while (pointer != init_pointer);
  return init_pointer;
}

int ipt_lru_swap(IPageTable *pt) {
  int target_frame = -1;
  int last_access = 9999999;

  for (int i = 0; i < pt->total; i++) {
    if (pt->frames[i].accessed_at < last_access) {
      last_access = pt->frames[i].accessed_at;
      target_frame = i;
    }
  }

  return target_frame;
}

int ipt_fifo_swap(IPageTable *pt) {
  int target_frame = -1;
  int last_created = 9999999;

  for (int i = 0; i < pt->total; i++) {
    if (pt->frames[i].created_at < last_created) {
      last_created = pt->frames[i].created_at;
      target_frame = i;
    }
  }

  return target_frame;
}

int ipt_random_swap(IPageTable *pt) {
  int target_frame = rand() % pt->total;
  return target_frame;
}

void ipt_access_page(IPageTable *pt, int page_id, int clock) {
  pt->access++;
  for (int i = 0; i < pt->total; i++) {
    if (pt->frames[i].page == page_id && !pt->frames[i].free) {
      pt->frames[i].referenced = true;
      pt->frames[i].accessed_at = clock;
      return;
    }
  }

  pt->faults++;

  int free_frame = -1;
  for (int i = 0; i < pt->total; i++) {
    if (pt->frames[i].free) {
      free_frame = i;
      break;
    }
  }

  if (free_frame != -1) {
    pt->frames[free_frame].page = page_id;
    pt->frames[free_frame].accessed_at = clock;
    pt->frames[free_frame].created_at = clock;
    pt->frames[free_frame].referenced = false;
    pt->frames[free_frame].free = false;
    return;
  }

  int target_frame = -1;

  if (pt->algorithm == SECOND_CHANCE) {
    target_frame = ipt_2a_algorithm(pt);
  } else if (pt->algorithm == LRU) {
    target_frame = ipt_lru_swap(pt);
  } else if (pt->algorithm == FIFO) {
    target_frame = ipt_fifo_swap(pt);
  } else if (pt->algorithm == RANDOM) {
    target_frame = ipt_random_swap(pt);
  } else {
    printf("Falha ao detectar algoritmo\n");
    exit(1);
  }

  if (target_frame == -1) {
    printf("Falha ao realizar o algortimo de swap.\n");
    exit(1);
  }

  pt->frames[target_frame].page = page_id;
  pt->frames[target_frame].accessed_at = clock;
  pt->frames[target_frame].created_at = clock;
  pt->frames[free_frame].referenced = false;
  pt->frames[target_frame].free = false;
}
