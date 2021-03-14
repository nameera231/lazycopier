#ifndef LAZYCOPY_H
#define LAZYCOPY_H

// Uniquelist used to store adresses pointing to lazily-copied chunks.
typedef struct list {
  struct element* head;
} list_t;

// Element in the uniquelist.
typedef struct element {
  void* copy;
  struct element* next;
} element_t;


// This defines the size of a chunk of data we can request or copy. Must be a multiple of page size.
#define CHUNKSIZE 0x10000

// This function will be called at startup so you can set up a signal handler
void chunk_startup();

// This function should return a new chunk of memory for use
void* chunk_alloc();

// This function should return a copy of a chunk created with eager (normal) copying
void* chunk_copy_eager(void* chunk);

// This function should return a copy of a chunk created with lazy copying
void* chunk_copy_lazy(void* chunk);

//void insert();

#endif
