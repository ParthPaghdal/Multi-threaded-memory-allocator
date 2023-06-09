#ifndef __MYALLOC_H__
#define __MYALLOC_H__

enum allocation_algorithm { FIRST_FIT, BEST_FIT, WORST_FIT };

struct Stats {
  int allocated_size;
  int allocated_chunks;
  int free_size;
  int free_chunks;
  int smallest_free_chunk_size;
  int largest_free_chunk_size;
};

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm);

void *allocate(int _size);
void deallocate(void *_ptr);
int available_memory();
void get_statistics(struct Stats *_stat);
int compact_allocation(void **_before, void **_after);
void destroy_allocator();

struct nodeStruct {
  void* blockptr;
  struct nodeStruct *next;
};

struct nodeStruct* List_createNode(void* blockptr);
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);
int List_countNodes (struct nodeStruct *head);
struct nodeStruct* List_findNode(struct nodeStruct *head, void* blockptr);
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);
void List_sort (struct nodeStruct **headRef);


#endif