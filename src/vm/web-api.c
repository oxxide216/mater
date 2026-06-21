#include "web-api.h"

#define HEAP_CAP 1024 * 1024 * 16

typedef Da(char) TemplateDa;

char heap[HEAP_CAP];
u32 heap_ptr = 0;

u32 str_len(char *str) {
  u32 len = 0;
  while (*str++)
    ++len;
  return len;
}

void *walloc(u32 size) {
  if (heap_ptr + size > HEAP_CAP)
    return NULL;

  void *ptr = heap + heap_ptr;
  heap_ptr += size;
  return ptr;
}

void wfree(void *ptr) {
  (void) ptr;
  // Memory leaks!!!
}

void da_reserve_space_impl(void *da, u32 element_size) {
  TemplateDa *tda = da;

  if (tda->cap <= tda->len) {
    if (tda->cap != 0) {
      while (tda->cap <= tda->len)
        tda->cap *= 2;
      char *new_items = walloc(element_size * tda->cap);
      for (u32 i = 0; i < element_size * tda->len; ++i)
        new_items[i] = tda->items[i];
      wfree(tda->items);
      tda->items = new_items;
    } else {
      tda->cap = 1;
      tda->items = walloc(element_size);
    }
  }
}
