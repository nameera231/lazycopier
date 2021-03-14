#define _GNU_SOURCE
#include "lazycopy.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * This function inserts an address into the address_book (uniquelist) of pointers to the start of chunks.
 */
void list_insert(list_t* s, void* copy1) {
  // Create a new pointer to scan the linked list.
  element_t* pointer = s->head;
  // If list is already empty
  if(pointer == NULL)
  {
     element_t* new_item = (element_t*) malloc(sizeof(element_t));
     s->head = new_item;
     new_item->copy = copy1;
     new_item->next = NULL;
  }
  else{
    while(pointer!=NULL){
      // We only add a new element if it is unique
      if(pointer->copy == copy1)
      {
        return;
      }
      if(pointer->next==NULL)
      { 
        element_t* new_item = (element_t*) malloc(sizeof(element_t));
        pointer->next = new_item;
        new_item->copy = copy1;
        new_item->next = NULL;
        break;
      }
      pointer = pointer->next;
    }
}}

// Uniquelist that keeps track of all the lazy copies.
list_t address_book;

/**
 * This function will catch segmentation faults, find the 
 * pointer that caused it, and give the chunk that pointer
 * points to writing permissions.
 */
void segv_handler(int signal, siginfo_t* info, void* ctx){
  // The address that caused the segmentation fault is converted to an intptr_t
  // for comparison with other ints.
  intptr_t addr = (intptr_t)info->si_addr;
  // A new pointer is created to scan through the address_book.
  element_t* cursor = (&address_book)->head;
  // An array is created to store a copy of the written chunk.
  uint8_t temp_arr[CHUNKSIZE];
  
  // The address_book is scanned through to find the chunk where the segfault occured.
  while(cursor!= NULL){
    // If the address that caused the segfault is within the range of a chunk in address_book,
    // the written chunk is copied, the pointer pointing to it is given writing permissions,
    // and the chunk is restored. If not, the next address in address_book is checked.
    if ((intptr_t)cursor->copy <= addr && addr < ((intptr_t)cursor->copy) + CHUNKSIZE){
      memcpy(&temp_arr, cursor->copy, CHUNKSIZE);
      void* new_chunk_address = mmap(cursor->copy, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED, -1, 0);
      if(new_chunk_address == MAP_FAILED){
        perror("mmap failure");
      }
      memcpy(new_chunk_address, &temp_arr, CHUNKSIZE);
      break;
    }
    cursor = cursor->next;
  }
return;
  
}

/**
 * This function will be called at startup so you can set up a signal handler.
 */
void chunk_startup() {
  // The signal handler is declared and initialized.
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_SIGINFO;

  // Set the signal handler, checking for errors
  if(sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }
}

/**
 * This function should return a new chunk of memory for use.
 *
 * \returns a pointer to the beginning of a 64KB chunk of memory that can be read, written, and
 * copied
 */
void* chunk_alloc() {
  // Call mmap to request a new chunk of memory. See comments below for description of arguments.
  void* result = mmap(NULL, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  // Arguments:
  //   NULL: this is the address we'd like to map at. By passing null, we're asking the OS to
  //   decide. CHUNKSIZE: This is the size of the new mapping in bytes. PROT_READ | PROT_WRITE: This
  //   makes the new reading readable and writable MAP_ANONYMOUS | MAP_SHARED: This mapes a new
  //   mapping to cleared memory instead of a file,
  //                               which is another use for mmap. MAP_SHARED makes it possible for
  //                               us to create shared mappings to the same memory.
  //   -1: We're not connecting this memory to a file, so we pass -1 here.
  //   0: This doesn't matter. It would be the offset into a file, but we aren't using one.

  // Check for an error
  if (result == MAP_FAILED) {
    perror("mmap failed in chunk_alloc");
    exit(2);
  }

  // Everything is okay. Return the pointer.
  return result;
}

/**
 * Create a copy of a chunk by copying values eagerly.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_eager(void* chunk) {
  // First, we'll allocate a new chunk to copy to
  void* new_chunk = chunk_alloc();

  // Now copy the data
  memcpy(new_chunk, chunk, CHUNKSIZE);

  // Return the new chunk
  return new_chunk;
}

/**
 * Create a copy of a chunk by copying values lazily.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_lazy(void* chunk) {
  // Just to make sure your code works, this implementation currently calls the eager copy version

  // Your implementation should do the following:
  // 1. Use mremap to create a duplicate mapping of the chunk passed in
  void* newest = mremap(chunk, 0, CHUNKSIZE, MREMAP_MAYMOVE);
  if(newest == MAP_FAILED){
    perror("mremap failure");
  }
  // 2. Mark both mappings as read-only
  //ADD ERROR CHECKS
  
  if(mprotect(chunk, CHUNKSIZE, PROT_READ) == -1){
    perror("mprotect failure");
  }
  if(mprotect(newest, CHUNKSIZE, PROT_READ) == -1){
    perror("mprotect failure");
  }
  
  // 3. Keep some record of both lazy copies so you can make them writable later.
  //    At a minimum, you'll need to know where the chunk begins and ends.
  list_insert(&address_book, chunk);
  list_insert(&address_book, newest);
  
  return newest;
  
  // Later, if either copy is written to you will need to:
  // 1. Save the contents of the chunk elsewhere (a local array works well)
  // 2. Use mmap to make a writable mapping at the location of the chunk that was written
  // 3. Restore the contents of the chunk to the new writable mapping
}
