#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <debug.h> // definition of debug_printf

// Each block stores its size, whether it is free, and the next block.
typedef struct block {
  size_t size;
  struct block *next;
  int free;
} block_t;

// Keep track of the beginning and end of our linked list.
static block_t *head = NULL;
static block_t *tail = NULL;

// Find the first free block that is large enough.
static block_t *find_free_block(size_t s) {
  block_t *current = head;

  while (current != NULL) {
    if (current->free && current->size >= s) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

// Request a new block from the operating system.
static block_t *request_block(size_t s) {
  if (s > SIZE_MAX - sizeof(block_t)) {
    return NULL;
  }

  block_t *block = sbrk(sizeof(block_t) + s);

  if (block == (void *) -1) {
    return NULL;
  }

  block->size = s;
  block->free = 0;
  block->next = NULL;

  if (head == NULL) {
    head = block;
  } else {
    tail->next = block;
  }

  tail = block;

  return block;
}

void *mymalloc(size_t s) {
  if (s == 0) {
    return NULL;
  }

  block_t *block = find_free_block(s);

  if (block == NULL) {
    block = request_block(s);
  }

  void *p = NULL;

  if (block != NULL) {
    block->free = 0;
    p = (void *)(block + 1);
  }

  if (!p) {
    // We are out of memory.
    return NULL;
  }

  debug_printf("Malloc %zu bytes\n", s);

  return p;
}

void *mycalloc(size_t nmemb, size_t s) {
  if (s != 0 && nmemb > SIZE_MAX / s) {
    return NULL;
  }

  size_t total = nmemb * s;

  void *p = mymalloc(total);

  if (!p) {
    // We are out of memory.
    return NULL;
  }

  memset(p, 0, total);

  debug_printf("Calloc %zu bytes\n", nmemb * s);

  return p;
}

void myfree(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  block_t *block = (block_t *)ptr - 1;

  size_t size = block->size;

  // Mark the block as free so it can be reused.
  block->free = 1;

  debug_printf("Freed %zu bytes\n", size);
}
